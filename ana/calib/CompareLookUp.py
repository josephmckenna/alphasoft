#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import argparse
from os import environ


def plot(runlist,B,raw=False,tmax=4200.):

    simdata='%s/ana/strs/garfppSTR_B%1.2fT_Ar70CO230_CERN.dat'%(environ['AGRELEASE'],B)
    print(simdata)
    tt,rr,pp = np.loadtxt(simdata, 
                          delimiter='\t', 
                          skiprows=2, unpack=True)
    plt.plot(tt,rr,'-',label='garf++')

    basename='%s/ana/LookUp_%1.2fT_STRR'%(environ['AGRELEASE'],B)
    for run in runlist:
        
        fitdata='%s%d_fit.dat'%(basename,int(run))
        print(fitdata)
        tt,rr,pp = np.loadtxt(fitdata, 
                              delimiter='\t', 
                              skiprows=2, unpack=True)
        plt.plot(tt,rr,'-',label='R%d fit'%int(run))

        if raw: 
            rawdata='%s%d.dat'%(basename,run)
            print(rawdata)
            t0, rmin, r0, rmax, phimin, phi0, phimax = np.loadtxt(rawdata, delimiter=' ', skiprows=2, unpack=True)
            plt.plot(t0,r0,'o',label='R%d'%run)

    plt.xlabel('t [ns]')
    plt.ylabel('r [mm]')

    plt.xlim(0.,tmax)
    plt.ylim(109.,185.)
    plt.legend(loc='best')
    plt.grid(True)

    fig=plt.gcf()
    fig.set_size_inches(18.5, 9.0)
    fig.tight_layout()
    
    plt.show()

if __name__=='__main__':

    parser = argparse.ArgumentParser(description='Plot data-driven and simulated STRs')

    parser.add_argument('-B', '--MagneticField', type=float, 
                        choices=[0.0, 1.0], default=1.0,
                        help='Magnetic Field in Tesla')

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-r','--run', type=int,
                       nargs='*',
                       help='Run number to plot')
    group.add_argument('-l','--runlist',
                       type=argparse.FileType('r', encoding='UTF-8'),
                       help='List of run numbers to plot')

    parser.add_argument("-w", "--raw", action="store_true",
                        help="Plot raw data")

    parser.add_argument('-t', '--max_time', type=float, 
                        default=4500.,
                        help='Maximum time to display in ns')

    args = parser.parse_args()
    
    if args.run == None:
        runlist=args.runlist
        print('reading from list')
    else:
        runlist=args.run

    plot(runlist,args.MagneticField,raw=args.raw,tmax=args.max_time)
