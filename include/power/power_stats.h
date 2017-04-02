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


#ifndef INCLUDED_POWER_POWER_STATS_H
#define INCLUDED_POWER_POWER_STATS_H

#include <power/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace power {

    /*!
     * \brief power statistics probe
     * \ingroup power
     *
     */
    class POWER_API power_stats : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<power_stats> sptr;

      /*!
       * \brief Make a power stats block.
       *
       * \param low_pwr_threshold  threshold for low to high power transition.
       * \param high_pwr_threshold threshold for high to low power transition.
       *
       * The threshold parameters are used by the statistics calculation
       * functions to detect power bursts. These bursts are counted during
       * operation and are provided to the user via the probe functions.
       *
       * The threshold values are usually in the range [0 ... 1.0] since
       * this is the value range from most source blocks.
       */
      static sptr make(float low_pwr_threshold, float high_pwr_threshold);

      /* probe functions */
      virtual int burst_count() const = 0;
      virtual int delta_burst_count() = 0;
      virtual float avg_duty_cycle() const = 0;
      virtual float delta_avg_duty_cycle() = 0;
    };

  } // namespace power
} // namespace gr

#endif /* INCLUDED_POWER_POWER_STATS_H */

