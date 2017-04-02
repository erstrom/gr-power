/* -*- c++ -*- */

#define POWER_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "power_swig_doc.i"

%{
#include "power/moving_rms_cf.h"
#include "power/moving_rms_ff.h"
#include "power/power_stats.h"
%}


%include "power/moving_rms_cf.h"
GR_SWIG_BLOCK_MAGIC2(power, moving_rms_cf);
%include "power/moving_rms_ff.h"
GR_SWIG_BLOCK_MAGIC2(power, moving_rms_ff);
%include "power/power_stats.h"
GR_SWIG_BLOCK_MAGIC2(power, power_stats);
