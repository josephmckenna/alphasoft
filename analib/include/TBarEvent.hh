#ifdef BUILD_AG
#ifndef __TBAREVENT__
#define __TBAREVENT__ 1


#include <iostream>
#include <vector>
#include "assert.h"
#include "TMath.h"
#include <TVector3.h>
#include <TVector.h>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

class SimpleTdcHit: public TObject
{
private:
   int fBarID=-1;
   double fTime=-1;
   int fFineTimeCount=-1;
   double fFineTime=-1;

public:
   SimpleTdcHit(); //ctor
   SimpleTdcHit(const SimpleTdcHit &tdchit);
  using TObject::Print;
  virtual void Print();
  virtual ~SimpleTdcHit(); // dtor

  void SetHit(int _fBarID, double _fTime, int _fFineTimeCount, double _fFineTime) 
  {
     fBarID=_fBarID;
     fTime=_fTime;
     fFineTimeCount=_fFineTimeCount;
     fFineTime=_fFineTime;
  }

  int GetBar() const {return fBarID;}
  double GetTime() const {return fTime;}
  int GetFineTimeCount() const {return fFineTimeCount;}
  double GetFineTime() const {return fFineTime;}

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
  EndHit(const EndHit &endhit); //Copy ctor
  using TObject::Print;
  virtual void Print();
  virtual ~EndHit(); // dtor

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
  double fTopTDCTime=-1; // Fully calibrated time
  double fTopADCTime=-1;
  double fTopAmp=-1;
  double fBotTDCTime=-1; // Fully calibrated time
  double fBotADCTime=-1;
  double fBotAmp=-1;
  bool fTPCMatched=false;
  TVector3 fTPC;
  double fZed=-9999;

public:
  BarHit(); // ctor
  BarHit(const BarHit &barhit); //copy ctor
  using TObject::Print;
  virtual void Print();
  virtual ~BarHit(); // dtor

  void SetBotHit(EndHit* _fBotHit) { 
     fBotTDCTime = _fBotHit->GetTDCTime();
     fBotADCTime = _fBotHit->GetADCTime();
     fBotAmp = _fBotHit->GetAmp();
  }
  void SetTopHit(EndHit* _fTopHit) { 
     fTopTDCTime = _fTopHit->GetTDCTime();
     fTopADCTime = _fTopHit->GetADCTime();
     fTopAmp = _fTopHit->GetAmp();
  }
  void SetBar(int _fBarID) { fBarID=_fBarID; }
  void SetZed(double _fZed) { fZed=_fZed; }
  void SetTPCHit(TVector3 _fTPC) {
     fTPC=_fTPC;
     fTPCMatched=true;
  }
  
  double GetAmpTop() const {return fTopAmp;}
  double GetAmpBot() const {return fBotAmp;}
  double GetTDCTop() const {return fTopTDCTime;}
  double GetTDCBot() const {return fBotTDCTime;}
  double GetADCTop() const {return fTopADCTime;}
  double GetADCBot() const {return fBotADCTime;}
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
  double GetAverageTDCTime() const
  {
     return (fTopTDCTime + fBotTDCTime)/2.;
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
  TVector3 Get3Vector()
  {
     double x,y;
     this->GetXY(x,y);
     TVector3 bv_point = TVector3(x*1000,y*1000,fZed*1000); // to mm
     return bv_point;
  }
  double GetDistToHit(BarHit* h)
  {
     TVector3 diff = this->Get3Vector() - h->Get3Vector();
     return diff.Mag();
  }
  double GetDPhiToHit(BarHit* h)
  {
     return (h->Get3Vector()).DeltaPhi(this->Get3Vector());
  }
  double GetDZToHit(BarHit* h)
  {
     return h->GetTDCZed() - fZed;
  }
  double GetTOFToHit(BarHit* h)
  {
     return h->GetAverageTDCTime() - this->GetAverageTDCTime();
  }
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
  void AddTdcHit(int fBarID, double fTime, int fFineTimeCount, double fFineTime)
  {
    SimpleTdcHit* hit = new SimpleTdcHit;
    hit->SetHit(fBarID, fTime, fFineTimeCount, fFineTime);
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



  int GetNBars() const { return fBarHit.size(); }
  int GetNEnds() const { return fEndHit.size(); }
  int GetNTDC() const { return fTdcHit.size(); }
  std::vector<BarHit*> GetBars() const { return fBarHit; }
  std::vector<EndHit*> GetEndHits() const { return fEndHit; }
  std::vector<SimpleTdcHit*> GetTdcHits() const { return fTdcHit; }

  std::vector<double> GetTOFs()
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
