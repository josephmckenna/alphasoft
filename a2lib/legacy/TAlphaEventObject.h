#ifndef __TAlphaEventObject__
#define __TAlphaEventObject__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventObject                                                    //
//                                                                      //
// Base object. Contains information about the module and position
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include "TAlphaEventMap.h"
#include <assert.h>
#include <TNamed.h>

class TAlphaEventMap;
class TAlphaEventObject : public TNamed {
public:
  TAlphaEventMap* map;
private:
  double fX;      //local x coordinate
  double fY;      //local y coordinate
  double fZ;      //local z coordinate 
  double fXMRS;
  double fYMRS;
  double fZMRS;
  Int_t    fSilNum; // module number

public:
  TAlphaEventObject(TAlphaEventMap* m);
  TAlphaEventObject(TAlphaEventMap* m ,const Char_t* SilName, const Bool_t IsSilModule=kFALSE );
  TAlphaEventObject(TAlphaEventMap* m ,const Int_t SilNum, const Bool_t IsSilModule=kFALSE);
  TAlphaEventObject(){};
  virtual ~TAlphaEventObject();

  virtual Double_t X() const;
  virtual Double_t Y() const;
  virtual Double_t Z() const;
  virtual Double_t XMRS() const;
  virtual Double_t YMRS() const;
  virtual Double_t ZMRS() const;
  virtual Double_t GetCos() const;
  virtual Double_t GetSin() const;
  virtual Double_t GetXCenter() const;
  virtual Double_t GetYCenter() const;
  virtual Double_t GetZCenter() const;
  virtual Int_t    GetLayer() const;
  virtual Int_t    GetSilNum() const  { return fSilNum; }
		
  virtual void     SetX(Double_t x) { fX = x; }
  virtual void     SetY(Double_t y) { fY = y; }
  virtual void     SetZ(Double_t z) { fZ = z; }
  virtual void     SetXYZ(Double_t x, Double_t y, Double_t z);
  virtual void     SetXMRS(Double_t x) { fXMRS = x; }
  virtual void     SetYMRS(Double_t y) { fYMRS = y; }
  virtual void     SetZMRS(Double_t z) { fZMRS = z; }
  virtual void     SetXYZMRS(Double_t x, Double_t y, Double_t z);
/*  virtual void     SetCos( Double_t cos ) { fCos = cos; }
  virtual void     SetSin( Double_t sin ) { fSin = sin; }
  virtual void     SetXCenter( Double_t x ) { fXCenter = x; }
  virtual void     SetYCenter( Double_t y ) { fYCenter = y; }
  virtual void     SetZCenter( Double_t z ) { fZCenter = z; }
  virtual void     SetLayer( Int_t layer ) { fLayer = layer; }*/
  virtual void     SetSilNum(Int_t num) { fSilNum = num; }

  
  virtual void     MRS();
  virtual Int_t    ReturnSilNum(const char* SilName);


  // in cm
  inline Double_t SilX() { return 0.030; }  // silicon thickness (300 microns)
  inline Double_t SilY() { return 5.8062; } // silicon pside strip length
  inline Double_t SilZ() { return 22.699; } // silicon nside strip length
  inline Double_t SilPPitch() { return 0.0227; } // distance between two strips in pside
  inline Double_t SilNPitch() { return 0.0875; } // distance between two strips in nside

  inline Int_t N_FAIL()   { return -1; }
  inline Int_t P_FAIL()   { return -2; }
  inline Int_t N_LEFT()   { return -3; }
  inline Int_t N_RIGHT()  { return -4; }
  inline Int_t N_MIDDLE() { return -5; }
  inline Int_t P_LEFT()   { return -6; }
  inline Int_t P_RIGHT()  { return -7; }
  inline Int_t P_MIDDLE() { return -8; }

  virtual Double_t GetnPos(Int_t strip);
  virtual Double_t GetpPos(Int_t strip);
  virtual Int_t ReturnPStrip( Double_t pos );
  virtual Int_t ReturnNStrip( Double_t pos );

  //void Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventObject,2);
};

#endif
