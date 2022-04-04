#ifndef _TBARENDHIT_
#define _TBARENDHIT_
#include "TObject.h"
#include "TMath.h"
#include <iostream>

class TBarEndHit: public TObject
{
private:
   int fBarID=-1;
   double fTDCTime=-1; // Fully calibrated time
   double fADCTime=-1;
   double fAmp=-1;
   double fAmpRaw=-1; // Without fitting
   bool fTDCMatched=false;

public:
   TBarEndHit(); // ctor
   TBarEndHit(const TBarEndHit &endhit); //Copy ctor
   using TObject::Print;
   virtual void Print();
   virtual ~TBarEndHit(); // dtor

   TBarEndHit& operator=(const TBarEndHit& h)
   {
      fBarID = h.fBarID;
      fTDCTime = h.fTDCTime;
      fADCTime = h.fADCTime;
      fAmp = h.fAmp;
      fAmpRaw = h.fAmpRaw;
      fTDCMatched =  h.fTDCMatched;
      return *this;
   }

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
   void GetXY(double &x, double &y) const
   {
      double r=(.223+.243)/2.;
      double offset_angle=TMath::Pi()+0.25698;
      double theta=fBarID*2.*TMath::Pi()/64; //Degrees
      x=r*TMath::Cos(theta + offset_angle);
      y=r*TMath::Sin(theta + offset_angle);
      return;
   }
   ClassDef(TBarEndHit, 3);
};

#endif
