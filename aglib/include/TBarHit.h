#ifndef _TBARHIT_
#define _TBARHIT_

#include "TBarEndHit.h"
#include "TVector3.h"

class TBarHit: public TObject
{
private:
  int fBarID=-1;
  TBarEndHit* fTopHit;
  TBarEndHit* fBotHit;
  bool fTPCMatched=false;
  TVector3 fTPC;
  double fZed=-9999;

public:
  TBarHit(); // ctor
  using TObject::Print;
  virtual void Print();
  virtual ~TBarHit(); // dtor

  void SetBotHit(TBarEndHit* _fBotHit) { fBotHit=_fBotHit; }
  void SetTopHit(TBarEndHit* _fTopHit) { fTopHit=_fTopHit; }
  void SetBar(int _fBarID) { fBarID=_fBarID; }
  void SetZed(double _fZed) { fZed=_fZed; }
  void SetTPCHit(TVector3 _fTPC) {
     fTPC=_fTPC;
     fTPCMatched=true;
  }
  
  TBarEndHit* GetTopHit() const {return fTopHit;}
  TBarEndHit* GetBotHit() const {return fBotHit;}
  double GetAmpTop() const {return fTopHit->GetAmp();}
  double GetAmpBot() const {return fBotHit->GetAmp();}
  double GetAmpRawTop() const {return fTopHit->GetAmpRaw();}
  double GetAmpRawBot() const {return fBotHit->GetAmpRaw();}
  double GetTDCTop() const {return fTopHit->GetTDCTime();}
  double GetTDCBot() const {return fBotHit->GetTDCTime();}
  TVector3 GetTPC() const {return fTPC;}
  bool IsTPCMatched() const {return fTPCMatched;}
  int GetBar() const {return fBarID;}
  double GetTDCZed() const {return fZed;}

  double GetPhi() const
  {
	  double offset_angle=TMath::Pi()+0.2;
     double theta=fBarID*2.*TMath::Pi()/64;
     return theta+offset_angle;
  }
  double GetAverageTDCTime() const
  {
     double t_top = fTopHit->GetTDCTime();
     double t_bot = fBotHit->GetTDCTime();
     return (t_top + t_bot)/2.;
  }
  void GetXY(double &x, double &y) const
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
  ClassDef(TBarHit,3);
};


#endif