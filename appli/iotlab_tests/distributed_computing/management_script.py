#! /usr/bin/env python
# -*- coding:utf-8 -*-

""" Get the iotlab-uip for all experiment nodes """

import sys
import re
import json
import time
import signal
import subprocess

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

class NodeResults(object):

    def __init__(self, outfile):
        self.outfile = outfile
        self.neighbours = {}

    def handle_line(self, identifier, line):
        """ Print one line prefixed by id in format: """
        if line.startswith(('DEBUG', 'INFO', 'ERROR')):
            return

        if 'Neighbours' in line:
            # A569;Neighbours;6;A869;A172;C280;9869;B679;A269
            values = line.split(';')
            node = values[0]
            neighbours = values[3:]
            self.neighbours[node] = neighbours
            return

        self.outfile.write(line + '\n')


    def write_neighbours_graph(self):
        out_png = '%s.png' % self.outfile.name
        cmd = ['dot', '-T', 'png', '-o', out_png]
        dot_process = subprocess.Popen(cmd, stdin=subprocess.PIPE)
        dot_process.communicate(self.neighbours_graph())
        print "Neighbours graph written to %s" % out_png

    def neighbours_graph(self):
        links = self.neighbours_links()
        res = ''
        res += 'digraph G {\n'
        res += '    center=""\n'
        res += '    concentrate=true\n'
        res += '    graph [ color=black ]\n'
        for (node, neigh) in links['double']:
            res += '    "%s" -> "%s"[color=black]\n' % (node, neigh)
            res += '    "%s" -> "%s"[color=black]\n' % (neigh, node)
        for (node, neigh) in links['simple']:
            res += '    "%s" -> "%s"[color=red]\n' % (node, neigh)
        res += '}\n'
        return res

    def neighbours_links(self):
        simple_links = set()
        double_links = set()
        for node, neighbours in self.neighbours.items():
            # detect double and one sided links
            for neigh in neighbours:
                link = (node, neigh)
                rev_link = (neigh, node)
                # put the link in 'double_links' if reverse was already present
                if rev_link not in simple_links:
                    simple_links.add(link)
                else:
                    simple_links.remove(rev_link)
                    double_links.add(link)
        return {'simple': simple_links, 'double': double_links}



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
    results = NodeResults(outfile)
    aggregator = serial_aggregator.NodeAggregator(
        nodes_list, print_lines=True, line_handler=results.handle_line)
    aggregator.start()
    try:
        algorithm_management.syncronous_mode(aggregator, opts.num_loop)
        time.sleep(3)
    finally:
        aggregator.stop()
        outfile.close()
        results.write_neighbours_graph()


if __name__ == "__main__":
    main()
