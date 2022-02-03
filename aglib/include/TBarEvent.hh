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
  using TObject::Print;
  virtual void Print();
  virtual ~TBarEvent(); //dtor
  
  void SetID(int ID){ fEventID=ID;}
  void SetRunTime(double time){ fEventTime=time;}
  int GetID(){ return fEventID;}
  double GetRunTime(){ return fEventTime;}
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
  void AddTdcHit(int fBarID, double fTime)
  {
    TBarSimpleTdcHit* hit = new TBarSimpleTdcHit;
    hit->SetHit(fBarID, fTime);
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
  std::vector<TBarHit*> GetBars() { return fBarHit; }
  std::vector<TBarEndHit*> GetEndHits() { return fEndHit; }
  std::vector<TBarSimpleTdcHit*> GetTdcHits() { return fTdcHit; }

  ClassDef(TBarEvent, 1);
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
