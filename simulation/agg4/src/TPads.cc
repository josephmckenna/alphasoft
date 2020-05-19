// TPC Pads class implementation
//------------------------------------------------
// Author: A.Capra   Apr. 2016
//------------------------------------------------

#include "TPads.hh"
#include "TElectronDrift.hh"
#include "TPCBase.hh"
#include <cmath>
#include <cassert>
#include <iostream>

extern double gPadTime;

TPads::TPads():TTPCelement(),fPad(-1),fPadBins(411)
{
  fCharge = 0.;
  //  fPadBins = fNbins/int(gPadTime); 
}

TPads::TPads(int pad):TTPCelement(),fPad(pad),fPadBins(411)
{ 
  TPCBase::TPCBaseInstance()->GetPadPosition(fPad, fPosZ, fPosRPhi);
  fPosZ *= 10.; // cm -> mm
  // std::cout<<"TPads::TPads( "<<fPad
  // 	   <<" ) z: "<<fPosZ
  // 	   <<" mm   phi: "<<fPosRPhi
  // 	   <<" rad = "<<fPosRPhi*TMath::RadToDeg()<<" deg"<<std::endl;
  fPosRPhi *= TPCBase::TPCBaseInstance()->GetROradius(true);

  fCharge = 0.;

  //  fPadBins = fNbins/int(gPadTime); 
 
  ResetSignal();
}

TPads::TPads( const TPads& right ):TTPCelement(right),
				   fPad(right.fPad),
				   fPosZ(right.fPosZ),fPosRPhi(right.fPosRPhi),
				   fPadBins(right.fPadBins)
{}

TPads::~TPads()
{
  Reset();
}

TPads& TPads::operator=( const TPads& right )
{
  fIDs   = right.fIDs;
  fPDGs  = right.fPDGs;
  fTimes = right.fTimes;

  fPad     = right.fPad;
  fPosZ    = right.fPosZ;
  fPosRPhi = right.fPosRPhi;

  fPadBins = right.fPadBins;

  fCharge = right.fCharge;
  ResetSignal();
  fSignal=right.fSignal;
  return *this;
}

int TPads::Locate(const double zed, const double phi )
{  
  auto p = TPCBase::TPCBaseInstance()->FindPad(zed*0.1, phi);
  int pad = TPCBase::TPCBaseInstance()->SectorAndPad2Index(p.first, p.second);

  int ret=1;
  if( fPad >=0 && fPad != pad )
    {
      std::cerr<<"Warning! TPads::Locate( z, phi ): Overriding existing pad. "<<std::endl;
      ret = 100000;
    }
  fPad = pad;

  TPCBase::TPCBaseInstance()->GetPadPosition(fPad, fPosZ, fPosRPhi);
  fPosZ *= 10.; // cm -> mm
  fPosRPhi *= TPCBase::TPCBaseInstance()->GetROradius(true);

  return fPad*ret;
}

int TPads::Locate(const int pad )
{
  if( pad >= TPCBase::TPCBaseInstance()->GetNumberOfPads() )
    {
      std::cerr<<"Warning! TPads::Locate( pad ): pad number out of range. "<<std::endl;
      return -1;
    }
  int ret=1;
  if( fPad >=0 && fPad != pad )
    {
      std::cerr<<"Warning! TPads::Locate( pad ): Overriding existing pad. "<<std::endl;
      ret = 100000;
    }

  fPad = pad;

  TPCBase::TPCBaseInstance()->GetPadPosition(fPad, fPosZ, fPosRPhi);
  fPosZ *= 10.; // cm -> mm
  fPosRPhi *= TPCBase::TPCBaseInstance()->GetROradius()*10.;

  return fPad*ret;
}

void TPads::SetSignal(double t, double w)
{
  fCharge+=w;

  int drift_time_bin = int(t/gPadTime),
    bin = 0,
    newbin = int(gPadTime);
  for(std::vector<double>::iterator it = fSignal.begin() ; it != fSignal.end(); ++it)
    {
      if( (it - fSignal.begin()) < drift_time_bin )
	continue;
      else
	{
	  for(int ib=0; ib<newbin; ++ib)
	    {
	      *it+=TElectronDrift::ElectronDriftInstance()->GetPadSignal(bin)*w;
	      ++bin;
	    }
	}
    }
}

void TPads::SetSignal(std::vector<double> signal)
{
  ResetSignal();
  //  std::cout<<"TPads::SetSignal signal size: "<<signal.size()<<"\t"<<fSignal.size()<<"\n";
  for(int i=0; i<fPadBins; ++i)
    {
      //      std::cout<<i<<"\t"<<signal.at(i)<<"\t";
      fSignal.at(i) = signal.at(i);
      //      std::cout<<fSignal.at(i)<<"\n";
    }
}

void TPads::ResetSignal()
{
  fSignal.resize(fPadBins);
  for(unsigned int i=0; i<fSignal.size(); ++i)
    fSignal.at(i)=0.;
}

ClassImp(TPads)
