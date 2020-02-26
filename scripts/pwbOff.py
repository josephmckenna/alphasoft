#!/usr/bin/env python

import sys
import socket
from powerCyclePWBs import turnoff

if socket.gethostname() == 'alphagdaq.cern.ch':
    print 'Good! We are on', socket.gethostname()
else:
    sys.exit('Wrong host %s'%socket.gethostname())

for c in range(0,7,2):
    # print "turnoff(",c,")"
    turnoff(c)
