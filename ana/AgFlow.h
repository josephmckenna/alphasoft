//
// AgFlow.h
//
// manalyzer flow objects for ALPHA-g events
// K.Olchanski
//

#ifndef AgFlow_H
#define AgFlow_H

#include "AgEvent.h"
#include "manalyzer.h"
#include "TStoreEvent.hh"
#include "Signals.hh"

class AgEventFlow: public TAFlowEvent
{
 public:
   AgEvent *fEvent;
   
 public:
 AgEventFlow(TAFlowEvent* flow, AgEvent* e) // ctor
    : TAFlowEvent(flow)
   {
      fEvent = e;
   }
   
   ~AgEventFlow() // dtor
      {
         if (fEvent) {
            delete fEvent;
            fEvent = NULL;
         }
      }
};

struct AgAwHit
{
   int chan;
   double time; // hit time
   double amp;  // hit amplitude
};

class AgAwHitsFlow: public TAFlowEvent
{
 public:
   std::vector<AgAwHit> fAwHits;
   
 public:
 AgAwHitsFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
   }
};

struct AgPadHit
{
   int ifeam; // feam position
   int seqsca; // sca*80+ri
   int col;
   int row;
   double time; // hit time
   double amp;  // hit amplitude
};

class AgPadHitsFlow: public TAFlowEvent
{
 public:
   std::vector<AgPadHit> fPadHits;
   
 public:
 AgPadHitsFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
   }
};

class AgAnalysisFlow: public TAFlowEvent
{
 public:
   TStoreEvent *fEvent;
   
 public:
 AgAnalysisFlow(TAFlowEvent* flow, TStoreEvent* e) // ctor
    : TAFlowEvent(flow)
   {
      fEvent = e;
   }
   
   // ~AgAnalysisFlow() // dtor
   //    {
   //       if (fEvent) {
   //          delete fEvent;
   //          fEvent = NULL;
   //       }
   //    }
};

class AgAwSignalsFlow: public TAFlowEvent
{
 public:
  std::vector<Signals::signal> fSig;
   
 public:
  AgAwSignalsFlow(TAFlowEvent* flow, std::vector<Signals::signal> sig) // ctor
    : TAFlowEvent(flow)
   {
     fSig = sig;
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


