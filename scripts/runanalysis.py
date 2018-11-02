#!/usr/bin/python3

from sys import argv, exit
from os import environ
from pathlib import Path
import subprocess as sp
from time import time
from datetime import timedelta

if __name__ == '__main__':

    if len(argv) < 2:
        print('Please provide run number')
        exit(1)

    for run in argv[1:]:
        sub_exists=True
        sub=0
        cmd='agana.exe'
        while sub_exists:
            subrun='%s/run%05dsub%03d.mid.lz4'%(environ['AGMIDASDATA'],int(run),sub)
            subfile=Path(subrun)
            if subfile.is_file():
                cmd+=' '
                cmd+=subrun
            else:
                sub_exists=False
            sub += 1
        print(cmd)

        logfile='R%d.log'%int(run)
        try:
            start_time=time()
            sp.call(cmd, shell=True, stdout=open(logfile,'w'),stderr=sp.STDOUT)
            elapsed_time = time() - start_time
            wall_clock=str(timedelta(seconds=elapsed_time))
            with open(logfile, "a") as f:
                f.write( '\nWall Clock: '+wall_clock+'\n' )
        except sp.CalledProcessError as err:
            print('Command:', err.cmd, 'returned:',err.output)
