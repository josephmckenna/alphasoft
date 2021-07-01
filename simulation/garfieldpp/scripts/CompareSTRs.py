#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate
from os import environ
import math

B=0.0
tmax=4500.
r_cath=109.25
#simdata=[f'{environ["AGRELEASE"]}/ana/strs/garfppSTR_B{B:1.2f}T_Ar70CO230_CERN.dat',f'garfppSTR_B{B:1.2f}T_Ar70CO230_CERN_1.dat',f'garfppSTR_B{-B:1.2f}T_Ar70CO230_CERN_1.dat']
#labels=['garf++CERN','new','map']

simdata=[f'{environ["AGRELEASE"]}/garfppSTR_B1.00T_Ar70CO230_CERN_chamber_driftRKF.dat',
         f'{environ["AGRELEASE"]}/garfppSTR_B1.00T_Ar70CO230_CERN_polar_driftMC.dat',
         f'{environ["AGRELEASE"]}/garfppSTR_B1.00T_Ar70CO230_CERN_polar_driftRKF.dat',
         f'{environ["AGRELEASE"]}/garfppSTR_B1.00T_Ar70CO230_CERN_polarsignal_driftMC.dat',
         f'{environ["AGRELEASE"]}/garfppSTR_B1.00T_Ar70CO230_CERN_polarsignal_driftRKF.dat']
labels=['cart DriftRKF','polar DriftMC','polar DriftRKF','polarsignal driftMC','polarsignal driftRKF']

fig,(ax1,ax2) = plt.subplots(2, sharex=True)

for f in simdata:
    tt,rr,pp = np.loadtxt(f, delimiter='\t', skiprows=2, unpack=True)

    str_1 = interpolate.interp1d(np.flip(rr), np.flip(tt))
    max_drift_time = str_1(r_cath)    

    ax1.plot(tt,rr,'-',label=labels[simdata.index(f)]+f' endpoint: {max_drift_time:1.0f}ns')

    str_2 = interpolate.interp1d(tt,pp)
    max_angle = str_2(max_drift_time) 

    ax2.plot(tt,pp,'-',label=labels[simdata.index(f)]+f' max: {math.degrees(max_angle):1.1f}deg')
    
    print(f'{labels[simdata.index(f)]} endpoint r={r_cath:1.2f}mm: {max_drift_time:1.0f}ns  {math.degrees(max_angle):1.1f}deg')


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
fig.savefig(f'plotCompareSTR.pdf',bbox_inches='tight')
    
plt.show()
