#!/usr/bin/env python3

from os import makedirs,chdir,environ,getcwd,path
from shutil import copy, rmtree
from time import time
import datetime
import re
import subprocess as sp
import multiprocessing as mp
from pathlib import Path
import argparse

def getlogname(cmd):
    run=int(re.search('[0-9]{6}',cmd).group(0))
    logname='R'+str(run)+'.log'
    temp=re.search('sub[0-9]{3}',cmd).group(0)
    sub=int(re.search('[0-9]{3}',temp).group(0))
    return run,sub,logname

def work(cmd):
    run_number,sub_run,logfile=getlogname(cmd)
    agana=environ['AGRELEASE']+'/agana.exe'
    agananr=environ['AGRELEASE']+'/agana_noreco.exe'

    newdir=workdir+'/R'+str(run_number)
    print('new dir:',newdir)
    makedirs(newdir, exist_ok=True)

    newsubdir=newdir+'/sub%03d'%sub_run
    print('new subdir:',newsubdir)
    makedirs(newsubdir, exist_ok=True)

    copy(agana,newsubdir)
    copy(agananr,newsubdir)
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

def parse_agana_args(subfile,aarg,nrec):
    cmd='agana.exe ' + subfile
    if nrec:
        cmd='agana_noreco.exe ' + subfile
    if len(aarg) > 0:
        cmd+=' -- '
    else:
        return cmd
    for a in aarg:
        isnum=True
        try:
            float(a)
        except ValueError:
            isnum=False
        if not isnum and not Path(a).is_file():
            cmd+='--'
        cmd+=a
        cmd+=' '
    return cmd

def assemble(run,limit,argx,nrec):
    cmdlist=[]
    sub=0
    subrun='%s/run%06dsub%03d.mid.lz4'%(environ['AGMIDASDATA'],run,sub)
    subfile=Path(subrun)
    while subfile.is_file():
        cmd=parse_agana_args(subrun,argx,nrec)
        cmdlist.append(cmd)
        print(cmd)
        sub+=1
        if limit > 0 and sub == limit:
            break
        subrun='%s/run%06dsub%03d.mid.lz4'%(environ['AGMIDASDATA'],run,sub)
        subfile=Path(subrun)
    return cmdlist

def addsubs(run):
    cmd='hadd -ff output%06d.root '% (run)
    sub=0
    subrun='%s/R%d/sub%03d/output%06d.root'%(workdir,run,sub,run)
    subfile=Path(subrun)
    while subfile.is_file():
        cmd+=subrun
        cmd+=' '
        sub+=1
        subrun='%s/R%d/sub%03d/output%06d.root'%(workdir,run,sub,run)
        subfile=Path(subrun)
    print(cmd)
    sp.call(cmd, shell=True, stdout=open('/dev/null','w'),stderr=sp.STDOUT)

def addstr(run):
    print('addstr Run',run,'does nothing for now')

def addlogs(run):
    foutname='%s/R%d.log'%(workdir,run)
    with open(foutname,'w') as fout:
        sub=0
        subrun='%s/R%d/sub%03d/output%06d.root'%(workdir,run,sub,run)
        subfile=Path(subrun)
        while subfile.is_file():
            finname='%s/R%d/sub%03d/R%d.log'%(workdir,run,sub,run)
            print(finname)
            with open(finname,'r') as fin:
                for line in fin:
                    fout.write(line)
            sub+=1
            subrun='%s/R%d/sub%03d/output%06d.root'%(workdir,run,sub,run)
            subfile=Path(subrun)

def addmaps(run):
    foutname='%s/pwbR%d.map'%(workdir,run)
    with open(foutname,'w') as fout:
        sub=0
        subrun='%s/R%d/sub%03d/output%06d.root'%(workdir,run,sub,run)
        subfile=Path(subrun)
        while subfile.is_file():
            finname='%s/R%d/sub%03d/pwbR%d.map'%(workdir,run,sub,run)
            print(finname)
            with open(finname,'r') as fin:
                for line in fin:
                    fout.write(line)
            sub+=1
            subrun='%s/R%d/sub%03d/output%06d.root'%(workdir,run,sub,run)
            subfile=Path(subrun)

def rmsubs(run):
    folder_name=workdir+'/R'+str(run)
    rmtree(folder_name, ignore_errors=True)


if __name__=='__main__':

    parser = argparse.ArgumentParser(description='Parallelize the analysis of multiple subruns of a given run')

    parser.add_argument('run', type=int,
                        help='Run number')
    parser.add_argument('-o','--opt', nargs='*',
                        default=[],
                        help='optional arguments for agana\'s modules')

    parser.add_argument('-m', '--merge', action='store_true',
                        help='merge log files')
    parser.add_argument('-r', '--remove', action='store_true',
                        help='remove subruns folders')

    parser.add_argument('-p', '--proc', type=int,
                        default=mp.cpu_count(),
                        help='number of concurrent subprocesses (default=%d)'%mp.cpu_count())

    parser.add_argument('-s', '--subs', action='store_false',
                        help='do not merge subruns (unusual)')

    parser.add_argument('-l', '--limit', type=int,
                        default=-1,
                        help='limit the number of subruns to analyze')

    parser.add_argument('-d', '--workdir', type=str,
                        default=environ['AGRELEASE'],
                        help='working/output directory')

    parser.add_argument('-n', '--noreco', action='store_true',
                        help='invoke basic analyzer, i.e., no tracking')

    args = parser.parse_args()

    commands=assemble(args.run,args.limit,args.opt,args.noreco)
    print(commands)

    workdir = path.abspath(args.workdir)
    print(workdir)

    pool=mp.Pool(processes=args.proc)
    pool.map(work, commands)
    pool.close()
    pool.join()

    if args.subs:
        addsubs(args.run)
        if 'calib' in args.opt:
            addstr(args.run)

    if args.merge:
        addlogs(args.run)
        if 'pwbmap' in args.opt:
            addmaps(args.run)

    if args.remove:
        rmsubs(args.run)
