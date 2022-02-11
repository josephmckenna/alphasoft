#ifndef _TSeqEventGetters_
#define _TSeqEventGetters_

#include "TSeq_Event.h"
#include "TreeGetters.h"

TSeq_Event* Get_Seq_Event(Int_t runNumber, const char* description, Bool_t IsStart, Int_t dumpIndex=0);
TSeq_Event* Get_Seq_Event(Int_t runNumber, const char* description, const char* DumpType, Int_t dumpIndex=0);

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
