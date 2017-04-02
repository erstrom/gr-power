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

#ifndef INCLUDED_POWER_POWER_STATS_IMPL_H
#define INCLUDED_POWER_POWER_STATS_IMPL_H

#include <power/power_stats.h>

namespace gr {
  namespace power {

    enum pwr_level {
        PWR_LEVEL_LOW,
        PWR_LEVEL_HIGH,
    };

    class power_stats_impl : public power_stats
    {
     private:

      float low_pwr_threshold;
      float high_pwr_threshold;
      enum pwr_level cur_pwr_level;
      int nbr_of_low_to_high_transitions;
      int nbr_of_low_to_high_transitions_last_read;
      long long nbr_of_low_pwr_samples;
      long long nbr_of_high_pwr_samples;
      long long nbr_of_low_pwr_samples_last_read;
      long long nbr_of_high_pwr_samples_last_read;

      void calc_stats(int n_items, const float *in);

     public:
      power_stats_impl(float low_pwr_threshold, float high_pwr_threshold);
      ~power_stats_impl();

      /* Probe functions */
      int burst_count() const { return this->nbr_of_low_to_high_transitions; }
      int delta_burst_count()
      {
          int delta = this->nbr_of_low_to_high_transitions -
              this->nbr_of_low_to_high_transitions_last_read;

          this->nbr_of_low_to_high_transitions_last_read =
              this->nbr_of_low_to_high_transitions;

          return delta;
      }

      float avg_duty_cycle() const
      {
          long long total_nbr_of_samples =
              this->nbr_of_low_pwr_samples + this->nbr_of_high_pwr_samples;

          return 100 * (float)this->nbr_of_high_pwr_samples /
              (float)total_nbr_of_samples;
      }

      float delta_avg_duty_cycle()
      {
          long long delta_nbr_of_low_pwr_samples, delta_nbr_of_high_pwr_samples,
                    delta_total_nbr_of_samples;

          delta_nbr_of_low_pwr_samples = this->nbr_of_low_pwr_samples -
              this->nbr_of_low_pwr_samples_last_read;
          delta_nbr_of_high_pwr_samples = this->nbr_of_high_pwr_samples -
              this->nbr_of_high_pwr_samples_last_read;
          delta_total_nbr_of_samples = delta_nbr_of_low_pwr_samples +
              delta_nbr_of_high_pwr_samples;
          this->nbr_of_low_pwr_samples_last_read = this->nbr_of_low_pwr_samples;
          this->nbr_of_high_pwr_samples_last_read = this->nbr_of_high_pwr_samples;

          return 100 * (float)delta_nbr_of_high_pwr_samples /
              (float)delta_total_nbr_of_samples;
      }

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace power
} // namespace gr

#endif /* INCLUDED_POWER_POWER_STATS_IMPL_H */

