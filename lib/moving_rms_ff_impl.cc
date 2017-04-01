/* -*- c++ -*- */
/* 
 * Copyright 2017 Erik Stromdahl <erik.stromdahl@gmail.com>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "moving_rms_ff_impl.h"
#include <cstdio>
#include <cmath>

namespace gr {
  namespace power {

    moving_rms_ff::sptr
    moving_rms_ff::make(size_t history_len)
    {
      return gnuradio::get_initial_sptr
        (new moving_rms_ff_impl(history_len));
    }

    void
    moving_rms_ff_impl::dbg_log(const char* fmt...)
    {
        va_list args;
        char print_buf[100];

        va_start(args, fmt);

        (void) std::vsnprintf(print_buf, sizeof(print_buf), fmt, args);
        GR_LOG_DEBUG(LOG, print_buf);

        va_end(args);
    }

    /*
     * The private constructor
     */
    moving_rms_ff_impl::moving_rms_ff_impl(size_t history_len)
      : gr::sync_block("moving_rms_ff",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(float))),
        history_len(history_len), square_sum(0)
    {
        if (!history_len)
            history_len = 1;

        set_history(history_len);
    }

    /*
     * Our virtual destructor.
     */
    moving_rms_ff_impl::~moving_rms_ff_impl()
    {
    }

    void
    moving_rms_ff_impl::calc_average_pwr(int n_items, const float *in, float *out)
    {
        size_t i;
        float new_square, old_square;

        for (i = this->history_len - 1; i < n_items + this->history_len - 1; i++) {
            new_square = in[i] * in[i];
            old_square = in[i - (this->history_len - 1)] *
                         in[i - (this->history_len - 1)];

            if (this->history_len == 1) {
                // Special case without history
                out[i] = std::sqrt(new_square / this->history_len);
            } else {
                /* Calculate the sum of the squared input samples:
                 * square_sum = 1 / history_len *
                 *    (history[n-(history_len-1)] + history[n-(history_len-2)] + ... + history[n])
                 */
                this->square_sum -= old_square;
                this->square_sum += new_square;
                out[i] = std::sqrt(this->square_sum / this->history_len);
            }
        }
    }

    int
    moving_rms_ff_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        float *out = (float *) output_items[0];

        this->calc_average_pwr(noutput_items, in, out);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace power */
} /* namespace gr */

