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

#include "chrono_module.h"

class AgChronoFlow: public TAFlowEvent
{
  public:
    Double_t RunTime;
    uint32_t Counts[CHRONO_N_CHANNELS];
    Int_t ChronoBoard;
  public:
   AgChronoFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
      ChronoBoard=-1;
      RunTime=-1;
      for (int i=0; i<CHRONO_N_CHANNELS; i++)
      {
         Counts[i]=0;
      }
   }
   void SetRunTime(Double_t time)
   {
      RunTime=time;
   }
   void SetCounts(int chan, uint32_t counts)
   {
      Counts[chan]=counts;
   }
   void SetChronoBoard(Int_t index)
   {
      ChronoBoard=index;
   }
   void PrintChronoFlow()
   {
      std::cout <<"Chronoflow: Board:\t"<<ChronoBoard<<std::endl;
      std::cout<<"RunTime:\t"<<RunTime<<std::endl;
      for (int i=0; i<CHRONO_N_CHANNELS; i++)
         std::cout <<i<<":"<<Counts[i]<<std::endl;
   }

};

  struct DumpMarker {
    TString Description;
    Int_t DumpType; //1= Start, 2=Stop, 3=AD spill?, 4=Positrons?
    Int_t fonCount;
    bool IsDone;
  };


class AgDumpFlow: public TAFlowEvent
{
  public:
    std::vector<DumpMarker> DumpMarkers[4];

  public:
  AgDumpFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
   }
   void AddEvent(Int_t _SequencerNum, TString _Description, Int_t _DumpType, Int_t _onCount)
   {
      DumpMarker Marker;
      Marker.Description=_Description;
      Marker.DumpType=_DumpType;
      Marker.fonCount=_onCount;
      Marker.IsDone = false;
      DumpMarkers[_SequencerNum].push_back(Marker);
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

/*struct EventTimer
{
   char[4] ModuleName;
   Double_t Time;
};*/


class AgAnalysisReportFlow: public TAFlowEvent
{
  public:
    //char[4] ModuleName;
    Double_t Time;
  AgAnalysisReportFlow(TAFlowEvent* flow, char _name, Double_t _time) : TAFlowEvent(flow)
  {
     //ModuleName=_name;
     Time=_time;
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


