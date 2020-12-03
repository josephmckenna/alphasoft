// TPC Anode Wires class implementation
//------------------------------------------------
// Author: A.Capra   Apr. 2016
//------------------------------------------------

#include "TAnode.hh"
#include "ElectronDrift.hh"
#include "TPCBase.hh"

#include <iostream>

extern double gAnodeTime;

TAnode::TAnode():TTPCelement(),fWire(-1),fAnodeBins(411)
{
  fCharge = 0.;
  //  fAnodeBins = fNbins/int(gAnodeTime);
}

TAnode::TAnode(int wire):TTPCelement(),fWire(wire),fAnodeBins(411)
{
  //  fPos = ( double(fWire) + 0.5 ) * gAnglePitch;
  double r;
  TPCBase::TPCBaseInstance()->GetAnodePosition(wire,r,fPos,true);
  fCharge = 0.;

  //  fAnodeBins = fNbins/int(gAnodeTime);
  ResetSignal();
}

TAnode::TAnode( const TAnode& right ):TTPCelement(right),
				      fWire(right.fWire),
				      fPos(right.fPos),
				      fAnodeBins(right.fAnodeBins),
				      fZeds(right.fZeds)
{}

TAnode::~TAnode()
{
  Reset();
}

TAnode& TAnode::operator=( const TAnode& right )
{
  fIDs   = right.fIDs;
  fPDGs  = right.fPDGs;
  fTimes = right.fTimes;

  fWire = right.fWire;
  fPos  = right.fPos;

  fAnodeBins = right.fAnodeBins;

  fZeds = right.fZeds;

  fCharge = right.fCharge;
  SetSignal( right.fSignal );
  // ResetSignal();
  // fSignal=right.fSignal;

  return *this;
}

int TAnode::Locate(const double phi)
{
  fPos = phi;
  // if(fPos<0.) fPos+=TMath::TwoPi();
  // int wire = int( fPos / gAnglePitch - 0.5);
  // if( wire == int(gNwires_d) ) wire = 0;
  int wire = (int) TPCBase::TPCBaseInstance()->FindAnode(fPos);

  int ret=1;
  if( fWire >= 0 && fWire != wire )
    {
      std::cerr<<"Warning! TAnode::Locate( position ): Overriding existing anode. "<<std::endl;
      ret = 1000;
    }

  fWire = wire;

  return fWire*ret;
}

int TAnode::Locate( const int w )
{
  if( w >= int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()) )
    {
      std::cerr<<"Warning! TAnode::Locate( wire ): wire number out of range. "<<std::endl;
      return -1;
    }
  
  int ret=1;
  if( fWire >= 0 && fWire != w )
    {
      std::cerr<<"Warning! TAnode::Locate( wire ): Overriding existing anode. "<<std::endl;
      ret = 1000;
    }

  fWire = w;
  //  fPos = ( double(fWire) + 0.5 ) * gAnglePitch;
  double r;
  TPCBase::TPCBaseInstance()->GetAnodePosition(fWire,r,fPos,true);

  return fWire*ret;
}

void TAnode::SetZed(std::vector<double> z)
{
  fZeds.reserve(z.capacity());
  for( std::vector<double>::iterator it = z.begin() ; it != z.end(); ++it )
    {
      fZeds.push_back( *it );
    }
}

void TAnode::SetSignal(double t, double w)
{
  //  std::cout<<"anode: "<<fWire<<"\t"<<w;
  fCharge+=TMath::Abs(w);
  //  std::cout<<"/"<<fCharge<<std::endl;

  int drift_time_bin = int(t/gAnodeTime),
    bin = 0,
    newbin = int(gAnodeTime);
  //  std::cout<<"TAnode::SetSignal() ADC bin: "<<drift_time_bin<<std::endl;
  for(std::vector<double>::iterator it = fSignal.begin() ; it != fSignal.end(); ++it)
    {
      if( (it - fSignal.begin()) < drift_time_bin )
	continue;
      else
	{
	  //std::cout<<"TAnode::SetSignal() ADC bin: "<<(it - fSignal.begin())<<std::endl;
	  for(int ib=0; ib<newbin; ++ib)
	    {
	      *it+=ElectronDrift::ElectronDriftInstance()->GetAnodeSignal(bin)*w;
	      ++bin;
	    }
	}
    }
}

void TAnode::SetSignal(std::vector<double> signal)
{
  ResetSignal();
  for(int i=0; i<fAnodeBins; ++i)
    fSignal.at(i) = signal.at(i);
}

void TAnode::ResetSignal()
{
  fSignal.resize(fAnodeBins);
  for(unsigned int i=0; i<fSignal.size(); ++i)
    fSignal.at(i)=0.;
}

void TAnode::Reset(bool all)
{
  fIDs.clear();
  fPDGs.clear();
  fTimes.clear();
  fZeds.clear();
  fCharge=0.;
  if(all)
    fSignal.clear();
  else
    ResetSignal();
}

ClassImp(TAnode)
