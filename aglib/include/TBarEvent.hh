#ifdef BUILD_AG
#ifndef __TBAREVENT__
#define __TBAREVENT__ 1


#include <iostream>
#include <vector>
#include "assert.h"
#include "TMath.h"
#include <TVector3.h>
#ifndef ROOT_TObject
#include "TObject.h"
#endif


#include "TBarEndHit.h"
#include "TBarSimpleTdcHit.h"
#include "TBarHit.h"

class TBarEvent: public TObject
{
private:
  int fEventID;
  double fEventTime;
  std::vector<TBarHit*> fBarHit;
  std::vector<TBarEndHit*> fEndHit;
  std::vector<TBarSimpleTdcHit*> fTdcHit;

public:
  TBarEvent(); //ctor
  TBarEvent(TBarEvent &barEvent); //copy ctor
  using TObject::Print;
  virtual void Print();
  virtual ~TBarEvent(); //dtor
  
  void SetID(int ID){ fEventID=ID;}
  void SetRunTime(double time){ fEventTime=time;}
  int GetID() const { return fEventID;}
  double GetRunTime() const { return fEventTime;}
  void Reset()
  {
    fEventID=-1;
    fEventTime=-1.;
    for (auto hit: fEndHit) delete hit;
    fEndHit.clear();
    for (auto hit: fBarHit) delete hit;
    fBarHit.clear();
    for (auto hit: fTdcHit) delete hit;
    fTdcHit.clear();
  }
  void AddEndHit(TBarEndHit* e)
  {
    fEndHit.push_back(e);
  }
  void AddBarHit(TBarHit* b)
  {
    fBarHit.push_back(b);
  }
  void AddTdcHit(TBarSimpleTdcHit* b)
  {
    fTdcHit.push_back(b);
  }
  void AddBarHit(TBarEndHit* fBotHit, TBarEndHit* fTopHit, int fBarID)
  {
    TBarHit* b = new TBarHit;
    b->SetBotHit(fBotHit);
    b->SetTopHit(fTopHit);
    b->SetBar(fBarID);
    fBarHit.push_back(b);
  }
  void AddTdcHit(int fBarID, double fTime, int fFineTimeCount, double fFineTime, double fCoarseTime)
  {
    TBarSimpleTdcHit* hit = new TBarSimpleTdcHit;
    hit->SetHit(fBarID, fTime, fFineTimeCount, fFineTime, fCoarseTime);
    AddTdcHit(hit);
  }
  void AddADCHit(int fBarID, double fAmp, double fAmpRaw, double fADCTime)
  {
     TBarEndHit* hit = new TBarEndHit;
     hit->SetADCHit( fBarID, fAmp, fAmpRaw, fADCTime);
     AddEndHit(hit);
  }
  void AddADCHit(int fBarID, double fAmp, double fADCTime)
  {
     TBarEndHit* hit = new TBarEndHit;
     hit->SetADCHit( fBarID, fAmp, fAmp, fADCTime);
     AddEndHit(hit);
  }

  int GetNBars() const { return fBarHit.size(); }
  int GetNEnds() const { return fEndHit.size(); }
  int GetNTDC() const { return fTdcHit.size(); }
  std::vector<TBarHit*> GetBars() { return fBarHit; }
  const std::vector<TBarHit*> GetBars() const { return fBarHit; }
  std::vector<TBarEndHit*> GetEndHits() { return fEndHit; }
  const std::vector<TBarEndHit*> GetEndHits() const { return fEndHit; }
  std::vector<TBarSimpleTdcHit*> GetTdcHits() { return fTdcHit; }
  const std::vector<TBarSimpleTdcHit*> GetTdcHits() const { return fTdcHit; }

  std::vector<double> GetTOFs() const
  {
     std::vector<double> TOFs;
     if (fBarHit.size()<2) return TOFs;
     for (int i=0; i<fBarHit.size()-1; i++) {
        for (int j=i+1; j<fBarHit.size(); j++) {
           TOFs.push_back( TMath::Abs( (fBarHit.at(i))->GetTOFToHit(fBarHit.at(j)) ));
        }
     }
     return TOFs;
  }
  std::vector<double> GetDists()
  {
     std::vector<double> Dists;
     if (fBarHit.size()<2) return Dists;
     for (int i=0; i<fBarHit.size()-1; i++) {
        for (int j=i+1; j<fBarHit.size(); j++) {
           Dists.push_back( (fBarHit.at(i))->GetDistToHit(fBarHit.at(j)) );
        }
     }
     return Dists;
  }

  ClassDef(TBarEvent, 2);
};


#endif
#endif
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
