#! /usr/bin/env python
# -*- coding:utf-8 -*-

""" Run radio characterizations on nodes """

import os
import sys
import time
import serial_aggregator
from serial_aggregator import NodeAggregator

FIRMWARE_PATH = "node_radio_characterization.elf"

class RadioCharac(object):
    """ Radio Characterization """
    def __init__(self, nodes_list):
        self.state = {}
        self.nodes = NodeAggregator(nodes_list)

    def start(self):
        self.nodes.start()

    def stop(self):
        self.nodes.stop()

    def _answers_handler(self, node_id, msg):
        """ Handle answers """
        try:
            line = msg.split(' ')

            if line[0] == 'config_radio':
                if line[1] == 'ACK':
                    self.state['config']['success'].append(node_id)
                else:
                    self.state['config']['failure'].append(node_id)
            elif line[0] == 'send_packets':
                if line[1] == 'ACK':
                    # send_packets ACK 0
                    self.state['send']['success'].append(node_id)
                else:
                    # send_packets NACK 42
                    self.state['send']['failure'].append('%s %s' %
                                                         (node_id, line[2]))

            elif line[0] == 'radio_rx':
                # "radio_rx m3-128 -17dBm 5 -61 255 sender power num rssi lqi"
                sender = line[1]

                # add list for this node
                results = self.state['radio'][sender].get(node_id, [])
                self.state['radio'][sender][node_id] = results

                # add rx informations
                #results.append("%s" % ' '.join(line[2:6]))
                results.append(tuple(line[2:6]))
            elif line[0] == 'radio_rx_error':
                print >> sys.stderr, ("Radio_rx_error %s %r" %
                                      (node_id, line[1]))

            else:
                print >> sys.stderr, "UNKOWN_MSG: %s %r" % (node_id, msg)

        except IndexError:
            print >> sys.stderr, "UNKOWN_MSG: %s %r" % (node_id, msg)


    def run_characterization(self, channel, power, num_pkts, delay):
        """ Run the radio characterizations on nodes"""

        self.start()

        self.state['config'] = {'success': [], 'failure': []}
        self.state['send'] = {'success': [], 'failure': []}

        #nodes = self.nodes.values()
        #self.nodes_cli('--update', firmware=FIRMWARE_PATH)
        #time.sleep(2)  # wait started

        # init all nodes handlers and radio config
        self.state['radio'] = {}

        for node in self.nodes.values():
            self.state['radio'][node.node_id] = {}
            node.line_handler.append(self._answers_handler)

        cmd = "config_radio -c {channel}\n"
        self.nodes.broadcast(cmd.format(channel=channel))

        time.sleep(10)  # wait

        cmd = "send_packets -i {node_id} -p {power} -n {num_pkts} -d {delay}\n"
        for node in self.nodes.values():
            print >> sys.stderr, "sending node %s" % node.node_id
            node.send(cmd.format(node_id=node.node_id, power=power,
                                 num_pkts=num_pkts, delay=delay))
            time.sleep(2)


        self.stop()
        return self.state


def main(argv):

    json_dict = serial_aggregator.extract_json(sys.stdin.read())
    nodes_list = serial_aggregator.extract_nodes(json_dict, os.uname()[1])

    rad_charac = RadioCharac(nodes_list)

    num_pkt = 32
    result = rad_charac.run_characterization(16, "0dBm", num_pkt, 10)

    if '--summary' in argv:
        for sender_node in result['radio'].values():
            for rx_node in sender_node:
                raw_result = sender_node[rx_node]

                rx_pkt = 100 * len(raw_result) / float(num_pkt)
                average_rssi = sum([int(res[2]) for res in raw_result]) \
                    / float(num_pkt)
                sender_node[rx_node] = {
                    'pkt_reception': "%.1f %%" % rx_pkt,
                    'average_rssi': average_rssi
                }



    import json
    result_str = json.dumps(result, sort_keys=True, indent=4)
    print >> sys.stderr, result_str


if __name__ == "__main__":
    main(sys.argv)
