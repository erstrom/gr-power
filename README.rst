
gr-power
========

GNU radio power library.

Currently only contains one block: Moving RMS.

Moving RMS
----------

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

Build
-----

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

The Moving RMS block can be found under the Power section in GRC.

There is currently only one configuration parameter for the block:
**Averaging history length**

This is the number of values that will be used in the calculation of the average,
i.e, the filter length *N*.
