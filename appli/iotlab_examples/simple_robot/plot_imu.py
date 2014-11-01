"""
plot_imu.py

Display off-line IMU data from IoT-LAB using Python (matplotlib).

The logging file is generated on IoT-LAB server  :
 pissardg@devgrenoble:~$ nc m3-381 20000 >robot.txt

"""

import numpy as np
import matplotlib.pyplot as plt
import argparse
import sys

DEFAULTFILE = "robot.txt"


def sensor_load(fname):
    ''' Load the file logging sensors values'''
    try:
        fid = open(fname, 'r')
    except IOError as err:
        sys.stderr.write("Error opening log IMU file:\n{0}\n".format(err))
        sys.exit(2)

    acc = []
    mag = []
    gyr = []
    ang = []

    for line in fid.readlines():
        row = line.replace('\n', '').split(';')
        if row[0] == 'Acc':
            acc.append([float(row[1]), float(row[2]), float(row[3])])
        elif row[0] == 'Mag':
            mag.append([float(row[1]), float(row[2]), float(row[3])])
        elif row[0] == 'Gyr':
            gyr.append([float(row[1]), float(row[2]), float(row[3])])
        elif row[0] == 'Ang':
            ang.append([float(row[1])])

    fid.close()

    return acc, mag, gyr, ang


def sensor_plot(fname):
    ''' Plot logging sensors values'''
    acc, mag, gyr, ang = sensor_load(fname)

    plt.figure()
    plt.title("Accelerometers")
    plt.plot(acc)

    plt.figure()
    plt.title("Magnetometers")
    plt.plot(mag)

    plt.figure()
    plt.title("Gyrometers")
    plt.plot(gyr)

    plt.figure()
    plt.title("Robot orientation")
    plt.plot(ang)
    plt.show()


def main():
    """  Main function """
    # create parser
    parser = argparse.ArgumentParser(description="Plot off-line IMU")
    # add expected arguments
    parser.add_argument('--file', dest='file', required=False)

    # parse args
    args = parser.parse_args()
    filename = args.file
    if filename is None:
        filename = DEFAULTFILE
    # Plot log IMU
    sensor_plot(filename)

if __name__ == '__main__':
    main()
