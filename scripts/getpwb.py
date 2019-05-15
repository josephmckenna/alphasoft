#!/usr/bin/env python3

import argparse
from os import environ

def readmap():
    pads=dict()
    with open(environ['AGRELEASE']+'/ana/pad.map','r') as fin:
        for line in fin:
            data=[int(x) for x in line.split()]
            pads[(data[0],data[1])] = data[2]
    return pads
    
def getpwb(padmap,sec,row):
    try:
        return padmap[(sec,row)]
    except KeyError:
        print("Out of Range")
        return -1

def getsca(sec,row):
    sca=[['A','D'],['B','C']]
    x=int(row/36)%2
    s=sec-1
    if s<0:
        s=31
    y=int(s/2)%2
    return sca[x][y]

if __name__=='__main__':

   parser = argparse.ArgumentParser(description='Find PWB from map given pad')

   parser.add_argument('sec', type=int,
                        help='Pad sector (phi)')
   parser.add_argument('row', type=int,
                        help='Pad row (z)')

   args = parser.parse_args()

   padmap = readmap()
   pwb = getpwb(padmap,args.sec,args.row)
   if pwb >= 0:
       sca = getsca(args.sec,args.row)
       print('PWB%02d SCA%c'%(pwb,sca))
       #print('SCA',sca[0],'or',sca[1])
