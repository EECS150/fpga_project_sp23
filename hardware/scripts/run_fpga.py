#!/usr/bin/env python3
from hex_to_serial import hex_to_serial
import os
import serial
import time
import argparse

def run_fpga(hex_file, port, com):
    hex_to_serial(hex_file, int('0x30000000', 16), port, com)

    # Windows
    if os.name == 'nt':
        ser = serial.Serial()
        ser.baudrate = 115200
        ser.port = com
        ser.open()
    # Linux
    else:
        ser = serial.Serial(port)
        ser.baudrate = 115200

    command = "run\r"
    print("Sending command: {}".format(command))
    for char in command:
        ser.write(bytearray([ord(char)]))
        time.sleep(0.01)

    ser.readline()

    cycle_count = -1
    inst_count = -1
    while cycle_count == -1 or inst_count == -1:
        line = ser.readline().decode('utf-8').strip()
        print(line)
        if 'Cycle Count' in line:
            line = line.split(': ')
            cycle_count = int(line[1], 16)
        if 'Instruction Count' in line:
            line = line.split(': ')
            inst_count = int(line[1], 16)

    print("Done")

    return cycle_count, inst_count

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Load and execute hex file on FPGA')
    parser.add_argument('hex_file', action="store", type=str)
    parser.add_argument('--port_name', action="store", type=str, default='/dev/ttyUSB0')
    parser.add_argument('--com_name', action="store", type=str, default='COM11')
    args = parser.parse_args()
    cycle_count, inst_count = run_fpga(args.hex_file, args.port_name, args.com_name)
