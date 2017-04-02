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
#include "power_stats_impl.h"

namespace gr {
  namespace power {

    power_stats::sptr
    power_stats::make(float low_pwr_threshold, float high_pwr_threshold)
    {
      return gnuradio::get_initial_sptr
        (new power_stats_impl(low_pwr_threshold, high_pwr_threshold));
    }

    /*
     * The private constructor
     */
    power_stats_impl::power_stats_impl(float low_pwr_threshold,
                                       float high_pwr_threshold)
      : gr::sync_block("power_stats",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(0, 0, 0)),
        low_pwr_threshold(low_pwr_threshold),
        high_pwr_threshold(high_pwr_threshold),
        cur_pwr_level(PWR_LEVEL_LOW),
        nbr_of_low_to_high_transitions(0),
        nbr_of_low_to_high_transitions_last_read(0),
        nbr_of_low_pwr_samples(0),
        nbr_of_high_pwr_samples(0),
        nbr_of_low_pwr_samples_last_read(0),
        nbr_of_high_pwr_samples_last_read(0)
    {}

    /*
     * Our virtual destructor.
     */
    power_stats_impl::~power_stats_impl()
    {
    }

    void
    power_stats_impl::calc_stats(int n_items, const float *in)
    {
        int i;

        for (i = 0; i < n_items; i++) {
            if (this->cur_pwr_level == PWR_LEVEL_LOW) {
                this->nbr_of_low_pwr_samples++;
                if (in[i] > this->high_pwr_threshold) {
                    /* There has been a low to high power transition */
                    this->nbr_of_low_to_high_transitions++;
                    this->cur_pwr_level = PWR_LEVEL_HIGH;
                }
            } else if (this->cur_pwr_level == PWR_LEVEL_HIGH) {
                this->nbr_of_high_pwr_samples++;
                if (in[i] < this->low_pwr_threshold) {
                    this->cur_pwr_level = PWR_LEVEL_LOW;
                }
            }
        }
    }

    int
    power_stats_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];

        (void) output_items;
        this->calc_stats(noutput_items, in);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace power */
} /* namespace gr */

