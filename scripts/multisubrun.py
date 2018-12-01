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

def parse_agana_args(subfile,aarg):
    cmd='agana.exe ' + subfile
    if len(aarg) > 0:
        cmd+=' -- '
    else:
        return cmd
    sett=False
    for a in aarg:
        if not a.isdigit() and not sett:
            cmd+='--'
        cmd+=a
        cmd+=' '
        sett=False
        if a == 'anasettings':
            sett=True
    return cmd

def assemble(run,limit,argx):
    cmdlist=[]
    sub=0
    subrun='%s/run%05dsub%03d.mid.lz4'%(environ['AGMIDASDATA'],run,sub)
    subfile=Path(subrun)
    while subfile.is_file():
        cmd=parse_agana_args(subrun,argx)
        cmdlist.append(cmd)
        print(cmd)
        sub+=1
        if limit > 0 and sub == limit:
            break
        subrun='%s/run%05dsub%03d.mid.lz4'%(environ['AGMIDASDATA'],run,sub)
        subfile=Path(subrun)
    return cmdlist

def addsubs(nproc,run):
    cmd='hadd -ff -j %d output%05d.root '% (nproc,run)
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

def addstr(run):
    print('addstr Run',run,'does nothing for now')
    
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

def addmaps(run):
    foutname='%s/pwbR%d.map'%(environ['AGRELEASE'],run)
    with open(foutname,'w') as fout:
        sub=0
        subrun='%s/R%d/sub%03d/output%05d.root'%(environ['AGRELEASE'],run,sub,run)
        subfile=Path(subrun)
        while subfile.is_file():
            finname='%s/R%d/sub%03d/pwbR%d.map'%(environ['AGRELEASE'],run,sub,run)
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
                        help='do not merge subruns (unusual)')

    parser.add_argument('-l', '--limit', type=int,
                        default=-1,
                        help='limit the number of subruns to analyze')
    
    args = parser.parse_args()

    commands=assemble(args.run,args.limit,args.opt)

    pool=mp.Pool(processes=args.proc)
    pool.map(work, commands)
    pool.close()
    pool.join()
    
    if args.subs:
        addsubs(args.proc,args.run)
        if 'calib' in args.opt:
            addstr(args.run)

    if args.merge:
        addlogs(args.run)
        if 'pwbmap' in args.opt:
            addmaps(args.run)

    if args.remove:
        rmsubs(args.run)
