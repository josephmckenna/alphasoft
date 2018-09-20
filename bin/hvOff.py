#!/usr/bin/env python

import sys
import socket
import requests
import json

if socket.gethostname() == 'alphagdaq.cern.ch':
    print 'Good! We are on', socket.gethostname()
else:
    sys.exit('Wrong host %s'%socket.gethostname())

url='http://localhost:8080?mjsonrpc'
cmd='turn_off'
args='all'
headers={'Content-Type':'application/json','Accept':'application/json'}

par={'client_name':'fecaen_hvps01','cmd':cmd,'args':args}
payload={'method':'jrpc','params':par,'jsonrpc':'2.0','id': 0}
resa=requests.post(url, json=payload, headers=headers).json()
print 'turn_off reply:', resa['result']['reply']
