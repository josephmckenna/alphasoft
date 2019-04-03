//
// AgFlow.h
//
// manalyzer flow objects for ALPHA-g events
// K.Olchanski
//

#include "AgFlow.h"
#ifndef A2Flow_H
#define A2Flow_H

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

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */


