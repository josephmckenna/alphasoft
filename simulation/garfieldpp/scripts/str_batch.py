#!/usr/bin/env python3

from math import radians
import numpy as np
import subprocess as sp
import multiprocessing as mp
import time
import datetime
import os

def work(cmd):
    ss=len(os.environ['AGRELEASE']+'bin/simulation')+1
    logfile=os.environ['GARFIELDPP']+'/chamber_drift/drift_logs/'+cmd[ss:].replace(' ','_')+'.log'
    start_time=time.time()
    print(cmd, 'START @', time.strftime("%Y %b %d, %H:%M:%S", time.localtime()))
    try:
        out=sp.check_output(cmd, shell=True, stderr=open(logfile,'w'))
        elapsed_time = time.time() - start_time
        wall_clock=str(datetime.timedelta(seconds=elapsed_time))
        print(cmd, 'DONE in', wall_clock, 's')
        with open(logfile, "a") as f:
            f.write( '\nWall Clock: '+wall_clock+'\n' )
    except sp.CalledProcessError as err:
        print('Command:', err.cmd, 'returned:',err.output)

def scan(zscan):
    CathodeVoltage = -4000.
    AnodeVoltage   = 3200.
    #AnodeVoltage   = 3100.
    #FieldVoltage   = -99.
    FieldVoltage   = -110.
    MagneticField  = 1.
    #MagneticField  = -0.26
    #MagneticField  = 1.
    QuenchFraction = 0.3

    if zscan:
        z=np.array([0.0,20.0,40.0,60.0]) # cm
        z=np.append(z, np.arange(80.,117.,0.5))
    else:
        z=np.array([0.0])

    phi=np.arange(1.,360.,3.)

    cmd=[]
    ok,bad,tot=0,0,0
    for InitialZed in z:
        for InitialPhi in phi:
            fname = '%s/chamber_drift/drift_tables/Drift_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT.dat' % (os.environ['GARFIELDPP'],radians(InitialPhi),InitialZed,(1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,CathodeVoltage,AnodeVoltage,FieldVoltage,MagneticField)
            if os.path.isfile(fname):
                ok=ok+1
                os.remove(fname)
            else:
                bad=bad+1
                #print 'NOT FOUND:', fname
            cmd.append( '%s/bin/simulation/ChamberDrift.exe %1.4f %1.2f %1.f %1.f %1.f %1.1f %1.2f' % (os.environ['AGRELEASE'],radians(InitialPhi), InitialZed, CathodeVoltage, AnodeVoltage, FieldVoltage, QuenchFraction, MagneticField) )

    print('ok: ', ok, 'bad: ', bad, 'tot: ', tot)
    return cmd


if __name__ == '__main__':

    commands=scan(False)
    '''
    for x in commands:
        print x
    '''

    count=mp.cpu_count()-2
    pool=mp.Pool(processes=count)
    pool.map(work, commands)
