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
   int adc_module; // ADC module, 1..20
   int adc_chan; // ADC channel, 0..15 and 16..47.
   int wire; // anode wire, 0..255 bottom, 256..511 top
   double time; // hit time, ns
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
   int pos; // pad board position: ring*8+column, 0..63
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

// class AgAwSignalsFlow: public TAFlowEvent
// {
//  public:
//   std::vector<Signals::signal> fSig;

//  public:
//   AgAwSignalsFlow(TAFlowEvent* flow, std::vector<Signals::signal> sig) // ctor
//     : TAFlowEvent(flow)
//    {
//      fSig = sig;
//    }
// };

class AgSignalsFlow: public TAFlowEvent
{
 public:
  std::vector<Signals::signal> awSig;
  std::vector<Signals::signal> pdSig;

  std::vector<TPCBase::electrode> awIndex;
  std::vector<TPCBase::electrode> pdIndex;

  std::vector<double> awResRMS;
  std::vector<double> pdResRMS;

  std::vector<double> awPed;
  std::vector<double> pdPed;

  const std::vector<Signals::wf_ref<int> > AWwf;
  const std::vector<Signals::wf_ref<int> > PADwf;

 public:
  AgSignalsFlow(TAFlowEvent* flow,
		const Signals* sig): TAFlowEvent(flow),
				     awSig(sig->sanode),pdSig(sig->spad),
				     awIndex(sig->aresIndex),pdIndex(sig->presIndex),
				     awResRMS(sig->resRMS_a),pdResRMS(sig->resRMS_p),
				     awPed(sig->PedestalAW),pdPed(sig->PedestalPD),
				     AWwf(sig->wirewaveforms),PADwf(sig->feamwaveforms)
   { }
};


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
