
gr-power
========

GNU radio power library.

Currently only contains two blocks: Moving RMS and power stats.

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

Power stats
-----------

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
