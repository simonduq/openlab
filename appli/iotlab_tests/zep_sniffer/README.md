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

grab two A8 nodes:
- 1 sniffer node (M3 and A8)
- 1 emitter node (M3 only required)

sniffer node needs script serial2loopback.py on A8
nodes need to be in range radio-wize

0. build emitter and sniffer firmwares
1. flash emitter node (a8-3)
2. flash sniffer node (a8-2)
3. run ./serial2loopback.py
4. run tcpdump -vvv -i lo
5. send packet from emitter node
6. check sniffer node's tcpdump output to see zep packet
```

(cd ../../../build.a8 && make tutorial_a8_m3 zep_sniffer)

scp ../../../build.a8/bin/zep_sniffer.elf node-a8-2:
scp ../../../build.a8/bin/tutorial_a8_m3.elf node-a8-3:
scp serial2loopback.py node-a8-2:

in a terminal, setup sniffer node:

ssh node-a8-2
flash_a8_m3 zep_sniffer.elf
./serial2loopback.py &
tcpdump -vvv -i lo

in another terminal, send a packet from emitter:

ssh node-a8-3
flash_a8_m3 tutorial_a8_m3.elf
miniterm.py --echo /dev/ttyA8_M3 500000
<type return to stop help screen>
<type 's' to send radio packet>

check the raw tcpdump output


To inspect sniffer packets using wireshark:
ssh node-a8-2 tcpdump -vvv -i lo -w dump.pcap
^C
scp node-a8-2:dump.pcap .
wireshark dump.pcap

see timestamp, sequence number (UDP packets)
```
