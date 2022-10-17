#!/usr/bin/python

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
