#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate
from os import environ

B=0.0
tmax=4500.
r_cath=109.25
#simdata=[f'{environ["AGRELEASE"]}/ana/strs/garfppSTR_B{B:1.2f}T_Ar70CO230_CERN.dat',f'garfppSTR_B{B:1.2f}T_Ar70CO230_CERN_1.dat',f'garfppSTR_B{-B:1.2f}T_Ar70CO230_CERN_1.dat']
#labels=['garf++CERN','new','map']
#simdata=['garfppSTR_B1.00Tmap_z800mm_Ar70CO230_1.dat', 'garfppSTR_Bmap_z800mm_Ar70CO230.dat']
#labels=['PlotSTRmap','FitSTR_map']
simdata=[f'{environ["AGRELEASE"]}/ana/strs/garfppSTR_B{B:1.2f}T_Ar70CO230_CERN.dat',f'{environ["AGRELEASE"]}/ana/strs/garfppSTR_B{B:1.2f}T_Ar70CO230_TRIUMF.dat']
labels=['CERN','TRIUMF']

fig,(ax1,ax2) = plt.subplots(2, sharex=True)

for f in simdata:
    tt,rr,pp = np.loadtxt(f, 
                          delimiter='\t', 
                          skiprows=2, unpack=True)
    
    ax1.plot(tt,rr,'-',label=labels[simdata.index(f)])
    ax2.plot(tt,pp,'-',label=labels[simdata.index(f)])
    str_1 = interpolate.interp1d(np.flip(rr), np.flip(tt))
    max_drift_time = str_1(r_cath)
    print(f'{labels[simdata.index(f)]} endpoint r={r_cath:1.2f}mm: {max_drift_time:1.2f}ns')


ax1.set_ylabel('r [mm]')
ax1.set_ylim(109.,185.)

ax1.legend(loc='best')
ax1.grid(True)    

ax2.set_xlabel('t [ns]')
ax2.set_xlim(0.,tmax)

ax2.set_ylabel('lorentz [rad]')

ax2.legend(loc='best')
ax2.grid(True)




fig.set_size_inches(18.5, 9.0)
fig.tight_layout()

    
plt.show()
