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

#ifndef INCLUDED_POWER_MOVING_RMS_FF_IMPL_H
#define INCLUDED_POWER_MOVING_RMS_FF_IMPL_H

#include <power/moving_rms_ff.h>
#include <cstdarg>

namespace gr {
  namespace power {

    class moving_rms_ff_impl : public moving_rms_ff
    {
     private:

      size_t history_len;
      float square_sum;

      void calc_average_pwr(int n_items, const float *in, float *out);
      static void dbg_log(const char* fmt...);

     public:
      moving_rms_ff_impl(size_t history_len);
      ~moving_rms_ff_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace power
} // namespace gr

#endif /* INCLUDED_POWER_MOVING_RMS_FF_IMPL_H */

