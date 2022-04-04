#ifndef _TDUMPMARKER_
#define _TDUMPMARKER_

#include "Sequencer_Channels.h"
#include "RtypesCore.h"

class TDumpMarker
{
   public:
   enum kDumpTypes { Info, Start, Stop, ADSpill, Positrons, Mixing, FRD, Other};
   std::string fDescription;
   int fSequencerID; //Unique to each sequencer
   int fSequenceCount; //Sequence number in run
   int fDumpType;
   int fonCount;
   int fonState;
   double fRunTime; //SIS/Chronobox time stamp (official time)
   uint32_t fMidasTime; //Sequence start time
   TDumpMarker();
   TDumpMarker(
      const char* _Description,
      Int_t _SequencerNum,
      Int_t _SeqCount,
      TDumpMarker::kDumpTypes _DumpType,
      Int_t _onCount,
      Int_t _onState,
      double runTime,
      uint32_t _MidasTime) ;
   TDumpMarker(const char* name,kDumpTypes type);
   TDumpMarker(const TDumpMarker& d);
   void Print() const;

};

#endif