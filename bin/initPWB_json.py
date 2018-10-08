#!/usr/bin/env python

import socket
import sys
import requests
import json

def jsonapp(cmd,address):
    headers={'Content-Type':'application/json','Accept':'application/json'}
    par={'client_name':'fectrl','cmd':cmd,'args':address}
    payload={'method':'jrpc','params':par,'jsonrpc':'2.0','id':0}
    
    url='http://localhost:8080?mjsonrpc'
    print 'request', cmd, address
    res=requests.post(url, json=payload, headers=headers).json()
    #print address, res['result']['reply']
    return res['result']['reply']

def reboot(address):
    cmd='reboot_pwb'
    out=jsonapp(cmd,address)
    print cmd, address, out


def init(address):
    cmd='init_pwb'
    out=jsonapp(cmd,address)
    print cmd, address, out



if __name__ == '__main__':
    if socket.gethostname() == 'alphagdaq.triumf.ca':
        print 'Good! We are on', socket.gethostname()
    else:
        sys.exit('Wrong host %s'%socket.gethostname())

    if len(sys.argv) != 2:
        print "Need PWB address"
        print "Usage: initPWB_json.py pwbXX"
        sys.exit()

    #reboot(sys.argv[1])
    init(sys.argv[1])
