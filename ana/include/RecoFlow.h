//
// RecoFlow.h
//
// manalyzer flow objects for ALPHA-g events
// 
// NM, LM, AC, JTKM
//

#ifndef _RECOFLOW_
#define _RECOFLOW_ 1

#ifdef BUILD_AG

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
            //   BarEvent->Reset();
            delete BarEvent;
         }
   }
};


#endif

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
#ifdef BUILD_AG
#include "TAGSpill.h"

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
  std::vector<ALPHAg::signal>* awSig;
  std::vector<ALPHAg::signal>* pdSig;

  std::vector< std::vector<ALPHAg::signal> > comb; //Intermediary within match_modules
  std::vector<ALPHAg::signal>* combinedPads; //Intermediary within match_modules

  std::vector< std::pair<ALPHAg::signal,ALPHAg::signal> >* matchSig;

  std::vector<ALPHAg::wf_ref>* AWwf;
  std::vector<ALPHAg::wf_ref>* PADwf;

  std::vector<ALPHAg::signal>* adc32max;
   //  std::vector<signal> adc32range;
  std::vector<ALPHAg::signal>* pwbMax;
   //  std::vector<ALPHAg::signal> pwbRange;

public:
  AgSignalsFlow(TAFlowEvent* flow,
		std::vector<ALPHAg::signal> *s):
    TAFlowEvent(flow)
  {
    AWwf=NULL;
    PADwf=NULL;
    awSig=s;
    pdSig=NULL;
    combinedPads=NULL;
    matchSig=NULL;
    adc32max=NULL;
    pwbMax=NULL;
  }

  AgSignalsFlow(TAFlowEvent* flow,
  		std::vector<ALPHAg::signal>* s,
  		std::vector<ALPHAg::signal>* p):
    TAFlowEvent(flow)
  {
    AWwf=NULL;
    PADwf=NULL;
    awSig=s;
    pdSig=p;
    combinedPads=NULL;
    matchSig=NULL;   
    adc32max=NULL;
    pwbMax=NULL;
  }

  AgSignalsFlow(TAFlowEvent* flow,
		std::vector<ALPHAg::signal>* s,std::vector<ALPHAg::signal>* p,
		std::vector<ALPHAg::wf_ref>* awf, std::vector<ALPHAg::wf_ref>* pwf):
    TAFlowEvent(flow)
  {
    AWwf=awf;
    PADwf=pwf;
    awSig=s;
    pdSig=p;
    combinedPads=NULL;
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
  void AddPadSignals( std::vector<ALPHAg::signal>* s )
  {
    pdSig=s;
  }

  void AddMatchSignals( std::vector< std::pair<ALPHAg::signal,ALPHAg::signal> >*ss )
  {
     //matchSig=ss;
     matchSig= new std::vector< std::pair<ALPHAg::signal,ALPHAg::signal> >(*ss);
  }

  void AddAWWaveforms(std::vector<ALPHAg::wf_ref>* af)
  {
    AWwf=af;
  }

  void AddPADWaveforms(std::vector<ALPHAg::wf_ref>* pf)
  {
    PADwf=pf;
  }

  void AddWaveforms(std::vector<ALPHAg::wf_ref>* af, std::vector<ALPHAg::wf_ref>* pf)
  {
    AWwf=af;
    PADwf=pf;
  }

   void AddAdcPeaks(std::vector<ALPHAg::signal>* s)
   {
      adc32max=s;
   }

   void AddPwbPeaks(std::vector<ALPHAg::signal>* s)
   {
      pwbMax=s;
   }
};

#endif
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
