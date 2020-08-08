#!/usr/bin/python3

from math import sqrt, pi, fabs, atan2, radians
import numpy as np
import matplotlib.pyplot as plt
import scipy.interpolate
from scipy.optimize import curve_fit

import argparse

aw_pitch=2.0*pi/256.


def ReadIn(pathto,Zed=0.0,Vaw=3200.0,Vfw=-99,B=-1.0):
    xxx=np.array([])
    yyy=np.array([])
    ttt=np.array([])
    lor=np.array([])
    rrr=np.array([])
    Nfiles=0
    for angle in range(0,360,6):
        if (angle%90)==0: continue
        phi=radians(float(angle))

        fname=f'{pathto}/drift_tables/Drift_phi{phi:.4f}_Z{Zed:.1f}cm_Ar70-CO230_Cathode-4000V_Anode{Vaw:.0f}V_Field{Vfw:.0f}V_B{B:.2f}T.dat'

        try:
            #print(fname)
            x,y,w,t,gain=np.loadtxt(fname,delimiter='\t', skiprows=0, unpack=True)
            r=np.sqrt(np.square(x)+np.square(y))
            xxx=np.append(xxx,x)
            yyy=np.append(yyy,y)
            ttt=np.append(ttt,t)
            lor=np.append(lor,w-phi)
            rrr=np.append(rrr,r)
            Nfiles = Nfiles + 1
        except OSError:
            print(f'NOT FOUND {fname}')

    #lor=np.where(np.fabs(lor)<aw_pitch,0.0,lor) # if the correction is smaller than the aw pitch, ignore it
    rrr=np.multiply(rrr,10.0) # cm -> mm
    rrr=np.where(rrr>182.,182.-(rrr-182.),rrr)
    return xxx,yyy,lor,ttt,rrr


def plot_simple(ax,t,r,w):
    #plt.subplot(211)
    ax[0].plot(t,r,'.r',label=f'z={args.zed:.2f}cm')
    ax[0].set_xlabel('t [ns]')
    ax[0].set_ylabel('r [mm]')
    ax[0].set_xlim(0.,args.maximum_time)
    ax[0].set_ylim(109.,190.)
    ax[0].grid(True)

    
    #plt.subplot(212)
    ax[1].plot(t,w,'.b',label=f'z={args.zed:.2f}cm')
    ax[1].set_xlabel('t [ns]')
    ax[1].set_ylabel('#Delta#phi [rad]')
    ax[1].set_xlim(0.,args.maximum_time)
    #ax[1].ylim(109.,190.)
    ax[1].grid(True)

    #plt.show()

def plot_smooth(ax,t,r):
    ax[0].plot(t,r,'-k',label='piecewise fit')

def plot_discreet(ax,t,phi):
    ax[1].plot(t,phi,'ok',label='')



def plot_map(ax,xxx,yyy,ttt,lor,Npoints=3811):

    print('drift time')
    # Set up a regular grid of interpolation points
    xi, yi = np.linspace(-19.0, 19.0, Npoints), np.linspace(-19.0, 19.0, Npoints)
    #ri = np.sqrt(xi*xi+yi*yi)
    #xi=xi[np.nonzero(ri>109.)]
    #yi=yi[np.nonzero(ri>109.)]
    xi, yi = np.meshgrid(xi, yi)
    
    # Interpolate
    drift_time = scipy.interpolate.griddata((xxx, yyy), ttt, (xi, yi), method='cubic')

    print('lorentz')
    # Set up a regular grid of interpolation points
    xj, yj = np.linspace(-19.0, 19.0, Npoints), np.linspace(-19.0, 19.0, Npoints)
    xj, yj = np.meshgrid(xj, yj)
    # Interpolate
    lorentz = scipy.interpolate.griddata((xxx, yyy), lor, (xj, yj), method='cubic')

 
    ax[2].contourf(xi, yi, drift_time, 13, alpha=.75, cmap=plt.cm.hot)
    Ci=ax[2].contour(xi, yi, drift_time, 13, colors='black', linewidth=.5)
    ax[2].clabel(Ci, inline=1, fontsize=10)

    ax[2].set_title('Drift Time', fontsize=12)
    ax[2].set_xlabel('x [cm]', fontsize=10)
    ax[2].set_ylabel('y [cm]', fontsize=10)
    
 
    ax[3].contourf(xj, yj, lorentz, 13, alpha=.75, cmap=plt.cm.hot)
    Cj=ax[3].contour(xj, yj, lorentz, 13, colors='black', linewidth=.5)
    ax[3].clabel(Cj, inline=1, fontsize=10)

    ax[3].set_title('Lorentz Correction', fontsize=12)
    ax[3].set_xlabel('x [cm]', fontsize=10)
    ax[3].set_ylabel('y [cm]', fontsize=10)
 

def pol3(x,a0,a1,a2,a3):
    return a0+a1*x+a2*x**2+a3*x**3
def pol2(x,a0,a1,a2):
    return a0+a1*x+a2*x**2
def pol1(x,a0,a1):
    return a0+a1*x


def smoother(t_d,rad,tjoin=140.,tmax=4300.):
    tfit3=t_d[np.nonzero(t_d>tjoin)]
    rfit3=rad[np.nonzero(t_d>tjoin)]
    par3,pcov3=curve_fit(pol3,tfit3,rfit3)

    drift_time_3=np.arange(tjoin,tmax,10.)    
    radius3=pol3(drift_time_3,*par3)

    tfit1=t_d[np.nonzero(t_d<=tjoin)]
    rfit1=rad[np.nonzero(t_d<=tjoin)]
    par1,pcov1=curve_fit(pol1,tfit1,rfit1)

    drift_time_1=np.arange(0.,tjoin,10.)    
    radius1=pol1(drift_time_1,*par1)

    drift_time=np.append(drift_time_1,drift_time_3)
    radius=np.append(radius1,radius3)

    return drift_time,radius


def find_nearest(array, value):
    #array = np.asarray(array)
    idx = (np.abs(array - value)).argmin()
    return array[idx]

def discreet_digi(td,phi,tmax=4300.):
    #baw=np.linspace(0.,2.*pi,257)
    #baw=[aw_pitch*i for i in range(256)]
    baw=np.arange(aw_pitch,2.*pi,aw_pitch)
    #baw=np.arange(0.,2.*pi,aw_pitch)
    #print(baw)
    res=np.digitize(phi,bins=baw)
    anode_w=res*aw_pitch

    bt=np.arange(0.,tmax,10.)
    tmap={kt:0.0 for kt in bt}
    #print(tmap)

    for it,iw in zip(td,anode_w):
        k=find_nearest(bt,it)
        #print(k,it,iw)
        if tmap[k] > iw: continue
        tmap[k]=iw

    return np.array(list(tmap.values()))


def discreet_hist(td,phi,tmax=4300.):
    H,x,y=np.histogram2d(td,phi,bins=[int(tmax/10.),256],range=[[0.,tmax],[0.,2.*pi]])
    #print(H.shape)
    print(H[:,:2])
    #print(H[:,:2].shape)
    #print(y)
    #print(y.shape)
    print(y[1])
    correction=np.zeros(int(tmax/10.))
    for idx in range(256):
        #print(idx)
        pos=y[idx]
        lor=H[:,idx]
        for a in range(lor.size):
            if lor[a] > 0. and correction[a]<pos: 
                correction[a]=pos
    #print(H[:,0].shape)
    return correction
    

def discreet(td,phi,tmax=4300.):
    #return discreet_digi(td,phi,tmax=4300.)
    return discreet_hist(td,phi,tmax=4300.)

  

def save(t_d,rad,lor,z,B):
    f=open(f'garfppSTR_Bmap_z{z}mm_Ar70CO230.dat','w')
    f.write(f'# B = {B:.2f} T, garfield++ simulation CERN\n')
    f.write('# t\tr\tphi\n')
    for x,y,p in sorted(zip(t_d,rad,lor)):
        f.write('%1.0f\t%1.2f\t%1.4f\n'%(x,y,p))
    f.close()



if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument("pathto", type=str, help="path to data")
    parser.add_argument("-z","--zed",type=float,help="axial slice in cm",default=0.0)
    parser.add_argument("-vaw","--voltage_aw",type=float,help="AW voltage in Volts",default=3200.0)
    parser.add_argument("-vfw","--voltage_fw",type=float,help="FW voltage in Volts",default=-99.0)
    parser.add_argument("-B","--Bfield",type=float,help="Magnetic Field in T",default=1.0)
    parser.add_argument("-tmax","--maximum_time",type=float,help="maximum time to display in ns",default=4300.)

    args=parser.parse_args()
    
    zzz=np.array([0.0,20.0,40.0,60.0])
    zzz=np.append(zzz, np.arange(80.,117.,0.5) )
    if args.zed in zzz:
        Zslice=args.zed
    else:
        idx=(np.abs(zzz-args.zed)).argmin()
        Zslice=zzz[idx]

    x,y,w,t,r=ReadIn(args.pathto,Zed=Zslice,Vaw=args.voltage_aw,Vfw=args.voltage_fw,B=args.Bfield)

    t1,r1=smoother(t,r)
    w1=discreet(t,w)
    print(t1.size,r1.size,w1.size)

    #save(t,r,w,Zslice*10.,fabs(args.Bfield))
    save(t1,r1,w1,Zslice*10.,fabs(args.Bfield))

    fig,axs=plt.subplots(2, 2)
    ax=axs.flatten()
    plot_simple(ax,t,r,w)
    plot_smooth(ax,t1,r1)
    plot_discreet(ax,t1,w1)
    
    plot_map(ax,x,y,t,w,Npoints=1000)
    
    ax[0].legend()
    ax[1].legend()

    fig.set_size_inches(18,19)
    fig.tight_layout()
    plt.show()
