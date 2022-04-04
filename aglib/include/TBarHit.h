#ifndef _TBARHIT_
#define _TBARHIT_

#include "TBarEndHit.h"
#include "TVector3.h"

class TBarHit: public TObject
{
private:
  int fBarID=-1;
  TBarEndHit fTopHit;
  TBarEndHit fBotHit;
  bool fTPCMatched=false;
  TVector3 fTPC;
  double fZed=-9999;

public:
  TBarHit(); // ctor
  TBarHit(const TBarHit &h);
  using TObject::Print;
  virtual void Print();
  virtual ~TBarHit(); // dtor

  void SetBotHit(TBarEndHit* _fBotHit) { fBotHit = *_fBotHit; }
  void SetTopHit(TBarEndHit* _fTopHit) { fTopHit = *_fTopHit; }
  void SetBar(int _fBarID) { fBarID=_fBarID; }
  void SetZed(double _fZed) { fZed=_fZed; }
  void SetTPCHit(TVector3 _fTPC) {
     fTPC=_fTPC;
     fTPCMatched=true;
  }
  
  const TBarEndHit& GetTopHit() const {return fTopHit;}
  const TBarEndHit& GetBotHit() const {return fBotHit;}
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

  double GetPhi() const
  {
	  double offset_angle=TMath::Pi()+0.25698;
     double theta=fBarID*2.*TMath::Pi()/64;
     return theta+offset_angle;
  }
  double GetAverageTDCTime() const
  {
     double t_top = fTopHit.GetTDCTime();
     double t_bot = fBotHit.GetTDCTime();
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
   TVector3 Get3Vector() const
   {
      double x,y;
      this->GetXY(x,y);
      TVector3 bv_point = TVector3(x*1000,y*1000,fZed*1000); // to mm
      return bv_point;
   }
   double GetDistToHit(const TBarHit* h) const
   {
      TVector3 diff = this->Get3Vector() - h->Get3Vector();
      return diff.Mag();
   }
   double GetDPhiToHit(const TBarHit* h) const
   {
      return (h->Get3Vector()).DeltaPhi(this->Get3Vector());
   }
   double GetDZToHit(const TBarHit* h) const
   {
      return h->GetTDCZed() - fZed;
   }
   double GetTOFToHit(const TBarHit* h) const
   {
      return h->GetAverageTDCTime() - this->GetAverageTDCTime();
   }

  ClassDef(TBarHit,4);
};


#endif
