#! /usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

from FitSTR import get_array, scipyfit_lorentz
from FitSTR import scipyfit_drift

def MapTable(par_drift,par_lor,B=-1.,Q=0.3,Vaw=3200.,z=0.0):
    t_step=8.
    t=np.arange(0.,8000.,t_step)
    r=_pol31(t, *par_drift)
    p=_pol30(t, *par_lor)
    with open(f"garfppSTR_Bmap_z{z*10.:.0f}mm_Ar{(1.-Q)*1.e2:.0f}CO2{Q*1.e2:.0f}.dat",'w') as fout:
        fout.write(f"# B map @ {fabs(B):.2f} T, AW = {Vaw:1.0f} V, garfield++ simulation CERN piecewise fit\n")
        fout.write("# t\tr\tphi\n")
        #print(t[0],r[0])
        for ti,ri,pi in zip(t,r,p):
            if ri<108.9: break
            fout.write(f'{ti:.1f}\t{ri:1.3f}\t{pi:1.5f}\n')


if __name__=='__main__':

    # simulation parameters
    B=-1.
    Q=0.3
    Vaw=3200.
    Vfw=-110.

    # acquire all the z positions
    z=np.array([0.0,20.0,40.0,60.0]) # cm
    z=np.append(z, np.arange(80.,117.,0.5))
    for zed in z:
        # get the simulation output
        rad,lor,td=get_array(B,Q,Vaw,Vfw,zed)
        # fitting
        par_drift=scipyfit_drift(rad,td) # piecewise
        par_lorentz=scipyfit_lorentz(lor,td) # piecewise
        # SAVE STR
        MapTable(par_drift,par_lorentz,B,Q,Vaw,zed)
