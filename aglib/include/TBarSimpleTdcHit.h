#ifndef _TBARSIMPLETDCHIT_
#define _TBARSIMPLETDCHIT_
#include "TObject.h"
#include <iostream>

class TBarSimpleTdcHit: public TObject
{
private:
   int fBarID=-1;
   double fTime=-1;

public:
   TBarSimpleTdcHit(); //ctor
  using TObject::Print;
  virtual void Print();
  virtual ~TBarSimpleTdcHit(); // dtor

  void SetHit(int _fBarID, double _fTime) 
  {
     fBarID=_fBarID;
     fTime=_fTime;
  }

  int GetBar() const {return fBarID;}
  double GetTime() const {return fTime;}

  ClassDef(TBarSimpleTdcHit, 3);
};
#endif