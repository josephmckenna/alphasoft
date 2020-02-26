#!/usr/bin/env python

import socket
import sys
import requests
import json

from initPWB_json import init

import time

################################################################################
def turn(s,col):
    url='http://localhost:8080?mjsonrpc'
    cmd='turn_'+s
    headers={'Content-Type':'application/json','Accept':'application/json'}

    col2ch={ 0:['0','2'], 1:['0','2'], 
             2:['1','3'], 3:['1','3'], 
             4:['4','6'], 5:['4','6'], 
             6:['5','7'], 7:['5','7'] }

    cha=col2ch[col][0]
    chb=col2ch[col][1]

    col1=-1
    if col%2 == 0:
        col1=col+1
    else:
        col1=col-1
    
    par={'client_name':'fewiener_lvps01','cmd':cmd,'args':cha}
    payload={'method':'jrpc','params':par,'jsonrpc':'2.0','id': 0}
    print 'request', cmd, 'columns:', col, 'and', col1, 'channel: ', cha
    resa=requests.post(url, json=payload, headers=headers).json()
    print 'col:', col, 'and', col1, 'ch:', cha, resa['result']['reply']

    par={'client_name':'fewiener_lvps01','cmd':cmd,'args':chb}
    payload={'method':'jrpc','params':par,'jsonrpc':'2.0','id': 0}
    print 'request', cmd, col, 'channels: ', cha
    print 'request', cmd, 'columns:', col, 'and', col1, 'channel: ', chb
    resb=requests.post(url, json=payload, headers=headers).json()
    print 'col:', col, 'and', col1, 'ch:', chb, resa['result']['reply']

    return resa['result']['reply'] and resb['result']['reply']

################################################################################

def turnoff(col):
    return turn('off',col)

def turnon(col):
    return turn('on',col)

def turnalloff():
    for i in range(0,8,2):
        if turnoff(i):
            print '.',
        else:
            print 'x',
    print ''

def turnallon():
    for i in range(0,8,2):
        if turnon(i):
            print '.',
        else:
            print 'x',
    print ''

################################################################################
import pythonMidas
from initPWB_json import init
import os

def initAllPWBs():
    key='/Equipment/CTRL/Settings/PWB/modules'
    pwbs=pythonMidas.getString( key ).split()

    for ipwb in pwbs:
        init(ipwb)


def logger(path,id):
    key='/Equipment/CTRL/Settings/PWB/modules'
    pwbs=pythonMidas.getString( key ).split()

    key='/Equipment/CTRL/Variables/pwb_state'
    stat=pythonMidas.getString( key ).split()

    s=dict(zip(pwbs, stat))

    fname='%s/cycle%03d.log'%(path,id)
    f=open(fname, 'w')
    for ipwb in sorted(s.keys()):
        f.write(ipwb+'\t'+s[ipwb]+'\n')
    f.close()



################################################################################

if __name__ == '__main__':
    if socket.gethostname() == 'alphagdaq.triumf.ca':
        print 'Good! We are on', socket.gethostname()
    else:
        sys.exit('Wrong host %s'%socket.gethostname())

    path=os.environ['HOME']+'/andrea/PCPWB'+time.strftime("%Y%b%d%H%M", time.localtime())
    try:
        os.mkdir(path)
        print path
    except OSError:
        print 'Path', path, 'exists'

    Ncycles=100
    #Ncycles=50
    #Ncycles=10
    #Ncycles=1
    for cycle in range(1,1+Ncycles):
        print '##### Cycle', cycle, '/', Ncycles, '#####'
        turnalloff()
        print 'sleep 30s...'
        time.sleep(30)
        turnallon()
        '''
        print 'sleep 5m...'
        time.sleep(300)
        
        initAllPWBs()
        print 'sleep 30s...'
        time.sleep(30)
        '''
        print 'sleep 3m...'
        time.sleep(180)
        print 'logging'
        logger(path,cycle)
