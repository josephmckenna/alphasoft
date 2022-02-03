
#ifndef _TDUMPPAIRMARKERPAIR_
#define _TDUMPPAIRMARKERPAIR_

#include "TDumpMarker.h"
#include "TDumpMarkerVertexCounts.h"
#include "TSequencerState.h"

template<typename VertexType, typename ScalerType, int NumScalers>
class TDumpMarkerPair
{
public:
   int fDumpID;
   TDumpMarker* fStartDumpMarker;
   TDumpMarker* fStopDumpMarker;
   //Enum for SIS, SVD and Chronnobox
   enum kSTATUS {NO_EQUIPMENT, NOT_FILLED, FILLED};
   //ALPHA 2:
   std::vector<kSTATUS> fScalerFilled;
   std::vector<ScalerType> fIntegratedScalerCounts;
   kSTATUS fVertexFilled;
   TDumpMarkerVertexCounts<VertexType> fIntegratedVertexCounts;
   bool fIsPaired = false;
   bool fIsFinished = false; //Only true if I have been printed (thus safely destroyed)
   std::vector<TSequencerState> fStates;
   TDumpMarkerPair();
   TDumpMarkerPair(const TDumpMarker& startDump);
   ~TDumpMarkerPair();
   bool Ready();
   void Print();
   //Maybe we should add MIDAS speaker announcements inside this 
   //function, most of these failure modes are critical
   std::vector<std::string> check(int DumpStart, int DumpStop);
   void clear();
   void AddStartDump(const TDumpMarker& d);
   bool AddStopDump(const TDumpMarker& d);
   int AddState(const TSequencerState& s);
   int AddScalerEvent(const ScalerType& s);
   int AddSVDEvent(const VertexType& s);
};
#endif
