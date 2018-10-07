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

def updateADC16():
    headers={'Content-Type':'application/json','Accept':'application/json'}
    par={'client_name':'fectrl','cmd':'init_adc_all','args':''}
    payload={'method':'jrpc','params':par,'jsonrpc':'2.0','id':0}    
    url='http://localhost:8080?mjsonrpc'
    res=requests.post(url, json=payload, headers=headers).json()
    return res['result']['reply']

def SetADC16threshold(thr):
    key='/Equipment/CTRL/Settings/ADC/adc16_threshold'
    setValue(key,thr)
    updateADC16()

def ReadADC16threshold():
    key='/Equipment/CTRL/Settings/ADC/adc16_threshold'
    thr=getString( key )
    return int(thr)

def ResetADC16threshold():
    key='/Equipment/CTRL/Settings/ADC/adc16_threshold'
    setValue(key,8192)
    updateADC16()

def ReadRates16():
    key='/Equipment/CTRL/Variables/scalers_rate'
    rates=getString( key ).split()
    rates=[float(r) for r in rates[24:32]]
    return rates

def AvgRates16():
    rates=ReadRates16()
    return sum(rates)/float(len(rates))

def ReadMLUrate():
    key='/Equipment/CTRL/Variables/scalers_rate'
    rates=getString( key ).split()
    return float(rates[67])

def Read4orMore():
    key='/Equipment/CTRL/Variables/scalers_rate'
    rates=getString( key ).split()
    return float(rates[11])

def TimeAverageRate():
    time_avg=0.0
    Nmeas=10
    for r in range(Nmeas):
        time_avg+=AvgRates16()
        time.sleep(2)
    return time_avg/float(Nmeas)

def Verbose():
    print('ADC16 threshold: ',ReadADC16threshold(),'ADC16 avg. rates: %1.0f Hz'%AvgRates16())

def plot(fname):
    thr,a16,mlu=np.loadtxt(fname,
                           delimiter='\t', unpack=True)

    plt.semilogy(thr,a16,'.',label='ADC avg.')
    #plt.semilogy(thr,mlu,'.',label='MLU')
    plt.semilogy(thr,mlu,'.',label='4 or more')
    plt.title('A-16 Threshold Scan')
    plt.xlabel('ADC counts above pedestal')
    plt.ylabel('counting rate in Hz')
    plt.grid(True)
    plt.legend(loc='best')

    fig=plt.gcf()
    fig.set_size_inches(18.5, 10.5)
    fig.tight_layout()
    fig.savefig(fname[:-4]+'.png')
    plt.show()
    

################################################################################

if __name__ == "__main__":
    if socket.gethostname() == 'alphagdaq.cern.ch':
        print('Good! We are on', socket.gethostname())
    else:
        sys.exit('Wrong host %s'%socket.gethostname())

    fname=__file__[:-3]+'_'+time.strftime("%Y%b%d_%H%M", time.localtime())+'.dat'
    print(fname)

    Verbose()
    test=False
    
    if test:
        thr=1000
        SetADC16threshold(thr) 
        time.sleep(30)
        Verbose()

        ResetADC16threshold()
        time.sleep(20)
        Verbose()
    else:
        # https://daq.triumf.ca/elog-alphag/alphag/1268
        #thr_list = [780, 800, 820, 840, 860, 880, 900, 920, 940, 960, 980, 1000, 
        #            1050, 1100, 1150, 1200, 1400, 1600, 1800, 2000, 2500, 3000, 
        #            4000, 6000, 8192, 12000, 16000, 20000, 32760] 

        thr_list = [780, 820, 860, 900, 940, 980, 1050, 1100, 
                    1200, 1400, 1600, 1800, 2000, 2500, 3000, 
                    4000, 5000, 6000, 8000, 8192, 11000, 15000, 
                    20000, 30000] 
        
        f=open(fname, 'w')
        for thr in thr_list:
            print('set:',thr, end=" ")
            
            # for high counting rate, fectrl scaler readout period 
            # has to be changed from 10 sec to 1 sec
            if thr < 2500:
                SetROperiod1()
                
            # set the new threshold and wait for plateau
            SetADC16threshold(thr)
            #time.sleep(30)
            time.sleep(10)
        
            # read the trigger rates and write them to file
            a16t=ReadADC16threshold()
            a16r=AvgRates16()
            a16r_time=TimeAverageRate()

            #mr=ReadMLUrate()
            mr=Read4orMore()
            #f.write('%d\t%1.1f\t%1.1f\n'%(a16t,a16r,mr))
            f.write('%d\t%1.1f\t%1.1f\n'%(a16t,a16r_time,mr))
            #print('  read: %d    ADC16 avg.rate: %1.1f Hz    MLU rate: %1.1f Hz'%(a16t,a16r,mr))
            print('  read: %d    ADC16 avg.rate: %1.1f Hz   Time Avg.: %1.1f Hz    4 or more rate: %1.1f Hz'%(a16t,a16r,a16r_time,mr))

            # reset the scaler readout period
            if thr < 2500:
                SetROperiod10()

        # reset the threshold
        ResetADC16threshold()
        
        f.close()
        #plot!
        plot(fname)
        Verbose()
