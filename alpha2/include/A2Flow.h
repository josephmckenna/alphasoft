//
// AgFlow.h
//
// manalyzer flow objects for ALPHA-g events
// K.Olchanski
//

#include "AgFlow.h"
#ifndef A2Flow_H
#define A2Flow_H
#include "UnpackVF48.h"
class VF48EventFlow: public TAFlowEvent
{
  public:
     std::vector<VF48event*> vf48events;
  public:
  VF48EventFlow(TAFlowEvent* flow)
       : TAFlowEvent(flow)
  {
  }
  ~VF48EventFlow()
  {
    for (uint i=0; i<vf48events.size(); i++)
       if (vf48events[i])
          delete vf48events[i];
    vf48events.clear();
  }
};
#include "TSiliconEvent.h"
class SilEventsFlow: public TAFlowEvent
{
  public:
     std::vector<TSiliconEvent*> silevents;
  public:
  SilEventsFlow(TAFlowEvent* flow)
       : TAFlowEvent(flow)
  {
  }
  ~SilEventsFlow()
  {
    for (uint i=0; i<silevents.size(); i++)
       if (silevents[i])
          delete silevents[i];
    silevents.clear();
  }
};

#include "TStoreA2Event.hh"
class A2AnalysisFlow: public TAFlowEvent
{
 public:
   std::vector<TStoreA2Event*> analyzed_events;

 public:
 A2AnalysisFlow(TAFlowEvent* flow) // ctor
   : TAFlowEvent(flow)
   {  }
   ~A2AnalysisFlow()
  {
    for (uint i=0; i<analyzed_events.size(); i++)
       if (analyzed_events[i])
          delete analyzed_events[i];
    analyzed_events.clear();
  }

};


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */


