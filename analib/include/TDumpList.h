#ifndef _DUMPHANDLING_
#define _DUMPHANDLING_

#include "TSequencerState.h"
#include "Sequencer_Channels.h"
#include <deque>
#include "TDumpMarker.h"
#include "TDumpMarkerVertexCounts.h"

//Universal headers
#include "TSequencerDriver.h"
//ALPHA 2 headers..


//ALPHA g headers...
#include "TDumpMarkerPair.h"

template<typename SpillType, typename VertexType, typename ScalerType, int NumScalers>//, typename ScalerType, typename VertexType >
class TDumpList
{
public:
   int fRunNo;
   int fSeqcount=-1;
   int fSequencerID;
   std::deque<TDumpMarkerPair<VertexType,ScalerType,NumScalers>*> fDumps;
private:
   //Sequentially sorted pointers to the above dump pairs
   std::deque<TDumpMarker*> fOrderedStarts;
   std::deque<TDumpMarker*> fOrderedStops;
   //Hold errors in queue for ansyncronus error reporting
public:
   std::deque<SpillType*> fErrorQueue;
   //Sequence ID and Start time:
   std::deque<std::pair<int,uint32_t>> fSequenceStartTimes;

   TDumpList();
   TDumpMarkerPair<VertexType,ScalerType,NumScalers>* GetPairOfStart(TDumpMarker* d);
   TDumpMarkerPair<VertexType,ScalerType,NumScalers>* GetPairOfStop(TDumpMarker* d);
   SpillType* GetError();

   bool AddStartDump(const TDumpMarker& d);
   //Return true if dump is paired
   bool AddStopDump(const TDumpMarker& d);
   void AddSequencerStartTime(const TDumpMarker& d);
   
   bool AddDump(const TDumpMarker& d);
   void AddStates(std::vector<TSequencerState> s);
   void RemoveAndCleanAbortedSequence(int SequenceCount);
   void AddStartTime(uint32_t midas_time, double t);
   void AddStopTime(uint32_t midas_time, double t);

   void AddAndSortScalerEvents(std::vector<std::vector<ScalerType>> events);

   void AddScalerEvents(std::vector<ScalerType> events);
   void AddScalerEvents(std::vector<ScalerType*> events);
   void AddSVDEvents(std::vector<VertexType*>* events);
   
   void check(TSequencerDriver* d);
   void Print();
   int countIncomplete();
   std::vector<SpillType*> flushComplete();
   void clear();
   void setup(uint32_t unixtime);
   void finish();
};

#endif
