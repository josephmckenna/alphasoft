#!/usr/bin/python3

import socket
import sys
import requests
import json
import time

from pythonMidas import setValue, getString

import numpy as np
import matplotlib.pyplot as plt

def SetROperiod1():
    key='/Equipment/CTRL/Settings/PeriodScalers'
    setValue(key,1)

def SetROperiod10():
    key='/Equipment/CTRL/Settings/PeriodScalers'
    setValue(key,10)

def UpdateADC():
    headers={'Content-Type':'application/json','Accept':'application/json'}
    par={'client_name':'fectrl','cmd':'init_adc_all','args':''}
    payload={'method':'jrpc','params':par,'jsonrpc':'2.0','id':0}    
    url='http://localhost:8080?mjsonrpc'
    res=requests.post(url, json=payload, headers=headers).json()
    return res['result']['reply']

def SetADC16threshold(thr):
    key='/Equipment/CTRL/Settings/ADC/adc16_threshold'
    setValue(key,thr)
    UpdateADC()

def ReadADC16threshold():
    key='/Equipment/CTRL/Settings/ADC/adc16_threshold'
    thr=getString( key )
    return int(thr)

def ResetADC16threshold():
    key='/Equipment/CTRL/Settings/ADC/adc16_threshold'
    setValue(key,8192)
    UpdateADC()

def SetADC32threshold(thr):
    key='/Equipment/CTRL/Settings/ADC/adc32_threshold'
    setValue(key,thr)
    UpdateADC()

def ReadADC32threshold():
    key='/Equipment/CTRL/Settings/ADC/adc32_threshold'
    thr=getString( key )
    return int(thr)

def ResetADC32threshold():
    key='/Equipment/CTRL/Settings/ADC/adc32_threshold'
    setValue(key,-8192)
    UpdateADC()

def ReadRates16():
    key='/Equipment/CTRL/Variables/scalers_rate'
    rates=getString( key ).split()
    rates=[float(r) for r in rates[24:32]]
    return rates

def ReadRates32():
    key='/Equipment/CTRL/Variables/scalers_rate'
    rates=getString( key ).split()
    rates=[float(r) for r in rates[48:64]]
    return rates

def AvgRates(adc):
    if adc == 'A16':
        rates = ReadRates16()
    elif adc == 'A32':
        rates = ReadRates32()
    else:
        print('Unknown adc type',adc)
    return sum(rates)/float(len(rates))

def ReadMLUrate():
    key='/Equipment/CTRL/Variables/scalers_rate'
    rates=getString( key ).split()
    return float(rates[67])

def Read4orMore():
    key='/Equipment/CTRL/Variables/scalers_rate'
    rates=getString( key ).split()
    return float(rates[11])

def TimeAverageRate(scaler):
    time_avg=0.0
    Nmeas=10
    for r in range(Nmeas):
        if scaler == 'A16':
            time_avg += AvgRates('A16')
        elif scaler == 'A32':
            time_avg += AvgRates('A32')
        elif scaler == 'mlu':
            time_avg += ReadMLUrate()
        elif scaler == '4':
            time_avg += Read4orMore()
        else:
            print("don't know this scaler", scaler)
        time.sleep(2)
    return time_avg/float(Nmeas)

def Verbose():
    print('==================================================')
    print('ADC16 threshold:',ReadADC16threshold(),'ADC16 avg. rates: %1.0f Hz'%AvgRates('A16'))
    print('ADC32 threshold:',ReadADC32threshold(),'ADC32 avg. rates: %1.0f Hz'%AvgRates('A32'))
    print('==================================================')


def plot(fname):
    thr,raw,trig=np.loadtxt(fname,
                           delimiter='\t', unpack=True)

    plot_title = 'ADC Threshold Scan'
    plot_label = 'Trigger Rate'
    if 'AW' in fname:
        plot_title += ' TPC AW'
        plot_label += ' MLU'
        plt.gca().invert_xaxis()
    elif 'BV' in fname:
        plot_title += 'Barrel Veto'
        plot_label += ' 4 or more'

    plt.plot(thr,raw,'.',label='ADC time avg.')
    plt.plot(thr,trig,'.',label=plot_label)
    plt.title(plot_title)
    plt.xlabel('Threshold: ADC counts above pedestal')
    plt.ylabel('Counting Rate in Hz')
    plt.grid(True)
    plt.legend(loc='best')

    fig=plt.gcf()
    fig.set_size_inches(18.5, 10.5)
    fig.tight_layout()
    fig.savefig(fname[:-4]+'.png')
    plt.show()


def BVscan():
    thr_list = [780, 820, 860, 900, 940, 980, 1050, 1100, 
                1200, 1400, 1600, 1800, 2000, 2500, 3000, 
                4000, 5000, 6000, 8000, 8192, 11000, 15000, 
                20000, 30000]

    print('Start...')

    fname=__file__[:-3]+'_BV_'+time.strftime("%Y%b%d_%H%M", time.localtime())+'.dat'
    print(fname)

    f=open(fname, 'w')
    for thr in thr_list:
        print('set:',thr, end=" ")
        
        # set the new threshold and wait for plateau
        SetADC16threshold(thr)
        time.sleep(10)
        
        # read the trigger rates and write them to file
        a16thr=ReadADC16threshold()
        a16rate=TimeAverageRate('A16')
        trig=Read4orMore()
            
        f.write('%d\t%1.1f\t%1.1f\n'%(a16thr,a16rate,trig))
        print('  read: %d    ADC16 time avg.rate: %1.1f Hz   4 or more rate: %1.1f Hz'%(a16thr,a16rate,trig))

    # reset the threshold
    ResetADC16threshold()
        
    f.close()
    print('Finished!')
    return fname

def AWscan():
    thr_list = [-780, -820, -860, -900, -940, -980, -1050, -1100,
                -1200, -1400, -1600, -1800, -2000, -2500, -3000,
                -4000, -5000, -6000, -8000, -8192, -11000, -15000,
                -20000, -30000]

    # for high counting rate, fectrl scaler readout period 
    # has to be changed from 10 sec to 1 sec
    SetROperiod1()

    print('Start...')

    fname=__file__[:-3]+'_AW_'+time.strftime("%Y%b%d_%H%M", time.localtime())+'.dat'
    print(fname)

    f=open(fname, 'w')
    for thr in thr_list:
        print('set:',thr, end=" ")
        
        # set the new threshold and wait for plateau
        SetADC32threshold(thr)
        time.sleep(10)
        
        # read the trigger rates and write them to file
        a32thr=ReadADC32threshold()
        a32rate=TimeAverageRate('A32')
        trig=ReadMLUrate()
            
        f.write('%d\t%1.1f\t%1.1f\n'%(a32thr,a32rate,trig))
        print('  read: %d    ADC32 time avg.rate: %1.1f Hz   mlu rate: %1.1f Hz'%(a32thr,a32rate,trig))

    # reset the scaler readout period
    SetROperiod10()
    # reset the threshold
    ResetADC32threshold()
        
    f.close()
    print('Finished!')
    return fname
    

################################################################################

if __name__ == "__main__":
    if socket.gethostname() == 'alphagdaq.cern.ch':
        print('Good! We are on', socket.gethostname())
    else:
        sys.exit('Wrong host %s'%socket.gethostname())

    Verbose()

    adc='a32'
    if len(sys.argv) == 2:
        adc = sys.argv[1]

    print('Scanning',adc)

    if adc == 'aw' or adc == 'AW' or adc == 'FMC' or adc == 'FMC32' or adc == 'fmc' or adc == 'fmc32' or adc == 'a32':
        fname = AWscan()
    elif adc == 'a16' or adc == 'A16' or adc == 'bv' or adc == 'BV':
        fname = BVscan()
    else:
        sys.exit('Unknown adc %s'%adc)

    plot(fname)

    Verbose()


