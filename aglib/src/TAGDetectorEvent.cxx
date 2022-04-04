#include "TAGDetectorEvent.hh"

TAGDetectorEvent::TAGDetectorEvent():
   fVertex(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown)
{
   fHasBarFlow = false;
   fHasAnalysisFlow = false;
   fRunNumber = -1;
   fEventNo = -1;
   fRunTime = -1;
   fCutsResult = 0;
   fVertexStatus = 0;
   fTPCTime = -1;
   fNumHelices = -1;
   fNumTracks = -1;
   fNumADCBars = -1;
   fNumTDCBars = -1;
   fBarTime = -1;
   fBarMaxTOF = -1;
   fBarTOFstdv = -1;
}
TAGDetectorEvent::~TAGDetectorEvent()
{
   
}
// Construct with TStoreEvent
TAGDetectorEvent::TAGDetectorEvent( const TStoreEvent* e ): TAGDetectorEvent()
{
   fHasBarFlow = false;
   fHasAnalysisFlow = false;
   AddAnalysisFlow(e);
}
TAGDetectorEvent::TAGDetectorEvent( const TBarEvent* b ): TAGDetectorEvent()
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
   fCutsResult = RadiusCut(event) & 1;
   fVertex = event->GetVertex();
   fVertexStatus = event->GetVertexStatus();
   fNumHelices = event->GetUsedHelices()->GetEntriesFast();
   fNumTracks = event->GetNumberOfTracks();
   fNumADCBars = -1;
   fNumTDCBars = -1;
   fBarMaxTOF = -1;
   fBarTOFstdv = -1;
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
   std::vector<double> tofs = b->GetTOFs();
   if (tofs.size())
   {
      fBarMaxTOF = *std::max_element(tofs.begin(),tofs.end());
      //fBarMaxTOF *= 1e9; //Convert to ns
      fBarTOFstdv = 0;
      for (const double& d: tofs)
         fBarTOFstdv += d*d;
      fBarTOFstdv = sqrt(fBarTOFstdv);
      //fBarTOFstdv *= 1e9; //convert to ns
   }
   else
   {
      fBarMaxTOF = -1;
      fBarTOFstdv = -1;
   }
   return true;
}

int TAGDetectorEvent::RadiusCut(const TStoreEvent* e)
{
   //Dummy example of ApplyCuts for a TStoreEvent... 
   //Change this when we have pbars!
   const Double_t R=e->GetVertex().Perp();
   const Int_t NTracks=e->GetNumberOfTracks();
   if (NTracks==2)
      if (R < 85) return 1;
   if (NTracks > 2)
      if (R < 80) return 1;
   return 0;
}


ClassImp(TAGDetectorEvent);