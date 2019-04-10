#ifndef __TAlphaEventCluster__
#define __TAlphaEventCluster__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventCluster                                                   //
//                                                                      //
// Object describing TAlphaEvent silicon cluster                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TObject.h"
#include "TObjArray.h"
#include "TAlphaEventStrip.h"
#include "TMemberInspector.h"

class TAlphaEventCluster : public TObject {
private:
  TObjArray fStrips;//strips container

  Double_t fnADC;   //n-side ADC value
  Double_t fpADC;   //p-side ADC value

  Double_t fX;      //calculated x coordinate
  Double_t fY;      //calculated y coordinate
  Double_t fZ;      //calculated z coordinate 
	
  Double_t fXMRS;   //hit x coordinate (master reference frame)
  Double_t fYMRS;   //hit y coordinate (master reference frame)
  Double_t fZMRS;   //hit z coordinate (master reference frame)
	
  Double_t fsigmax;
  Double_t fsigmay;
  Double_t fsigmaz;

  Double_t fCosAng;
  Double_t fSinAng;
  
  Double_t fXCenter;
  Double_t fYCenter;
  Double_t fZCenter;
	
  Double_t fdtheta;	// Euler angle corrections
  Double_t fdphi;
  Double_t fdpsi;
	
  Int_t fLayer;
  Int_t fSilNum;

public:
  TAlphaEventCluster(const char* SilName);
  TAlphaEventCluster() {};
  virtual ~TAlphaEventCluster();

  Double_t GetnADC()		{ return fnADC; }
  Double_t GetpADC()		{ return fpADC; }

  Double_t X()			{ return fX; }
  Double_t Y()			{ return fY; }
  Double_t Z()			{ return fZ; }

  Double_t XMRS()		{ return fXMRS; }
  Double_t YMRS()		{ return fYMRS; }
  Double_t ZMRS()		{ return fZMRS; }

  Double_t Getsigmax()          { return fsigmax; }
  Double_t Getsigmay()          { return fsigmay; }
  Double_t Getsigmaz()          { return fsigmaz; }

  Int_t GetLayer()		{ return fLayer; }
  void  SetLayer( Int_t layer ) { fLayer = layer; }
  Int_t GetSilNum()		{ return fSilNum; }
  void  SetSilNum(Int_t num)	{ fSilNum = num; }
	
  Int_t GetNumStrips()		{ return fStrips.GetEntries(); }
  TAlphaEventStrip * GetStrip(Int_t strip) { return (TAlphaEventStrip*) fStrips.At(strip); }
  void AddStrip(TAlphaEventStrip *strip) { fStrips.AddLast(strip); }
	
  void SetnADC(Double_t nADC)   { fnADC   = nADC; }
  void SetpADC(Double_t pADC)   { fpADC   = pADC; }
	
  void SetX(Double_t x)		{ fX = x; }
  void SetY(Double_t y)		{ fY = y; }
  void SetZ(Double_t z)		{ fZ = z; }
  void SetXYZ(Double_t x, Double_t y, Double_t z)  { fX = x; fY = y; fZ = z; }

  void SetXMRS(Double_t x)		{ fXMRS = x; }
  void SetYMRS(Double_t y)		{ fYMRS = y; }
  void SetZMRS(Double_t z)		{ fZMRS = z; }
  void SetXYZMRS(Double_t x, Double_t y, Double_t z)  { fXMRS = x; fYMRS = y; fZMRS = z; }
	
  void SetCos( Double_t cos )	{ fCosAng = cos; }
  void SetSin( Double_t sin ) 	{ fSinAng = sin; }
  void SetXCenter( Double_t x ) { fXCenter = x; }
  void SetYCenter( Double_t y ) { fYCenter = y; }
  void SetZCenter( Double_t z ) { fZCenter = z; }
  void SetSigma();
	
  Double_t GetCos()	       		{ return fCosAng; }
  Double_t GetSin()	       		{ return fSinAng; }
  Double_t GetXCenter()			{ return fXCenter;}
  Double_t GetYCenter()			{ return fYCenter;}
  Double_t GetZCenter()			{ return fZCenter;}
  Double_t Getdphi()			{ return fdphi; }
  Double_t Getdtheta()			{ return fdtheta; }
  Double_t Getdpsi()			{ return fdpsi; }
 
  Double_t GetnPos( Int_t strip );
  Double_t GetpPos( Int_t strip );
	
  void Calculate();
  void SetMRS(Double_t CosAng, Double_t SinAng,
	      Double_t XCenter, Double_t YCenter, Double_t ZCenter,
	      Double_t dphi, Double_t dtheta, Double_t dpsi)
  {
    fCosAng = CosAng;
    fSinAng = SinAng;
    fXCenter = XCenter;
    fYCenter = YCenter;
    fZCenter = ZCenter;
    fdphi = dphi;
    fdtheta = dtheta;
    fdpsi = dpsi;
  }
  void MRS();
  Int_t ReturnSilNum(const char* SilName);

  void Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventCluster,1);
};

#endif
