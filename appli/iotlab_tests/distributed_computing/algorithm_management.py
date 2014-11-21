# -*- coding:utf-8 -*-
import time


def broadcast_slow(aggregator, message, delay=0.02):
    for node in aggregator.iterkeys():
        aggregator._send(node, message)
        time.sleep(delay)


def syncronous_mode(aggregator, loop_number):
    """ Run messages sending and all in syncronous mode """

    broadcast_slow(aggregator, 'i')        # init sensors values and reset graphs

    broadcast_slow(aggregator, 'g', delay=0.1)  # create connection graph
    time.sleep(0.5)
    broadcast_slow(aggregator, 'G', delay=0.1)  # validate graph with neighbours

    broadcast_slow(aggregator, 'p') # get nodes graph

    broadcast_slow(aggregator, 'v') # get values
    for i in range(0, loop_number):
        broadcast_slow(aggregator, 's', delay=0.1)  # send values to all nodes
        broadcast_slow(aggregator, 'c')  # compute new value
        broadcast_slow(aggregator, 'v')  # get values

