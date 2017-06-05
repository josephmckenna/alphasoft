#!/usr/bin/python

from scipy import interpolate
from numpy import genfromtxt
import sys

if(len(sys.argv)<2 or sys.argv[1] == "-h"):
    print "Calculates appropriate field wire voltage for given anode (and cathode) voltages"
    print "Usage: v_fw.py <anode voltage> [cathode voltage]"
    print "cathode voltage defaults to (-)4000V, minus sign can be omitted."
    exit()

va_select = float(sys.argv[1])
Vc = 4000.

if(len(sys.argv)>2):
    Vc=float(sys.argv[2])
    if(Vc < 0): Vc *= -1.

V_FW = genfromtxt('ALPHA-G_FW_Balancing_grid.csv', delimiter=',')
V_C = range(0,8001,500)
V_A = range(0,4001,250)

Vfw = interpolate.RectBivariateSpline(V_A,V_C,V_FW)

print Vfw(va_select,Vc)[0][0]
