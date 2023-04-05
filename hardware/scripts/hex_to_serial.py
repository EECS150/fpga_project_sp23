#!/usr/bin/env python3
# ported from /home/ff/eecs151/tools-151/bin/coe_to_serial
import os
import serial
import time
import argparse

def hex_to_serial(hex_file, addr, port, com):
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

    with open(hex_file, "r") as f:
        program = f.readlines()
    if ('@' in program[0]):
        program = program[1:] # remove first line '@0'
    program = [inst.rstrip() for inst in program]
    size = len(program)*4 # in bytes

    # write a newline to clear any input tokens before entering the command
    ser.write(bytearray([ord('\r')]))
    time.sleep(0.01)

    command = "file {:08x} {:d} ".format(addr, size)
    print("Sending command: {}".format(command))
    for char in command:
        ser.write(bytearray([ord(char)]))
        time.sleep(0.01)

    for inst_num, inst in enumerate(program):
        for char in inst:
            ser.write(bytearray([ord(char)]))
            time.sleep(0.001)
        time.sleep(0.001)
        if (inst_num == len(program)-1):
            print("Sent {:d}/{:d} bytes".format(4+inst_num*4, size), end='\n')
        else:
            print("Sent {:d}/{:d} bytes".format(4+inst_num*4, size), end='\r')

    print("Done")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Load hex file to FPGA via serial UART')
    parser.add_argument('hex_file', action="store", type=str)
    parser.add_argument('--port_name', action="store", type=str, default='/dev/ttyUSB0')
    parser.add_argument('--com_name', action="store", type=str, default='COM11')
    args = parser.parse_args()
    hex_to_serial(args.hex_file, int('0x30000000', 16), args.port_name, args.com_name)
