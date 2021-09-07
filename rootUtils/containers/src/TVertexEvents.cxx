#include "TVertexEvents.h"

ClassImp(TVertexEvents)

//Copy and assign operators
TVertexEvents::TVertexEvents()
{

}

TVertexEvents::~TVertexEvents()
{

}

//Basic copy constructor.
TVertexEvents::TVertexEvents(const TVertexEvents& vertexEvents) : TObject(vertexEvents)
{
   //Making deep copies of the vectors, we require loop method as the vectors are already initialised earlier.
   for(size_t i = 0; i < vertexEvents.fXVertex.size(); i++)
   {
      fRunNumbers.push_back(vertexEvents.fRunNumbers[i]);
      fEventNos.push_back(vertexEvents.fEventNos[i]);
      fCutsResults.push_back(vertexEvents.fCutsResults[i]);
      fVertexStatuses.push_back(vertexEvents.fVertexStatuses[i]);
      fXVertex.push_back(vertexEvents.fXVertex[i]);
      fYVertex.push_back(vertexEvents.fYVertex[i]);
      fZVertex.push_back(vertexEvents.fZVertex[i]);
      fTimes.push_back(vertexEvents.fTimes[i]);
      fEventTimes.push_back(vertexEvents.fEventTimes[i]);
      fRunTimes.push_back(vertexEvents.fRunTimes[i]);
      fNumHelices.push_back(vertexEvents.fNumHelices[i]);
      fNumTracks.push_back(vertexEvents.fNumTracks[i]);
   }
}

//Assignment operator.
TVertexEvents& TVertexEvents::operator=(const TVertexEvents& rhs)
{
   //Making deep copies of the vectors, we require loop method as the vectors are already initialised earlier.
   for(size_t i = 0; i < rhs.fXVertex.size(); i++)
   {
      this->fRunNumbers.push_back(rhs.fRunNumbers[i]);
      this->fEventNos.push_back(rhs.fEventNos[i]);
      this->fCutsResults.push_back(rhs.fCutsResults[i]);
      this->fVertexStatuses.push_back(rhs.fVertexStatuses[i]);
      this->fXVertex.push_back(rhs.fXVertex[i]);
      this->fYVertex.push_back(rhs.fYVertex[i]);
      this->fZVertex.push_back(rhs.fZVertex[i]);
      this->fTimes.push_back(rhs.fTimes[i]);
      this->fEventTimes.push_back(rhs.fEventTimes[i]);
      this->fRunTimes.push_back(rhs.fRunTimes[i]);
      this->fNumHelices.push_back(rhs.fNumHelices[i]);
      this->fNumTracks.push_back(rhs.fNumTracks[i]);
   }
   return *this;
}

//=+ Operator.
TVertexEvents TVertexEvents::operator+=(const TVertexEvents &rhs) 
{
   //std::cout << "TAVertexEvents += operator" << std::endl;
   this->fRunNumbers       .insert(this->fRunNumbers.end(),      rhs.fRunNumbers.begin(),     rhs.fRunNumbers.end());
   this->fEventNos         .insert(this->fEventNos.end(),        rhs.fEventNos.begin(),       rhs.fEventNos.end());
   this->fCutsResults      .insert(this->fCutsResults.end(),     rhs.fCutsResults.begin(),    rhs.fCutsResults.end()); 
   this->fVertexStatuses   .insert(this->fVertexStatuses.end(),  rhs.fVertexStatuses.begin(), rhs.fVertexStatuses.end());    
   this->fXVertex               .insert(this->fXVertex.end(),              rhs.fXVertex.begin(),             rhs.fXVertex.end());
   this->fYVertex               .insert(this->fYVertex.end(),              rhs.fYVertex.begin(),             rhs.fYVertex.end());
   this->fZVertex               .insert(this->fZVertex.end(),              rhs.fZVertex.begin(),             rhs.fZVertex.end());
   this->fTimes               .insert(this->fTimes.end(),              rhs.fTimes.begin(),             rhs.fTimes.end());
   this->fEventTimes       .insert(this->fEventTimes.end(),      rhs.fEventTimes.begin(),     rhs.fEventTimes.end());
   this->fRunTimes         .insert(this->fRunTimes.end(),        rhs.fRunTimes.begin(),       rhs.fRunTimes.end());
   this->fNumHelices         .insert(this->fNumHelices.end(),        rhs.fNumHelices.begin(),       rhs.fNumHelices.end());
   this->fNumTracks          .insert(this->fNumTracks.end(),         rhs.fNumTracks.begin(),        rhs.fNumTracks.end());
   return *this;
}

std::string TVertexEvents::CSVTitleLine() const
{
   return std::string("Run Number,") + 
           "Event Number," +
           "Plot Time (Time axis of TAPlot)," +
           "Detector Time (detectors internal clock)," +
           "OfficialTime (run time)," +
           "Pass Cuts," +
           "Pass MVA," +
           "Vertex Status," +
           "X," +
           "Y," +
           "Z," +
           "Number of Helices," +
           "Number of Tracks\n";
}

std::string TVertexEvents::CSVLine(size_t i) const
{
    //This is a little fugly
    return std::to_string(fRunNumbers.at(i)) + "," +
                std::to_string(fEventNos.at(i)) + "," +
                std::to_string(fTimes.at(i)) + "," +
                std::to_string(fEventTimes.at(i)) + "," +
                std::to_string(fRunTimes.at(i)) + "," +
                std::to_string(bool(fCutsResults.at(i)&1)) + "," +
                std::to_string(bool(fCutsResults.at(i)&2)) + "," +
                std::to_string(fVertexStatuses.at(i)) + "," +
                std::to_string(fXVertex.at(i)) + "," +
                std::to_string(fYVertex.at(i)) + "," +
                std::to_string(fZVertex.at(i)) + "," +
                std::to_string(fNumHelices.at(i)) + "," +
                std::to_string(fNumTracks.at(i)) + "\n";

}

size_t TVertexEvents::size() const
{
   return fRunNumbers.size();
}
