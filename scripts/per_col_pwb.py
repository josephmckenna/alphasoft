import numpy as np
import midas.client
import argparse
from sys import exit
from per_slot_pwb import *



if __name__=='__main__':

  parser=argparse.ArgumentParser()
  parser.add_argument("column", type=int,
                      help="select a pwb power column",choices=range(0,8))
  parser.add_argument("--master", help="set indicated column as SATA master",
                    action="store_true")
  parser.add_argument("--slave", help="set indicated column as SATA slave",
                    action="store_true")
  parser.add_argument("--clock", help="switch indicated column to SATA clock",
                    action="store_true")
  parser.add_argument("--trigger", help="switch indicated column to SATA trigger",
                    action="store_true")
  parser.add_argument('switch',help='on or off',nargs='?', choices=('on','off'))
  parser.add_argument("-v", "--verbose", help="increase output verbosity",
                      action="store_true")
  args=parser.parse_args()
  if args.column > 7:
    print('There are only 8 columns, numbered from 0 to 7')
    exit(1)

  client = midas.client.MidasClient("SATAswitch")
  start_index=args.column*8
  stop_index=start_index+8
  pwb_array=get_pwb(client,args.verbose)

  stat=True if args.switch == 'on' else False

  print('Changing settings for '+' '.join(pwb_array[start_index:start_index+7]))
  if args.master:
    for idx in range(start_index,stop_index):
      var=f"/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_master[{idx}]"
      print(var)
      client.odb_set(var,stat)
  elif args.slave:
    for idx in range(start_index,stop_index):
      client.odb_set(f"/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_slave[{idx}]",stat)

  if args.clock:
    for idx in range(start_index,stop_index):
      client.odb_set(f"/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_clock[{idx}]",stat)

  if args.trigger:
    for idx in range(start_index,stop_index):
      client.odb_set(f"/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_trigger[{idx}]",stat)

  #sata_master=client.odb_get("/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_master")
  #print(np.reshape(sata_master,(8,8)).T)

  
