#!/usr/bin/env python3

# Copyright 2017 OpenSWE1R Maintainers
# Licensed under GPLv2 or any later version
# Refer to the included LICENSE.txt file.

# This script takes information from files created with DumpProfilingHeat() and
# tells you in which file each instruction was encountered.

import sys
from collections import defaultdict

if len(sys.argv) <= 1:
  print("Usage: %s <input file> [input file [...]]" % sys.argv[0])
  exit(1)

def GetInteger(v):
  v = v.strip()
  if v[0:2] == "0x":
    return int(v[2:], 16)
  return int(v)

instructions = defaultdict(list)

for index, path in enumerate(sys.argv[1:]):
  with open(path,'r') as f:
    lines = f.readlines()
    for line in lines:
      s = line.split()
      if s[0] != "PROF":
        continue

      address = GetInteger(s[1])
      count = GetInteger(s[2])
      duration = GetInteger(s[3])

      if count > 0:
        instructions[address] += [True]

      i = 4
      c = len(s)

      if i < c and s[i] == "CALLED":
        i += 1

      if i < c and s[i] == "BLOCK_ENTER":
        i += 1

      if i < c and s[i] == "BLOCK_EXIT":
        i += 1

  # Pad those which were not encountered this time
  for address in instructions:
    while(len(instructions[address]) <= index):
      instructions[address] += [False]

for address in instructions:
  pattern = ""
  for encountered in instructions[address]:
    pattern += "X" if encountered else "-"
  print("0x%08X %s" % (address, pattern))

exit(0)
