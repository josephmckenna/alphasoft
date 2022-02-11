#ifndef _TA2PLOTSISPLOTEVENTS_
#define _TA2PLOTSISPLOTEVENTS_

#include "TObject.h"
#include <vector>

#include "TSISEvent.h"
#include "TSISChannels.h"

class TA2PlotSISPlotEvents: public TObject
{
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fTime;  //Plot time (based off official time)
      std::vector<double> fOfficialTime;
      std::vector<int> fCounts;
      std::vector<TSISChannel> fSISChannel;
   public:
      //Std ctor and dtor
      TA2PlotSISPlotEvents()
      {
      }
      ~TA2PlotSISPlotEvents()
      {
      }
      //Copy ctor - !!!
      TA2PlotSISPlotEvents(const TA2PlotSISPlotEvents& sisPlotEvents) : TObject(sisPlotEvents)
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
      TA2PlotSISPlotEvents operator+=(const TA2PlotSISPlotEvents &rhs) 
      {
         //std::cout << "TA2PlotSISPlotEvents += operator" << std::endl;
         this->fRunNumber   .insert(this->fRunNumber.end(),      rhs.fRunNumber.begin(),      rhs.fRunNumber.end() );
         this->fTime           .insert(this->fTime.end(),              rhs.fTime.begin(),              rhs.fTime.end() );
         this->fOfficialTime.insert(this->fOfficialTime.end(),   rhs.fOfficialTime.begin(),   rhs.fOfficialTime.end() );
         this->fCounts      .insert(this->fCounts.end(),         rhs.fCounts.begin(),         rhs.fCounts.end() );
         this->fSISChannel .insert(this->fSISChannel.end(),    rhs.fSISChannel.begin(),    rhs.fSISChannel.end() );
         return *this;
      }
      TA2PlotSISPlotEvents& operator=(const TA2PlotSISPlotEvents& sisPlotEvents)
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
      friend TA2PlotSISPlotEvents operator+(const TA2PlotSISPlotEvents& lhs, const TA2PlotSISPlotEvents& rhs)
      {
         //std::cout << "TA2PlotSISPlotEvents addition operator" << std::endl;
         TA2PlotSISPlotEvents outputplot(lhs); //Create new from copy
         //Vectors- need concacting
         outputplot.fRunNumber.insert(outputplot.fRunNumber.end(), rhs.fRunNumber.begin(), rhs.fRunNumber.end() );
         outputplot.fTime.insert(outputplot.fTime.end(), rhs.fTime.begin(), rhs.fTime.end() );
         outputplot.fOfficialTime.insert(outputplot.fOfficialTime.end(), rhs.fOfficialTime.begin(), rhs.fOfficialTime.end() );
         outputplot.fCounts.insert(outputplot.fCounts.end(), rhs.fCounts.begin(), rhs.fCounts.end() );
         outputplot.fSISChannel.insert(outputplot.fSISChannel.end(), rhs.fSISChannel.begin(), rhs.fSISChannel.end() );
         return outputplot;
      }
      void AddEvent(int runNumber, double time, double officialTime, int counts, TSISChannel channel)
      {
         fRunNumber.push_back(runNumber);
         fTime.push_back(time);  
         fOfficialTime.push_back(officialTime);
         fCounts.push_back(counts);
         fSISChannel.push_back(channel);
      }
      int GetEventRunNumber(int event) const { return fRunNumber.at(event); }
      
      std::string CSVTitleLine() const
      {
         return std::string("Run Number,") + 
                "Plot Time (Time axis of TAPlot)," +
                "OfficialTime (run time)," +
                "SIS Channel, SIS Module," +
                "Counts\n";
      }

      std::string CSVLine(size_t i) const
      {
         //This is a little fugly
         std::string line;
         line =std::string("") + fRunNumber.at(i) + "," +
                fTime.at(i) + "," +
                fOfficialTime.at(i) + "," +
                fSISChannel.at(i).fChannel + "," + fSISChannel.at(i).fModule + "," +
                fCounts.at(i) + "\n";
         return line;
      }

      size_t size() const
      {
         return fRunNumber.size();
      }
      
      int CountTotalCountsInChannel(int ch) const
      {
         int events = 0;
         for(size_t i = 0; i<fTime.size(); i++)
         {
            if (fSISChannel[i] == ch)
               events += fCounts[i];
         }
         return events;
      }
      ClassDef(TA2PlotSISPlotEvents,1);
};

#endif