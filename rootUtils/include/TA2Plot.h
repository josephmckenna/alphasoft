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

class TSISPlotEvents: public TObject
{
   public:
      std::vector<int> runNumber;
      std::vector<double> t;  //Plot time (based off official time)
      std::vector<double> OfficialTime;
      std::vector<int> Counts;
      std::vector<int> SIS_Channel;
   public:
      //Std ctor and dtor
      TSISPlotEvents()
      {
      }
      ~TSISPlotEvents()
      {
      }
      //Copy ctor
      /*TSISPlotEvents(const TSISPlotEvents& m_SISEvent)
      {
         //Deep copy vectors.
         for(int i = 0; i<m_SISEvent.t.size(); i++)
         {
            runNumber.push_back( m_SISEvent.runNumber[i]);
            t.push_back( m_SISEvent.t[i]);
            OfficialTime.push_back( m_SISEvent.OfficialTime[i]);
            Counts.push_back( m_SISEvent.Counts[i]);
            SIS_Channel.push_back( m_SISEvent.SIS_Channel[i]);
         }
      }*/
      TSISPlotEvents operator+=(const TSISPlotEvents &plotB) 
      {
         std::cout << "TSISPlotEvents += operator" << std::endl;
         this->runNumber   .insert(this->runNumber.end(),      plotB.runNumber.begin(),      plotB.runNumber.end() );
         this->t           .insert(this->t.end(),              plotB.t.begin(),              plotB.t.end() );
         this->OfficialTime.insert(this->OfficialTime.end(),   plotB.OfficialTime.begin(),   plotB.OfficialTime.end() );
         this->Counts      .insert(this->Counts.end(),         plotB.Counts.begin(),         plotB.Counts.end() );
         this->SIS_Channel .insert(this->SIS_Channel.end(),    plotB.SIS_Channel.begin(),    plotB.SIS_Channel.end() );
         return *this;
      }
      TSISPlotEvents& operator=(const TSISPlotEvents& m_SISEvent)
      {
         for(int i = 0; i<m_SISEvent.t.size(); i++)
         {
            this->runNumber.push_back( m_SISEvent.runNumber[i]);
            this->t.push_back( m_SISEvent.t[i]);
            this->OfficialTime.push_back( m_SISEvent.OfficialTime[i]);
            this->Counts.push_back( m_SISEvent.Counts[i]);
            this->SIS_Channel.push_back( m_SISEvent.SIS_Channel[i]);
         }
         return *this;
      }
      friend TSISPlotEvents operator+(const TSISPlotEvents& plotA, const TSISPlotEvents& plotB)
      {
         std::cout << "TSISPlotEvents addition operator" << std::endl;
         TSISPlotEvents outputplot(plotA); //Create new from copy
         //Vectors- need concacting
         outputplot.runNumber.insert(outputplot.runNumber.end(), plotB.runNumber.begin(), plotB.runNumber.end() );
         outputplot.t.insert(outputplot.t.end(), plotB.t.begin(), plotB.t.end() );
         outputplot.OfficialTime.insert(outputplot.OfficialTime.end(), plotB.OfficialTime.begin(), plotB.OfficialTime.end() );
         outputplot.Counts.insert(outputplot.Counts.end(), plotB.Counts.begin(), plotB.Counts.end() );
         outputplot.SIS_Channel.insert(outputplot.SIS_Channel.end(), plotB.SIS_Channel.begin(), plotB.SIS_Channel.end() );
         return outputplot;
      }
      void AddEvent(int m_runNumber, double m_ti, double m_OfficialTime, int m_Counts, int m_SIS_Channel)
      {
         runNumber.push_back(m_runNumber);
         t.push_back(m_ti);  
         OfficialTime.push_back(m_OfficialTime);
         Counts.push_back(m_Counts);
         SIS_Channel.push_back(m_SIS_Channel);
      }
      int GetEventRunNumber(int event) const { return runNumber.at(event); }
      
      ClassDef(TSISPlotEvents,1);
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
      TSISPlotEvents SISEvents;  

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
      TA2Plot(const TAPlot& m_TAPlot);
      virtual ~TA2Plot();
      friend TA2Plot operator+(const TA2Plot& PlotA, const TA2Plot& PlotB);
      TA2Plot& operator+=(const TA2Plot& plotB);
      TA2Plot& operator=(const TA2Plot& plotA);
      
      void SetUpHistograms();
      void FillHisto(bool ApplyCuts=true, int MVAMode=0);
      TCanvas* DrawCanvas(const char* Name="cVTX",bool ApplyCuts=true, int MVAMode=0);
      ClassDef(TA2Plot, 1)
};

#endif
#endif
