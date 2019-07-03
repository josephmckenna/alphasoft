#ifndef _TProjClusterAna_
#define _TProjClusterAna_

#include <TObject.h>
#include <TObjArray.h>

#include "TProjCluster.h"
#include "TAlphaEvent.h"
class TAlphaEvent;
class TProjClusterAna : public TObject
{
private:
  TObjArray *fprojclusters;
  Int_t      fi;
  Int_t      fj;
  Int_t      fSeed;
  TAlphaEvent* event;
public:
  TProjClusterAna(TAlphaEvent* e);
  virtual  ~TProjClusterAna();

  void          AddLast( TProjCluster * proj ) { fprojclusters->Add( (TObject*)proj ); }
  void          CombineMinPair();
  virtual void  Draw(const Option_t *option="");
  Double_t      FindMinPair();
  Double_t      FindClosestPoint();
  Int_t         GetEntries() { return fprojclusters->GetEntries(); }
  Int_t         GetEntriesFast() { return fprojclusters->GetEntriesFast(); }
  TObjArray    *GetClusters() { return fprojclusters; }
  TProjCluster *GetProjCluster( Int_t i ) { return (TProjCluster*) fprojclusters->At(i); }
  Int_t         GetSeed() { return fSeed; }
  void          SetSeed(Int_t Seed) { fSeed = Seed; }
  
  Double_t 	ProjDistance(Double_t Z1,Double_t Z2,Double_t RPhi1,Double_t RPhi2);
  Double_t 	ProjDistance(Double_t Z1,Double_t Z2,Double_t RPhi1,Double_t RPhi2,
  			Double_t ResZ1, Double_t ResZ2, Double_t ResRPhi1, Double_t ResRPhi2);  

  virtual void  Clear(Option_t */* option*/ ="");
  virtual void  Print( Option_t *option="" ) const;

  ClassDef( TProjClusterAna, 1 );
};
#endif
