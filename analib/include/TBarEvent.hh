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

class EndHit: public TObject
{
private:
  int fBarID=-1;
  //TDC data
  double fTDCTime=-1;
  //ADC data'
  double fADCTime=-1;
  double fAmp=-1;
  double fIntegral=-1;
  bool fTDCMatched=false;

public:
  EndHit(); // ctor
  using TObject::Print;
  virtual void Print();
  virtual ~EndHit(); // dtor

  void SetADCHit(int _fBarID, double _fAmp, double _fADCTime, double _fIntegral) 
  {
     fBarID=_fBarID;
     fAmp=_fAmp;
     fADCTime=_fADCTime;
     fIntegral=_fIntegral;
  }
  void SetTDCHit(int _fBarID, double _fTDCTime)
  {
     assert(fBarID==_fBarID);
     fTDCTime=_fTDCTime;
     fTDCMatched=true;
  } 
  
  bool IsTDCMatched() const {return fTDCMatched;}
  int GetBar() const {return fBarID;}
  double GetAmp() const {return fAmp;}
  double GetADCTime() const {return fADCTime;}
  double GetIntegral() const {return fIntegral;}
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
  ClassDef(EndHit, 1);
};

class BarHit: public TObject
{
private:
  int fBarID=-1;
  EndHit* fTopHit;
  EndHit* fBotHit;
  bool fTPCMatched=false;
  TVector3 fTPC;

public:
  BarHit(); // ctor
  using TObject::Print;
  virtual void Print();
  virtual ~BarHit(); // dtor

  void SetBotHit(EndHit* _fBotHit)
  {
     fBarID=_fBotHit->GetBar();
     fBotHit=_fBotHit;
  }
  void SetTopHit(EndHit* _fTopHit)
  {
     assert(fBarID+64==_fTopHit->GetBar());
     fTopHit=_fTopHit;
  }
  void SetTPCHit(TVector3 _fTPC)
  {
     fTPC=_fTPC;
     fTPCMatched=true;
  }
  
  EndHit* GetTopHit() const {return fTopHit;}
  EndHit* GetBotHit() const {return fBotHit;}
  double GetAmpTop() const {return fTopHit->GetAmp();}
  double GetAmpBot() const {return fBotHit->GetAmp();}
  double GetTDCTop() const {return fTopHit->GetTDCTime();}
  double GetTDCBot() const {return fBotHit->GetTDCTime();}
  TVector3 GetTPC() const {return fTPC;}
  bool IsTPCMatched() const {return fTPCMatched;}
  int GetBar() const {return fBarID;}

  double GetPhi()
  {
	  double offset_angle=TMath::Pi()+0.2;
     double theta=fBarID*2.*TMath::Pi()/64;
     return theta+offset_angle;
  }
  double GetAverageTDCTime()
  {
     double t_top = fTopHit->GetTDCTime();
     double t_bot = fBotHit->GetTDCTime();
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
  double GetTDCZed() {
      return (fBotHit->GetTDCTime() - fTopHit->GetTDCTime())*120.8686*1e9/2;
  }
  ClassDef(BarHit, 2);
};


class TBarEvent: public TObject
{
private:
  int fEventID;
  double fEventTime;
  std::vector<BarHit*> fBarHit;
  std::vector<EndHit*> fEndHit;

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
  }
  void AddEndHit(EndHit* e)
  {
    fEndHit.push_back(e);
  }
  void AddBarHit(BarHit* b)
  {
    fBarHit.push_back(b);
  }
  void AddBarHit(EndHit* fBotHit, EndHit* fTopHit)
  {
    BarHit* b = new BarHit;
    b->SetBotHit(fBotHit);
    b->SetTopHit(fTopHit);
    fBarHit.push_back(b);
  }

  void AddADCHit(int fBarID, double fAmp, double fADCTime, double fIntegral)
  {
     EndHit* hit = new EndHit;
     hit->SetADCHit( fBarID, fAmp, fADCTime, fIntegral);
     AddEndHit(hit);
  }

  int GetNBars() { return fBarHit.size(); }
  int GetNEnds() { return fEndHit.size(); }
  std::vector<BarHit*> GetBars() { return fBarHit; }
  std::vector<EndHit*> GetEndHits() { return fEndHit; }

  ClassDef(TBarEvent, 1);
};


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
