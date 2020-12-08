// TPC Element class definition
//------------------------------------------------
// Author: A.Capra   Apr. 2016
//------------------------------------------------

#ifndef __TTPCELEMENT__ 
#define __TTPCELEMENT__ 1

#include <TObject.h>
#include <TH1D.h>
#include <TMath.h>
#include <vector>

#include "TPCBase.hh"

class TString;
class TTPCelement: public TObject
{
protected:
  std::vector<int> fIDs;
  std::vector<int> fPDGs;
  std::vector<double> fTimes;

  double fCharge;
  std::vector<double> fSignal;

public:
  TTPCelement() {};
  TTPCelement( const TTPCelement& );
  virtual ~TTPCelement();
  TTPCelement& operator=( const TTPCelement& );

  inline virtual void SetDriftTimes(std::vector<double> times) { fTimes=times; }

  inline std::vector<int> GetTrackID()       const { return fIDs;   }
  inline std::vector<int> GetTrackPDG()      const { return fPDGs;  }
  inline std::vector<double> GetDriftTimes() const { return fTimes; }

  inline double GetLeadingEdgeDriftTime() const 
  { 
    if( fTimes.size() > 0 )
      return *std::min_element(fTimes.begin(), fTimes.end());
    else
      return 0.;
  };

  inline void AddTrackID(int id)     { fIDs.push_back(id);   }
  inline void AddTrackPDG(int pdg)   { fPDGs.push_back(pdg); }
  inline void AddDriftTime(double t) { fTimes.push_back(t);  }

  inline void SetCharge(double q) { fCharge = q; }
  inline double GetCharge() const { return fCharge; }

  virtual double Increment();
  virtual double Increment(double);
  virtual double Increment(double, double);
  virtual double Increment(int, int, double);
  void SortDriftTime() {std::sort(fTimes.begin(), fTimes.end());}

  inline virtual const std::vector<double> GetSignal() const { return fSignal; }
  virtual void SetSignal(double t, double w=1.);
  virtual void SetSignal(std::vector<double> signal);// { fSignal = signal; }
  virtual void ResetSignal();

  virtual int GetNumberOfSamples() const { return 0; }
  virtual double GetSample(int i) const  { return fSignal.at(i); }

  virtual int Analyze() {return 0;}

  virtual void Reset(bool all=true);

  ClassDef(TTPCelement,3)
};

#endif
