/* -*- c++ -*- */

#define POWER_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "power_swig_doc.i"

%{
#include "power/moving_rms_cf.h"
%}


%include "power/moving_rms_cf.h"
GR_SWIG_BLOCK_MAGIC2(power, moving_rms_cf);
