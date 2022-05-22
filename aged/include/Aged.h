//==============================================================================
// File:        Aged.h
//
// Description: ALPHA-g Event Display main class
//
// Created:     2017-08-01 - Phil Harvey
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================

class AgAnalysisFlow;
class AgSignalsFlow;
class AgBarEventFlow;
class TBarEvent;
#include "AgFlow.h"
#include "RecoFlow.h"
class TARunInfo;
class PWindow;
struct ImageData;

class Aged {
public:
   Aged();
   ~Aged();

//    TAFlags *ShowEvent(AgEvent *age, AgAnalysisFlow *anaFlow, AgSignalsFlow *sigFlow, AgBarEventFlow *barFlow,
//                       TAFlags *flags, TARunInfo *runinfo);
   TAFlags *ShowEvent(TStoreEvent &evt, const std::vector<TBarHit *> &bars, const std::vector<ALPHAg::wf_ref> &AWwf,
                      const std::vector<ALPHAg::wf_ref> &PADwf, long runNo = 0, TAFlags *flags = nullptr);
#ifdef BUILD_AG_SIM
//    int ShowEvent(TStoreEvent &evt, TClonesArray *awSignals, TClonesArray *padSignals, TClonesArray *barSignals);
#endif
private:
   ImageData *fData;
   PWindow *  fWindow;
};
