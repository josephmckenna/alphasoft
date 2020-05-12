#!/usr/bin/python3

import requests
import sys
import time

url='http://sipmps/'

r=requests.get(url)
#print(r.text)

url+='control.cgi'

if str(sys.argv[1])=="read":
    data={'cmd':'VO?'}
    r = requests.post(url, data)
    print("Voltage =" ,r.text[r.text.find('Reply')+21:r.text.find('</PRE></TD></TR>')])
    data={'cmd':'IO?'}
    r = requests.post(url, data)
    print("Current =" ,r.text[r.text.find('Reply')+21:r.text.find('</PRE></TD></TR>')])
elif str(sys.argv[1])=="set":
    data={'cmd': 'VV '+ str(sys.argv[2])}
    r = requests.post(url, data)
    time.sleep(1)
    data={'cmd':'VO?'}
    r = requests.post(url, data)
    print("Volt =" ,r.text[r.text.find('Reply')+21:r.text.find('</PRE></TD></TR>')])
    data={'cmd':'IO?'}
    r = requests.post(url, data)
    print("Current =" ,r.text[r.text.find('Reply')+21:r.text.find('</PRE></TD></TR>')])

elif str(sys.argv[1])=="output":
    if str(sys.argv[2])=="on":
        data={'cmd':'OP 1'}
        r = requests.post(url, data)
        time.sleep(2)
        data={'cmd':'VO?'}
        r = requests.post(url, data)
        print("Voltage =" ,r.text[r.text.find('Reply')+21:r.text.find('</PRE></TD></TR>')])
    elif str(sys.argv[2])=="off":
         data={'cmd':'OP 0'}
         r = requests.post(url, data)
         time.sleep(2)
         data={'cmd':'VO?'}
         r = requests.post(url, data)
         print("Voltage =" ,r.text[r.text.find('Reply')+21:r.text.find('</PRE></TD></TR>')])

#data={'cmd': 'VV 5'}
#r = requests.post(url, data)
#print(r)

#data={'cmd':'V?'}
#r = requests.post(url, data)
#print(r.text.find('Reply'))
#debut=r.text.find('Reply')+21
#fin=r.text.find('</PRE></TD></TR>')
#print("Voltage =" ,r.text[r.text.find('Reply')+22:r.text.find('</PRE></TD></TR>')], "Volt")


#data={'cmd':'buzz'}
#r = requests.post(url, data)
#print(r)

