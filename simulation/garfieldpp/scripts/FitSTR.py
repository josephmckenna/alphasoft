#! /usr/bin/python3
from math import sqrt, pi, atan2, radians
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from os import environ, path
from sys import exit

def get_array(B=0.,Q=0.3,Vaw=3200.,Vfw=-110.,z=0.0):
    rad=np.array([])
    lor=np.array([])
    td=np.array([])
    
    Nfiles=0
    for angle in range(1,360,3):
        if angle % 90 == 0: continue
        phi=radians(float(angle))
        fname='%s/chamber_drift/drift_tables/Drift_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode-4000V_Anode%.0fV_Field%.0fV_B%1.2fT.dat' % (environ['GARFIELDPP'],phi,z,(1.-Q)*1.e2,Q*1.e2,Vaw,Vfw,B)
        if not path.isfile(fname): 
            print(fname)
            continue
        x,y,w,time,gain = np.loadtxt(fname,delimiter='\t', skiprows=0, unpack=True)
        r=np.sqrt(np.square(x)+np.square(y))
        r*=10. # cm -> mm
        rad=np.append(rad,r)

        dphi=w-phi # wrap around
        dphi=np.where(dphi<0.,dphi+2.*pi,dphi)
        dphi=np.where(dphi>=pi,dphi-2.*pi,dphi)
        lor=np.append(lor,dphi)

        td=np.append(td,time)

        Nfiles+=1
    print(f'Number of Files: {Nfiles} z={z}cm B={B}T')
    return rad,lor,td

def polyfit_drift(rad,td,tFW):
    par3=np.polyfit(td[td>tFW],rad[td>tFW],3)
    pol3=np.poly1d(par3)
    return pol3

def polyfit_prop(rad,td,tFW):
    par1=np.polyfit(td[(td<tFW)&(rad<182.)],rad[(td<tFW)&(rad<182.)],1)
    pol1=np.poly1d(par1)
    return pol1

def _pol1(x,q,m):
    return x*m+q
 
def _pol3(x,a0,a1,a2,a3):
    return np.power(x,3)*a3+np.square(x)*a2+x*a1+a0

def _pol31(x,m,a0,a1,a2,a3):
    return np.where(x<125.,_pol1(x,182.,m),_pol3(x,a0,a1,a2,a3))

def scipyfit_drift(rad,td,tFW):
    popt, pcov = curve_fit(_pol3,td[td>tFW],rad[td>tFW])
    return popt

def scipyfit_drift(rad,td):
    popt, pcov = curve_fit(_pol31,td[rad<=182.],rad[rad<=182.])
    return popt


def dumpSTR(pol_prop,pol_drift,B=0.,Q=0.3,Vaw=3200.,tFW=150.):
    print('polyfit')
    phi=0.0
    t_step=8.
    with open(f"garfppSTR_B{B:.2f}T_Ar{(1.-Q)*1.e2:.0f}CO2{Q*1.e2:.0f}_CERN_1.dat",'w') as fout:
        fout.write(f"# B = {B:.2f} T, AW = {Vaw:1.0f}, garfield++ simulation CERN piecewise fit\n")
        fout.write("# t\tr\tphi\n")
        r=182.0
        t=0.0
        fout.write(f'{t:.1f}\t{r:1.3f}\t{phi:1.2f}\n')
        t+=t_step
        while r>109.:
            if t<tFW: r=pol_prop(t)
            else: r=pol_drift(t)
            fout.write(f'{t:.1f}\t{r:1.3f}\t{phi:1.2f}\n')
            t+=t_step

def dumpSTR(pars31,B=0.,Q=0.3,Vaw=3200.):
    print("piecewise fit")
    phi=0.0
    t_step=8.
    t=np.arange(0.,8000.,t_step)
    r=_pol31(t, *pars31)
    with open(f"garfppSTR_B{B:.2f}T_Ar{(1.-Q)*1.e2:.0f}CO2{Q*1.e2:.0f}_CERN_1.dat",'w') as fout:
        fout.write(f"# B = {B:.2f} T, AW = {Vaw:1.0f} V, garfield++ simulation CERN piecewise fit\n")
        fout.write("# t\tr\tphi\n")
        #print(t[0],r[0])
        for ti,ri in zip(t,r):
            if ri<109.: break
            fout.write(f'{ti:.1f}\t{ri:1.3f}\t{phi:1.2f}\n')


def endpointSTR(pars,r):
    coeff=[c for c in reversed(pars[1:])]
    coeff[-1]-=r
    roots=np.roots(coeff)
    return np.asscalar( np.real(roots[np.isreal(roots)]) )

            

if __name__=='__main__':

    # simulation parameters
    B=0.
    Q=0.3
    Vaw=3200.
    Vfw=-110.

    # fitting parameters
    xmax=4300.
    tFW=125.
    

    # get the simulation output
    rad,_,td=get_array(B,Q,Vaw,Vfw)


    # fitting
    # numpy fitting with polyfit
    pol_drift=polyfit_drift(rad,td,tFW)
    pol_prop=polyfit_prop(rad,td,tFW)
    # scipy fitting with user-def
    # par_drift=scipyfit_drift(rad,td,tFW)
    par_drift=scipyfit_drift(rad,td) # piecewise


    # SAVE STR
    #dumpSTR(pol_prop,pol_drift,B,Q,Vaw,tFW)
    dumpSTR(par_drift,B,Q,Vaw)

    r_cath=109.25
    td_max=endpointSTR(par_drift,r_cath)
    print(f'Max Drift time at r={r_cath}mm is {td_max:1.1f}ns')
    #exit(0)

 
    # define x-axis
    xdata0=np.linspace(0.,xmax,1000.)
    xdata1=np.linspace(0.,tFW,10.)
    xdata2=np.linspace(tFW,xmax,100.)
    
    '''
    # plotting
    plt.subplot(211)
    #plot data
    plt.plot(td,rad,'.b',label='data')
    #plot fit
    plt.plot(xdata2,pol_drift(xdata2),'-k',label='poly fit: a3=%1.5f,a2=%1.5f,a1=%1.2f,a0=%1.2f' % tuple(pol_drift))
    #plt.plot(xdata2,pol_drift(xdata2),'-k',label='poly fit: a2=%1.5f,a1=%1.2f,a0=%1.2f' % tuple(pol_drift))
    plt.plot(xdata1,pol_prop(xdata1),'--k',label='fit')
    # cosmetics
    plt.xlabel('t [ns]')
    plt.ylabel('r [mm]')
    plt.xlim(0.,xmax)
    plt.ylim(109.,190.)
    plt.legend()
    plt.grid(True)
    


    plt.subplot(212)
    '''
    # plot data: piece-wise
    plt.plot(td[td>tFW],rad[td>tFW],'.r',label='data drift')
    plt.plot(td[td<tFW],rad[td<tFW],'.g',label='data pc')
    # plot fit
    plt.plot(xdata0, _pol31(xdata0, *par_drift), '--y',label='scipy fit: m=%1.2f,a0=%1.2f,a1=%1.2e\na2=%1.2e,a3=%1.2e' % tuple(par_drift))

    # plot endpoint
    plt.vlines(td_max,r_cath,190.,label=f'Max Drift Time: {td_max:1.1f}ns')

    # cosmetics
    plt.xlabel('t [ns]')
    plt.ylabel('r [mm]')
    plt.xlim(0.,xmax)
    plt.ylim(109.,190.)
    plt.legend()
    plt.grid(True)

    # display plot
    fig=plt.gcf()
    fig.set_size_inches(15,8)
    fig.tight_layout()
    fig.savefig(f"plotSTR_B{B:.2f}T_Ar{(1.-Q)*1.e2:.0f}CO2{Q*1.e2:.0f}_CERN.pdf", bbox_inches='tight')    
    plt.show()
