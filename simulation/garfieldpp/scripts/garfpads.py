from math import pi, degrees

nrows=576
PadSideZ=0.4 # cm
HalfLengthZ=115.2

nsecs=32
PadSidePhi=2.*pi/nsecs
radius=19.0

PadPosZmin = -0.5 * nrows * PadSideZ
PadPosZmax = PadPosZmin + PadSideZ
for r in range(nrows):
    z=( ( r + 0.5 ) * PadSideZ ) - HalfLengthZ
    #print("{:03d}\tlimits=[{:+.1f},{:+.1f}]cm\t\tp{:03d}\tcenter={:+.2f}cm".format(r,PadPosZmin,PadPosZmax,r,z))
    PadPosZmax += PadSideZ
    PadPosZmin += PadSideZ




PadPosPhiMin = 0.
PadPosPhiMax = PadSidePhi
for s in range(nsecs):
    phi = (s + 0.5) * PadSidePhi
    print("""{:02d}\t
             [{:.3f},{:.3f}]rad\t{:.3f}rad\t
             [{:.2f},{:.2f}]deg\t{:.2f}deg\t
             [{:.3f},{:.3f}]cm\t{:.3f}cm""".format(s,
                                                   PadPosPhiMin,PadPosPhiMax,phi,
                                                   degrees(PadPosPhiMin),degrees(PadPosPhiMax),degrees(phi),
                                                   PadPosPhiMin*radius,PadPosPhiMax*radius,phi*radius))

    PadPosPhiMin,PadPosPhiMax = PadPosPhiMin+PadSidePhi, PadPosPhiMax+PadSidePhi
