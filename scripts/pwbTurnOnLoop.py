#!/usr/bin/env python

from powerCyclePWBs import turnon,turnoff,turnallon

import pythonMidas
import time
import socket
import sys

if socket.gethostname() == 'alphagdaq.cern.ch':
    print 'Good! We are on', socket.gethostname()
else:
    sys.exit('Wrong host %s'%socket.gethostname())

def getStat():
    key='/Equipment/CTRL/Variables/pwb_state'
    stat=pythonMidas.getString( key ).split()
    return stat

key='/Equipment/CTRL/Settings/PWB/modules'
pwbs=pythonMidas.getString( key ).split()

turnallon()

for i in range(10):
    print 'sleep 90s...'
    time.sleep(90)

    stat = getStat()

    try:
        idx=0
        #dead = [ idx=stat[idx:].index("3") while idx < len(stat)]
        dead = [index for index in range(len(stat)) if stat[index] == '3']
    except ValueError:
        print "SUCCESS!!!"
        break
    
    print dead
    cols = set([ d/8 if (d/8)%2==0 else d/8-1 for d in dead ])

    for d in dead:
        col = d/8
        print "dead: ", pwbs[d], "column ", col

    if(len(cols)==0):
        print "All PWBs reporting back OK."
        break

    for c in cols:
        turnoff(c)
        time.sleep(10)
        turnon(c)
