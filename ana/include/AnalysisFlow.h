#ifndef _ANALYSIS_FLOW_
#define _ANALYSIS_FLOW_
//
// AnalysisFlow.h
//
// manalyzer flow objects for ALPHA-g and ALPHA 2 events
// 
// JTKM
//
#include "manalyzer.h"
#include <iostream>

//This should probably live somewhere else as its a A2 & Ag data type
#include "TInfoSpill.h"
class TInfoSpillFlow: public TAFlowEvent
{
  public:
  std::vector<TInfoSpill*> spill_events;

  TInfoSpillFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
  }
  ~TInfoSpillFlow()
  {
     for (size_t i=0; i<spill_events.size(); i++)
        delete spill_events[i];
     spill_events.clear();
  }
};

#endif
