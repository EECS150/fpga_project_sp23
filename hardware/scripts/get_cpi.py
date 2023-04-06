#!/usr/bin/env python3

from subprocess import Popen, PIPE
import numpy as np
import argparse

def get_cpi_sim():
  print('Running simulation...')
  p = Popen('make small-tests -B', shell=True, stdout=PIPE, stderr=PIPE)
  stdout, stderr = p.communicate()
  lines = stdout.decode('ascii', errors='ignore').splitlines()
  cyc_cnts = []
  inst_cnts = []
  for line in lines[2:]:
    line = line.strip()
    print(line)
    if line.startswith('Cycle Count:'):
      cyc_cnts.append(int(line.split()[2], 16))
    elif line.startswith('Instruction Count:'):
      inst_cnts.append(int(line.split()[2], 16))
  print('...Simulation complete')
  print('Cycle Counts: {}'.format(cyc_cnts))
  print('Instruction Counts: {}'.format(inst_cnts))
  if len(cyc_cnts) != len(inst_cnts):
    print('Error: got {} cycle counts and {} instruction counts'.format(len(cyc_cnts), len(inst_cnts)))
    return None
  cyc_cnts = np.array(cyc_cnts)
  inst_cnts = np.array(inst_cnts)
  cpis = cyc_cnts / inst_cnts
  np.set_printoptions(precision=2)
  print('CPIs: ' + np.array2string(cpis, separator=', '))
  cpi = cpis.prod()**(1.0/len(cpis))
  print('CPI (geomean): {:.2f}'.format(cpi))
  return cpi

def get_cpi():
  return None

if __name__ == '__main__':
  get_cpi_sim()
