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
  double fZedTDC=-1;

  //ADC data
  double fAmpTop=-1;
  double fAmpBot=-1;
  double fADCTimeTop=-1;
  double fADCTimeBot=-1;
  double fZedADC=-1;


public:
  BarHit(); // ctor
  using TObject::Print;
  virtual void Print();
  virtual ~BarHit(); // dtor
  
  void SetADCHit(int _fBarID, double _fAmpTop, double _fAmpBot, double _fTimeTop, double _fTimeBot, double _fZedADC)
  {
     fBarID=_fBarID;
     fAmpTop=_fAmpTop;
     fAmpBot=_fAmpBot;
     fADCTimeTop=_fTimeTop;
     fADCTimeBot=_fTimeBot;
     fZedADC=_fZedADC;
  }
  void SetTDCHit(int _fBarID, double _fTimeTop, double _fTimeBot)
  {
     assert(fBarID==_fBarID);
     fTimeTop=_fTimeTop;
     fTimeBot=_fTimeBot;
     CalculateZed();
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
  double GetTDCTop() const {return fTimeTop; }
  double GetTDCBot() const {return fTimeBot; }
  void CalculateZed();
  void GetXY(double &x, double &y)
  {
	  double r=0.3; //meters? 
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
  void AddADCHit(int fBarID,double fAmpTop, double fAmpBot,double fTimeTop, double fTimeBot, double fZedADC)
  {
     BarHit hit;
     hit.SetADCHit( fBarID, fAmpTop, fAmpBot,fTimeTop, fTimeBot, fZedADC);
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
