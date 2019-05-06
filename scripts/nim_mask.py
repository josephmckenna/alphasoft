#!/usr/bin/env python

import argparse
from math import log

def generateMask(link, port):
    bit = 0x0
    if (port == "r"):
        bit = 0x1
    elif (port == "l"):
        bit = 0x2
    else:
        return 0
    return bit<<(2*link)

def translateMask(mask):
    link = log(mask,2)/2
    if(link%2):
        port = "left"
        link = link//2
    else:
        port = "right"
    print "Mask", hex(mask), "\trefers to the\n"
    print port, "LEMO port\tof the module on"
    print "link", int(link), "\n"
    print "Refer to MIDAS page TRG if necessary to find out module number"

if __name__=='__main__':
 
    parser = argparse.ArgumentParser(description='Generate NIM mask for given trigger arrangement, or translate NIM mask')

    parser.add_argument('-l','--link', type = int,
                        help='Data link, see MIDAS page TRG')
    parser.add_argument('-p','--port', type = str, choices = ['l','r'],
                        help='l/r for left or right LEMO port on A16')
    parser.add_argument('-r','--reverse', type = str, metavar = "MASK",
                        help='NIM mask to translate into trigger port')
    args = parser.parse_args()

    if (args.link and args.port):
        # print "Link: ", args.link, args.port
        mask = generateMask(args.link, args.port)
        print "Mask:", hex(mask), "(", mask, ")"
    elif args.reverse:
        mask = int(args.reverse,0)
        # print "Mask:  ", mask
        translateMask(mask)
    else:
        parser.help()
