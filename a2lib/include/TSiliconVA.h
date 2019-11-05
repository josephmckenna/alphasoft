#ifndef __TSiliconVA__
#define __TSiliconVA__

// TSiliconVA Class =========================================================================================
//
// Class representing a single VA chip.
//
// JWS 10/10/2008
//
// ==========================================================================================================

#include "TH2.h"
#include "TF1.h"
#include "TMath.h"


class TSiliconVA : public TObject {

private:  
  Int_t ASICNumber;
  Int_t VF48ChannelNumber;
  Bool_t PSide;
  Bool_t HitOR;

  int nStrips;

  Double_t RawADCMean;
  Double_t RawADCRms;

  Double_t FilteredADCMean;

  Double_t PedFitP0;
  Double_t PedFitP1;
  Double_t PedFitP2;
  Double_t PedFitChi;


public:
  int RawADC[128];
  double PedSubADC[128];
  double stripRMS[128];
  bool Hit[128];
  TSiliconVA();
  TSiliconVA( const int _ASICNumber, const int _VF48ChannelNumber );
  TSiliconVA( TSiliconVA* & );
  virtual ~TSiliconVA();

  // getters
  Int_t GetASICNumber(){ return ASICNumber; }
  Int_t GetVF48ChannelNumber(){ return VF48ChannelNumber; }
  Int_t GetNumberOfStrips(){ return nStrips; }
  //TSiliconStrip* GetStripNumber(Int_t i){ return &Strips[i];}
  //TSiliconStrip* GetStrip( Int_t i ){ return &Strips[i];}

  Double_t GetRawADCMean(){ return RawADCMean; }
  Double_t GetRawADCRms(){ return RawADCRms; }
  Double_t GetFilteredADCMean(){ return FilteredADCMean; }
  Double_t GetPedFitP0(){ return PedFitP0; }
  Double_t GetPedFitP1(){ return PedFitP1; }
  Double_t GetPedFitP2(){ return PedFitP2; }
  Double_t GetPedFitChi(){ return PedFitChi; }
 
  Double_t GetPedADCForStrip( Int_t strip ) { 
  //  std::cout << PedFitP0 <<" + "<<
  //   strip<<"*"<<PedFitP1<<" + " <<
  //   strip<<"*"<<strip<<"*"<<PedFitP2<<std::endl;
  return PedFitP0 + PedFitP1*strip + PedFitP2*strip*strip; }
  Bool_t IsAPSide(){ return PSide; }
  Bool_t IsAHitOR(){ return HitOR; }
  //std::vector<TSiliconStrip*> GetStrips(){ return Strips; }

  // setters
  void AddStrip(const int i, const int adc,const double rms);
  void Reset();
  Bool_t NoStrips(){ return !nStrips; }
  int GetNoStrips() { return nStrips; }
  void SetPSide( Bool_t _PSide ){ PSide = _PSide; }
  void RemoveStrip( const Int_t i )
  {
    RawADC[i]=-9999;
    PedSubADC[i]=-9999;
    stripRMS[i]=-9999;
    Hit[i]=false;
  }
  void SetPol2Fit( Double_t pol0, Double_t pol1, Double_t pol2, Double_t chi) 
  { 
    PedFitP0=  pol0; 
    PedFitP1=  pol1; 
    PedFitP2=  pol2; 
    PedFitChi = chi;
    
  }
  // calculators
  Int_t CalcRawADCMeanSigma();
  Int_t CalcFilteredADCMean();
  Int_t FitP2Pedestal();
  Int_t FitP2Pedestal(Double_t* StripRMSs, int & SiModNumber);
  Int_t CalcPedSubADCs();
  Int_t CalcPedSubADCs_NoFit();
  Int_t CalcHits();
  Int_t CalcHits( Double_t & nsigma, int & SiModNumber );
  Int_t CalcNRawHits();

  // utilisites
  using TObject::Print;
  virtual void Print();
  void PrintToFile( FILE * f, Int_t mod_num );
  Int_t CompressStrips();
  Int_t SuppressNoiseyStrips();


  ClassDef(TSiliconVA,3)
};

#endif
