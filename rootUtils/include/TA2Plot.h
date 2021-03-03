#ifdef BUILD_A2

#ifndef _TALPHA2PLOT_
#define _TALPHA2PLOT_
#include "TCanvas.h"
#include "TLatex.h"
#include "TAPlot.h"
#include "TSISEvent.h"
#include "TSVD_QOD.h"
#include "TSISChannels.h"
#include "TA2Spill.h"
#include "TA2SpillGetters.h"

class TA2Plot: public TAPlot
{
private:
   std::vector<int> SISChannels;

  //Detector SIS channels
   int trig;
   int trig_nobusy;
   int atom_or;
   //new method
   //std::map<int, int> trig;

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
   
   double ZMinCut;
   double ZMaxCut;

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
   void LoadRun(int runNumber, double first_time, double last_time);
   void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition );
   void AddDumpGates(int runNumber, std::vector<TA2Spill> spills );
   //If spills are from one run, it is faster to call the function above
   void AddDumpGates(std::vector<TA2Spill> spills );

   TA2Plot(bool zerotime = true);
   TA2Plot(double zmin, double zmax,bool zerotime = true);
   TA2Plot(const TA2Plot& m_TA2Plot);
   virtual ~TA2Plot();

   friend TA2Plot& operator+=(const TA2Plot& plotA, const TA2Plot& plotB);
   friend TA2Plot& operator+(const TA2Plot& plotA, const TA2Plot& plotB);
   TA2Plot& operator=(const TA2Plot& plotA);
   
   void SetUpHistograms();
   void FillHisto(bool ApplyCuts=true, int MVAMode=0);
   TCanvas* DrawCanvas(const char* Name="cVTX",bool ApplyCuts=true, int MVAMode=0);
   ClassDef(TA2Plot, 1)

   //void PrintFull();

   
};
#endif
#endif
