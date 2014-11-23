# -*- coding:utf-8 -*-
import time


def broadcast_slow(aggregator, message, delay=0.05):
    for node in aggregator.iterkeys():
        aggregator._send(node, message)
        time.sleep(delay)


def syncronous_mode(aggregator, loop_number):
    """ Run messages sending and all in syncronous mode """

    broadcast_slow(aggregator, 'i', 0)   # init sensors values and reset graphs

    broadcast_slow(aggregator, 't', 0)  # set low tx power
    time.sleep(0.5)
    broadcast_slow(aggregator, 'g')  # create connection graph

    broadcast_slow(aggregator, 'T', 0)  # set high tx power
    time.sleep(0.5)
    broadcast_slow(aggregator, 'G')  # validate graph with neighbours

    broadcast_slow(aggregator, 'p') # get nodes graph

    broadcast_slow(aggregator, 'v', 0) # get values
    for i in range(0, loop_number):
        broadcast_slow(aggregator, 's')  # send values to all nodes
        broadcast_slow(aggregator, 'c', 0)  # compute new value
        broadcast_slow(aggregator, 'v', 0)  # get values

