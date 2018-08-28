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
   int imodule; // pwbNN
   int seqsca; // sca*80+ri
   int tpc_col; // pad column
   int tpc_row; // pad row
   double time_ns; // hit time in ns
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


#define N_CHRONO_CHANNELS 59
class AgChronoFlow: public TAFlowEvent
{
  public:
    Double_t RunTime[N_CHRONO_CHANNELS];
    uint32_t Counts[N_CHRONO_CHANNELS];
    Int_t ChronoBoard;
  public:
   AgChronoFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
      ChronoBoard=-1;
      for (int i=0; i<N_CHRONO_CHANNELS; i++)
      {
         RunTime[i]=0.;
         Counts[i]=0;
      }
   }
   void SetRunTime(int chan, Double_t time)
   {
      RunTime[chan]=time;
   }
   void SetCounts(int chan, uint32_t counts)
   {
      Counts[chan]=counts;
   }
   void SetChronoBoard(Int_t index)
   {
      ChronoBoard=index;
   }
};

class AgDumpFlow: public TAFlowEvent
{
  public:
    std::vector<TString> Description;
    std::vector<Int_t> DumpType; //1=Start, 2=Stop
    std::vector<Int_t> fonCount;
    Int_t SequencerNum;
  public:
  AgDumpFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
   }
   void AddEvent(Int_t _SequencerNum, TString _Description, Int_t _DumpType, Int_t _onCount)
   {
      SequencerNum=_SequencerNum;
      Description.push_back(_Description);
      DumpType.push_back(_DumpType);
      fonCount.push_back(_onCount);
   }
};

#include "TStoreEvent.hh"
class AgAnalysisFlow: public TAFlowEvent
{
 public:
   TStoreEvent *fEvent;

 public:
 AgAnalysisFlow(TAFlowEvent* flow, TStoreEvent* e) // ctor
   : TAFlowEvent(flow),fEvent(e)
   {  }

};

#include "SignalsType.h"
class AgSignalsFlow: public TAFlowEvent
{
public:
  std::vector<signal> awSig;
  std::vector<signal> pdSig;
  std::vector< std::pair<signal,signal> > matchSig;

  std::vector<wf_ref> AWwf;
  std::vector<wf_ref> PADwf;

public:
  AgSignalsFlow(TAFlowEvent* flow, 
		std::vector<signal> s): 
    TAFlowEvent(flow), awSig(s)
  {}  
  
  AgSignalsFlow(TAFlowEvent* flow, 
		std::vector<signal> s,std::vector<signal> p,
		std::vector<wf_ref> awf, std::vector<wf_ref> pwf): 
    TAFlowEvent(flow), awSig(s), pdSig(p), AWwf(awf), PADwf(pwf)
  {}

  void AddPadSignals( std::vector<signal> s )
  {
    pdSig.clear();
    pdSig=s;
  }

  void AddMatchSignals( std::vector< std::pair<signal,signal> > ss )
  {
    matchSig.clear();
    matchSig=ss;
  }

  void AddWaveforms(std::vector<wf_ref> af, std::vector<wf_ref> pf)
  {
    AWwf.clear();
    AWwf=af;
    PADwf.clear();
    PADwf=pf;
  }

};

class AgTrigUdpFlow: public TAFlowEvent
{
 public:
   std::vector<uint32_t> fData;
   
 public:
 AgTrigUdpFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
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


