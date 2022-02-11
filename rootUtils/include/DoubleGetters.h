



#ifndef _DoubleGetter_
#define _DoubleGetter_
#include "TTree.h"

#include "TreeGetters.h"
#include "TChronoChannelGetters.h"

#ifdef BUILD_AG
#include "TStoreEvent.hh"
Double_t GetTotalRunTimeFromChrono(Int_t runNumber, const std::string& Board);
Double_t GetTotalRunTimeFromTPC(Int_t runNumber);
Double_t GetAGTotalRunTime(Int_t runNumber);
#endif

#ifdef BUILD_AG

Double_t GetTrigTimeBefore(Int_t runNumber, Double_t mytime);
Double_t GetTrigTimeAfter(Int_t runNumber, Double_t mytime);
#endif

#ifdef BUILD_A2
#include "TSISEvent.h"
#include "TSVD_QOD.h"
Double_t GetTotalRunTimeFromSIS(Int_t runNumber);
Double_t GetTotalRunTimeFromSVD(Int_t runNumber);
Double_t GetA2TotalRunTime(Int_t runNumber);
#endif

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
