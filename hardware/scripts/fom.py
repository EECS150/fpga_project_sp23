#!/usr/bin/env python3
# usage should be fom fmax cycle_count instr_count
# Default input file is post_synth_resource_utilization.rpt first then post_place_utilization.rpt second
# cycle_count and instr_count can be overridden with -c <cpi> or --cpi <cpi>
# cost can be overridden with -s <cost> or --cost <cost>
# The default input file can be overriden with -f <filename> or --file <filename>

version = '1.0'

import os
import sys
import argparse

report1 = "build/synth/post_synth_utilization.rpt"
report2 = "build/impl/post_place_utilization.rpt"

elements = {'LUT6': 1713,
            'FDRE': 107,
            'LUT4': 429,
            'LUT5': 857,
            'LUT3': 429,
            'LUT2': 429,
            'CARRY4': 54,
            'RAMD32': 857,
            'RAMB36E1': 0,
            'RAMS32': 857,
            'FDSE': 107,
            'LUT1': 429,
            'OBUFT': 0,
            'IBUF': 0,
            'FDCE': 107,
            'BUFG': 0,
            'SRL16E': 429,
            'PLLE2_ADV': 0,
            'OBUF': 0
            }

def get_cost(file):
  cost = 0
  for line in file:
    if line.startswith('|  Ref Name | Used | Functional Category |'):
      next(file)
      line = next(file)
      while not(line.startswith('+')):
        x = line.split('|')
        ele = x[1].strip()
        cnt = int(x[2].strip())
        if ele in elements:
          cst = cnt * elements[ele]
          cost += cst
        else:
          print(ele + ' not a known element!')
        line = next(file)
  return cost

parser = argparse.ArgumentParser(
    description="FOM calculation program: The default report file is post_synth_utilization.rpt, tried first, then post_place_utilization.rpt, tried second.  Override the report file with '-f'.  fmax is always required.  Specify either both cyc_cnt and inst_cnt or use '-c' to specify the CPI.")

def auto_int(x):
  return int(x, 0)

parser.add_argument('fmax', action="store", type=float, help='in MegaHertz, int or float')
parser.add_argument('cyc_cnt', action="store", type=auto_int, help='hex string with format 0x..', nargs='?')
parser.add_argument('inst_cnt', action="store", type=auto_int, help='hex string with format 0x..', nargs='?')
parser.add_argument('-c', '--cpi', action="store", dest="cpi", type=float)
parser.add_argument('-s', '--cost', action="store", dest="cost", type=int)
parser.add_argument('-f', '--file', action="store", dest="file")
parser.add_argument('--version', action='version', version='%(prog)s ' + version)

args = parser.parse_args()

if not(args.file is None):
    if os.path.isfile(args.file):
        print('Using ' + args.file)
        f = open(args.file, 'r')
    else:
      print(args.file + ' not found')
      exit()
elif os.path.isfile(report1) and os.path.isfile(report2):
    if os.path.gettime(report1) < os.path.gettime(report2):
        print('Using ' + report1)
        f = open(report1, 'r')
    else:
        print('Using ' + report1)
        f = open(report1, 'r')
elif os.path.isfile(report1):
    print('Using ' + report1)
    f = open(report1, 'r')
elif os.path.isfile(report2):
    print('Using ' + report2)
    f = open(report2, 'r')
else:
  print(report1 + ' nor ' + report2 + ' found')
  exit()

if (args.cpi is None):
  if ((args.cyc_cnt is None) or (args.inst_cnt is None)):
    print('Error: either us -c <cpi> or specify both cyc_cnt and inst_cnt')
    exit()
  else:
    cpi = float(args.cyc_cnt) / args.inst_cnt
else:
  cpi = args.cpi

if (args.cost is None):
  cost = get_cost(f)
else:
  cost = args.cost

fom = (1000000.0 * args.fmax) / cpi / cost

print('Fmax: ' + str(args.fmax))
print("CPI: {:.2f}".format(cpi))
print('Cost: ' + str(cost))
print('')
print("FOM: {:.2f}".format(fom))




  
  

