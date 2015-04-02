#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys

import matplotlib.pyplot

import matplotlib.pylab as plt


def read_file(logfile):
    values_tuple = []
    with open(logfile) as infile:
        for line in infile:
            line = line.strip()
            try:
                line = line.split(';')[2]
                if not line.startswith('T:'):
                    continue
                line = line.split(':')[1]
                line = line.split(' ')
                values_tuple.append((float(line[1]), line[0]))
            except IndexError:
                pass

        # line.split(';')[2] for line in infile)
        # msgs = (line.split(':')[1] for line in msgs_lines if line.startswith('T:'))
        # values = [line.split(' ') for line in msgs]
    return values_tuple

def extract_ranges_dict(values_tuple):
    assert (len(values_tuple) % 2) == 0

    ranges = {}
    for i in range(32, 40, 2):
        start, end = values_tuple[i: i + 2]
        print start, end

        # check events are the 'same' but end and start
        start_n, val = start[1].rsplit('_', 1)
        assert val == 'start'
        end_n, val = end[1].rsplit('_', 1)
        assert val == 'end'
        assert start_n == end_n

        ranges.setdefault(start_n, []).append((start[0], end[0] - start[0]))
    return ranges

Y_CFG = {
    'pkt_irq': ((0, 2), 'red'),
    'pkt_rx': ((2, 2), 'green'),
    'ack_irq': ((4, 2), 'blue'),
    'ack_rx': ((6, 2), 'yellow'),
}


def graph(ranges_dict):
    plt.plot()

    # I want them in order
    for vals in ('pkt_irq', 'pkt_rx', 'ack_irq', 'ack_rx'):

        yrange, color = Y_CFG[vals]

        ranges = ranges_dict[vals]
        print ranges
        matplotlib.pyplot.broken_barh(ranges, yrange, facecolors=color)
    plt.show()

def main():
    logfile = sys.argv[1]
    values_tuple = read_file(logfile)
    assert values_tuple[0][1].endswith('start')
    assert values_tuple[-1][1].endswith('end')

    ranges_dict = extract_ranges_dict(values_tuple)

    graph(ranges_dict)



if __name__ == '__main__':
    main()
