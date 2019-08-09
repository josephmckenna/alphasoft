#ifndef __TBAREVENT__
#define __TBAREVENT__ 1


#include <iostream>
#include <vector>
#include "assert.h"
#include "TMath.h"
#ifndef ROOT_TObject
#include "TObject.h"
#endif


class BarHit: public TObject
{
private:
  int fBarID=-1;
  //TDC data
  double fTimeTop=-1;
  double fTimeBot=-1;
  double fZedTDC=-999;

  //ADC data
  double fAmpTop=-1;
  double fAmpBot=-1;
  double fADCTimeTop=-1;
  double fADCTimeBot=-1;
  double fIntegralTop=-1;
  double fIntegralBot=-1;
  double fZedADC=-999.;


public:
  BarHit(); // ctor
  using TObject::Print;
  virtual void Print();
  virtual ~BarHit(); // dtor
  
  void SetADCHit(int _fBarID, double _fAmpTop, double _fAmpBot, double _fTimeTop, double _fTimeBot, double _fIntegralTop, double _fIntegralBot)
  {
     fBarID=_fBarID;
     fAmpTop=_fAmpTop;
     fAmpBot=_fAmpBot;
     fADCTimeTop=_fTimeTop;
     fADCTimeBot=_fTimeBot;
     fIntegralTop=_fIntegralTop;
     fIntegralBot=_fIntegralBot;
     fZedADC=CalculateZed(_fTimeTop,_fTimeBot);
  }
  void SetTDCHit(int _fBarID, double _fTimeTop, double _fTimeBot)
  {
     assert(fBarID==_fBarID);
     fTimeTop=_fTimeTop;
     fTimeBot=_fTimeBot;
     fZedTDC=CalculateZed(_fTimeTop,_fTimeBot);
  } 

  void SetZedTdc(double _fZedTDC)
  {
    fZedTDC=_fZedTDC;
  }
  //void SetTDCHit(int _fBarID, double _fAmpTop, double _fAmpBot)
  double GetADCZed() const {return fZedADC;}
  double GetTDCZed() const {return fZedTDC;}
  int GetBar() const {return fBarID;}
  double GetAmpTop() const {return fAmpTop;}
  double GetAmpBot() const {return fAmpBot;}
  double GetADCTimeTop() const {return fADCTimeTop;}
  double GetADCTimeBot() const {return fADCTimeBot;}
  double GetIntegralTop() const {return fIntegralTop;}
  double GetIntegralBot() const {return fIntegralBot;}
  double GetTDCTop() const {return fTimeTop; }
  double GetTDCBot() const {return fTimeBot; }
  double CalculateZed( double _TimeTop, double _TimeBot );
  void GetXY(double &x, double &y)
  {
	  double r=(.223+.243)/2.;
	  double offset_angle=TMath::Pi()+0.2;
      double theta=fBarID*2.*TMath::Pi()/64; //Degrees
      x=r*TMath::Cos(theta + offset_angle);
      y=r*TMath::Sin(theta + offset_angle);
      return;
  }
  ClassDef(BarHit, 1);
};


class TBarEvent: public TObject
{
private:
  int fEventID;
  double fEventTime;
  std::vector<BarHit> fBarHit;

public:
  TBarEvent(); //ctor
  using TObject::Print;
  virtual void Print();
  virtual ~TBarEvent(); //dtor
  
  void SetID(int ID){ fEventID=ID;}
  void SetRunTime(double time){ fEventTime=time;}
  void Reset()
  {
    fEventID=-1;
    fEventTime=-1.;
    fBarHit.clear();
  }
  void AddHit(BarHit b)
  {
    fBarHit.push_back(b);
  }
  void AddADCHit(int fBarID, double fAmpTop, double fAmpBot, double fTimeTop, double fTimeBot, double fIntegralTop, double fIntegralBot)
  {
     BarHit hit;
     hit.SetADCHit( fBarID, fAmpTop, fAmpBot, fTimeTop, fTimeBot, fIntegralTop, fIntegralBot);
     AddHit(hit);
  }
  void AddTDCHit(int fBarID, double fTimeTop, double fTimeBot)
  {
     BarHit hit;
     hit.SetTDCHit( fBarID, fTimeTop, fTimeBot);
     AddHit(hit);
  }

  int GetNBars() { return fBarHit.size(); }
  std::vector<BarHit>* GetBars() { return &fBarHit; }
  //std::vector<BarHit> GetHitsVector() const {return fBarHit;} 

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
