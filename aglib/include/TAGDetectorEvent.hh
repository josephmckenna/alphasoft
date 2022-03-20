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
   TVector3 fVertex;
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
   ClassDef(TAGDetectorEvent,1);
};

#endif