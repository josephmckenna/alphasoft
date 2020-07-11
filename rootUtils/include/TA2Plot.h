#ifndef _TALPHA2PLOT_
#define _TALPHA2PLOT_

#include "TAPlot.h"
#include "TSISEvent.h"
#include "TSVD_QOD.h"

#include "TreeGetters.h"

class TA2Plot: public TAPlot
{
private:
   std::vector<int> SIS_Channels;
   int NSIS_Channels;
public:
   struct SISPlotEvent {
      int runNumber; // I don't get set yet...
      //int clock
      double t; //Plot time (based off offical time)
      double OfficialTime;
      int Counts;
      int SIS_Channel;
   };
   std::vector<SISPlotEvent> SISEvents;
   void AddEvent(TSISEvent* event, int channel,double time_offset=0);
   void AddEvent(TSVD_QOD* event,double time_offset=0);
   void LoadRun(int runNumber);
   void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition );
   
   TA2Plot();
   virtual ~TA2Plot();
   ClassDef(TA2Plot, 1)
};
#endif
