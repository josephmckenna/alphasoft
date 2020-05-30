// TPC Pad class definition
//------------------------------------------------
// Author: A.Capra   Apr. 2016
//------------------------------------------------

#ifndef __TPADS__ 
#define __TPADS__ 1

#include "TTPCelement.hh"
#include <cassert>

class TPads: public TTPCelement
{
private:
  int fPad;
  double fPosZ;
  double fPosRPhi;

  int fPadBins;

public:
  TPads();
  TPads(int);
  TPads( const TPads& );
  virtual ~TPads();
  TPads& operator=( const TPads& );

  inline void SetPad(int p) { 
    assert(p < TPCBase::TPCBaseInstance()->GetNumberOfPads() ); 
    fPad = p; }
  inline int GetPad() const { return fPad;}
  
  inline void SetZ(double z)       { fPosZ = z; }
  inline void SetRphi(double rphi) { fPosRPhi = rphi; }
  inline double GetZ() const         { return fPosZ; }
  inline double GetRphi() const      { return fPosRPhi; }

  int Locate( const double z, const double rphi );
  int Locate( const int pad );

  virtual void SetSignal(double t, double w=1.);
  virtual void SetSignal(std::vector<double> signal);
  virtual void ResetSignal();

  virtual int GetNumberOfSamples() const { return fPadBins; }

  ClassDef(TPads,3)
};

#endif

