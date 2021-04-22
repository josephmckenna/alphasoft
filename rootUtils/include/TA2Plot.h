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

class SISPlotEvent
{
   public:
      int runNumber; // I don't get set yet...
      //int clock
      double t; //Plot time (based off offical time)
      double OfficialTime;
      int Counts;
      int SIS_Channel;

      //LMG - Copy and assign operators
      SISPlotEvent()
      {
      }
      ~SISPlotEvent()
      {
      }
      //Basic copy constructor.
      SISPlotEvent(const SISPlotEvent& m_SISPlotEvent)
      {
         runNumber      =  m_SISPlotEvent.runNumber   ;
         t              =  m_SISPlotEvent.t           ;
         OfficialTime   =  m_SISPlotEvent.OfficialTime;
         Counts         =  m_SISPlotEvent.Counts      ;
         SIS_Channel    =  m_SISPlotEvent.SIS_Channel ;
      }
      //Assignment operator.
      SISPlotEvent operator=(const SISPlotEvent m_SISPlotEvent)
      {
         this->runNumber      =  m_SISPlotEvent.runNumber   ;
         this->t              =  m_SISPlotEvent.t           ;
         this->OfficialTime   =  m_SISPlotEvent.OfficialTime;
         this->Counts         =  m_SISPlotEvent.Counts      ;
         this->SIS_Channel    =  m_SISPlotEvent.SIS_Channel ;
         return *this;
      }
};

class TA2Plot: public TAPlot
{
protected:
   std::vector<int> SISChannels;

  //Detector SIS channels
   std::map<int, int> trig;
   std::map<int, int> trig_nobusy;
   std::map<int, int> atom_or;

  //Dump marker SIS channels:
   std::map<int, int> CATStart;
   std::map<int, int> CATStop;
   std::map<int, int> RCTStart;
   std::map<int, int> RCTStop;
   std::map<int, int> ATMStart;
   std::map<int, int> ATMStop;
  
  //Beam injection/ ejection markers:
   std::map<int, int> Beam_Injection;
   std::map<int, int> Beam_Ejection;
   
   double ZMinCut;
   double ZMaxCut;

public:
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
   //Intercept negative tmax values and replace with total run time
   virtual void AddTimeGate(const int runNumber, const double tmin, double tmax)
   {
      if (tmax<0)
         tmax = GetA2TotalRunTime(runNumber);
      return TAPlot::AddTimeGate(runNumber, tmin, tmax);
   }
   virtual void AddTimeGate(const int runNumber, const double tmin, double tmax, const double tzero)
   {
      if (tmax<0)
         tmax = GetA2TotalRunTime(runNumber);
      return TAPlot::AddTimeGate(runNumber, tmin, tmax, tzero);
   }
   virtual void AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax)
   {
      for (size_t i=0; i<tmin.size(); i++)
      {
        if (tmax[i]<0) tmax[i] = GetA2TotalRunTime(runNumber);
      }
      return TAPlot::AddTimeGates(runNumber, tmin, tmax);
   }
   virtual void AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax, std::vector<double> tzero)
   {
      for (size_t i=0; i<tmin.size(); i++)
      {
        if (tmax[i]<0) tmax[i] = GetA2TotalRunTime(runNumber);
      }
      return TAPlot::AddTimeGates(runNumber, tmin, tmax, tzero);
   }
         
   //If spills are from one run, it is faster to call the function above
   void AddDumpGates(std::vector<TA2Spill> spills );

   TA2Plot(bool zerotime = true);
   TA2Plot(double zmin, double zmax,bool zerotime = true);
   TA2Plot(const TA2Plot& m_TA2Plot);
   TA2Plot(const TAPlot& m_TAPlot);
   virtual ~TA2Plot();
   friend TA2Plot operator+(const TA2Plot& PlotA, const TA2Plot& PlotB);
   TA2Plot& operator=(const TA2Plot& plotA);
   
   void SetUpHistograms();
   void FillHisto(bool ApplyCuts=true, int MVAMode=0);
   TCanvas* DrawCanvas(const char* Name="cVTX",bool ApplyCuts=true, int MVAMode=0);
   ClassDef(TA2Plot, 1)

   //void PrintFull();

   
};

#endif
#endif
