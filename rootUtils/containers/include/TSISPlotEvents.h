#ifndef __TSISPlotEvents__
#define __TSISPlotEvents__

#include "TSISEvent.h"

#include "TSISChannels.h"


class TSISPlotEvents: public TObject
{
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fTime;  //Plot time (based off official time)
      std::vector<double> fOfficialTime;
      std::vector<int> fCounts;
      std::vector<int> fSISChannel;
   public:
      //Std ctor and dtor
      TSISPlotEvents();
      ~TSISPlotEvents();
      TSISPlotEvents(const TSISPlotEvents& sisPlotEvents);
      TSISPlotEvents operator+=(const TSISPlotEvents &rhs);
      TSISPlotEvents& operator=(const TSISPlotEvents& sisPlotEvents);
      friend TSISPlotEvents operator+(const TSISPlotEvents& lhs, const TSISPlotEvents& rhs);
      void AddEvent(int runNumber, double time, double officialTime, int counts, int channel);
      int GetEventRunNumber(int event) const;
      
      std::string CSVTitleLine() const;

      std::string CSVLine(size_t i) const;

      size_t size() const;

   ClassDef(TSISPlotEvents,1);
};

#endif
