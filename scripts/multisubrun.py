#!/usr/bin/env python3

from os import mkdir,chdir,environ,getcwd,path
from shutil import copy, rmtree
from time import time
import datetime
import re
import subprocess as sp
import multiprocessing as mp
from pathlib import Path
import argparse

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
            if not a.isdigit():
                cmd+='--'
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
    
def addlogs(run):
    foutname='%s/R%d.log'%(environ['AGRELEASE'],run)
    with open(foutname,'w') as fout:
        sub=0
        subrun='%s/R%d/sub%03d/output%05d.root'%(environ['AGRELEASE'],run,sub,run)
        subfile=Path(subrun)
        while subfile.is_file():
            finname='%s/R%d/sub%03d/R%d.log'%(environ['AGRELEASE'],run,sub,run)
            print(finname)
            with open(finname,'r') as fin:
                for line in fin:
                    fout.write(line)
            sub+=1
            subrun='%s/R%d/sub%03d/output%05d.root'%(environ['AGRELEASE'],run,sub,run)
            subfile=Path(subrun)

def rmsubs(run):
    folder_name=environ['AGRELEASE']+'/R'+str(run)
    rmtree(folder_name, ignore_errors=True)


if __name__=='__main__':
 
    parser = argparse.ArgumentParser(description='Parallelize the analysis of multiple subruns of a given run')

    parser.add_argument('run', type=int,
                        help='Run number')
    parser.add_argument('-o','--opt', nargs='*',
                        help='optional arguments for agana\'s modules')
    
    parser.add_argument('-m', '--merge', action='store_true',
                        help='merge log files')
    parser.add_argument('-r', '--remove', action='store_true',
                        help='remove subruns folders')

    parser.add_argument('-p', '--proc', type=int,
                        default=mp.cpu_count(),
                        help='number of concurrent subprocesses')

    parser.add_argument('-s', '--subs', action='store_false',
                        help='do not merge subruns')
    
    args = parser.parse_args()

    commands=assemble(args.run,args.opt)

    pool=mp.Pool(processes=args.proc)
    pool.map(work, commands)
    pool.close()
    pool.join()
    
    if args.subs:
        addsubs(args.run)

    if args.merge:
        addlogs(args.run)
    if args.remove:
        rmsubs(args.run)
