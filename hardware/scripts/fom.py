#!/usr/bin/env python3

version = '1.0'

from get_cost import get_cost
from get_fmax import get_fmax
from get_cpi import get_cpi, get_cpi_sim
import os
import sys
import time
import argparse

surpt = "build/synth/post_synth_utilization.rpt"
strpt = "build/synth/post_synth_timing_summary.rpt"
iurpt = "build/impl/post_place_utilization.rpt"
itrpt = "build/impl/post_route_timing_summary.rpt"

def open_file(fname):
  if os.path.isfile(fname):
    print('Using {} (last modified: {})'.format(fname, time.ctime(os.path.getmtime(fname))))
    return open(fname, 'r')
  else:
    print(fname + ' not found')
    return None

def open_rpts(urpt, trpt, fcost, ffmax):
  u = None
  if fcost is None:
    u = open_file(urpt)
  t = None
  if ffmax is None:
    t = open_file(trpt)
  return u, t

parser = argparse.ArgumentParser(
    description="FOM calculation program: by default, reads the newest set of reports for cost and fmax, runs simulation for CPI, and displays the estimated FOM. '-s' or '-i' forces it use the reports in 'build/synth' or 'build/impl', respectively. You can manually specify the reports with '-u' and '-t'. '-r' runs benchmarks on FPGA to get real CPI. You can override cost, fmax, and CPI with '-c', '-f', and '-p', respectively.")

parser.add_argument('-s', '--synth', action='store_true', help='use syntheis reports')
parser.add_argument('-i', '--impl', action='store_true', help='use implementation reports')

parser.add_argument('-u', '--urpt', action='store', help='resource utilization report')
parser.add_argument('-t', '--trpt', action='store', help='timing summary report')

parser.add_argument('-r', '--run', action='store_true', help='get real CPI by running on FPGA')
parser.add_argument('--port_name', action='store', default='/dev/ttyUSB0')
parser.add_argument('--com_name', action='store', default='COM11')

parser.add_argument('-c', '--cost', action='store', type=int)
parser.add_argument('-f', '--fmax', action='store', type=float, help='in MegaHertz, int or float')
parser.add_argument('-p', '--cpi', action='store' , type=float)

args = parser.parse_args()

if (args.urpt is None) != (args.trpt is None):
  print('both urpt and trpt must be specified when one of them are specified')
  exit()
if args.synth and args.impl:
  print('specify none or either of -s or -i')
  exit()
if not(args.cpi is None) and args.run:
  print('specify none or either of -p or -r')
  exit()

if not(args.urpt is None):
  u, t = open_rpts(args.urpt, args.trpt, args.cost, args.fmax)
elif args.synth:
  u, t = open_rpts(surpt, strpt, args.cost, args.fmax)
elif args.impl:
  u, t = open_rpts(iurpt, itrpt, args.cost, args.fmax)
elif os.path.isfile(surpt) and os.path.isfile(iurpt):
  if os.path.getmtime(surpt) > os.path.getmtime(iurpt):
    u, t = open_rpts(surpt, strpt, args.cost, args.fmax)
  else:
    u, t = open_rpts(iurpt, itrpt, args.cost, args.fmax)
elif os.path.isfile(surpt):
  u, t = open_rpts(surpt, strpt, args.cost, args.fmax)
elif os.path.isfile(iurpt):
  u, t = open_rpts(iurpt, itrpt, args.cost, args.fmax)
else:
  u, t = None, None

if u is None and args.cost is None:
  print('utilization report not found and cost not specified')
  exit()
elif args.cost is None:
  cost = get_cost(u)
else:
  cost = args.cost

if t is None and args.fmax is None:
  print('timing report not found and fmax not specified')
  exit()
elif args.fmax is None:
  fmax = get_fmax(t)
else:
  fmax = args.fmax

if args.run:
  cpi = get_cpi(args.port_name, args.com_name)
elif args.cpi is None:
  cpi = get_cpi_sim()
else:
  cpi = args.cpi

fom = (1000000.0 * fmax) / cpi / cost

print('')
print('Fmax: ' + str(fmax))
print("CPI: {:.2f}".format(cpi))
print('Cost: ' + str(cost))
print('')
if args.run:
  print("FOM: {:.2f}".format(fom))
else:
  print("FOM (estimate): {:.2f}".format(fom))
