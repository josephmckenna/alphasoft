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
#include "TAlphaEventObject.h"
#include "TAlphaEventMap.h"

class TAlphaEvent;
class TAlphaEventMap;
class TAlphaEventSil : public TAlphaEventObject {
 private:
  // Hybrid ASIC chips
  Double_t   fASIC[4][128];
  
  Double_t   fRMS[4][128];
  
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
  TAlphaEventSil(){};
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

  Double_t            *GetADCn() { return fASIC[0]; }
  Double_t            *GetADCp() { return fASIC[2]; }
  void                 GetStrippStartEnd (Int_t n, TVector3 &a, TVector3 &b);
  void                 GetStripnStartEnd(Int_t n, TVector3 &a, TVector3 &b);
  Double_t            *GetASIC1() { return fASIC[0]; }
  Double_t            *GetASIC2() { return fASIC[1]; }
  Double_t            *GetASIC3() { return fASIC[2]; }
  Double_t            *GetASIC4() { return fASIC[3]; }
  Double_t            *GetRMS1() { return fRMS[0]; }
  Double_t            *GetRMS2() { return fRMS[1]; }
  Double_t            *GetRMS3() { return fRMS[2]; }
  Double_t            *GetRMS4() { return fRMS[3]; }
  Int_t                GetOrPhi();
  Int_t                GetOrN();
  void                 MapASICtoStrips();
  void                 RemoveHit(TAlphaEventHit* remove);

  void                 RecHit();
  void                 RecCluster();
  void                 SetASIC( Int_t asic, Int_t strip, Double_t adc, Double_t RMS ) { fASIC[asic][strip] += adc; fRMS[asic][strip] = RMS; }
  Bool_t boundary_flag(Int_t f);

  virtual void Print(Option_t *option="") const;

  ClassDef(TAlphaEventSil,2);
};

#endif //end
