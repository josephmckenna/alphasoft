#ifndef __TAlphaEventSil__
#define __TAlphaEventSil__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventSil                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <TNamed.h>
#include <TObject.h>
#include <TObjArray.h>
#include <TVector3.h>
#include "TAlphaEvent.h"
#include "TAlphaEventHit.h"
#include "TAlphaEventNCluster.h"
#include "TAlphaEventPCluster.h"
#include "TAlphaEventNStrip.h"
#include "TAlphaEventPStrip.h"
#include "TAlphaEventObject.h"
#include "TAlphaEventMap.h"

class TAlphaEvent;
class TAlphaEventMap;
class TAlphaEventSil : public TAlphaEventObject {
 private:
  // Hybrid ASIC chips
  Double_t   fASIC1[128];
  Double_t   fASIC2[128];
  Double_t   fASIC3[128];
  Double_t   fASIC4[128];
  
  Double_t   fRMS1[128];
  Double_t   fRMS2[128];
  Double_t   fRMS3[128];
  Double_t   fRMS4[128];
  
  Double_t   fADCp[256];
  Double_t   fADCn[256];
  
  Double_t   fRMSp[256];
  Double_t   fRMSn[256];
  
  Double_t   nClusterSigmaCut;
  Double_t   pClusterSigmaCut;
  
  std::vector<TAlphaEventNCluster*>  fNClusters;
  std::vector<TAlphaEventPCluster*>  fPClusters;
  std::vector<TAlphaEventHit*>       fHits;
  
  TAlphaEvent* Event;
  
 public:
  TAlphaEventSil(TAlphaEvent* e, TAlphaEventMap* m ): TAlphaEventObject(m) { Event=e;}
  TAlphaEventSil(Char_t *n, TAlphaEvent* e, TAlphaEventMap* m);
  TAlphaEventSil(const int num, TAlphaEvent* e,TAlphaEventMap* m);
  virtual ~TAlphaEventSil();

  void                 AddMCHitMRS(Double_t x, Double_t y, Double_t z, Double_t adc );
  void                 AddMCHit( Double_t en_x, Double_t en_y, Double_t en_z,
                                 Double_t ex_x, Double_t ex_y, Double_t ex_z,
                                 Double_t edep,
                                 TObjArray * strips ); 
  void                 AddMCStrips( Double_t en_x, Double_t en_y, Double_t en_z,
				    Double_t ex_x, Double_t ex_y, Double_t ex_z,
				    Double_t edep, TObjArray * strips );
  void                 AddHit( TAlphaEventHit * hit ) { fHits.push_back( hit ); }
  Int_t                GetNHits() { return fHits.size(); }
  TAlphaEventHit      *GetHit( Int_t i ) { return fHits.at(i); }
  Int_t                GetNNClusters() { return fNClusters.size(); }
  TAlphaEventNCluster *GetNCluster( Int_t i ) { return (TAlphaEventNCluster*) fNClusters.at(i); }
  Int_t                GetNPClusters() { return fPClusters.size(); }
  TAlphaEventPCluster *GetPCluster( Int_t i ) { return (TAlphaEventPCluster*) fPClusters.at(i); }

  Double_t            *GetADCp() { return fADCp; }
  Double_t            *GetADCn() { return fADCn; }
  void                 GetStrippStartEnd (Int_t n, TVector3 &a, TVector3 &b);
  void                 GetStripnStartEnd(Int_t n, TVector3 &a, TVector3 &b);
  Double_t            *GetASIC1() { return fASIC1; }
  Double_t            *GetASIC2() { return fASIC2; }
  Double_t            *GetASIC3() { return fASIC3; }
  Double_t            *GetASIC4() { return fASIC4; }
  Double_t            *GetRMS1() { return fRMS1; }
  Double_t            *GetRMS2() { return fRMS2; }
  Double_t            *GetRMS3() { return fRMS3; }
  Double_t            *GetRMS4() { return fRMS4; }
  Int_t                GetOrPhi();
  Int_t                GetOrN();
  void                 MapASICtoStrips();
  void                 RemoveHit(TAlphaEventHit* remove);

  void                 RecHit();
  void                 RecCluster();
  void                 SetASIC1( Int_t strip, Double_t adc, Double_t RMS ) { fASIC1[strip] += adc; fRMS1[strip] = RMS; }
  void                 SetASIC2( Int_t strip, Double_t adc, Double_t RMS ) { fASIC2[strip] += adc; fRMS2[strip] = RMS; }
  void                 SetASIC3( Int_t strip, Double_t adc, Double_t RMS ) { fASIC3[strip] += adc; fRMS3[strip] = RMS; }
  void                 SetASIC4( Int_t strip, Double_t adc, Double_t RMS ) { fASIC4[strip] += adc; fRMS4[strip] = RMS; }
  Bool_t boundary_flag(Int_t f);

  virtual void Print(Option_t *option="") const;

  ClassDef(TAlphaEventSil,2);
};

#endif //end
