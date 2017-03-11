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


#ifndef INCLUDED_POWER_MOVING_RMS_CF_H
#define INCLUDED_POWER_MOVING_RMS_CF_H

#include <power/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace power {

    /*!
     * \brief Moving RMS average power
     * \ingroup power
     *
     */
    class POWER_API moving_rms_cf : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<moving_rms_cf> sptr;

      /*!
       * \brief Make a moving RMS calc. block.
       * \param history_len length of the moving average filter.
       *
       * MA (Moving Average) calculation of an RMS value.
       */
      static sptr make(size_t history_len);
    };

  } // namespace power
} // namespace gr

#endif /* INCLUDED_POWER_MOVING_RMS_CF_H */

