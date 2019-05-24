#!/usr/bin/env python3

from os import environ
import subprocess as sp
from pathlib import Path
from shutil import rmtree
import argparse


def addsubs(run,doit):
    cmd='hadd -ff output%05d.root '% (run)
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
    if doit:
        sp.call(cmd, shell=True, stdout=open('/dev/null','w'),stderr=sp.STDOUT)

def rmsubs(run):
    folder_name=environ['AGRELEASE']+'/R'+str(run)
    rmtree(folder_name, ignore_errors=True)

if __name__=='__main__':
 
    parser = argparse.ArgumentParser(description='Merge subruns of a given run')

    parser.add_argument('run', type=int,
                        help='Run number')
    parser.add_argument('-r', '--remove', action='store_true',
                        help='remove subruns folders')
    parser.add_argument('-t', '--test', action='store_false',
                        help='print the command and exit')

    args = parser.parse_args()
    addsubs(args.run,args.test)

    if args.remove:
        rmsubs(args.run)
