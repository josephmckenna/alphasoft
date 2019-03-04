#include "TPCBase.hh"
#include <TMath.h>
#include <cmath>
#include <map>
#include <sstream>
#include <fstream>
#include <cassert>

//#include "Alpha16.h"

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::string;

TPCBase* TPCBase::fTPC=0;

TPCBase::TPCBase(bool proto):fPrototype(proto),
			     CathodeRadius(10.925),// cm
			     FieldWiresR(17.4),AnodeWiresR(18.2),
			     ROradius(19.),
			     NanodeWires(256),NfieldWires(256),
			     diamFieldWires(0.0075),tensionFieldWires(300.),// g
			     diamAnodeWires(0.003),tensionAnodeWires(60.),
			     AnodeWiresDensity(19.25),// g/cm^3 tungsten
			     trap_radius(5),
			     PadSideZ(0.4),npadsec(32),
			     TrapR(2.2275),
                             phi0(0)
{
  AnodePitch = 2.*M_PI / double(NanodeWires);

  if( fPrototype )
    HalfLengthZ = 14.4; //cm
  else
    {
      HalfLengthZ = 115.2;
      ROradius = 19.03;
      diamFieldWires = 0.0152;
    }

  FullLengthZ = 2.*HalfLengthZ;
  npads = int(FullLengthZ/PadSideZ+0.001);
  totpads=npadsec*npads;
  PadWidthPhi = 1.0/double(npadsec); // in units of 2 pi
}

void TPCBase::SetPrototype(bool flag)
{
  fPrototype=flag;
  HalfLengthZ=fPrototype?14.4:115.2;
  FullLengthZ = 2.*HalfLengthZ;
  ROradius = fPrototype?19.:19.03;
  diamFieldWires = fPrototype?0.0075:0.0152;
  npads = int(FullLengthZ/PadSideZ+0.001);
  totpads=npadsec*npads;
}

int TPCBase::MapElectrodes(short run, std::vector<electrode> &anodes, std::vector<electrode> &pads){
    anodes.clear();
    pads.clear();

    if(run < 0){
      for(unsigned int i = 0; i < (unsigned int) 2*NanodeWires; i++){
            int ii = i % NanodeWires;
            short us_ds = (i / NanodeWires);
            anodes.emplace_back(us_ds, ii);
        }
        for(unsigned int s = 0; s < (unsigned int) npadsec; s++)
            for(unsigned int i = 0; i < (unsigned int) npads; i++){
                pads.emplace_back(s, i);
            }
        phi0 = 0;
    }
    //    cout << "Electrode map set: " << anodes.size()+pads.size() << endl;
    //    for(unsigned int i = 0; i < anodes.size(); i++) cout << i << '\t' << (anodes[i].sec?'b':'t') << '\t' << anodes[i].i << endl;
    return anodes.size()+pads.size();
}

void TPCBase::GetAnodePosition(int i, double &x, double &y, bool mm, bool polar){
    double AngleAnodeWires = GetAnodePitch();
    double AngleOffsetAnodeWires = 0.5*AngleAnodeWires;
    double phi = AngleAnodeWires * i + AngleOffsetAnodeWires + phi0;
    if(phi > 2.*M_PI) phi -= 2.*M_PI;
    if(polar){
        x = AnodeWiresR;
        y = phi;
    } else {
        x = AnodeWiresR*cos(phi);
        y = AnodeWiresR*sin(phi);
    }
    if( mm )
      {
	x*=10.;
	y*=10.;
      }
}

void TPCBase::GetWirePosition(int i, double &x, double &y, bool mm, bool polar){
    double AngleFieldWires = 2.*M_PI / double(NfieldWires);
    double phi = AngleFieldWires * i + phi0;
    if(phi > 2.*M_PI) phi -= 2.*M_PI;
    if(polar){
        x = FieldWiresR;
        y = phi;
    } else {
        x = FieldWiresR*cos(phi);
        y = FieldWiresR*sin(phi);
    }    
    if( mm )
      {
	x*=10.;
	y*=10.;
      }
}

void TPCBase::GetPadPosition(int i, double &z){  // Currently no phi-segmentation
    z = (((double) npads) * 0.5 - (i + 0.5))* PadSideZ;
}

void TPCBase::GetPadPosition(int i, double &z, double &phi)
{
  double ring_d = double(i)/double(npadsec);
  double intpart;
  double frac = modf(ring_d,&intpart);
  //  assert(intpart<gNpadsZ);
  z = ( intpart + 0.5 ) * PadSideZ;
  z -= HalfLengthZ; // from -ve z to +ve -> increasing pad number
  phi = frac * 2.*M_PI + phi0;
  if(phi > 2.*M_PI) phi -= 2.*M_PI;
}

void TPCBase::GetPadPosition(int i, int s, double &z, double &phi)
{
  double phi_c = 2.*M_PI*PadWidthPhi;
  double phi_ch = M_PI*PadWidthPhi;
  if( s >= 0 && s < npadsec )
    {
      phi = double(s) * phi_c + phi_ch + phi0;
      if(phi > 2.*M_PI) phi -= 2.*M_PI;
    }
  else
    {
      phi=kUnknown;
      std::cerr<<"TPCBase::GetPadPosition Error: Sector Out Of Range: "<<s<<"[0,"<<npadsec<<")\n";
      return;
    }

  if( i >= 0 && i < npads)
    {
      z = ( double(i) + 0.5 ) * PadSideZ;
      z -= HalfLengthZ; // from -ve z to +ve -> increasing pad number
    }
  else
    {
      z=kUnknown;
      if(!(i>-4 && i<0))
          std::cerr<<"TPCBase::GetPadPosition Error: Pads Out Of Range: "<<i<<"[0,"<<npads<<")\n";
      return;
    }
}

std::pair<int,int> TPCBase::FindPad(const double zed, const double phi)
{
  const double z = zed + HalfLengthZ; // from -ve z to +ve -> increasing pad number
  double f = phi-phi0;
  if(f<0.) f+=2.*M_PI;
  //  std::cout<<"z: "<<zed<<"\tphi: "<<phi<<std::endl;

  std::pair<int,int> pad;
  // pad sector
  double sec = f/(2.*M_PI)*npadsec;
  pad.first = (ceil(sec)-sec)<(sec-floor(sec))?int(ceil(sec)):int(floor(sec));
  if( pad.first == npadsec ) pad.first = 0;
  else if( pad.first > npadsec ) 
    {
      std::cerr<<"TPCBase::FindPad sector error: "<<pad.first<<std::endl;
      pad.first = -1;
    }

  // pad index in sector
  double col = z/(FullLengthZ)*npads;
  pad.second = (ceil(col)-col)<(col-floor(col))?int(ceil(col)):int(floor(col));
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

unsigned int TPCBase::FindAnode(const double phi){
    double AngleAnodeWires = GetAnodePitch();
    double phi_ = phi-phi0;
    if( phi_ < 0. ) phi_ += 2.*M_PI;
    double w = phi_/AngleAnodeWires-0.5;
    uint anode = (ceil(w)-w)<(w-floor(w))?uint(ceil(w)):uint(floor(w));
    if( anode == uint(NanodeWires) ) anode = 0;
    return anode;
}

TPCBase* TPCBase::TPCBaseInstance()
{
  if( !fTPC ) fTPC = new TPCBase;
  return fTPC;
}
