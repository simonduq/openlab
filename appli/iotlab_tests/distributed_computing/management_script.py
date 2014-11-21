#! /usr/bin/env python
# -*- coding:utf-8 -*-

""" Get the iotlab-uip for all experiment nodes """

import sys
import re
import json
import time
import signal

import serial_aggregator
import iotlabcli
from iotlabcli.parser import common as common_parser
from iotlabcli.parser import node as node_parser

import algorithm_management


def opts_parser():
    """ Argument parser object """
    import argparse
    parser = argparse.ArgumentParser()
    common_parser.add_auth_arguments(parser)

    nodes_group = parser.add_argument_group(
        description="By default, select currently running experiment nodes",
        title="Nodes selection")

    nodes_group.add_argument('-i', '--id', dest='experiment_id', type=int,
                             help='experiment id submission')

    nodes_group.add_argument(
        '-l', '--list', type=node_parser.nodes_list_from_str,
        dest='nodes_list', help='nodes list, may be given multiple times')

    nodes_group.add_argument(
        '-n', '--num-loop', type=int, required=True,
        dest='num_loop', help='number_of_loops_to_run')

    nodes_group.add_argument(
        '-o', '--out-file', required=True,
        dest='outfile', help='Files where to output traces')

    return parser



def main():
    """ Reads nodes from ressource json in stdin and
    aggregate serial links of all nodes
    """
    parser = opts_parser()
    opts = parser.parse_args()

    try:
        username, password = iotlabcli.get_user_credentials(
            opts.username, opts.password)
        api = iotlabcli.Api(username, password)
        nodes_list = serial_aggregator.get_nodes(
            api, opts.experiment_id, opts.nodes_list, with_a8=True)
    except RuntimeError as err:
        sys.stderr.write("%s\n" % err)
        exit(1)

    outfile = open(opts.outfile, 'w')
    def handle_received_line(identifier, line):
        """ Print one line prefixed by id in format: """
        if line.startswith('DEBUG'):
            pass
        elif line.startswith('INFO'):
            pass
        else:
            outfile.write(line + '\n');

    aggregator = serial_aggregator.NodeAggregator(
        nodes_list, print_lines=True, line_handler=handle_received_line)
    aggregator.start()
    try:
        algorithm_management.syncronous_mode(aggregator, opts.num_loop)
        time.sleep(3)
    finally:
        aggregator.stop()
        outfile.close()


if __name__ == "__main__":
    main()
