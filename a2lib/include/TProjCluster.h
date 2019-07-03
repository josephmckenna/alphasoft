#ifndef _TProjCluster_
#define _TProjCluster_

#include <TObject.h>
#include <TObjArray.h>
#include "TProjClusterBase.h"

class TProjCluster : public TObject
{
private:
  TObjArray *fProjClusters;
  Bool_t fIsVertex;

public:
  TProjCluster();
  TProjCluster( TProjCluster *pc );
  TProjCluster( TProjClusterBase * base );
  ~TProjCluster();

  void              Add( TProjClusterBase *c ) { fProjClusters->Add( c ); }
  TProjClusterBase *At( Int_t i ) { return (TProjClusterBase*) fProjClusters->At(i); }
  Int_t             GetEntries() { return fProjClusters->GetEntries(); }
  Int_t             GetEntriesFast() { return fProjClusters->GetEntriesFast(); }
  TProjClusterBase *GetMean();
  Bool_t            IsVertex() { return fIsVertex; }
  void              SetIsVertex(Bool_t isvertex) { fIsVertex = isvertex; }
  virtual void      Print( Option_t *option="" ) const;
  ClassDef( TProjCluster, 1 );
};

#endif
