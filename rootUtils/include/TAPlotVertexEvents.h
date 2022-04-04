#ifndef _TAPlotVertexEvents_
#define _TAPlotVertexEvents_
//SVD / TPC vertex
class TAPlotVertexEvents: public TObject
{
   public:
      std::vector<int> fRunNumbers; 
      std::vector<int> fEventNos;
      std::vector<int> fCutsResults;
      std::vector<int> fVertexStatuses;
      std::vector<double> fXVertex;
      std::vector<double> fYVertex;
      std::vector<double> fZVertex;
      std::vector<double> fTimes; //Plot time (based off official time)
      std::vector<double> fEventTimes; //TPC time stamp
      std::vector<double> fRunTimes; //Official Time
      std::vector<int> fNumHelices; // helices used for vertexing
      std::vector<int> fNumTracks; // reconstructed (good) helices

      //Copy and assign operators
      TAPlotVertexEvents()
      {
      }
      ~TAPlotVertexEvents()
      {
      }
      //Basic copy constructor.
      TAPlotVertexEvents(const TAPlotVertexEvents& vertexEvents) : TObject(vertexEvents)
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
      TAPlotVertexEvents& operator=(const TAPlotVertexEvents& rhs)
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
      TAPlotVertexEvents operator+=(const TAPlotVertexEvents &rhs) 
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
      std::string CSVTitleLine() const
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
      std::string CSVLine(size_t i) const
      {
         //This is a little fugly
         std::string line;
         line =std::string("") + fRunNumbers.at(i) + "," +
                fEventNos.at(i) + "," +
                fTimes.at(i) + "," +
                fEventTimes.at(i) + "," +
                fRunTimes.at(i) + "," +
                bool(fCutsResults.at(i)&1) + "," +
                bool(fCutsResults.at(i)&2) + "," +
                fVertexStatuses.at(i) + "," +
                fXVertex.at(i) + "," +
                fYVertex.at(i) + "," +
                fZVertex.at(i) + "," +
                fNumHelices.at(i) + "," +
                fNumTracks.at(i) + "\n";
         return line;
      }
      size_t size() const
      {
         return fRunNumbers.size();
      }

   ClassDef(TAPlotVertexEvents, 1);
};


#endif