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
#include "TracksFinder.hh"
#include "TFitVertex.hh"
class AgSignalsFlow: public TAFlowEvent
{
public:
  std::vector<ALPHAg::TWireSignal> awSig;
  std::vector<ALPHAg::TPadSignal> pdSig;

  std::vector<ALPHAg::wfholder> PadWaves; //Intermediary within deconv_pad_module
  std::vector<ALPHAg::electrode> PadIndex; //Intermediary within deconv_pad_module

  std::vector< std::vector<ALPHAg::TPadSignal> > comb; //Intermediary within match_modules
  std::vector<ALPHAg::TPadSignal> combinedPads; //Intermediary within match_modules

  std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal> > matchSig;

  // Reco associated containers
  bool fSkipReco = false;
  std::vector< TSpacePoint> fSpacePoints;
  std::vector< track_t> fTrackVector;
  std::vector<TTrack> fTracksArray;
  std::vector<TFitLine*> fLinesArray;
  std::vector<TFitHelix> fHelixArray;

  std::vector<ALPHAg::wf_ref> AWwf;
  std::vector<ALPHAg::wf_ref> PADwf;

  std::vector<ALPHAg::TWireSignal> adc32max;
   //  std::vector<signal> adc32range;
  std::vector<ALPHAg::TPadSignal> pwbMax;
   //  std::vector<ALPHAg::signal> pwbRange;
  int fitStatus;
  TFitVertex fitVertex;
public:
  AgSignalsFlow(TAFlowEvent* flow):
    TAFlowEvent(flow)
  {
     fSkipReco = false;
     fitStatus = 0;
  }
  
  AgSignalsFlow(TAFlowEvent* flow,
		std::vector<ALPHAg::TWireSignal> s):
    TAFlowEvent(flow)
  {
    fSkipReco = false;
    awSig = std::move(s);
    fitStatus = 0;
  }

  AgSignalsFlow(TAFlowEvent* flow,
  		std::vector<ALPHAg::TWireSignal>& s,
  		std::vector<ALPHAg::TPadSignal>& p):
    TAFlowEvent(flow)
  {
    fSkipReco = false;
    awSig = std::move(s);
    pdSig = std::move(p);
    fitStatus = 0;
  }

  AgSignalsFlow(TAFlowEvent* flow,
		std::vector<ALPHAg::TWireSignal>& s,std::vector<ALPHAg::TPadSignal>& p,
		std::vector<ALPHAg::wf_ref>& awf, std::vector<ALPHAg::wf_ref>& pwf):
    TAFlowEvent(flow)
  {
    fSkipReco = false;
    AWwf = std::move(awf);
    PADwf = std::move(pwf);
    awSig = std::move(s);
    pdSig = std::move(p);
    fitStatus = 0;
  }

  ~AgSignalsFlow()
  {
     awSig.clear();
     pdSig.clear();
     matchSig.clear();
     AWwf.clear();
     PADwf.clear();
     adc32max.clear();
     pwbMax.clear();
     fHelixArray.clear();
     for (TFitLine* h: fLinesArray)
        delete h;
     fLinesArray.clear();
  }

  void DeletePadSignals()
  {
     pdSig.clear();
  }

  void AddPadSignals( std::vector<ALPHAg::TPadSignal> s )
  {
    pdSig = std::move(s);
  }

  void AddMatchSignals( std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal>> ss )
  {
     //matchSig=ss;
     matchSig = std::move(ss);
  }

  void AddAWWaveforms(std::vector<ALPHAg::wf_ref> af)
  {
    AWwf = std::move(af);
  }

  void AddPADWaveforms(std::vector<ALPHAg::wf_ref> pf)
  {
    PADwf = std::move(pf);
  }

  void AddWaveforms(std::vector<ALPHAg::wf_ref> af, std::vector<ALPHAg::wf_ref> pf)
  {
    AWwf = std::move(af);
    PADwf = std::move(pf);
  }

   void AddAdcPeaks(std::vector<ALPHAg::TWireSignal> s)
   {
      adc32max = std::move(s);
   }

   void AddPwbPeaks(std::vector<ALPHAg::TPadSignal> s)
   {
      pwbMax = std::move(s);
   }
};

#include "TKDTree.h"
#include <string>
#include "TKDTreeMatch.hh"


class AgKDTreeMatchFlow: public TAFlowEvent
{
   public:
      std::list<KDTreeIDContainer2D*> f2DTrees;
      AgKDTreeMatchFlow(TAFlowEvent* flow):
         TAFlowEvent(flow)
      {

      }
      ~AgKDTreeMatchFlow()
      {
         for (KDTreeIDContainer2D* t: f2DTrees)
            delete t;
         f2DTrees.clear();
      }
      KDTreeIDContainer2D* AddKDTree(int entries, const char* name)
      {
         f2DTrees.emplace_back(new KDTreeIDContainer2D(entries,name));
         return f2DTrees.back();
      }
      KDTreeIDContainer2D* GetTree(const std::string name)
      {
         for (KDTreeIDContainer2D* t: f2DTrees)
         {
            if (t->IsMatch(name))
               return t;
         }
         return nullptr;
      }
      std::vector<double>* GetXArray(const std::string name)
      { 
         KDTreeIDContainer2D* t = GetTree(name);
         if (t)
            return &t->X();
         else
            return nullptr;
      }
      std::vector<double>* GetYArray(std::string name)
      { 
         KDTreeIDContainer2D* t = GetTree(name);
         if (t)
            return &t->Y();
         else
            return nullptr;
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
