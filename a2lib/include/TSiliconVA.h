#ifndef __TSiliconVA__
#define __TSiliconVA__

// TSiliconVA Class =========================================================================================
//
// Class representing a single VA chip.
//
// JWS 10/10/2008
//
// ==========================================================================================================

#include "TSiliconStrip.h"

class TSiliconVA : public TObject {

private:  
  Int_t ASICNumber;
  Int_t VF48ChannelNumber;
  Bool_t PSide;
  Bool_t HitOR;

  std::vector<TSiliconStrip*> Strips;

  Double_t RawADCMean;
  Double_t RawADCRms;

  Double_t FilteredADCMean;

  Double_t PedFitP0;
  Double_t PedFitP1;
  Double_t PedFitP2;
  Double_t PedFitChi;

  Double_t PHitThreshold;
  Double_t NHitThreshold;

public:
  TSiliconVA();
  TSiliconVA( Int_t _ASICNumber, Int_t _VF48ChannelNumber );
  TSiliconVA( TSiliconVA* & );
  virtual ~TSiliconVA();

  // getters
  Int_t GetASICNumber(){ return ASICNumber; }
  Int_t GetVF48ChannelNumber(){ return VF48ChannelNumber; }
  Int_t GetNumberOfStrips(){ return Strips.size(); }
  TSiliconStrip* GetStripNumber(Int_t i){ return Strips.at(i);}
  TSiliconStrip* GetStrip( Int_t i )
  {
    for( uint s = 0; s < Strips.size(); s++ )
      {
        TSiliconStrip * strip = (TSiliconStrip*)Strips.at(s);
        if( strip->GetStripNumber() == i )
          return strip;
      }
    return (TSiliconStrip*) NULL;
  }


  Double_t GetRawADCMean(){ return RawADCMean; }
  Double_t GetRawADCRms(){ return RawADCRms; }
  Double_t GetFilteredADCMean(){ return FilteredADCMean; }
  Double_t GetPedFitP0(){ return PedFitP0; }
  Double_t GetPedFitP1(){ return PedFitP1; }
  Double_t GetPedFitP2(){ return PedFitP2; }
  Double_t GetPedFitChi(){ return PedFitChi; }
 
  Double_t GetPHitThreshold() { return PHitThreshold; }
  Double_t GetNHitThreshold() { return NHitThreshold; }
  Double_t GetPedADCForStrip( Int_t strip ) { 
  //  std::cout << PedFitP0 <<" + "<<
  //   strip<<"*"<<PedFitP1<<" + " <<
  //   strip<<"*"<<strip<<"*"<<PedFitP2<<std::endl;
  return PedFitP0 + PedFitP1*strip + PedFitP2*strip*strip; }
  Bool_t IsAPSide(){ return PSide; }
  Bool_t IsAHitOR(){ return HitOR; }
  std::vector<TSiliconStrip*> GetStrips(){ return Strips; }

  // setters
  void AddStrip( TSiliconStrip* strip );
  void DeleteStrips();
  void Reset(){ Strips.clear(); }
  Bool_t NoStrips(){ return !Strips.size(); }
  void SetPSide( Bool_t _PSide ){ PSide = _PSide; }
  void RemoveStrip( Int_t i ){ std::cout<<"Warning... maybe use a list?"<<std::endl; delete Strips.at(i); }
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
  Int_t CalcThresholds( Double_t sigma, Double_t nsigma );
  Int_t CalcHits();
  Int_t CalcHits( Double_t & nsigma, Double_t* StripRMSs, int & SiModNumber );
  Int_t CalcNRawHits();

  // utilisites
  using TObject::Print;
  virtual void Print();
  void PrintToFile( FILE * f, Int_t mod_num );
  Int_t CompressStrips();
  Int_t SuppressNoiseyStrips();


  ClassDef(TSiliconVA,2)
};

#endif
