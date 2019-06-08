#ifndef __TAlphaEvent__
#define __TAlphaEvent__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEvent                                                         //
//                                                                      //
// Description of the raw event in root format                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <TObjArray.h>
#include <TVector3.h>
#include <TVector.h>
#include <TH1.h>
#include <TH2.h>

#include "TAlphaEventSil.h"
#include "TAlphaEventHit.h"
#include "TAlphaEventTrack.h"
#include "TAlphaEventHelix.h"
#include "TAlphaEventCosmicHelix.h"
#include "TAlphaEventVertex.h"
#include "TProjCluster.h"
#include "TProjClusterBase.h"
#include "TProjClusterAna.h"
#include "TAlphaEventVerbose.h"
#include "TAlphaEventMap.h"
#include "SiMod.h"

#define TrapRadius 2.2275

class TObject;
class TObjArray;
class TAlphaEventSil;
class TAlphaEventMap;
class TAlphaEvent : public TObject 
{
private:
  TAlphaEventMap*    map;
  std::vector<TAlphaEventSil*> fSil; // hit silicon (TAlphaEventSil) 
  TAlphaEventVertex*  fVertex; // reconstructed vertex
  TVector3           fMCVertex; // Monte Carlo vertex
  Double_t           fMCtime; // MC time -- used in FRD sim
  //TObjArray          fHits; // container of hits
  std::vector<TAlphaEventTrack*>  fTrack; // container of tracks
  std::vector<TAlphaEventHelix*>  fHelices; // container of helices (Tracks with exactly 3 hits)
  TObjArray         *fCosmicHelices; // container of helices 
  TObjArray          fMCPoint; // MC points (TVector3)
  TAlphaEventVerbose fVerbose; // message handler
  
  //Hit cluster cuts
  Double_t           nClusterSigmaCut;
  Double_t           pClusterSigmaCut;
  
  //Hit threshold cut 
  Double_t           HitSignificance; 
  Double_t           HitThreshold;
  Int_t              fNHitsCut;
  
  //Track cuts
  Double_t           fHitSepCutPhi;
  Double_t           fHitSepCutZ;
  Double_t           fCorCut;
  Double_t           fChi2Cut;
  Double_t           fd0_trapCut;
  
  //Vertex candidate cuts
  Double_t           fVertRadCut;
  Double_t           fVertDCACut;
  
  // 'Projection Method' cut valuses
  Double_t           fMinDistCut;
  Double_t           fMinClosestCut;
  
  Int_t              fNGoodHelices;
  TObjArray          fxylines;
  TObjArray          fyzlines;
  Bool_t             fIsCosmic; // cosmic run flag
  TObjArray          fprojp;
  TProjClusterBase  *fProjClusterVertex;
  
  TAlphaEventTrack   fCosmic;  // Set in CosmicTest -->

  Bool_t             fDebug;


 public:
  TAlphaEvent(TAlphaEventMap*);
  virtual ~TAlphaEvent();

  void                AddSil(TAlphaEventSil *sil) { fSil.push_back(sil); }
  void                AddMCPoint(TVector3 *p) { fMCPoint.AddLast(p); }
  //void                AddHit( TAlphaEventHit * Hit ) { fHits.AddLast( Hit ); }
  void                AddTrack( TAlphaEventTrack * Track) { fTrack.push_back( Track ); }
  //void                AddTrackAt( TAlphaEventTrack * Track, Int_t i) { fTrack->AddAt( Track, i ); }
  void                AddHelix( TAlphaEventHelix * Helix ) { fHelices.push_back( Helix ); }
  void                AddHelix( TAlphaEventCosmicHelix * Helix ) { fCosmicHelices->AddLast( Helix ); }
  void                Addxyline( TVector3 * line ) { fxylines.AddLast( line ); }
  void                Addyzline( TVector3 * line ) { fyzlines.AddLast( line ); }
  Int_t               Classify();
  //void                ClearHits() { fHits.Clear(); }
  void                ClearTracks()
                      {
                        //fTrack->SetOwner(kTRUE);
                        fTrack.clear();
                      }
  void                CosmicHitEfficiency(TH1D *phits, TH1D *pexpected,
					  TH1D *nhits, TH1D *nexpected);
  Double_t            CosmicTest();
  Double_t            STCosmicTest();
  Double_t            CosmicHelixTest();
  void                DeleteEvent();
  TAlphaEventCosmicHelix *FindHelix();
  TAlphaEventTrack   *FindCosmic( Int_t hlimit );
  Bool_t              GetDebug() { return fDebug; }
  TVector3           *GetCosmicVector();
  TObjArray          *GatherHits();
  Int_t               GatherTrackCandidates();
  TVector3           *GetMCVertex();
  Double_t            GetMCtime() { return fMCtime;}
  Int_t               GetMCNumPoint() { return fMCPoint.GetLast()+1; }
  TVector3           *GetMCPoint(Int_t n) { return (TVector3*) fMCPoint.At(n); }
  Int_t               GetNSil() { return fSil.size(); }
  //Int_t               GetNHits() { return fHits.GetEntriesFast(); }
  Int_t               GetNTracks() { return fTrack.size(); }
  Int_t               GetNCosmicHelices() { return fCosmicHelices->GetEntriesFast(); }
  Int_t               GetNHelices() { return fHelices.size(); }
  Int_t               GetNGoodHelices() { return fNGoodHelices; }
  TObjArray          *GetProjP() { return &fprojp; }
  Int_t               GetResidual(TAlphaEventTrack *best_track,
                                  Int_t hitnumber,
                                  Double_t &yres,
                                  Double_t &zres);
  TAlphaEventSil     *GetSil(Int_t n) { return fSil.at(n); }
  TAlphaEventSil     *GetSilByNumber(Int_t n, bool read_only=false); 
  //TAlphaEventHit     *GetHit( Int_t i ) { return (TAlphaEventHit*)fHits.At( i ); }
  TAlphaEventTrack   *GetTrack( Int_t i ) { return fTrack.at( i ); }
  TAlphaEventCosmicHelix   *GetCosmicHelix( Int_t i) { return (TAlphaEventCosmicHelix*)fCosmicHelices->At( i ); }
  TAlphaEventHelix   *GetHelix( Int_t i) { return fHelices.at( i ); }
  TAlphaEventVertex  *GetVertex() { return fVertex; }
  TVector3           *GetSTVertex();
  TVector3           *GetMTVertex();
  TAlphaEventVerbose *GetVerbose() { return &fVerbose; }
  TProjClusterBase   *GetProjClusterVertex() { return fProjClusterVertex; }
  Int_t               Getnxylines() { return fxylines.GetEntries(); }
  TVector3*           Getxyline( Int_t n ) { return (TVector3*)fxylines.At(n); }
  Int_t               Getnyzlines() { return fyzlines.GetEntries(); }
  TVector3*           Getyzline( Int_t n ) { return (TVector3*)fyzlines.At(n); }
  TAlphaEventTrack*   GetCosmicTrack() {return &fCosmic;}
  Int_t               IsCosmic();
  Int_t               IsROTrig();
  Int_t               IsSig1Trig();
  Bool_t              IsTrig(Int_t inner=2, Int_t middle=1, Int_t outer=1);
  Bool_t              IsACosmic() { return fIsCosmic; }
  Bool_t              IsSameHelix(int &a, int &b, Bool_t DeleteOne=kTRUE);
  Int_t               IsSameHit( TAlphaEventHit * hit1, TAlphaEventHit * hit2 );
  Int_t               IsGhostTrack();
  Int_t               LayerMulti( const char * layernum );
  Int_t               PruneTracks();
  Int_t               ModuleMulti( Int_t sinum );
  Int_t               MissHitStatus(TAlphaEventHelix* h);
  void                RecEvent( Bool_t debug = kFALSE );
  Int_t               RecSTEvent();
  Int_t               RecMTEvent();
  void                RecHits();
  void                RecClusters();
  Int_t               RecTrackCandidates();
  Double_t            RecRPhi( Bool_t PlotProj = kFALSE );
  Int_t               RecVertex();
  Int_t               ImproveVertex();
  void                CalcGoodHelices();
  void                Reset();
  void                RemoveTrackAt( Int_t i) { delete fTrack[i]; fTrack[i]=NULL; }    
  void                RemoveDuplicateHelices();
  void                SetCosmic( Bool_t yes ) { fIsCosmic = yes; } 
  void                SetChii2Cut(Double_t Chi2Cut) { fChi2Cut = Chi2Cut; }
  void                Setd0_trapCut(Double_t d0_trapCut) { fd0_trapCut = d0_trapCut; }
  void                SetNHitsCut(Int_t NHitsCut) { fNHitsCut = NHitsCut; }
  void                SetHitSepCutPhi(Double_t HitSepCut) { fHitSepCutPhi = HitSepCut; }
  void                SetHitSepCutZ(Double_t HitSepCut) { fHitSepCutZ = HitSepCut; }
  void                SetCorCut(Int_t CorCut) { fCorCut = CorCut; }
  void                SetMCVertex(TVector3 v)       { fMCVertex=v; }
  void                SetMCtime(Double_t t) { fMCtime=t; }
  void                SetVerboseLevel( Int_t verboseLevel ) { fVerbose.SetLevel( verboseLevel ); }
  void                ShiftHits( Double_t z );
  void                SetMinDistCut( Double_t dist ) { fMinDistCut = dist; }
  void                SetMinClosestCut( Double_t dist ) { fMinClosestCut = dist; }
  void                SetCosmicTrack(TAlphaEventTrack*);
  void                SetNClusterSigma(Double_t _nClusterSigmaCut) { nClusterSigmaCut=_nClusterSigmaCut;}
  Double_t            GetNClusterSigma() { return nClusterSigmaCut; }
  void                SetPClusterSigma(Double_t _pClusterSigmaCut) { pClusterSigmaCut=_pClusterSigmaCut;}
  Double_t            GetPClusterSigma() { return pClusterSigmaCut; }
  void                SetHitSignificance(Double_t _HitSignificance) { HitSignificance = _HitSignificance; }
  Double_t            GetHitSignificance() { return HitSignificance; }
  void                SetHitThreshold( Double_t _HitThreshold ) { HitThreshold = _HitThreshold; } 
  Double_t            GetHitThreshold() { return HitThreshold; }
  void                SetTrackCuts(Double_t _fHitSepCutPhi = 0.35, 
                        Double_t _fHitSepCutZ = 5., 
                        Double_t _fCorCut = .95, 
                        Double_t _fChi2Cut = 63., 
                        Double_t _fd0_trapCut = 7.) //default values
  {
    fHitSepCutPhi = _fHitSepCutPhi;
    fHitSepCutZ   = _fHitSepCutZ;
    fCorCut       = _fCorCut;
    fChi2Cut      = _fChi2Cut;
    fd0_trapCut   = _fd0_trapCut;
  }
  void                SetVertCuts(Double_t _fVertRadCut, Double_t _fVertDCACut)
  {
    fVertRadCut=_fVertRadCut;
    fVertDCACut=_fVertDCACut;
  }
  
  void FlipZofHits();
  ClassDef(TAlphaEvent,6);
  
};

//_____________________________________________________________________
inline TVector3 *TAlphaEvent::GetMCVertex() {
  if (fMCVertex.X()!=0 || fMCVertex.Y()!=0 || fMCVertex.Z()!=0) return &fMCVertex;
  return (TVector3*) NULL;
}

#endif
