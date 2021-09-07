#ifdef BUILD_A2

#ifndef _TALPHA2PLOT_
#define _TALPHA2PLOT_
#include "TCanvas.h"
#include "TLatex.h"
#include "TAPlot.h"
#include "TSVD_QOD.h"
#include "TA2Spill.h"
#include "TA2SpillGetters.h"
#include "TSISPlotEvents.h"

class TA2Plot: public TAPlot
{
   protected:
      std::vector<int> fSISChannels;

      //Detector SIS channels
      std::map<int, int> fTrig;
      std::map<int, int> fTrigNobusy;
      std::map<int, int> fAtomOr;

      //Dump marker SIS channels:
      std::map<int, int> fCATStart;
      std::map<int, int> fCATStop;
      std::map<int, int> fRCTStart;
      std::map<int, int> fRCTStop;
      std::map<int, int> fATMStart;
      std::map<int, int> fATMStop;
   
      //Beam injection/ ejection markers:
      std::map<int, int> fBeamInjection;
      std::map<int, int> fBeamEjection;
      
      double fZMinCut;
      double fZMaxCut;

   public:
      TSISPlotEvents SISEvents;  
      
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

      //Adding events and dumps
      void AddSVDEvent(TSVD_QOD* SVDEvent);
      void AddSISEvent(TSISEvent* SISEvent);
      void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition );
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
      void AddEvent(TSISEvent* event, int channel,double timeOffset=0);
      void AddEvent(TSVD_QOD* event,double timeOffset=0);
   
   ClassDef(TA2Plot, 1)
};

#endif
#endif
