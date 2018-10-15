#!/usr/bin/python3

# ------------------------------------------------------------------------------------------
# TCP/IP utility
import socket

class NetworkSocket():

    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connected = False

    def connect_socket(self, ip, port, password):
        print("connecting...")
        ### Try to connect
        self.connected = 0
        try:
            self.sock.connect((ip, port))
        except Exception as e:
            print(e)
            self.sock.close()
            return 0

        ### check for correct module
        self.write("\x10")
        d = []
        d = self.read(3)
        if d[0] != 18:
            print("Wrong module type found")
            self.sock.close()
            return 0


        ### Check to see if password is enabled
        self.write("\x7a")
        d = []
        d = self.read(1)
        if d[0] == 0:                          ### Password is enabled
            passwordString = '\x79' + password      ### Put together password command and send it
            self.write(passwordString)
            d = self.read(1)
            if d[0] != 1:                      ### The password was wrong
                print("Wrong password")
                self.sock.close()
                return 0
        return 1


    def write(self, mesg):
        try:
            self.sock.sendall(mesg.encode())
        except Exception as e:
            print("Error writing message")

    def read(self, readnum):
        chunks = []
        bytes_recd = 0
        while bytes_recd < readnum:
            chunk=''
            try:
                chunk = self.sock.recv(min(readnum - bytes_recd, 2048))
            except Exception as e:
                print(e)
            if chunk == '':
                print("Error reading message")
                raise RuntimeError("socket connection broken")
            chunks.extend(chunk)
            bytes_recd = bytes_recd + len(chunk)
        return chunks

    def close_socket(self):
        try:
            self.sock.close()
        except Exception as e:
            print(e)
        print("Disconnected")

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# Network Driver for ETH002

eth002 = NetworkSocket()

#These will be the relays

def connect(ip, port, password):
    return eth002.connect_socket(ip, port, password)

def disconnect():
    eth002.close_socket()

def digital_command(command):

    parts = command.split(" ")     # Seperate the parts of the command
    message = None

    if parts[1] == "on":           # Are we turning the io on or off
        message = '\x20'
    else:
        message = '\x21'

    message += chr(int(parts[0]))  # Which io is it

    message += '\x00'              # 0 pulse time

    eth002.write(message)          # send command and read back responce byte
    eth002.read(1)

def get_states():
    command = '\x24'
    eth002.write(command)          # send command and read back responce byte
    states = eth002.read(1)

    print('Relay states 2->1 : ' + ''.join('{0:02b}'.format(x, 'b') for x in states))

# ------------------------------------------------------------------------------------------
# POWER Cycle the TDC

from sys import version_info
from time import sleep
#from eth002 import *
import os

def tdc_ping():
    hostname = 'tdc01'
    response = os.system("ping -c 1 " + hostname)
    #and then check the response...
    if response == 0:
        print(hostname, 'is up!')
    else:
        print(hostname, 'is down!')
    return response

if __name__ == '__main__':
    
    ip_address = '192.168.1.55'
    port = 17494
    password = ''
    
    wait = 5 # secs
    connect(ip_address, int(port), password)
    get_states()
    print('TDC off')
    digital_command('1 off')
    get_states()
    print('Wait for',wait,'seconds...')
    sleep(wait)
    print('TDC on')
    digital_command('1 on')
    get_states()
    disconnect()
    while tdc_ping():
        print('Wait for',wait,'seconds...')
        sleep(wait)
