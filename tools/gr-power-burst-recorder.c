#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>

struct iq_data {
	float i;
	float q;
};

enum pwr_level {
	PWR_LEVEL_LOW,
	PWR_LEVEL_HIGH,
};

enum operation_mode {
	OP_MODE_BURST_DETECTION,
	OP_MODE_BURST_RECORDING,
};

#define READ_CHUNK_SIZE (10000u)
#define HISTORY_SIZE    (200000u)

#define FILE_NAME_LEN   (100)

static FILE *in_file, *out_file;
char input_file_path[FILE_NAME_LEN], output_file_path[FILE_NAME_LEN];

/* Current state related */
static struct iq_data iq_data_buf[READ_CHUNK_SIZE + HISTORY_SIZE];
static size_t sample_count, cur_burst_start, cur_burst_end, cur_burst_len;
static enum pwr_level cur_pwr_level;
static size_t cur_burst_count;
static float square_sum;

/* Settings */
static enum operation_mode opmode;
static bool record_burst;
/* The burst number to record */
static size_t record_burst_nbr;
/* Desired outfile length of the recording. The copied burst will be padded
 * with zeroes to match the selected length
 */
static size_t outfile_len;
/* The number of samples before and after a detected burst that should
 * be saved.
 */
static size_t record_pre_burst_len, record_post_burst_len;

/* Detection settings */
static float detect_thresh_high, detect_thresh_low;
static size_t detect_avg_len;

static int open_input_file(void)
{
	int ret = 0;

	if (strnlen(input_file_path, FILE_NAME_LEN) == 0) {
		fprintf(stderr, "Missing input file (-i). Reading from stdin\n");
		in_file = stdin;
	} else {
		in_file = fopen(input_file_path, "rb");
		if (!in_file) {
			fprintf(stderr, "Unable to open file: %s\n",
				input_file_path);
			ret = -ENOENT;
		}
	}

	return ret;
}

static int open_output_file(void)
{
	int ret = 0;

	if (strnlen(output_file_path, FILE_NAME_LEN) == 0) {
		fprintf(stderr, "Missing output file for recording (-o). Writing to stdout\n");
		out_file = stdout;
	} else {
		out_file = fopen(output_file_path, "wb");
		if (!out_file) {
			fprintf(stderr, "Unable to open file: %s\n", output_file_path);
			ret = -ENOENT;
		}
	}

	return ret;
}

static void age_iq_buf(void)
{
	memmove(iq_data_buf,
		iq_data_buf + READ_CHUNK_SIZE,
		sizeof(iq_data_buf) - READ_CHUNK_SIZE * sizeof(struct iq_data));
}

static size_t read_chunk(void)
{
	size_t cur_chunk_size = 0;
	struct iq_data *iq;

	/* Make room for the new samples before reading from file */
	age_iq_buf();

	iq = iq_data_buf + HISTORY_SIZE;

	while (cur_chunk_size < READ_CHUNK_SIZE) {
		size_t read_count;

		read_count = fread(iq + cur_chunk_size,
				   sizeof(struct iq_data),
				   READ_CHUNK_SIZE - cur_chunk_size,
				   in_file);
		if (!read_count)
			break; /* EOF or error? */
		cur_chunk_size += read_count;
	}

	return cur_chunk_size;
}

static void write_output_burst(struct iq_data *burst, size_t burst_len)
{
	size_t nwritten = 0, i;
	uint8_t zero = 0;

	if (outfile_len && (burst_len > outfile_len))
		burst_len = outfile_len;

	while (nwritten < burst_len) {
		int write_count;

		write_count = fwrite(burst + nwritten,
				     sizeof(struct iq_data),
				     burst_len - nwritten,
				     out_file);
		if (!write_count) {
			fprintf(stderr, "Unable to write output. errno = %d\n",
				errno);
			return;
		}
		nwritten += write_count;
	}

	if (outfile_len) {
		/* zero-pad the output file */
		for (i = 0; i < (outfile_len - burst_len) * sizeof(struct iq_data); i++)
			(void)fwrite(&zero, 1, 1, out_file);
	}

	fflush(out_file);
}

static float calc_avg_power(struct iq_data *iq)
{
	float new_square, old_square;
	struct iq_data *iq_old;

	iq_old = iq - detect_avg_len;
	new_square = iq->i * iq->i + iq->q * iq->q;
	old_square = iq_old->i * iq_old->i + iq_old->q * iq_old->q;
	square_sum -= old_square;
	square_sum += new_square;

	return square_sum;
}

static int process_chunk(size_t cur_chunk_size)
{
	int ret = 0;
	size_t i;
	float pwr;
	struct iq_data *iq;

	iq = iq_data_buf + HISTORY_SIZE;

	for (i = 0; i < cur_chunk_size; i++) {
		pwr = calc_avg_power(iq + i);
		if ((pwr > detect_thresh_high) &&
		    (cur_pwr_level == PWR_LEVEL_LOW)) {
			if (opmode == OP_MODE_BURST_DETECTION)
				fprintf(stdout,
					"Burst %3zd start detected at sample %zd\n",
					cur_burst_count, sample_count + i);
			cur_pwr_level = PWR_LEVEL_HIGH;
			cur_burst_start = sample_count + i;
		} else if ((pwr < detect_thresh_low) &&
			   (cur_pwr_level == PWR_LEVEL_HIGH)) {
			cur_pwr_level = PWR_LEVEL_LOW;
			cur_burst_end = sample_count + i;
			cur_burst_len = cur_burst_end - cur_burst_start;
			if (opmode == OP_MODE_BURST_DETECTION)
				fprintf(stdout,
					"Burst %3zd   end detected at sample %zd. Burst len = %zd samples\n",
					cur_burst_count,
					sample_count + i,
					cur_burst_len);
			else if (opmode == OP_MODE_BURST_RECORDING &&
				 (cur_burst_count == record_burst_nbr))
				record_burst = true;
			cur_burst_count++;
		}
		if (opmode == OP_MODE_BURST_RECORDING && record_burst &&
		    sample_count + i == cur_burst_end + record_post_burst_len) {
			size_t rec_len = cur_burst_len + record_pre_burst_len +
					 record_post_burst_len;
			struct iq_data *iq_req = iq + i - rec_len;

			if (iq_req < iq_data_buf) {
				fprintf(stderr,
					"Burst too long for recording (%zd bytes). Not enough history (%u bytes)!\n",
					rec_len, HISTORY_SIZE);
				ret = -ENOMEM;
				goto out;
			}
			write_output_burst(iq_req, rec_len);
			ret = 1; /* Always abort after recording */
		}
	}

	sample_count += cur_chunk_size;
	fprintf(stderr, "%zd\r", sample_count);

out:
	return ret;
}

static int do_work(void)
{
	int ret;
	size_t cur_chunk_size;

	ret = open_input_file();
	if (ret)
		goto out;

	if (opmode == OP_MODE_BURST_RECORDING) {
		ret = open_output_file();
		if (ret)
			goto out;
	}

	for (;;) {
		cur_chunk_size = read_chunk();
		if (cur_chunk_size == 0)
			break;
		ret = process_chunk(cur_chunk_size);
		if (ret == 1) {
			ret = 0; /* Successful recording */
			break;
		}
		if (ret)
			break;
		if (cur_chunk_size != READ_CHUNK_SIZE)
			break; /* probably last chunk in input */
	}

out:
	return ret;
}

static void print_usage(const char *argv0)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "%s OPTIONS\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "gr-power-burst-recorder is a tool used to extract bursts from an\n");
	fprintf(stderr, "input and write those bursts to an output. It can also be used to\n");
	fprintf(stderr, "detect bursts from the input and report info (burst count and lengths)\n");
	fprintf(stderr, "to stdout\n");
	fprintf(stderr, "A burst is a series of samples with a power significantly higher than\n");
	fprintf(stderr, "the other samples in a data stream\n");
	fprintf(stderr, "The input is a sequence of float's where each tuple is a pair of iq\n");
	fprintf(stderr, "samples.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "The program has 2 modes of operation: recording and detection.\n");
	fprintf(stderr, "Detection is the default mode (when launched without the\n");
	fprintf(stderr, "--record option)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "During detection, samples from input will be power estimated with a\n");
	fprintf(stderr, "moving RMS value calculation. The start and end (sample offsets) and\n");
	fprintf(stderr, "length of each burst will be printed to stdout.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "During recording, the same type of detection will be performed, but no\n");
	fprintf(stderr, "reporting will be made. Instead, when the burst specified with the\n");
	fprintf(stderr, "--record-burst-nbr option is found, it will be recorded and the program\n");
	fprintf(stderr, "will terminate.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "The recommended usage of the program is to first run without --record\n");
	fprintf(stderr, "in order to get an overview of the contents of the file. Then, run again\n");
	fprintf(stderr, "with the --record option in order to record the burst of interest.\n");
	fprintf(stderr, "It is important that the same settings are used for both the detection and\n");
	fprintf(stderr, "recording. Otherwise, different bursts might be detected (and recorded) in\n");
	fprintf(stderr, "the recording phase.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "In the case of recording, the program will terminate when the burst\n");
	fprintf(stderr, "specified by the --record-burst-nbr has been recorded.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "The arguments --detection-threshold-high and --detection-threshold-low is\n");
	fprintf(stderr, "used to set the RMS thresholds for burst detection. Normally these values\n");
	fprintf(stderr, "should be in the range from 0.0 to 1.0 since this is the range normally\n");
	fprintf(stderr, "produced by GNU radio sources.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -i, --input\n");
	fprintf(stderr, "                   gnuradio iq sample file (written by a file sink).\n");
	fprintf(stderr, "                   If omitted, the samples will be read from stdin.\n");
	fprintf(stderr, "  -o, --output\n");
	fprintf(stderr, "                   extracted/recorded and (optionally) zero-padded iq samples\n");
	fprintf(stderr, "                   from input.\n");
	fprintf(stderr, "                   If omitted, the output will be written to stdout.\n");
	fprintf(stderr, "  -r, --record\n");
	fprintf(stderr, "                   Record burst from input and write to output.\n");
	fprintf(stderr, "                   If this option is omitted, burst will only be detected\n");
	fprintf(stderr, "                   and reported (see description above)\n");
	fprintf(stderr, "  -n, --record-burst-nbr\n");
	fprintf(stderr, "                   The burst number (starting from zero) in input to record.\n");
	fprintf(stderr, "                   The program will terminate as soon as the burst has been\n");
	fprintf(stderr, "                   written to output\n");
	fprintf(stderr, "  -b, --record-pre-burst-len\n");
	fprintf(stderr, "                   The number of samples before the detected burst start\n");
	fprintf(stderr, "                   that should be added to the recording.\n");
	fprintf(stderr, "  -a --record-post-burst-len\n");
	fprintf(stderr, "                   The number of samples after the detected burst end\n");
	fprintf(stderr, "                   that should be added to the recording.\n");
	fprintf(stderr, "  -l, --outfile-length\n");
	fprintf(stderr, "                   The length in samples of the output file. If the length of\n");
	fprintf(stderr, "                   the extracted burst is less than this value, the output\n");
	fprintf(stderr, "                   will be zero-padded.\n");
	fprintf(stderr, "                   If the length of the extracted burst is greater than this\n");
	fprintf(stderr, "                   value, the output will be truncated.\n");
	fprintf(stderr, "  -u, --detection-threshold-high\n");
	fprintf(stderr, "                   High power burst detection threshold. If the average\n");
	fprintf(stderr, "                   RMS value of the input exceeds this value, a start-of-\n");
	fprintf(stderr, "                   burst is detected\n");
	fprintf(stderr, "  -w, --detection-threshold-low\n");
	fprintf(stderr, "                   Low power burst detection threshold. If the average\n");
	fprintf(stderr, "                   RMS value of the input drops below this value, an end-of-\n");
	fprintf(stderr, "                   burst is detected.\n");
	fprintf(stderr, "  -N, --detection-averaging-len\n");
	fprintf(stderr, "                   The number of samples that will be used when calculating\n");
	fprintf(stderr, "                   the Moving Average RMS value of the input.\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	bool error = false;
	int opt, optind = 0;
	struct option long_opts[] = {
		{"input", 1, 0, 'i'},
		{"output", 1, 0, 'o'},
		{"record", 0, 0, 'r'},
		{"record-burst-nbr", 1, 0, 'n'},
		{"record-pre-burst-len", 1, 0, 'b'},
		{"record-post-burst-len", 1, 0, 'a'},
		{"outfile-length", 1, 0, 'l'},
		{"detection-threshold-high", 1, 0, 'u'},
		{"detection-threshold-low", 1, 0, 'w'},
		{"detection-averaging-len", 1, 0, 'N'},
		{"help", 0, 0, 'h'},
		{NULL, 0, 0, 0},
	};

	while ((opt = getopt_long(argc, argv, "i:o:rn:b:a:l:u:w:N:h", long_opts,
		&optind)) != -1) {
		switch (opt) {
		case 'i':
			strncpy(input_file_path, optarg, FILE_NAME_LEN);
			break;
		case 'o':
			strncpy(output_file_path, optarg, FILE_NAME_LEN);
			break;
		case 'r':
			opmode = OP_MODE_BURST_RECORDING;
			break;
		case 'n':
			record_burst_nbr = (size_t) strtol(optarg, NULL, 10);
			break;
		case 'b':
			record_pre_burst_len = (size_t) strtol(optarg, NULL, 10);
			break;
		case 'a':
			record_post_burst_len = (size_t) strtol(optarg, NULL, 10);
			break;
		case 'l':
			outfile_len = (size_t) strtol(optarg, NULL, 10);
			break;
		case 'u':
			detect_thresh_high = strtof(optarg, NULL);
			detect_thresh_high *= detect_thresh_high;
			break;
		case 'w':
			detect_thresh_low = strtof(optarg, NULL);
			detect_thresh_low *= detect_thresh_low;
			break;
		case 'N':
			detect_avg_len = (size_t) strtol(optarg, NULL, 10);
			break;
		case 'h':
			print_usage(argv[0]);
			exit(0);
			break;
		};
	}

	if (!detect_thresh_high) {
		fprintf(stderr,
			"Missing high burst detection threshold (-u)!\n");
		error = true;
	}
	if (!detect_thresh_low) {
		fprintf(stderr,
			"Missing low burst detection threshold (-w)!\n");
		error = true;
	}
	if (!detect_avg_len) {
		fprintf(stderr,
			"Missing averaging length for burst power calculation (-N)!\n");
		error = true;
	}

	if (error)
		exit(-1);

	return do_work();
}
