#!/usr/bin/env python3

from math import radians
import numpy as np
import subprocess as sp
import multiprocessing as mp
import time
import datetime
import os

def work(cmd):
    logfile=os.environ['GARFIELDPP']+'/polar_driftsignal/drift_logs/'+cmd[len(os.environ['GARFIELDPP']):].replace(' ','_')+'.log'
    start_time=time.time()
    print(cmd, 'START @', time.strftime("%Y %b %d, %H:%M:%S", time.localtime()))
    try:
        out=sp.check_output(cmd, shell=True, stderr=sp.STDOUT)
        elapsed_time = time.time() - start_time
        wall_clock=str(datetime.timedelta(seconds=elapsed_time))
        print(cmd, 'DONE in', wall_clock, 's')
        with open(logfile, "a") as f:
            f.write(out.decode())
            f.write( '\nWall Clock: '+wall_clock+'\n' )
    except sp.CalledProcessError as err:
        print('Command:', err.cmd, 'returned:',err.output)

def scan(zscan):
    CathodeVoltage = -4000.
    AnodeVoltage   = 3200.
    FieldVoltage   = -110.
    MagneticField  = 1.
    QuenchFraction = 0.3
    track = 'driftMC'

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
            fname = '%s/polar_driftsignal/drift_tables/PolarDriftLine_%s_phi%1.4f_Z%2.1fcm.dat' % (os.environ['GARFIELDPP'],track,radians(InitialPhi),InitialZed)
            if os.path.isfile(fname):
                ok=ok+1
                os.remove(fname)
            else:
                bad=bad+1
                #print 'NOT FOUND:', fname
            cmd.append( '%s/bin/simulation/PolarDriftSignal.exe %1.4f %1.2f %s' % (os.environ['AGRELEASE'],InitialPhi, InitialZed, track) )

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
