
gr-power
========

GNU radio power library.

Currently only contains two GNU radio blocks: Moving RMS and power stats.

Also contains one tool: `gr-power-burst-recorder`_

GNU radio blocks
----------------

Moving RMS
++++++++++

Moving RMS is an alternative to the standard RMS block that comes with GNU Radio.

The standard RMS block calculates a first-order recursive average of the input
whereas the moving RMS block use a Moving Average (MA) calculation of the RMS
value.

The recursive average can be viewed as an IIR filter approximation of the true
RMS value.

A Moving Average with a rectangular window is a special case of a regular FIR filter
where all filter coefficients are set to 1/N. N is the length of the filter (the
number of values to be used in the average calculation).

Since a rectangular window is used, the implementation can be made quite efficient.

Power stats
+++++++++++

Power stats provides two statistics probe functions:

- Duty cycle
- Burst count

Duty cycle is the percentage of time when the input signal is above a
configurable threshold.

The burst count is the number of detected power bursts over time.

All statistics probe functions have a ``delta_`` version that is used
to read the difference since the last probe call.

See section `GNURadioCompanion`_. for more details on usage,

Build
+++++

It is recommended to build GNU Radio modules in a build directory
outside the source tree:

.. code-block:: bash

	mkdir build
	cd build
	cmake ..
	make
	sudo make install
	sudo ldconfig

GNURadioCompanion
-----------------

Moving RMS
++++++++++

The Moving RMS block can be found under the Power section in GRC.

There is currently only one configuration parameter for the block:
**Averaging history length**

This is the number of values that will be used in the calculation of the average,
i.e, the filter length *N*.

Power stats
+++++++++++

The Power stats block provide a set of statistics probe functions
(as described above).

The probe functions should be called periodically from a ``Function Probe``
block and the obtained vales can be added to an appropriate GUI widget
(like ``QT GUI Entry``) for display.

The available probe functions are:

- ``burst_count``
- ``delta_burst_count``
- ``avg_duty_cycle``
- ``delta_avg_duty_cycle``

Each function should be called without arguments.

In a ``Function Probe`` block, set the ``Block ID`` and ``Function Name``
according to below:

::

	Block ID = <ID value of the Power stats block>
	Function Name = <probe function to poll>

In a ``QT GUI Entry`` block, set the ``Default Value`` according to below:

::

	Default Value = <ID value of the Function Probe block>

Tools
-----

The ``tools`` subdirectory contains stand-alone tools (no dependecies to any
gnuradio libraries).

Currently only one tool is included: gr-power-burst-recorder

gr-power-burst-recorder
+++++++++++++++++++++++

Usage:
gr-power-burst-recorder OPTIONS

gr-power-burst-recorder is a tool used to extract bursts from an
input and write those bursts to an output. It can also be used to
detect bursts from the input and report info (burst count and lengths)
to stdout
A burst is a series of samples with a power significantly higher than
the other samples in a data stream
The input is a sequence of float's where each tuple is a pair of iq
samples.

The program has 2 modes of operation: recording and detection.
Detection is the default mode (when launched without the
``--record`` option)

During detection, samples from input will be power estimated with a
moving RMS value calculation. The start and end (sample offsets) and
length of each burst will be printed to stdout.

During recording, the same type of detection will be performed, but no
reporting will be made. Instead, when the burst specified with the
``--record-burst-nbr`` option is found, it will be recorded and the program
will terminate.

The recommended usage of the program is to first run without ``--record``
in order to get an overview of the contents of the file. Then, run again
with the ``--record`` option in order to record the burst of interest.
It is important that the same settings are used for both the detection and
recording. Otherwise, different bursts might be detected (and recorded) in
the recording phase.

In the case of recording, the program will terminate when the burst
specified by the ``--record-burst-nbr`` has been recorded.

The arguments ``--detection-threshold-high`` and ``--detection-threshold-low`` is
used to set the RMS thresholds for burst detection. Normally these values
should be in the range from 0.0 to 1.0 since this is the range normally
produced by GNU radio sources.

Options:
  ``-i``, ``--input``
                   gnuradio iq sample file (written by a file sink).
                   If omitted, the samples will be read from stdin.
  ``-o``, ``--output``
                   extracted/recorded and (optionally) zero-padded iq samples
                   from input.
                   If omitted, the output will be written to stdout.
  ``-r``, ``--record``
                   Record burst from input and write to output.
                   If this option is omitted, burst will only be detected
                   and reported (see description above)
  ``-n``, ``--record-burst-nbr``
                   The burst number (starting from zero) in input to record.
                   The program will terminate as soon as the burst has been
                   written to output
  ``-b``, ``--record-pre-burst-len``
                   The number of samples before the detected burst start
                   that should be added to the recording.
  ``-a`` ``--record-post-burst-len``
                   The number of samples after the detected burst end
                   that should be added to the recording.
  ``-l``, ``--outfile-length``
                   The length in samples of the output file. If the length of
                   the extracted burst is less than this value, the output
                   will be zero-padded.
                   If the length of the extracted burst is greater than this
                   value, the output will be truncated.
  ``-u``, ``--detection-threshold-high``
                   High power burst detection threshold. If the average
                   RMS value of the input exceeds this value, a start-of-
                   burst is detected
  ``-w``, ``--detection-threshold-low``
                   Low power burst detection threshold. If the average
                   RMS value of the input drops below this value, an end-of-
                   burst is detected.
  ``-N``, ``--detection-averaging-len``
                   The number of samples that will be used when calculating
                   the Moving Average RMS value of the input.
