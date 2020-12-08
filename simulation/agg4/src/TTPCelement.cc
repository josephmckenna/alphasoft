// TPC Element class implementation
//------------------------------------------------
// Author: A.Capra   Apr. 2016
//------------------------------------------------

#include "TTPCelement.hh"
#include "TString.h"

TTPCelement::TTPCelement( const TTPCelement& right ):TObject(),
						     fIDs(right.fIDs),
						     fPDGs(right.fPDGs),
						     fTimes(right.fTimes),
						     fCharge(right.fCharge),
						     fSignal(right.fSignal)
						     
{
  //  ResetSignal();
}

TTPCelement& TTPCelement::operator=( const TTPCelement& right )
{
  fIDs   = right.fIDs;
  fPDGs  = right.fPDGs;
  fTimes = right.fTimes;
  
  fCharge = right.fCharge;
  ResetSignal();
  fSignal=right.fSignal;
  return *this;
}

TTPCelement::~TTPCelement()
{
  Reset();
}

double TTPCelement::Increment()
{
  ++fCharge;  
  return fCharge;
}

double TTPCelement::Increment(double t)
{
  AddDriftTime(t);
  SetSignal(t);
  return fCharge;
}

double TTPCelement::Increment(double t, double w)
{
  AddDriftTime(t);
  SetSignal(t,w);
  return fCharge;
}

double TTPCelement::Increment(int id, int pdg, double t)
{
  AddDriftTime(t);
  SetSignal(t);
  AddTrackPDG(pdg);
  AddTrackID(id);
  return fCharge;
}

void TTPCelement::SetSignal(double, double)
{
  ++fCharge;
  for(std::vector<double>::iterator it = fSignal.begin() ; it != fSignal.end(); ++it)
    *it+=0.;
}

void TTPCelement::SetSignal(std::vector<double> signal)
{
  ResetSignal();
  // for(std::vector<double>::iterator it = signal.begin() ; it != signal.end(); ++it)
  //   fSignal.at(it-signal.begin()) = *it;
  fSignal = signal;
}

void TTPCelement::ResetSignal()
{
  //  fSignal.resize(fNbins);
  for(unsigned int i=0; i<fSignal.size(); ++i)
    fSignal.at(i)=0.;
}

void TTPCelement::Reset(bool all)
{
  fIDs.clear();
  fPDGs.clear();
  fTimes.clear();
  fCharge=0.;
  if(all)
    fSignal.clear();
  else
    {
      ResetSignal();
      fTimes.push_back(0.);
    }
}

ClassImp(TTPCelement)
