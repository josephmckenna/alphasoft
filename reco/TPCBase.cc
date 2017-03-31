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

void TPCBase::GetPadPosition(int i, double &z, double &phi)
{
  double ring_d = double(i)/double(npadsec);
  double intpart;
  double frac = modf(ring_d,&intpart);
  //  assert(intpart<gNpadsZ);
  z = ( intpart + 0.5 ) * PadSideZ;
  z = HalfWidthZ - z;
  phi = frac * TMath::TwoPi();
}

void TPCBase::GetPadPosition(int i, int s, double &z, double &phi)
{
  double phi_c = TMath::TwoPi()*PadWidthPhi;
  double phi_ch = TMath::Pi()*PadWidthPhi;
  if( s >= 0 && s < npadsec )
    //    phi = double(s) * phi_c;
    phi = double(s) * phi_c + phi_ch;
  else
    {
      phi=-9999999.;
      std::cerr<<"TPCBase::GetPadPosition Error: Sector Out Of Range: "<<s<<"[0,"<<npadsec<<")\n";
      return;
    }

  if( i >= 0 && i < npads)
    {
      z = ( double(i) + 0.5 ) * PadSideZ;
      z = HalfWidthZ - z;
    }
  else
    {
      z=-9999999.;
      std::cerr<<"TPCBase::GetPadPosition Error: Pads Out Of Range: "<<i<<"[0,"<<npads<<"31)\n";
      return;
    }
}

std::pair<int,int> TPCBase::FindPad(const double zed, const double phi)
{  // // Currently no phi-segmentation
   //  return (0.5*(npads - 1) - z/PadSideZ);
  double z = HalfWidthZ - zed;
  double f = phi;
  if(f<0.) f+=TMath::TwoPi();
  //  std::cout<<"z: "<<zed<<"\tphi: "<<phi<<std::endl;

  std::pair<int,int> pad;
  pad.first = int(f/TMath::TwoPi()*npadsec);  // pad sector
  pad.second = int(z/(2*HalfWidthZ)*npads);         // pad index in sector
  if(pad.first >= npadsec) pad.first = -1;
  if(pad.second >= npads) pad.second = -1;
  return pad;
}

int TPCBase::SectorAndPad2Index(const int ps, const int pi){
    return (ps+pi*npadsec);
}

int TPCBase::SectorAndPad2Index(std::pair<int,int> p){
  int ps = p.first, pi = p.second;
  return (ps+pi*npadsec);
}

std::pair<int,int> TPCBase::Index2SectorAndPad(const int padindex){
    int pi = padindex/npadsec;
    int ps = padindex%npadsec;
    return std::pair<int,int>(ps,pi);
}

unsigned int TPCBase::FindAnode(const double phi, double phi0){
    double AngleAnodeWires = GetAnodePitch();
    int anode((phi-phi0)/AngleAnodeWires);
    if(anode < 0) anode += NanodeWires;
    return anode;
}
