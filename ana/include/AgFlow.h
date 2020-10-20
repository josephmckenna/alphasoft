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
#include <iostream>

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
#ifdef LASER
   double dtime; // drift time, ns, laser runs only
#endif
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
   ~AgAwHitsFlow(){}
   
};

// NM, AC, JTKM Style (Development branch agana.exe)

#include "TBarEvent.hh"

class AgBarEventFlow: public TAFlowEvent
{
   public:
      TBarEvent* BarEvent;
   public:
   AgBarEventFlow(TAFlowEvent* flow, TBarEvent* b) //ctor
     : TAFlowEvent(flow)
   {
      BarEvent=b;
   }
   ~AgBarEventFlow() //dtor
   {
      if (BarEvent)
         {
            BarEvent->Reset();
            delete BarEvent;
         }
   }
};

// KO Style (Master branch agana.exe or development branch ag_noreco.exe)

struct AgBscAdcHit
{
   int adc_module; // ADC module, 1..20
   int adc_chan; // ADC channel, 0..15 and 16..47.
   int bar; // bar number, 0..63 bottom, 64..127 top
   double time; // hit time, ns
   double amp;  // hit amplitude
};

class AgBscAdcHitsFlow: public TAFlowEvent
{
 public:
   std::vector<AgBscAdcHit> fBscAdcHits;

 public:
 AgBscAdcHitsFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
   }
~AgBscAdcHitsFlow(){}
};

struct AgPadHit
{
   int imodule; // pwbNN
   int seqsca; // sca*80+ri
   int tpc_col; // pad column
   int tpc_row; // pad row
   double time_ns; // hit time in ns
#ifdef LASER
   double dtime_ns; // drift time in ns, laser runs only
#endif
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
   ~AgPadHitsFlow(){}
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


#include "Sequencer_Channels.h"
// To avoid parsing the sequencer XML in the main thread, we copy the 
// text into the flow to be processed in its own thread
class SEQTextFlow: public TAFlowEvent
{
  public:
  char* data;
  int size=0;
  void AddData(char* _data, int _size)
  {
    size=_size;
    data=(char*) malloc(size);
    memcpy(data, _data, size);
    return;
  }
  void Clear()
  {
     if (size)
        free( data );
     size=0;
  }
  SEQTextFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
  }
  ~SEQTextFlow()
  {
    Clear();
  }
};
#include "DumpHandling.h"
#include "TSequencerState.h"
#include "TSequencerDriver.h"
class AgDumpFlow: public TAFlowEvent
{
  public:
    int SequencerNum=-1;
    std::vector<DumpMarker> DumpMarkers;
    std::vector<TSequencerState*> states;
    TSequencerDriver* driver;
  public:
  AgDumpFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
   }
   ~AgDumpFlow()
   {
      if (driver)
         delete driver;
      for ( auto & state: states )
         delete state;
      states.clear();
      DumpMarkers.clear();
   }
  void AddDumpEvent(Int_t _SequencerNum,Int_t _SeqCount, uint32_t SequenceStartTime, TString _Description, DumpMarker::DumpTypes _DumpType, Int_t _onCount, Int_t _onState) 
   {
      if (SequencerNum<0) SequencerNum=_SequencerNum;
      else if (SequencerNum!=_SequencerNum)
      {
         std::cout<<"ERROR! Parsing sequencer data that has data for more than one sequencer... something went very wrong"<<std::endl;
         exit(1);
      }
      DumpMarker Marker;
      Marker.Description   = _Description;
      Marker.fSequencerID  = _SequencerNum;
      Marker.fSequenceCount= _SeqCount;
      Marker.DumpType      = _DumpType;
      Marker.fonCount      = _onCount;
      Marker.fonState      = _onState;
      Marker.fRunTime      = -1.;
      Marker.MidasTime     = SequenceStartTime;
      DumpMarkers.push_back(Marker);
   }
   void AddDumpEvent(Int_t _SequencerNum,Int_t _SeqCount, uint32_t _SequenceStartTime, TString _Description, const char* _DumpType, Int_t _onCount, Int_t _onState) 
   {
     DumpMarker::DumpTypes type= DumpMarker::DumpTypes::Other;
     if (strcmp(_DumpType,"startDump")==0)
        type=DumpMarker::DumpTypes::Start;
     else if (strcmp(_DumpType,"stopDump")==0)
        type=DumpMarker::DumpTypes::Stop;
      AddDumpEvent(_SequencerNum, _SeqCount, _SequenceStartTime, _Description, type, _onCount, _onState);
   }

   void AddStateEvent(TSequencerState* s )
   {
      states.push_back(s);
   }
};
#include "TSpill.h"

class AGSpillFlow: public TAFlowEvent
{
  public:
  std::vector<TAGSpill*> spill_events;

  AGSpillFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
  }
  ~AGSpillFlow()
  {
     for (size_t i=0; i<spill_events.size(); i++)
        delete spill_events[i];
     spill_events.clear();
  }

};

#include "TStoreEvent.hh"
class AgAnalysisFlow: public TAFlowEvent
{
 public:
   TStoreEvent *fEvent;

 public:
 AgAnalysisFlow(TAFlowEvent* flow, TStoreEvent* e) // ctor
   : TAFlowEvent(flow)
   { 
      fEvent=e;
   }
  ~AgAnalysisFlow()
  {
     if (fEvent) delete fEvent;
     fEvent=NULL;
  }

};

#include "SignalsType.hh"
class AgSignalsFlow: public TAFlowEvent
{
public:
  std::vector<asignal>* awSig;
  std::vector<asignal>* pdSig;
  std::vector< std::pair<asignal,asignal> >* matchSig;

  std::vector<wf_ref>* AWwf;
  std::vector<wf_ref>* PADwf;

  std::vector<asignal>* adc32max;
   //  std::vector<asignal> adc32range;
  std::vector<asignal>* pwbMax;
   //  std::vector<asignal> pwbRange;

public:
  AgSignalsFlow(TAFlowEvent* flow,
		std::vector<asignal> *s):
    TAFlowEvent(flow)
  {
    AWwf=NULL;
    PADwf=NULL;
    awSig=s;
    pdSig=NULL;
    matchSig=NULL;
    adc32max=NULL;
    pwbMax=NULL;
  }

  AgSignalsFlow(TAFlowEvent* flow,
  		std::vector<asignal>* s,
  		std::vector<asignal>* p):
    TAFlowEvent(flow)
  {
    AWwf=NULL;
    PADwf=NULL;
    awSig=s;
    pdSig=p;
    matchSig=NULL;   
    adc32max=NULL;
    pwbMax=NULL;
  }

  AgSignalsFlow(TAFlowEvent* flow,
		std::vector<asignal>* s,std::vector<asignal>* p,
		std::vector<wf_ref>* awf, std::vector<wf_ref>* pwf):
    TAFlowEvent(flow)
  {
    AWwf=awf;
    PADwf=pwf;
    awSig=s;
    pdSig=p;
    matchSig=NULL;   
    adc32max=NULL;
    pwbMax=NULL;
  }

  ~AgSignalsFlow()
  {
    if (awSig)
    {
       awSig->clear();
       delete awSig;
    }
    if (pdSig)
    {
       pdSig->clear();
       delete pdSig;
    }
    if (matchSig)
    {
       matchSig->clear();
       delete matchSig;
    }
    if (AWwf)
    {
       // for (size_t i=0; i<AWwf->size(); i++)
       //    delete AWwf->at(i).wf;
       AWwf->clear();
       delete AWwf;
    }

    if (PADwf)
    {
       // for (size_t i=0; i<PADwf->size(); i++)
       //    delete PADwf->at(i).wf;
       PADwf->clear();
       delete PADwf;
    }

    if( adc32max )
       {
          adc32max->clear();
          delete adc32max;
       }
    //    adc32range.clear();
    if( pwbMax )
       {
          pwbMax->clear();
          delete pwbMax;
       }
    //    pwbRange.clear();
  }

  void DeletePadSignals()
  {
    delete pdSig;
    pdSig=0;
  }
  void AddPadSignals( std::vector<asignal>* s )
  {
    pdSig=s;
  }

  void AddMatchSignals( std::vector< std::pair<asignal,asignal> >*ss )
  {
    matchSig=ss;
  }

  void AddAWWaveforms(std::vector<wf_ref>* af)
  {
    AWwf=af;
  }

  void AddPADWaveforms(std::vector<wf_ref>* pf)
  {
    PADwf=pf;
  }

  void AddWaveforms(std::vector<wf_ref>* af, std::vector<wf_ref>* pf)
  {
    AWwf=af;
    PADwf=pf;
  }

   void AddAdcPeaks(std::vector<asignal>* s)
   {
      adc32max=s;
   }

   void AddPwbPeaks(std::vector<asignal>* s)
   {
      pwbMax=s;
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
  ~AgTrigUdpFlow(){}
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
