ZEP SNIFFER and GPS SYNCED timestamps
=====================================


Overview
--------

4 components:
- sniffer outputing zep
- gps timestamping lib
- `control_node_i2c` lib to get current time
- serial2loopback.py client script

Running a demo
--------------

- 2 a8 nodes
  - 1 sniffer node (M3 on a8)
  - 1 emitter node (M3)

sniffer node needs python script serial2loopback.py on A8

0. build emitter and sniffer firmwares
1. flash emitter node (a8-3)
2. flash sniffer node (a8-2)
3. run ./serial2loopback.py
4. run tcpdump -vvv -i lo
5. send packet on emitter node
6. check tcpdump output to see zep packet
```

cd build.a8
make tutorial_a8_m3 zep_sniffer

scp bin/zep_sniffer.elf a8-2:
scp bin/tutorial_m3.elf a8-3:

ssh a8-2 flash_a8.sh zep_sniffer.elf

ssh a8-3 flash_a8.sh tutorial_m3.elf

scp serial2loopback.py a8-2:
ssh a8-2 ./serial2loopback.py &
ssh a8-2 tcpdump -vvv -i lo


check sniffer packets using wireshark:
ssh a8-2 tcpdump -vvv -i lo -w dump.pcap
^C
scp a8-2:dump.pcap .
wireshark dump.pcap

see timestamp, sequence number (UDP packets)
```

