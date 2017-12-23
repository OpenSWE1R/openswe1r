#!/usr/bin/env python3

# Copyright 2017 OpenSWE1R Maintainers
# Licensed under GPLv2 or any later version
# Refer to the included LICENSE.txt file.

# This script takes information from DumpProfilingHeat() and tries
# to accumulate times for each function

import sys

if len(sys.argv) != 2:
  print("Usage: %s <input file>" % sys.argv[0])
  exit(1)

def GetInteger(v):
  v = v.strip()
  if v[0:2] == "0x":
    return int(v[2:], 16)
  return int(v)
  
total_instructions = 0
total_count = 0
total_duration = 0

function_address = 0
function_calls = 0
function_instructions = 0
function_count = 0
function_duration = 0

with open(sys.argv[1],'r') as f:
  lines = f.readlines()
  for line in lines:
    s = line.split()
    if s[0] != "PROF":
      continue

    address = GetInteger(s[1])
    count = GetInteger(s[2])
    duration = GetInteger(s[3])

    total_instructions += 1
    total_count += count
    total_duration += duration

    function_instructions += 1
    function_count += count
    function_duration += duration

    i = 4
    c = len(s)

    if i < c and s[i] == "CALLED":
      # Print the current running function
      if function_calls > 0:
        print("sub_%X %u calls, %u instructions, %u executions, duration %u" % (function_address, function_calls, function_instructions, function_count, function_duration))
      # Start a new function
      function_instructions = 0
      function_count = 0
      function_duration = 0
      function_address = address
      function_calls = count
      i += 1

    if i < c and s[i] == "BLOCK_ENTER":
      i += 1

    if i < c and s[i] == "BLOCK_EXIT":
      i += 1


print("total %u instructions, %u executions, duration %u" % (total_instructions, total_count, total_duration))

exit(0)
