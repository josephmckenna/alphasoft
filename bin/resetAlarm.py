#!/usr/bin/env python

import sys
import socket
import requests
import json
from pythonMidas import (getValue, setValue, getString)

def resetAlarm(alid):
    setValue("/Alarms/Alarms/%s/Triggered" % alid, 0)
    setValue("/Alarms/Alarms/%s/Time triggered first" % alid, 0)
    setValue("/Alarms/Alarms/%s/Time triggered last" % alid, 0)
    setValue("/Alarms/Alarms/%s/Checked last" % alid, 0)
    alclass = getString("/Alarms/Alarms/%s/Alarm Class" % alid)
    setValue("/Alarms/Classes/%s/Execute Last" % alclass, 0)

if socket.gethostname() == 'alphagdaq.cern.ch':
    print 'Good! We are on', socket.gethostname()
else:
    sys.exit('Wrong host %s'%socket.gethostname())

alarmname = sys.argv[2]

s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.setblocking(0)

process_name = 'alarmresetter'

message = 'STAY ALIVE!!!'

alclass = getString("/Alarms/Alarms/%s/Alarm Class" % alarmname)
print 'ALARM CLASS: ', alclass
interval = getValue("/Alarms/Classes/%s/Execute interval" % alclass)

try:
    s.connect('\0' + process_name)
    print 'Connected!'
    s.sendall(message)
    sys.exit()
except socket.error:
    print 'Not Connected!'

    try:
        print 'Trying bind'
        s.bind('\0' + process_name)
        print 'Bound!'
        go_on = 1
        while(go_on):
            go_on = 0
            try:
                s.settimeout(3.*interval)
                s.listen(1)
                conn, addr = s.accept()
                while 1:
                    data = conn.recv(1024)
                    if not data: break
                    if(data == message):
                        go_on = 1

            except socket.timeout:
                print 'Timeout!'
    except socket.error as msg:
        s.close()
        print 'Fail!', msg
    
s.close()

resetAlarm(alarmname)
