#!/bin/python3

import subprocess as sp
import multiprocessing as mp
import time
import datetime
import re
import os
import sys
from pathlib import Path

def get_logname(cmd):
    run=int(re.search('[0-9]{5}',cmd).group(0))
    logname='R'+str(run)+'.log'
    return logname

def work(cmd):
    logfile=get_logname(cmd)
    start_time=time.time()
    try:
        sp.call(cmd, shell=True, stdout=open(logfile,'w'),stderr=sp.STDOUT)
        elapsed_time = time.time() - start_time
        wall_clock=str(datetime.timedelta(seconds=elapsed_time))
        with open(logfile, "a") as f:
            f.write( '\nWall Clock: '+wall_clock+'\n' )
    except sp.CalledProcessError as err:
        print('Command:', err.cmd, 'returned:',err.output)


def assemble(runlist, subs, exearg):
    cmdlist=[]
    for run in runlist:
        cmd='agana.exe '
        if subs < 0 :
            cmd+=os.environ['AGMIDASDATA']+'/run0'+str(run)+'sub*.mid.lz4'
        else:
            for sub in range(0,subs):
                subrun='%s/run%05dsub%03d.mid.lz4'%(os.environ['AGMIDASDATA'],run,sub)
                subfile=Path(subrun)
                if subfile.is_file():
                    cmd+=subrun
                else:
                    break
                cmd+=' '
        for a in exearg:
            cmd+=a
        cmdlist.append(cmd)
    return cmdlist

if __name__=='__main__':

    argc=len(sys.argv)
    fname='run.list'
    N_sub_runs=-1 # all subruns
    exearg=[]
    if argc == 2:
        fname=sys.argv[1]
    elif argc == 3:
        fname=sys.argv[1]
        N_sub_runs=int(sys.argv[2])
    elif argc > 3:
        fname=sys.argv[1]
        N_sub_runs=int(sys.argv[2])
        exearg.append(' -- ')
        for a in sys.argv[3:]:
            exearg.append(a)
            exearg.append(' ')
        exearg[-1]=exearg[-1].strip()

    #wd=os.environ['AGRELEASE']+'/ana/'
    os.chdir(os.environ['AGRELEASE'])
    print('Current working directory',os.getcwd())
            
    runfile=Path(fname)
    if runfile.is_file():
        runlist=open(fname)
    else:
        print('Please provide run list')
        sys.exit(1)
    
    runs=[]
    for line in runlist:
        if line[0] == '#':
            continue
        runs.append(int(line))
    runlist.close()
    print('N runs',len(runs))

    commands=assemble(runs,N_sub_runs,exearg)
    print(commands)

    count=15
    pool=mp.Pool(processes=count)
    pool.map(work, commands)
    
