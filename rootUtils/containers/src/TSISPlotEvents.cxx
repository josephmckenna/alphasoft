#include "TSISPlotEvents.h"

ClassImp(TSISPlotEvents)

//Std ctor and dtor
TSISPlotEvents::TSISPlotEvents()
{

}

TSISPlotEvents::~TSISPlotEvents()
{

}

//Copy ctor - !!!
TSISPlotEvents::TSISPlotEvents(const TSISPlotEvents& sisPlotEvents) : TObject(sisPlotEvents)
{
   //Deep copy vectors.
   for(size_t i=0; i<sisPlotEvents.fTime.size(); i++)
   {
      fRunNumber.push_back( sisPlotEvents.fRunNumber[i]);
      fTime.push_back( sisPlotEvents.fTime[i]);
      fOfficialTime.push_back( sisPlotEvents.fOfficialTime[i]);
      fCounts.push_back( sisPlotEvents.fCounts[i]);
      fSISChannel.push_back( sisPlotEvents.fSISChannel[i]);
   }
}

TSISPlotEvents TSISPlotEvents::operator+=(const TSISPlotEvents &rhs) 
{
   //std::cout << "TSISPlotEvents += operator" << std::endl;
   this->fRunNumber   .insert(this->fRunNumber.end(),      rhs.fRunNumber.begin(),      rhs.fRunNumber.end() );
   this->fTime           .insert(this->fTime.end(),              rhs.fTime.begin(),              rhs.fTime.end() );
   this->fOfficialTime.insert(this->fOfficialTime.end(),   rhs.fOfficialTime.begin(),   rhs.fOfficialTime.end() );
   this->fCounts      .insert(this->fCounts.end(),         rhs.fCounts.begin(),         rhs.fCounts.end() );
   this->fSISChannel .insert(this->fSISChannel.end(),    rhs.fSISChannel.begin(),    rhs.fSISChannel.end() );
   return *this;
}

TSISPlotEvents& TSISPlotEvents::operator=(const TSISPlotEvents& sisPlotEvents)
{
   for(size_t i = 0; i<sisPlotEvents.fTime.size(); i++)
   {
      this->fRunNumber.push_back( sisPlotEvents.fRunNumber[i]);
      this->fTime.push_back( sisPlotEvents.fTime[i]);
      this->fOfficialTime.push_back( sisPlotEvents.fOfficialTime[i]);
      this->fCounts.push_back( sisPlotEvents.fCounts[i]);
      this->fSISChannel.push_back( sisPlotEvents.fSISChannel[i]);
   }
   return *this;
}

TSISPlotEvents operator+(const TSISPlotEvents& lhs, const TSISPlotEvents& rhs)
{
   //std::cout << "TSISPlotEvents addition operator" << std::endl;
   TSISPlotEvents outputplot(lhs); //Create new from copy
   //Vectors- need concacting
   outputplot.fRunNumber.insert(outputplot.fRunNumber.end(), rhs.fRunNumber.begin(), rhs.fRunNumber.end() );
   outputplot.fTime.insert(outputplot.fTime.end(), rhs.fTime.begin(), rhs.fTime.end() );
   outputplot.fOfficialTime.insert(outputplot.fOfficialTime.end(), rhs.fOfficialTime.begin(), rhs.fOfficialTime.end() );
   outputplot.fCounts.insert(outputplot.fCounts.end(), rhs.fCounts.begin(), rhs.fCounts.end() );
   outputplot.fSISChannel.insert(outputplot.fSISChannel.end(), rhs.fSISChannel.begin(), rhs.fSISChannel.end() );
   return outputplot;
}

void TSISPlotEvents::AddEvent(int runNumber, double time, double officialTime, int counts, int channel)
{
   fRunNumber.push_back(runNumber);
   fTime.push_back(time);  
   fOfficialTime.push_back(officialTime);
   fCounts.push_back(counts);
   fSISChannel.push_back(channel);
}

int TSISPlotEvents::GetEventRunNumber(int event) const
{
   return fRunNumber.at(event);
}

std::string TSISPlotEvents::CSVTitleLine() const
{
   std::string line = std::string("Run Number,") + 
           "Plot Time (Time axis of TAPlot)," +
           "OfficialTime (run time)," +
           "SIS Channel," +
           "Counts\n";
   return line;
}

std::string TSISPlotEvents::CSVLine(size_t i) const
{
   //This is a little fugly
   std::string line = std::to_string(fRunNumber.at(i)) + "," +
                std::to_string(fTime.at(i)) + "," +
                std::to_string(fOfficialTime.at(i)) + "," +
                std::to_string(fSISChannel.at(i)) + "," +
                std::to_string(fCounts.at(i)) + std::string("\n");
	return line;
}

size_t TSISPlotEvents::size() const
{
   return fRunNumber.size();
}
