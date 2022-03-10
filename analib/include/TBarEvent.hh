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

class SimpleTdcHit: public TObject
{
private:
   int fBarID=-1;
   double fTime=-1;

public:
   SimpleTdcHit(); //ctor
  using TObject::Print;
  virtual void Print();
  virtual ~SimpleTdcHit(); // dtor

  void SetHit(int _fBarID, double _fTime) 
  {
     fBarID=_fBarID;
     fTime=_fTime;
  }

  int GetBar() const {return fBarID;}
  double GetTime() const {return fTime;}

  ClassDef(SimpleTdcHit, 3);
};


class EndHit: public TObject
{
private:
  int fBarID=-1;
  double fTDCTime=-1; // Fully calibrated time
  double fADCTime=-1;
  double fAmp=-1;
  double fAmpRaw=-1; // Without fitting
  bool fTDCMatched=false;

public:
  EndHit(); // ctor
  using TObject::Print;
  virtual void Print();
  virtual ~EndHit(); // dtor

  EndHit(const EndHit& h):
     fBarID(h.fBarID),
    fTDCTime(h.fTDCTime),
    fADCTime(h.fADCTime),
    fAmp(h.fAmp),
    fAmpRaw(h.fAmpRaw),
    fTDCMatched(h.fTDCMatched)
  {
  }
  EndHit& operator=(const EndHit& h)
  {
    fBarID = h.fBarID;
    fTDCTime = h.fTDCTime;
    fADCTime = h.fADCTime;
    fAmp = h.fAmp;
    fAmpRaw = h.fAmpRaw;
    fTDCMatched =  h.fTDCMatched;
    return *this;
  }
  void SetADCHit(int _fBarID, double _fAmp, double _fAmpRaw, double _fADCTime) 
  {
     fBarID=_fBarID;
     fAmp=_fAmp;
     fAmpRaw=_fAmpRaw;
     fADCTime=_fADCTime;
  }

  void SetTDCHit(double _fTDCTime)
  {
     fTDCTime=_fTDCTime;
     fTDCMatched=true;
  } 

  bool IsTDCMatched() const {return fTDCMatched;}
  int GetBar() const {return fBarID;}
  double GetAmp() const {return fAmp;}
  double GetAmpRaw() const {return fAmpRaw;}
  double GetADCTime() const {return fADCTime;}
  double GetTDCTime() const {return fTDCTime; }
  void GetXY(double &x, double &y)
  {
	  double r=(.223+.243)/2.;
	  double offset_angle=TMath::Pi()+0.2;
      double theta=fBarID*2.*TMath::Pi()/64; //Degrees
      x=r*TMath::Cos(theta + offset_angle);
      y=r*TMath::Sin(theta + offset_angle);
      return;
  }
  ClassDef(EndHit, 3);
};


class BarHit: public TObject
{
private:
  int fBarID=-1;
  EndHit fTopHit;
  EndHit fBotHit;
  bool fTPCMatched=false;
  TVector3 fTPC;
  double fZed=-9999;

public:
  BarHit(); // ctor
  using TObject::Print;
  virtual void Print();
  virtual ~BarHit(); // dtor

  void SetBotHit(EndHit* _fBotHit) { fBotHit = *_fBotHit; }
  void SetTopHit(EndHit* _fTopHit) { fTopHit = *_fTopHit; }
  void SetBar(int _fBarID) { fBarID=_fBarID; }
  void SetZed(double _fZed) { fZed=_fZed; }
  void SetTPCHit(TVector3 _fTPC) {
     fTPC=_fTPC;
     fTPCMatched=true;
  }
  
  const EndHit& GetTopHit() const {return fTopHit;}
  const EndHit& GetBotHit() const {return fBotHit;}
  double GetAmpTop() const {return fTopHit.GetAmp();}
  double GetAmpBot() const {return fBotHit.GetAmp();}
  double GetAmpRawTop() const {return fTopHit.GetAmpRaw();}
  double GetAmpRawBot() const {return fBotHit.GetAmpRaw();}
  double GetTDCTop() const {return fTopHit.GetTDCTime();}
  double GetTDCBot() const {return fBotHit.GetTDCTime();}
  TVector3 GetTPC() const {return fTPC;}
  bool IsTPCMatched() const {return fTPCMatched;}
  int GetBar() const {return fBarID;}
  double GetTDCZed() const {return fZed;}

  double GetPhi()
  {
	  double offset_angle=TMath::Pi()+0.2;
     double theta=fBarID*2.*TMath::Pi()/64;
     return theta+offset_angle;
  }
  double GetAverageTDCTime()
  {
     double t_top = fTopHit.GetTDCTime();
     double t_bot = fBotHit.GetTDCTime();
     return (t_top + t_bot)/2.;
  }
  void GetXY(double &x, double &y)
  {
	  double r=(.223+.243)/2.;
	  double offset_angle=TMath::Pi()+0.2;
      double theta=fBarID*2.*TMath::Pi()/64; //Degrees
      x=r*TMath::Cos(theta + offset_angle);
      y=r*TMath::Sin(theta + offset_angle);
      return;
  }
//  double GetTDCZed() { // This should probably not be done here. The value of the speed of light should be put into the analysis settings, and this should be done in the tdc module.
//      return (fBotHit->GetTDCTime() - fTopHit->GetTDCTime())*120.8686*1e9/2;
//  }
  ClassDef(BarHit,3);
};


class TBarEvent: public TObject
{
private:
  int fEventID;
  double fEventTime;
  std::vector<BarHit*> fBarHit;
  std::vector<EndHit*> fEndHit;
  std::vector<SimpleTdcHit*> fTdcHit;

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
    fEventID=-1;
    fEventTime=-1.;
  }
  void AddEndHit(EndHit* e)
  {
    fEndHit.push_back(e);
  }
  void AddBarHit(BarHit* b)
  {
    fBarHit.push_back(b);
  }
  void AddTdcHit(SimpleTdcHit* b)
  {
    fTdcHit.push_back(b);
  }
  void AddBarHit(EndHit* fBotHit, EndHit* fTopHit, int fBarID)
  {
    BarHit* b = new BarHit;
    b->SetBotHit(fBotHit);
    b->SetTopHit(fTopHit);
    b->SetBar(fBarID);
    fBarHit.push_back(b);
  }
  void AddTdcHit(int fBarID, double fTime)
  {
    SimpleTdcHit* hit = new SimpleTdcHit;
    hit->SetHit(fBarID, fTime);
    AddTdcHit(hit);
  }
  void AddADCHit(int fBarID, double fAmp, double fAmpRaw, double fADCTime)
  {
     EndHit* hit = new EndHit;
     hit->SetADCHit( fBarID, fAmp, fAmpRaw, fADCTime);
     AddEndHit(hit);
  }
  void AddADCHit(int fBarID, double fAmp, double fADCTime)
  {
     EndHit* hit = new EndHit;
     hit->SetADCHit( fBarID, fAmp, fAmp, fADCTime);
     AddEndHit(hit);
  }

  int GetNBars() { return fBarHit.size(); }
  int GetNEnds() { return fEndHit.size(); }
  std::vector<BarHit*> GetBars() { return fBarHit; }
  std::vector<EndHit*> GetEndHits() { return fEndHit; }
  std::vector<SimpleTdcHit*> GetTdcHits() { return fTdcHit; }

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
