#!/usr/bin/python3

from os import mkdir,chdir,environ,getcwd,path
from shutil import copy
from time import time
import datetime
import re
import subprocess as sp
import multiprocessing as mp
from sys import argv
from pathlib import Path

def getlogname(cmd):
    run=int(re.search('[0-9]{5}',cmd).group(0))
    logname='R'+str(run)+'.log'
    temp=re.search('sub[0-9]{3}',cmd).group(0)
    sub=int(re.search('[0-9]{3}',temp).group(0))
    return run,sub,logname

def work(cmd):
    run_number,sub_run,logfile=getlogname(cmd)
    agana=environ['AGRELEASE']+'/agana.exe'

    newdir=environ['AGRELEASE']+'/R'+str(run_number)
    print('new dir:',newdir)
    try:
        mkdir(newdir)
    except FileExistsError:
        print(newdir,'exists')
        
    newsubdir=newdir+'/sub%03d'%sub_run
    print('new subdir:',newsubdir)
    try:
        mkdir(newsubdir)
    except FileExistsError:
        print(newsubdir,'exists')

    copy(agana,newsubdir)
    chdir(newsubdir)
    print(getcwd())

    try:
        print('starting:',cmd)
        start_time = time()
        sp.call(cmd, shell=True, stdout=open(logfile,'w'),stderr=sp.STDOUT)
        elapsed_time = time() - start_time
        wall_clock=str(datetime.timedelta(seconds=elapsed_time))
        with open(logfile, "a") as f:
            f.write( '\nWall Clock: '+wall_clock+'\n' )
    except sp.CalledProcessError as err:
        print('Command:', err.cmd, 'returned:',err.output)

def assemble(run,argx):
    cmdlist=[]
    sub=0
    subrun='%s/run%05dsub%03d.mid.lz4'%(environ['AGMIDASDATA'],run,sub)
    subfile=Path(subrun)
    while subfile.is_file():
        cmd='agana.exe '
        cmd+=subrun
        if len(argx) > 0:
            cmd+=' -- '
        for a in argx:
            cmd+=a
            cmd+=' '
        cmdlist.append(cmd)
        print(cmd)
        sub+=1
        subrun='%s/run%05dsub%03d.mid.lz4'%(environ['AGMIDASDATA'],run,sub)
        subfile=Path(subrun)
    return cmdlist

def addsubs(run):
    cmd='hadd -ff output%05d.root '%run
    sub=0
    subrun='%s/R%d/sub%03d/output%05d.root'%(environ['AGRELEASE'],run,sub,run)
    subfile=Path(subrun)
    while subfile.is_file():
        cmd+=subrun
        cmd+=' '
        sub+=1
        subrun='%s/R%d/sub%03d/output%05d.root'%(environ['AGRELEASE'],run,sub,run)
        subfile=Path(subrun)
    print(cmd)
    sp.call(cmd, shell=True, stdout=open('/dev/null','w'),stderr=sp.STDOUT)
    

if __name__=='__main__':
 
    run=int(argv[1])
    argx=[a for a in argv[2:]]

    commands=assemble(run,argx)

    count=int(len(commands)/8)
    pool=mp.Pool(processes=count)
    pool.map(work, commands)
    pool.close()
    pool.join()
    
    addsubs(run)
