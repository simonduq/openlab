M3 tutorial
===========

This source code is used for the two first tutorial "First Steps" on the
<a href="https://www.iot-lab.info/tutorials/">IoT-LAB Website</a>.

It provides following features:

* read sensors
* send a radio packet with [CSMA MAC layer implementation](https://github.com/iot-lab/openlab/tree/master/net/mac_csma)
* blink LEDs (red, green, blue) with a 1hz period
* get nodes UID and match it with real node type and number using {UID: node} table
* i2c interact with `control_node` to get time
