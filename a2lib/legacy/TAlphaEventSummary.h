#ifndef __TAlphaEventSummary__
#define __TAlphaEventSummary__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventSummary                                                   //
//                                                                      //
// Object describing TAlphaEvent reconstruction info                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TNamed.h"
#include "TRandom.h"
#include "TObjArray.h"
#include "TVector3.h"


using namespace ROOT::Math;

class TAlphaEventSummary : public TNamed {
private:
  TVector3  fVertex;      // vertex
  Int_t		fNumTrack;

public:
  TAlphaEventSummary();
  virtual ~TAlphaEventSummary(){ fVertex.SetXYZ(0,0,0); };

  TVector3* GetVertex();
  void SetVertex(TVector3* vtx){ fVertex.SetXYZ(vtx->X(),
  						vtx->Y(),
						vtx->Z());  }
  void SetNumTrack( Int_t tracknum ) { fNumTrack = tracknum; }
  Int_t GetNumTrack()				{ return fNumTrack; }
  
  ClassDef(TAlphaEventSummary,1);
};

inline TVector3 *TAlphaEventSummary::GetVertex() {
     if (fVertex.X()!=0 || fVertex.Y()!=0 || fVertex.Z()!=0) return &fVertex;
     return (TVector3*) NULL;
}


#endif
