#ifndef _ANALYSIS_FLOW_
#define _ANALYSIS_FLOW_
//
// AnalysisFlow.h
//
// manalyzer flow objects for ALPHA-g and ALPHA 2 events
// 
// JTKM
//
#include "manalyzer.h"
#include <iostream>

//This should probably live somewhere else as its a A2 & Ag data type
#include "TInfoSpill.h"
class TInfoSpillFlow: public TAFlowEvent
{
  public:
  std::vector<TInfoSpill*> spill_events;

  TInfoSpillFlow(TAFlowEvent* flow): TAFlowEvent(flow)
  {
  }
  ~TInfoSpillFlow()
  {
     for (size_t i=0; i<spill_events.size(); i++)
        delete spill_events[i];
     spill_events.clear();
  }
};


#include "TDumpList.h"
#include "TSequencerState.h"
#include "TSequencerDriver.h"
class DumpFlow: public TAFlowEvent
{
  public:
    int SequencerNum=-1;
    std::vector<TDumpMarker> DumpMarkers;
    std::vector<TSequencerState> fStates;
    TSequencerDriver* driver;
  public:
  DumpFlow(TAFlowEvent* flow) // ctor
    : TAFlowEvent(flow)
   {
   }
   ~DumpFlow()
   {
      if (driver)
         delete driver;
      fStates.clear();
      DumpMarkers.clear();
   }
  void AddDumpEvent(Int_t _SequencerNum, Int_t _SeqCount, uint32_t SequenceStartTime, TString _Description, TDumpMarker::kDumpTypes _DumpType, Int_t _onCount, Int_t _onState) 
   {
      if (SequencerNum<0) SequencerNum=_SequencerNum;
      else if (SequencerNum!=_SequencerNum)
      {
         std::cout<<"ERROR! Parsing sequencer data that has data for more than one sequencer... something went very wrong"<<std::endl;
         exit(1);
      }

      TDumpMarker Marker(
         _Description,
         _SequencerNum,
         _SeqCount,
         _DumpType,
         _onCount,
         _onState,
         -1.,
         SequenceStartTime
         );
      DumpMarkers.push_back(Marker);
   }
   void AddDumpEvent(Int_t _SequencerNum,Int_t _SeqCount, uint32_t _SequenceStartTime, TString _Description, const char* _DumpType, Int_t _onCount, Int_t _onState) 
   {
     TDumpMarker::kDumpTypes type= TDumpMarker::kDumpTypes::Other;
     if (strcmp(_DumpType,"startDump")==0)
        type=TDumpMarker::kDumpTypes::Start;
     else if (strcmp(_DumpType,"stopDump")==0)
        type=TDumpMarker::kDumpTypes::Stop;
      AddDumpEvent(_SequencerNum, _SeqCount, _SequenceStartTime, _Description, type, _onCount, _onState);
   }

   void AddStateEvent(const TSequencerState& s )
   {
      fStates.push_back(s);
   }
};

#endif
