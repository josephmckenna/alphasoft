import numpy as np
import midas.client
import argparse
from sys import exit


def get_pwb(client,verbose=True):
  
  pwb=client.odb_get("/Equipment/CTRL/Settings/PWB/per_pwb_slot/modules")
  if verbose:
    pwb_m=np.reshape(pwb,(8,8)).T
    print('-----------------------------------------')
    for r in range(8): 
      print(f'ring #{r} {pwb_m[r,:]}')
    print('-----------------------------------------')
    for c in range(8): 
      print(f'col #{c} {pwb_m[:,c]}')
    print('-----------------------------------------')
  return pwb



if __name__=='__main__':

  parser=argparse.ArgumentParser()
  parser.add_argument("pwb", type=str,
                    help="select a pwb")
  parser.add_argument('switch',help='on or off',nargs='?', choices=('on','off'))
  parser.add_argument("--master", help="set indicated pwb as SATA master",
                    action="store_true")
  parser.add_argument("--slave", help="set indicated pwb as SATA slave",
                    action="store_true")
  parser.add_argument("--clock", help="switch indicated pwb to SATA clock",
                    action="store_true")
  parser.add_argument("--trigger", help="switch indicated pwb to SATA trigger",
                    action="store_true")
  parser.add_argument("-v", "--verbose", help="increase output verbosity",
                      action="store_true")
  args=parser.parse_args()

  client = midas.client.MidasClient("SATAswitch")
  pwb_array=get_pwb(client,args.verbose)
  try:
    idx=pwb_array.index(args.pwb)
  except ValueError as e:
    print(e)
    exit(1)

  stat=True if args.switch == 'on' else False

  print(f'Changing settings for {args.pwb}')
  if args.master:
    var=f"/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_master[{idx}]"
    print(var)
    client.odb_set(var,stat)

  elif args.slave:
    client.odb_set(f"/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_slave[{idx}]",stat)

  if args.clock:
    client.odb_set(f"/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_clock[{idx}]",stat)

  if args.trigger:
    client.odb_set(f"/Equipment/CTRL/Settings/PWB/per_pwb_slot/sata_trigger[{idx}]",stat)


  
