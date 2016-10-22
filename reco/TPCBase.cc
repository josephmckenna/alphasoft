#include "TPCBase.hh"
#include <cmath>

void TPCBase::GetAnodePosition(int i, double &x, double &y, bool polar, double phi0){
    double AngleAnodeWires = GetAnodePitch();
    double AngleOffsetAnodeWires = 0.5*AngleAnodeWires;
    double phi = AngleAnodeWires * i + AngleOffsetAnodeWires + phi0;
    if(polar){
        x = AnodeWiresR;
        y = phi;
    } else {
        x = AnodeWiresR*cos(phi);
        y = AnodeWiresR*sin(phi);
    }
}

void TPCBase::GetWirePosition(int i, double &x, double &y, bool polar, double phi0){
    double AngleFieldWires = 2.*M_PI / double(NfieldWires);
    double phi = AngleFieldWires * i + phi0;
    if(polar){
        x = FieldWiresR;
        y = phi;
    } else {
        x = FieldWiresR*cos(phi);
        y = FieldWiresR*sin(phi);
    }
}

void TPCBase::GetPadPosition(int i, double &z, double &phi){  // Currently no phi-segmentation
    z = (((double) npads) * 0.5 - (i + 0.5))* PadSideZ;
    phi = unknown;
}

int TPCBase::FindPad(const double z, const double phi){  // Currently no phi-segmentation
    return (0.5*(npads - 1) - z/PadSideZ);
}

unsigned int TPCBase::FindAnode(const double phi, double phi0){
    double AngleAnodeWires = GetAnodePitch();
    int anode((phi-phi0)/AngleAnodeWires);
    if(anode < 0) anode += NanodeWires;
    return anode;
}
