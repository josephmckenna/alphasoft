#! /usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

from FitSTR import get_array, _pol3, _pol31, scipyfit_drift


def _pol30(x,a0,a1,a2,a3):
    return np.where(x<125.,0.,_pol3(x,a0,a1,a2,a3))

def scipyfit_lorentz(lor,td):
    popt, pcov = curve_fit(_pol30,td,lor)
    return popt

def DumpTable(par_drift,par_lor,B=1.,Q=0.3,Vaw=3200.,tag="1"):
    t_step=8.
    t=np.arange(0.,8000.,t_step)
    r=_pol31(t, *par_drift)
    p=_pol30(t, *par_lor)
    with open(f"garfppSTR_B{B:.2f}T_Ar{(1.-Q)*1.e2:.0f}CO2{Q*1.e2:.0f}_CERN_{tag}.dat",'w') as fout:
        fout.write(f"# B = {B:.2f} T, AW = {Vaw:1.0f} V, garfield++ simulation CERN piecewise fit\n")
        fout.write("# t\tr\tphi\n")
        #print(t[0],r[0])
        for ti,ri,pi in zip(t,r,p):
            if ri<108.9: break
            fout.write(f'{ti:.1f}\t{ri:1.3f}\t{pi:1.5f}\n')





if __name__=='__main__':

    # simulation parameters
    B=1.
    Q=0.3
    Vaw=3200.
    Vfw=-110.

    # plotting parameters
    xmax=4500.

    # tagging STR dump
    #tag='chamber_driftRKF'
    tag='polar_driftMC'
    #tag='polar_driftRKF'
    #tag='polarsignal_driftMC'
    #tag='polarsignal_driftRKF'
    
    # get the simulation output
    rad,lor,td=get_array(B,Q,Vaw,Vfw)

    # fitting
    par_drift=scipyfit_drift(rad,td) # piecewise
    par_lorentz=scipyfit_lorentz(lor,td) # piecewise
    
    # SAVE STR
    DumpTable(par_drift,par_lorentz,B,Q,Vaw,tag)

    # define x-axis
    xdata0=np.linspace(0.,xmax,1000)

    # plotting
    plt.subplot(211)
    #plot data
    plt.plot(td,rad,'.b',label='data')
    # plot fit
    plt.plot(xdata0, _pol31(xdata0, *par_drift), 'r',label='scipy fit: m=%1.2f,a0=%1.2f,a1=%1.2f,a2=%1.5f,a3=%1.5f' % tuple(par_drift))

    # cosmetics
    plt.xlabel('t [ns]')
    plt.ylabel('r [mm]')
    plt.xlim(0.,xmax)
    #plt.ylim(109.,190.)
    plt.legend()
    plt.grid(True)

    # plotting
    plt.subplot(212)
    #plot data
    plt.plot(td,lor,'.g',label='data')
    # plot fit
    plt.plot(xdata0, _pol30(xdata0, *par_lorentz), 'y',label='scipy fit: a0=%1.2f,a1=%1.2f,a2=%1.5f,a3=%1.5f' % tuple(par_lorentz))
    max_lor_angle=np.amax(_pol30(xdata0, *par_lorentz))
    # plot aw wires
    pitch=2.*np.pi/256.
    #plt.hlines(np.arange(0.,np.amax(lor),pitch),xmin=0.,xmax=xmax,linestyles='dashed',color='grey',label='AW spacing')
    plt.hlines(np.arange(0.,max_lor_angle,pitch),xmin=0.,xmax=xmax,linestyles='dashed',color='grey',label='AW spacing')

    # cosmetics
    plt.xlabel('t [ns]')
    plt.ylabel('lorentz [rad]')
    plt.xlim(0.,xmax)
    plt.ylim(0.,max_lor_angle*1.1)
    plt.legend()
    plt.grid(axis='x')
    

    # display plot
    fig=plt.gcf()
    fig.set_size_inches(15,10)
    fig.tight_layout()
    fig.savefig(f'plotSTR_B{B:.2f}T_Ar{(1.-Q)*1.e2:.0f}CO2{Q*1.e2:.0f}_CERN_{tag}.pdf',bbox_inches='tight')
    plt.show()
