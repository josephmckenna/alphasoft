#ifndef __TBAREVENT__
#define __TBAREVENT__ 1


#include <iostream>
#include <vector>
#ifndef ROOT_TObject
#include "TObject.h"
#endif


class BarHit
{
private:
  int fBarID;
  double fTimeTop;
  double fTimeBot;
  double fAmpTop;
  double fAmpBot;
  double fZed;

public:
  BarHit(); // ctor?
  double GetZed() const {return fZed;}
  int GetBar() const {return fBarID;}
};


class TBarEvent: public TObject
{
private:
  int fEventID;
  double fEventTime;
  std::vector<BarHit> fBarHit;

public:
  TBarEvent(); //ctor
  //std::vector<BarHit> GetHitsVector() const {return fBarHit;} 

};


#endif
