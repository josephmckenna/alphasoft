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

#include "TPaveText.h"

#include "TA2PlotSISPlotEvents.h"

class TA2Plot: public TAPlot
{
   protected:
      std::vector<TSISChannel> fSISChannels;

      //Detector SIS channels
      std::map<int, TSISChannel> fTrig;
      std::map<int, TSISChannel> fTrigNobusy;
      std::map<int, TSISChannel> fAtomOr;

      //Dump marker SIS channels:
      std::map<int, TSISChannel> fCATStart;
      std::map<int, TSISChannel> fCATStop;
      std::map<int, TSISChannel> fRCTStart;
      std::map<int, TSISChannel> fRCTStop;
      std::map<int, TSISChannel> fATMStart;
      std::map<int, TSISChannel> fATMStop;
   
      //Beam injection/ ejection markers:
      std::map<int, TSISChannel> fBeamInjection;
      std::map<int, TSISChannel> fBeamEjection;
      
      double fZMinCut;
      double fZMaxCut;

   public:
      TA2PlotSISPlotEvents SISEvents;  
      
      //Default members, operators, and prints.
      TA2Plot(bool zeroTime = true);
      TA2Plot(double zMin, double zMax,bool zeroTime = true);
      TA2Plot(const TAPlot& object);
      TA2Plot(const TA2Plot& object);
      virtual ~TA2Plot();
      friend TA2Plot operator+(const TA2Plot& lhs, const TA2Plot& rhs);
      TA2Plot& operator+=(const TA2Plot& rhs);
      TA2Plot& operator=(const TA2Plot& rhs);
   
      //Setters and getters
      void SetSISChannels(int runNumber);
      
      TSISChannel GetTrigNoBusyChannel(int runNumber)
      {
         return fTrigNobusy[runNumber];
      }
      TSISChannel GetTrigChannel(int runNumber)
      {
         return fTrig[runNumber];
      }
      
      //Adding events and dumps
      void AddSVDEvent(TSVD_QOD* SVDEvent);
      void AddSISEvent(TSISEvent* SISEvent);
      void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> dumpIndex );
      void AddDumpGates(int runNumber, std::vector<TA2Spill> spills );
      //If spills are from one run, it is faster to call the function above
      void AddDumpGates(std::vector<TA2Spill> spills );

      //Load, fill, draw, or save the object      
      void LoadRun(int runNumber, double firstTime, double lastTime);
      void SetUpHistograms();
      void FillHisto(bool applyCuts=true, int mode=0);
      TCanvas* DrawCanvas(const char* name="cVTX",bool applyCuts=true, int mode=0);
      void WriteEventList(std::string fileName, bool append = true);
      void ExportCSV(std::string filename, bool PassedCutOnly = true);
   private:
      void AddEvent(TSISEvent* event, TSISChannel channel,double timeOffset=0);
      void AddEvent(TSVD_QOD* event,double timeOffset=0);
   
   ClassDef(TA2Plot, 1)
};

#endif
#endif
