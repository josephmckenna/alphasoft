#!/usr/bin/env python

import sys
import socket
import requests
import json
import os
import time
from pythonMidas import getValue

def hvOff():
    url='http://localhost:8080?mjsonrpc'
    cmd='turn_off'
    args='all'
    headers={'Content-Type':'application/json','Accept':'application/json'}
    
    par={'client_name':'fecaen_hvps01','cmd':cmd,'args':args}
    payload={'method':'jrpc','params':par,'jsonrpc':'2.0','id': 0}
    resa=requests.post(url, json=payload, headers=headers).json()
    print 'turn_off reply:', resa['result']['reply']

def get_lock(process_name):
    # Without holding a reference to our socket somewhere it gets garbage
    # collected when the function exits
    get_lock._lock_socket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)

    try:
        get_lock._lock_socket.bind('\0' + process_name)
        print 'I got the lock'
        return True
    except socket.error:
        print 'lock exists'
        return False

if socket.gethostname() == 'alphagdaq.cern.ch':
    print 'Good! We are on', socket.gethostname()
else:
    sys.exit('Wrong host %s'%socket.gethostname())

interval = getValue("/Alarms/Alarms/Gas Flow/Check Interval")

if(get_lock('midas_hv_interlock')):
    time.sleep(interval+30)
else:
    print 'There is one of me already'
    print 'Turning off HV'
    hvOff()

