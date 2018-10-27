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
    std::vector<ChronoEvent*>* events;
  public:
   AgChronoFlow(TAFlowEvent* flow, std::vector<ChronoEvent*>* _events) // ctor
    : TAFlowEvent(flow)
   {
      events=_events;
   }
   ~AgChronoFlow()
   {
      if (events)
      {
         for (uint i=0; i<events->size(); i++)
         {
            delete events->at(i);
         }
      }
      delete events;
   }
   void AddEvent(ChronoEvent e)
   {
      ChronoEvent* a = new ChronoEvent();
      memcpy(a,&e,sizeof e);
      events->push_back(a);
   }
   void PrintChronoFlow()
   {
      for (int i=0; i<CHRONO_N_CHANNELS; i++)
      {
         for (uint j=0; j<events->size(); j++)
         {
            std::cout <<"Chronoflow: Board:\t"<<events->at(j)->ChronoBoard<<std::endl;
            std::cout<<"RunTime:\t"<<events->at(j)->RunTime<<std::endl;
            std::cout <<i<<":"<<events->at(j)->Counts;
         }
      std::cout<<std::endl;
      }
   }

};

  struct DumpMarker {
    TString Description;
    Int_t DumpType; //1= Start, 2=Stop, 3=AD spill?, 4=Positrons?
    Int_t fonCount;
    bool IsDone;
  };

#include "Sequencer_Channels.h"
class AgDumpFlow: public TAFlowEvent
{
  public:
    std::vector<DumpMarker> DumpMarkers[NUMSEQ];

  public:
  AgDumpFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
   }
  void AddDumpEvent(Int_t _SequencerNum, TString _Description, Int_t _DumpType, Int_t _onCount) // ctor
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

  std::vector<signal> adc32max;
  std::vector<signal> adc32range;
  std::vector<signal> pwbMax;

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


#include "AnalysisTimer.h"

class AgAnalysisReportFlow: public TAFlowEvent
{
  public:
   std::vector<const char*> ModuleName;
   clock_t* time;
   std::vector<double> SecondAxis;
  AgAnalysisReportFlow(TAFlowEvent* flow, const char* _name) : TAFlowEvent(flow)
  {
     ModuleName.push_back(_name);
     time=new clock_t(clock());
  }
  AgAnalysisReportFlow(TAFlowEvent* flow, std::vector<const char*> _name, std::vector<double> second_axis) : TAFlowEvent(flow)
  {
     //ModuleName[0] is the main title (also used to fill a 1D histogram)
     //ModuleName[1+] are addition bits of a title for 2D histogram added to ModuleName[0]
     ModuleName=_name;
     time=new clock_t(clock());
     SecondAxis=second_axis;
  }
  
  ~AgAnalysisReportFlow() // dtor
   {
      //if (ModuleName) delete ModuleName;
      if (time) delete time;
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

