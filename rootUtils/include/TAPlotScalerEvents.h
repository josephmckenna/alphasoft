#ifndef _TAPLOTSCALERPLOTEVENTS_
#define _TAPLOTSCALERPLOTEVENTS_

#include "TObject.h"
#include <vector>

class TAPlotScalerEvents: public TObject
{
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fTime;  //Plot time (based off official time)
      std::vector<double> fOfficialTime;
      std::vector<int> fCounts;
   public:
      //Std ctor and dtor
      TAPlotScalerEvents()
      {
      }

      ~TAPlotScalerEvents()
      {
      }

      //Copy ctor - !!!
      TAPlotScalerEvents(const TAPlotScalerEvents& sisPlotEvents) : TObject(sisPlotEvents)
      {
         //Deep copy vectors.
         for(size_t i=0; i<sisPlotEvents.fTime.size(); i++)
         {
            fRunNumber.push_back( sisPlotEvents.fRunNumber[i]);
            fTime.push_back( sisPlotEvents.fTime[i]);
            fOfficialTime.push_back( sisPlotEvents.fOfficialTime[i]);
            fCounts.push_back( sisPlotEvents.fCounts[i]);
         }
      }

      TAPlotScalerEvents& operator=(const TAPlotScalerEvents& sisPlotEvents)
      {
         for(size_t i = 0; i<sisPlotEvents.fTime.size(); i++)
         {
            this->fRunNumber.push_back( sisPlotEvents.fRunNumber[i]);
            this->fTime.push_back( sisPlotEvents.fTime[i]);
            this->fOfficialTime.push_back( sisPlotEvents.fOfficialTime[i]);
            this->fCounts.push_back( sisPlotEvents.fCounts[i]);
         }
         return *this;
      }

      void AddEvent(int runNumber, double time, double officialTime, int counts)
      {
         fRunNumber.push_back(runNumber);
         fTime.push_back(time);  
         fOfficialTime.push_back(officialTime);
         fCounts.push_back(counts);
      }

      int GetEventRunNumber(int event) const { return fRunNumber.at(event); }
      
      virtual std::string CSVTitleLine() const = 0;
      
      virtual std::string CSVLine(size_t i) const = 0;

      size_t size() const
      {
         return fRunNumber.size();
      }

      ClassDef(TAPlotScalerEvents,1);
};

#endif