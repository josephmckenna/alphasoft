#ifndef _TAGPlotChronoPlotEvents_
#define _TAGPlotChronoPlotEvents_

#include "TAPlotScalerEvents.h"

#include "TChronoChannel.h"

class TAGPlotChronoPlotEvents: public TAPlotScalerEvents
{
   public:
      std::vector<TChronoChannel> fChronoChannel;
   public:
      //Std ctor and dtor
      TAGPlotChronoPlotEvents()
      {
      }
      ~TAGPlotChronoPlotEvents()
      {
      }
      //Copy ctor - !!!
      TAGPlotChronoPlotEvents(const TAGPlotChronoPlotEvents& sisPlotEvents) : TAPlotScalerEvents(sisPlotEvents)
      {
         //Deep copy vectors.
         for(size_t i=0; i<sisPlotEvents.fTime.size(); i++)
         {
            fChronoChannel.push_back( sisPlotEvents.fChronoChannel[i]);
         }
      }
      TAGPlotChronoPlotEvents operator+=(const TAGPlotChronoPlotEvents &rhs) 
      {
         //std::cout << "TAGPlotChronoPlotEvents += operator" << std::endl;
         this->fRunNumber   .insert(this->fRunNumber.end(),      rhs.fRunNumber.begin(),      rhs.fRunNumber.end() );
         this->fTime           .insert(this->fTime.end(),              rhs.fTime.begin(),              rhs.fTime.end() );
         this->fOfficialTime.insert(this->fOfficialTime.end(),   rhs.fOfficialTime.begin(),   rhs.fOfficialTime.end() );
         this->fCounts      .insert(this->fCounts.end(),         rhs.fCounts.begin(),         rhs.fCounts.end() );
         this->fChronoChannel .insert(this->fChronoChannel.end(),    rhs.fChronoChannel.begin(),    rhs.fChronoChannel.end() );
         return *this;
      }
      TAGPlotChronoPlotEvents& operator=(const TAGPlotChronoPlotEvents& sisPlotEvents)
      {
         for(size_t i = 0; i<sisPlotEvents.fTime.size(); i++)
         {
            this->fRunNumber.push_back( sisPlotEvents.fRunNumber[i]);
            this->fTime.push_back( sisPlotEvents.fTime[i]);
            this->fOfficialTime.push_back( sisPlotEvents.fOfficialTime[i]);
            this->fCounts.push_back( sisPlotEvents.fCounts[i]);
            this->fChronoChannel.push_back( sisPlotEvents.fChronoChannel[i]);
         }
         return *this;
      }
      friend TAGPlotChronoPlotEvents operator+(const TAGPlotChronoPlotEvents& lhs, const TAGPlotChronoPlotEvents& rhs)
      {
         //std::cout << "TAGPlotChronoPlotEvents addition operator" << std::endl;
         TAGPlotChronoPlotEvents outputplot(lhs); //Create new from copy
         //Vectors- need concacting
         outputplot.fRunNumber.insert(outputplot.fRunNumber.end(), rhs.fRunNumber.begin(), rhs.fRunNumber.end() );
         outputplot.fTime.insert(outputplot.fTime.end(), rhs.fTime.begin(), rhs.fTime.end() );
         outputplot.fOfficialTime.insert(outputplot.fOfficialTime.end(), rhs.fOfficialTime.begin(), rhs.fOfficialTime.end() );
         outputplot.fCounts.insert(outputplot.fCounts.end(), rhs.fCounts.begin(), rhs.fCounts.end() );
         outputplot.fChronoChannel.insert(outputplot.fChronoChannel.end(), rhs.fChronoChannel.begin(), rhs.fChronoChannel.end() );
         return outputplot;
      }
      void AddEvent(int runNumber, double time, double officialTime, int counts, const TChronoChannel& channel)
      {
         TAPlotScalerEvents::AddEvent(runNumber, time, officialTime, counts);
         fChronoChannel.push_back(channel);
      }
      int GetEventRunNumber(int event) const { return fRunNumber.at(event); }
      
      std::string CSVTitleLine() const override
      {
         return std::string("Run Number,") + 
                "Plot Time (Time axis of TAPlot)," +
                "OfficialTime (run time)," +
                "SIS Channel, SIS Module," +
                "Counts\n";
      }

      std::string CSVLine(size_t i) const override
      {
         //This is a little fugly
         std::string line;
         line =std::string("") + fRunNumber.at(i) + "," +
                fTime.at(i) + "," +
                fOfficialTime.at(i) + "," +
                fChronoChannel.at(i).GetChannel() + "," + fChronoChannel.at(i).GetBoard() + "," +
                fCounts.at(i) + "\n";
         return line;
      }
      
      int CountTotalCountsInChannel(const TChronoChannel& ch) const
      {
         int events = 0;
         for(size_t i = 0; i<fTime.size(); i++)
         {
            if (fChronoChannel[i] == ch)
               events += fCounts[i];
         }
         return events;
      }
      ClassDefOverride(TAGPlotChronoPlotEvents,2);
};

#endif