import struct
import sys

with open(sys.argv[1], 'rb') as f:
    byte = f.read(1)
    while byte != "":
        c = struct.unpack("B",byte)[0]
        print bin(c)
        byte = f.read(1)
