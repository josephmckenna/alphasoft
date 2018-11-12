#ifndef __TBAREVENT__
#define __TBAREVENT__ 1


#include <iostream>
#include <vector>
#ifndef ROOT_TObject
#include "TObject.h"
#endif


class BarHit: public TObject
{
private:
  int fBarID;
  double fTimeTop;
  double fTimeBot;
  double fZedTDC;

  double fAmpTop;
  double fAmpBot;
  double fZedADC;


public:
  BarHit(); // ctor?
  virtual ~BarHit(); // dtor?
  void SetADCHit(int _fBarID, double _fAmpTop, double _fAmpBot, double _fTimeTop, double _fTimeBot, double _fZedADC)
  {
     fBarID=_fBarID;
     fAmpTop=_fAmpTop;
     fAmpBot=_fAmpBot;
     fTimeTop=_fTimeTop;
     fTimeBot=_fTimeBot;
     fZedADC=_fZedADC;
  } 
  //void SetTDCHit(int _fBarID, double _fAmpTop, double _fAmpBot)
  double GetADCZed() const {return fZedADC;}
  double GetTDCZed() const {return fZedTDC;}
  int GetBar() const {return fBarID;}
  void CalculateZed();
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
  std::vector<BarHit> GetBars() { return fBarHit; }
  //std::vector<BarHit> GetHitsVector() const {return fBarHit;} 
  ClassDef(TBarEvent, 1);
};


#endif
