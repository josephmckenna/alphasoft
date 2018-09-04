#include "RootUtils.h"
#include "TSeq_Event.h"

#ifndef _TSeqEventGetters_
#define _TSeqEventGetters_

TSeq_Event* Get_Seq_Event(Int_t runNumber, const char* description, Bool_t IsStart, Int_t repetition=1);
TSeq_Event* Get_Seq_Event(Int_t runNumber, const char* description, const char* DumpType, Int_t repetition=1);

#endif
