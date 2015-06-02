============
banet
=====

body-area network dedicated to collect IMU and sensors data based on
the TDMA mac protocol.

Overview
--------


Implementation 
---- 


Code
---- 

node-cli -i 13784 -l grenoble,m3,30 -up bin/banet_sink.elf
node-cli -i 13784 -l grenoble,m3,31 -up bin/banet_node.elf

nc m3-30 20000