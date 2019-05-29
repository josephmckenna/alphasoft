#ifndef __TSiliconStrip__
#define __TSiliconStrip__

#include <iostream>
#include <assert.h>

#include "TFile.h"
#include "TH2.h"
#include "TF1.h"
#include "TLatex.h"
#include "TText.h"
#include "TBox.h"
#include "TStyle.h"
#include "TMath.h"
#include "TTree.h"

// TSiliconStrip Class =====================================================================================
//
// Class representing a single Si strip.
//
// JWS 10/10/2008
//
// =========================================================================================================

class TSiliconStrip: public TObject {

private:
  Int_t StripNumber;
  Int_t RawADC;
  Double_t PedSubADC;
  Double_t stripRMS;
  Bool_t Hit;

public:
  TSiliconStrip();
  TSiliconStrip( Int_t _StripNumber, Int_t _RawADC );
  TSiliconStrip( TSiliconStrip* & );
  virtual ~TSiliconStrip();
  
  Int_t GetStripNumber(){ return StripNumber; }
  Int_t GetRawADC(){ return RawADC; }
  Double_t GetPedSubADC(){ return PedSubADC; }
  Double_t GetStripRMS(){ return stripRMS; }
  Bool_t IsAHit(){ return Hit; }

  void SetStripNumber( Int_t _StripNumber ){ StripNumber = _StripNumber; }
  void SetRawADC( Int_t _RawADC ){ RawADC = _RawADC; }
  void SetPedSubADC( Double_t _PedSubADC ){ PedSubADC = _PedSubADC; }
  void SetStripRMS( Double_t _stripRMS ){ stripRMS = _stripRMS; }
  void SetHit( Bool_t _Hit ){ Hit = _Hit; }

  using TObject::Print;
  virtual void Print();
  void PrintToFile( FILE * f );
  
  ClassDef(TSiliconStrip,1)
};

#endif
