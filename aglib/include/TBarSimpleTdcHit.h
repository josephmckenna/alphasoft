#ifndef _TBARSIMPLETDCHIT_
#define _TBARSIMPLETDCHIT_
#include "TObject.h"
#include <iostream>

class TBarSimpleTdcHit: public TObject
{
private:
   int fBarID=-1;
   double fTime=-1;
   int fFineTimeCount=-1;
   double fFineTime=-1;

public:
   TBarSimpleTdcHit(); //ctor
   TBarSimpleTdcHit(const TBarSimpleTdcHit &tdchit);
   using TObject::Print;
   virtual void Print();
   virtual ~TBarSimpleTdcHit(); // dtor

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

   ClassDef(TBarSimpleTdcHit, 3);
};
#endif
