#ifndef _TAlphaEventMap_
#define _TAlphaEventMap_

#include "TAlphaEventObject.h"
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TMath.h>
#include "SiMod.h"

class TAlphaEventMap: public TNamed{
private:
   double fCos[nSil];     //cos from vertical
   double fSin[nSil];     //sin from vertical
   double fXCenter[nSil]; //x center of module
   double fYCenter[nSil]; //y center of module
   double fZCenter[nSil]; //z center of module
   int    fLayer[nSil];   // layer of module
   public:
  virtual double GetCos(int i)     const { return fCos[i];     };
  virtual double GetSin(int i)     const { return fSin[i];     };
  virtual double GetXCenter(int i) const { return fXCenter[i]; };
  virtual double GetYCenter(int i) const { return fYCenter[i]; };
  virtual double GetZCenter(int i) const { return fZCenter[i]; };
  virtual int    GetLayer(int i)   const { return fLayer[i];   };

  virtual int      ReturnLayer(Int_t SilNum);
  virtual char *   ReturnSilName(Int_t SilNum);

  virtual void  SetValues();
  void Print(Option_t* option = "") const;
   TAlphaEventMap();
   ~TAlphaEventMap(){};

   ClassDef(TAlphaEventMap,1);
};



#endif
