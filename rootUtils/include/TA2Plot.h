#ifndef _TALPHA2PLOT_
#define _TALPHA2PLOT_

#include "TAPlot.h"
#include "TSISEvent.h"
#include "TSVD_QOD.h"
#include "TSISChannels.h"
#include "TreeGetters.h"

class TA2Plot: public TAPlot
{
private:
   std::vector<int> SISChannels;

  //Detector SIS channels
   int trig;
   int trig_nobusy;
   int atom_or;

  //Dump marker SIS channels:
   int CATStart;
   int CATStop;
   int RCTStart;
   int RCTStop;
   int ATMStart;
   int ATMStop;
  
  //Beam injection/ ejection markers:
   int Beam_Injection;
   int Beam_Ejection;

public:
   struct SISPlotEvent {
      int runNumber; // I don't get set yet...
      //int clock
      double t; //Plot time (based off offical time)
      double OfficialTime;
      int Counts;
      int SIS_Channel;
   };
   void SetSISChannels(int runNumber);
   std::vector<SISPlotEvent> SISEvents;
   
   void AddSVDEvent(TSVD_QOD* SVDEvent);
   void AddSISEvent(TSISEvent* SISEvent);
private:
   void AddEvent(TSISEvent* event, int channel,double time_offset=0);
   void AddEvent(TSVD_QOD* event,double time_offset=0);
public:
   void LoadRun(int runNumber);
   void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition );
   TA2Plot();
   virtual ~TA2Plot();
   
   void SetUpHistograms(bool zeroTime=true);
   void FillHisto(bool ApplyCuts=true, int MVAMode=0);
   TCanvas* Draw(const char* Name="cVTX",bool ApplyCuts=true, int MVAMode=0);
   ClassDef(TA2Plot, 1)
};
#endif
