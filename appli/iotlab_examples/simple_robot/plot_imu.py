#!/usr/bin/python
# -*- coding: utf-8 -*-
import numpy as np

""" plot_imu.py

plot_imu

"""

import sys
import numpy as np
import matplotlib.pyplot as plt

FIELDS = {'time': 0, 'name': 1, 'type': 2, 'X': 3, 'Y': 4, 'Z': 5}

FILE = 'robot1.txt'
#NODES = ['m3-29', 'm3-30', 'm3-31', 'm3-32']
NODES = ['m3-30']

def imu_load(filename):
    """ Load iot-lab imu file

    Parameters:
    ------------
    filename: string
              imu filename saved from smart_tiles firmware

    Returns:
    -------
    data : numpy array
    [timestamp node_name sensor_type X Y Z]
    """

    try:
        mytype = [('name', '|S11'), ('type', '|S11'),
                  ('X', '<f8'), ('Y', '<f8'), ('Z', '<f8')]
        # pylint:disable=I0011,E1101
        data = np.genfromtxt(filename, skip_header=1, invalid_raise=False,
                             delimiter=";", dtype=mytype)
    except IOError as err:
        sys.stderr.write("Error opening oml file:\n{0}\n".format(err))
        sys.exit(2)
    except (ValueError, StopIteration) as err:
        sys.stderr.write("Error reading oml file:\n{0}\n".format(err))
        sys.exit(3)

    # Start time to 0 sec
    #data['time'] = 0

    return data


def imu_extract(data, node_name='', sensor_type='Acc'):
    """ Extract iot-lab imu data for node_name, sensor_type

    Parameters:
    ------------
    data: numpy array
      [time name type X Y Z]
    node_name: string
       name of the iot-lab name to be extracted
    sensor_type: string
       type of the sensor to be extracted 'Acc' or 'Mag'

    """
    if node_name != '':
        condition = data['name'] == node_name
        # pylint:disable=I0011,E1101
        filtname_data = np.extract(condition, data)
    else:
        filtname_data = data
    condition = filtname_data['type'] == sensor_type
    # pylint:disable=I0011,E1101
    filt_data = np.extract(condition, filtname_data)
    return filt_data


def imu_plot(data, title):
    """ Plot iot-lab imu data

    Parameters:
    ------------
    data: numpy array
      [time name type X Y Z]
    title: string
       title of the plot
    """
    plt.figure()
    plt.grid()
    plt.title(title)
    plt.plot(data['X'])
    plt.plot(data['Y'])
    plt.plot(data['Z'])
    plt.ylabel('Acceleration (G)')
    plt.xlabel('Sample Time (sec)')

    return


def imu_plot_norm(data, title):
    """ Plot iot-lab imu data

    Parameters:
    ------------
    data: numpy array
      [time name type X Y Z]
    title: string
       title of the plot
    """
    plt.figure()
    plt.grid()
    plt.title(title)
    norm = np.sqrt(data['X']**2 + data['Y']**2 + data['Z']**2)
    plt.plot(norm)
    plt.ylabel('Norm')
    plt.xlabel('Sample Time (sec)')

    return

def imu_all_plot(data, title, ylabel, nodes):
    """ Plot iot-lab imu data

    Parameters:
    ------------
    data: numpy array
      [time name type X Y Z]
    title: string
       title of the plot
    ylabel: stringx
       ylabel of the plot
    nodes: tab of string
       list of nodes_names
    """
    nbplots = len(nodes)

    if nbplots > 0:
        plt.figure()
        i = 0
        for node in nodes:
            i = i + 1
            node_plot = plt.subplot(nbplots, 1, i)
            node_plot.grid()
            plt.title(title + nodes[i-1])
            datanode = imu_extract(data, nodes[i-1], 'Acc')
            peaknode = imu_extract(data, nodes[i-1], 'Peak')
            print nodes[i-1], len(datanode)
            norm = np.sqrt(datanode['X']**2 + datanode['Y']**2
                           + datanode['Z']**2)
            node_plot.plot(datanode['time'], norm)
            node_plot.plot(peaknode['time'], peaknode['Z'], 'ro')
            plt.ylabel(ylabel)
        plt.xlabel('Sample Time (sec)')

    return


def test0(index):
    """ test 0
    """
    data = imu_load(FILE)
    print NODES[index]
    datanode = imu_extract(data, NODES[index], sensor_type='Acc')
    imu_plot(datanode, "Accelerometers " + NODES[index])
    datanode = imu_extract(data, NODES[index], sensor_type='Mag')
    imu_plot(datanode, "Magnetometers " + NODES[index])
    imu_plot_norm(datanode, "Norm Magnetometers " + NODES[index])


def test1():
    """ test 1
    """
    data = imu_load(FILE)
    imu_all_plot(data, "Accelerometers ", "Acceleration (G)", NODES)
    plt.show()


if __name__ == "__main__":
    test0(0)
#    test0(1)
#    test0(2)
#    test0(3)
#    test1()
    plt.show()
