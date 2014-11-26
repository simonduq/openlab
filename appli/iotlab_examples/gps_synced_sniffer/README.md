ZEP SNIFFER with GPS SYNCED timestamps
======================================


Overview
--------

4 components:
- radio sniffer outputing zep on serial line
- gps timestamping lib
- `control_node_i2c` lib to get current time
- serial2loopback.py script wrapping zep as udp

Running a demo
--------------

0. grab two A8 nodes in rocquencourt - all have GPS:
   - 1 sniffer node (M3 on A8 and A8 with GPS required)
   - 1 emitter node (M3 only required)

nodes need to be in range radio-wize

1. build M3 firmware files and copy to nodes
2. copy serial2loopback.py to sniffer node
3. flash sniffer and emitter M3's
4. run ./serial2loopback.py on sniffer node
5. run tcpdump -vvv -i lo on sniffer node
6. send packet from emitter node
7. check sniffer node's tcpdump output to see zep packet

Full trace of demo
------------------
```
ssh rocquencourt.iot-lab.info
experiment-cli submit -d 10 -l rocquencourt,a8,2+3

(use auth-cli if this is a first time init)

git clone https://github.com/iot-lab/openlab.git
cd openlab/ && mkdir build.a8
cd build.a8/ && cmake .. -DPLATFORM=iotlab-a8-m3
make tutorial_a8_m3 gps_synced_sniffer

scp bin/gps_synced_sniffer.elf root@node-a8-2:
scp bin/tutorial_a8_m3.elf root@node-a8-3:
scp ../appli/iotlab_examples/gps_synced_sniffer/serial2loopback.py root@node-a8-2:

setup sniffer node:

ssh root@node-a8-2
flash_a8_m3 gps_synced_sniffer.elf
./serial2loopback.py &
tcpdump -vvv -i lo

in another terminal, send a packet from emitter:

ssh rocquencourt.iot-lab.info
ssh root@node-a8-3
flash_a8_m3 tutorial_a8_m3.elf
miniterm.py --echo /dev/ttyA8_M3 500000
<type return to stop help screen>
<type 's' to send radio packet>
<type crtl-] to exit>

check the raw tcpdump output


To inspect sniffer packets using wireshark from your PC:

make sure you have wireshark installed on your PC
make sure you have ~/.ssh/config configured as follows:

Host node-a8-*.rocquencourt.iot-lab.info
  User root
  ProxyCommand ssh fit-roc -W %h:%p
  StrictHostKeyChecking no


in another terminal:

ssh node-a8-2.rocquencourt.iot-lab.info tcpdump -vvv -i lo -w - | wireshark -i -


back to the "emitter" terminal:

send enough packets for wireshark to kick in

see timestamp, sequence number (UDP packets)
```
