#!/usr/bin/python

###############################################################################
# hex-to-int.py
#   - Converts a hexadecimal value into an integer value
#
# Hex values can be with or without the '0x' prefix, e.g.:
#   py hext-to-int.py 0xF37B
#   py hext-to-int.py F37B
###############################################################################

import sys

def main(argv):
  hexstr = argv[0]
  hexlen = len(hexstr)
  while hexlen < 4:
    hexstr = '0' + hexstr
    hexlen = len(hexstr)
  print("Hex value: ", hexstr)
  print("Int value: ", int(hexstr, 16))

if __name__ == "__main__":
   main(sys.argv[1:])
