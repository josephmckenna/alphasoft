#ifndef _TSeq_Dump_
#include "TSeq_Dump.h"
#endif

#include <stdlib.h>
#include <assert.h>

ClassImp( TSeq_Dump )

Int_t TSeq_Dump::GetDetIntegral(Int_t index){  
  assert(index<fNDet);
  return fdet_integral[index];
}


void TSeq_Dump::SetDetIntegral(Int_t index, Int_t value){
  assert(index<fNDet);
  fdet_integral[index] = value;
}

TSeq_Dump::TSeq_Dump(){
// ctor
  fSeqNum = 0;
  fID = 0;
  fStartonCount = 0;
  fStoponCount = 0;

  for (int i=0; i<fNDet; i++)
    fdet_integral[i] = 0;

  fIsDone = kFALSE;
  fHasStarted = kFALSE;
}

TSeq_Dump::~TSeq_Dump(){


}

//



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
