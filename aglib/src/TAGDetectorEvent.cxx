#include "TAGDetectorEvent.hh"


TAGDetectorEvent::TAGDetectorEvent():
   fVertex(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown)
{
   fHasBarFlow = false;
   fHasAnalysisFlow = false;
   fRunNumber = -1;
   fEventNo = -1;
   fRunTime = -1;
   fTPCTime = -1;
   fNumHelices = -1;
   fNumTracks = -1;
   fNumADCBars = -1;
   fNumTDCBars = -1;
   fBarTime = -1;
}
TAGDetectorEvent::~TAGDetectorEvent()
{
   
}
// Construct with TStoreEvent
TAGDetectorEvent::TAGDetectorEvent( const TStoreEvent* e )
{
   fHasBarFlow = false;
   fHasAnalysisFlow = false;
   AddAnalysisFlow(e);
}
TAGDetectorEvent::TAGDetectorEvent( const TBarEvent* b )
{
   fHasBarFlow = false;
   fHasAnalysisFlow = false;
   AddBarFlow(b);
}
bool TAGDetectorEvent::AddAnalysisFlow(const TStoreEvent* event)
{
   //This already has TPC data
   if (fHasAnalysisFlow)
      return false;
   // If this contains bar data, the event number must match!
   if (fHasBarFlow && fEventNo != event->GetEventNumber())
      return false;
   fHasAnalysisFlow = true;
   fRunNumber =  event->GetRunNumber();
   fEventNo = event->GetEventNumber();
   fRunTime = event->GetTimeOfEvent();
   fTPCTime = event->GetTimeOfEvent();
   fVertex = event->GetVertex();
   fNumHelices = event->GetUsedHelices()->GetEntriesFast();
   fNumTracks = event->GetNumberOfTracks();
   fNumADCBars = -1;
   fNumTDCBars = -1;
   return true;
}
bool TAGDetectorEvent::AddBarFlow( const TBarEvent* b)
{
   //This already has bar data
   if (fHasBarFlow)
      return false;
   if (fHasAnalysisFlow && fEventNo != b->GetID())
      return false;
   fHasBarFlow = true;
   fEventNo = b->GetID();
   fNumADCBars = b->GetNBars();
   fNumTDCBars = b->GetTdcHits().size();
   fBarTime = b->GetRunTime();
   return true;
}

ClassImp(TAGDetectorEvent);