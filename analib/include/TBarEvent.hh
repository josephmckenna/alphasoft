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
  double fAmpTop;
  double fAmpBot;
  double fZedAnalogue;
  double fZedTDC;

public:
  BarHit(); // ctor?
  virtual ~BarHit(); // dtor?
  void SetHit(int _fBarID,double _fTimeTop, double _fTimeBot, 
         double _fAmpTop, double _fAmpBot, double _fZed)
  {
     fBarID=_fBarID;
     fTimeTop=_fTimeTop;
     fTimeBot=_fTimeBot;
     fAmpTop=_fAmpTop;
     fAmpBot=_fAmpBot;
     fZed=_fZed;
  } 
  double GetZed() const {return fZed;}
  int GetBar() const {return fBarID;}
  void CalculateZed()
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
  void AddHit(int fBarID,double fTimeTop, double fTimeBot, double fAmpTop, double fAmpBot, double fZed)
  {
     BarHit hit;
     hit.SetHit( fBarID, fTimeTop, fTimeBot, fAmpTop, fAmpBot, fZed);
     AddHit(hit);
  }
  int GetNBars() { return fBarHit.size(); }
  //std::vector<BarHit> GetHitsVector() const {return fBarHit;} 
  ClassDef(TBarEvent, 1);
};


#endif
