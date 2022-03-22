#ifndef _TAGDETECTOREVENT_
#define _TAGDETECTOREVENT_
#include "TObject.h"
#include "TVector3.h"
#include <iostream>

#include "TStoreEvent.hh"
#include "TBarEvent.hh"

// Compound mini container that holds data for "online" analysis
// TStoreEvent +  TBarEvent but on a diet for small root file size (therefore speed)!
class TAGDetectorEvent: public TObject
{
public:
   bool fHasBarFlow;
   bool fHasAnalysisFlow;
   
   
   int fRunNumber;
   int fEventNo;
   double fRunTime; // Official time
   double fTPCTime; //TPC time stamp
   int fCutsResult;
   TVector3 fVertex;
   int fVertexStatus;
   int fNumHelices;  // helices used for vertexing
   int fNumTracks; // reconstructed (good) helices
   int fNumADCBars;
   int fNumTDCBars;
   double fBarTime;
   TAGDetectorEvent();
   ~TAGDetectorEvent();
   // Construct with TStoreEvent
   TAGDetectorEvent( const TStoreEvent* e );
   TAGDetectorEvent( const TBarEvent* b );
   bool AddAnalysisFlow(const TStoreEvent* event);
   bool AddBarFlow( const TBarEvent* b);
   
   // Online analysis cuts
   int RadiusCut(const TStoreEvent* e);

   // Spill log:
   double GetTimeOfEvent() const { return fRunTime; }
   int GetEventNumber() const { return fEventNo; }
   int GetVertexStatus() const { return fVertexStatus; }
   int GetOnlinePassCuts() const { return fCutsResult & 1;}
   int GetOnlinePassMVA() const { return fCutsResult & 2;}


   
   
   ClassDef(TAGDetectorEvent,1);
};

#endif