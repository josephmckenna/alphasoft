#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

xmax=np.array([])
basename='LookUp_0.00T_STRR'

runlist=[]

#fname='run.list'
#runlist=open(fname)
for run in runlist:

    '''    
    rawdata='%s%d.dat'%(basename,run)
    #print(rawdata)
    t0, rmin, r0, rmax, phimin, phi0, phimax = np.loadtxt(rawdata, 
                                                          delimiter=' ', 
                                                          skiprows=2, unpack=True)
    xmax = np.append(xmax,max(t0)*1.1)
    plt.plot(t0,r0,'o',label='R%d'%run)
    '''    

    fitdata='%s%d_fit.dat'%(basename,int(run))
    print(fitdata)
    tt,rr,pp = np.loadtxt(fitdata, 
                          delimiter='\t', 
                          skiprows=2, unpack=True)
    xmax = np.append(xmax,max(tt)*1.1)
    plt.plot(tt,rr,'-',label='R%d fit'%int(run))

    '''
    fitdata='./calib/%s%d_fit.dat'%(basename,int(run))
    tt,rr,pp = np.loadtxt(fitdata, 
                          delimiter='\t', 
                          skiprows=2, unpack=True)
    xmax = np.append(xmax,max(tt)*1.1)
    plt.plot(tt,rr,'-',label='R%d calib fit'%int(run))
    '''
#runlist.close()

tt,rr,pp = np.loadtxt('strs/LookUp_0.00T_good.dat', 
                      delimiter='\t', 
                      skiprows=2, unpack=True)
xmax = np.append(xmax,max(tt)*1.1)
plt.plot(tt,rr,'-',label='good')

tt,rr,pp = np.loadtxt('strs/garfppSTR_B0.00T_Ar70CO230_CERN.dat', 
                      delimiter='\t', 
                      skiprows=2, unpack=True)
xmax = np.append(xmax,max(tt)*1.1)
plt.plot(tt,rr,'-',label='garf++ CH')

tt,rr,pp = np.loadtxt('strs/garfppSTR_B0.00T_Ar70CO230_TRIUMF.dat', 
                      delimiter='\t', 
                      skiprows=2, unpack=True)
xmax = np.append(xmax,max(tt)*1.1)
#plt.plot(tt,rr,'-',label='garf++ CA')

plt.xlabel('t [ns]')
plt.ylabel('r [mm]')
#plt.xlim(0.,max(xmax))
plt.xlim(0.,4200.)
plt.ylim(109.,185.)
plt.legend(loc='best')
plt.grid(True)

fig=plt.gcf()
fig.set_size_inches(18.5, 10.5)
fig.tight_layout()

plt.show()
