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
      TAGPlotChronoPlotEvents();
      ~TAGPlotChronoPlotEvents();
      //Copy ctor - !!!
      TAGPlotChronoPlotEvents(const TAGPlotChronoPlotEvents& sisPlotEvents);
      TAGPlotChronoPlotEvents operator+=(const TAGPlotChronoPlotEvents &rhs);
      TAGPlotChronoPlotEvents& operator=(const TAGPlotChronoPlotEvents& sisPlotEvents);
      friend TAGPlotChronoPlotEvents operator+(const TAGPlotChronoPlotEvents& lhs, const TAGPlotChronoPlotEvents& rhs);
      void AddEvent(int runNumber, double time, double officialTime, int counts, const TChronoChannel& channel);
      int GetEventRunNumber(int event) const;
      std::string CSVTitleLine() const override;

      std::string CSVLine(size_t i) const override;
      
      int CountTotalCountsInChannel(const TChronoChannel& ch) const;
      ClassDefOverride(TAGPlotChronoPlotEvents,2);
};

#endif
