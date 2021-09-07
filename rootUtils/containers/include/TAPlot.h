
#ifndef _TALPHAPLOT_
#define _TALPHAPLOT_


#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TLegend.h"
#include "TreeGetters.h"
#include <iostream>
#include <fstream>
#include "AlphaColourWheel.h"
#include "TMultiGraph.h"
#include "TTimeStamp.h"
#include <numeric>

#define SCALECUT 0.6

//Basic internal containers:

#include "TVertexEvents.h"
#include "TTimeWindows.h"
#include "TEnvDataPlot.h"
#include "TFEGEMData.h"
#include "TFELabVIEWData.h"

class TAPlot: public TObject
{
   protected:
      
      std::string fCanvasTitle; //Used to give the TCanvas a title
      int fMVAMode; //Default 0;
      int fNumBins; // 100;
      int fDrawStyle; //Switch between colour modes
      int fLegendDetail; // = 1;
      double fClassifierCut;
      double fFirstTMin;
      double fLastTMax;
      double fBiggestTZero;
      double fMaxDumpLength;
      double fTotalTime;
      bool fVerbose;
      bool fApplyCuts;
      double fTimeFactor=1.; //Time scaling factor (used to switch between seconds and milliseconds)

      //Hold historams in a vector so that saved TAGPlot objects can be backwards and forwards compatable
      TObjArray fHistos;
      std::map<std::string,int> fHistoPositions;
   
      std::vector<double> fEjections;
      std::vector<double> fInjections;
      std::vector<double> fDumpStarts;
      std::vector<double> fDumpStops;
      std::vector<int> fRuns; //check dupes - ignore copies. AddRunNumber
      std::vector<TFEGEMData> fFEGEM;
      std::vector<TFELabVIEWData> fFELV;

      TTimeWindows fTimeWindows;
      TVertexEvents fVertexEvents;

      TTimeStamp fObjectConstructionTime{0};
      TTimeStamp fDataLoadedTime{0};

   public:

      const bool kZeroTimeAxis; //Use a time axis that counts from Zero of a dump window (default true)

      //Setters.
      void SetTAPlotTitle(const std::string& title)      {  fCanvasTitle = title;  }
      void LoadingDataLoadingDone()                      {  fDataLoadedTime = TTimeStamp(); }
      void SetTimeFactor(double time)                    {  fTimeFactor = time; }
      void SetCutsOn()                                   {  fApplyCuts = kTRUE; }
      void SetCutsOff()                                  {  fApplyCuts = kFALSE; }
      void SetMVAMode(int mode)                          {  fMVAMode = mode; }
      void SetBinNumber(int bin)                         {  fNumBins = bin; }
      void SetVerbose(bool verbose)                      {  fVerbose = verbose; }
      //Setters defined in .cxx
      void SetGEMChannel(const std::string& name, int arrayEntry, std::string title="");
      void SetGEMChannel(const std::string& category, const std::string& varName, int arrayEntry, std::string title="");
      void SetLVChannel(const std::string& name, int ArrayEntry, std::string title="");

      //Getters.
      TVertexEvents*       GetVertexEvents()        {  return &fVertexEvents; }
      TTimeWindows*        GetTimeWindows()         {  return &fTimeWindows; }
      std::string          GetTAPlotTitle()         {  return fCanvasTitle;  }
      size_t               GetNVertexEvents()       {  return fVertexEvents.fXVertex.size(); }
      double               GetTimeFactor() const    {  return fTimeFactor; }
      double               GetMaxDumpLength() const {  return fMaxDumpLength; }
      double               GetFirstTmin() const     {  return fFirstTMin;     }
      double               GetLastTmax() const      {  return fLastTMax;      }
      double               GetBiggestTzero() const  {  return fBiggestTZero;  }
      int                  GetNBins() const         {  return fNumBins; }
      int                  GetNVerticies()          {  return GetNVertexType(1);  }
      int                  GetNPassedCuts()         {  return GetNPassedType(1);  }
      bool                 IsGEMData()              {  return (fFEGEM.size() != 0);   }
      bool                 IsLVData()               {  return (fFEGEM.size() != 0);   }
      bool                 GetCutsSettings() const  {  return fApplyCuts; }
      TObjArray            GetHisto()               {  return fHistos;}
      std::map<std::string,int> GetHistoPosition()  {  return fHistoPositions;}
      const std::vector<int> GetArrayOfRuns()       {  return fRuns; }
      //Getters defined in .cxx
      std::vector<std::pair<std::string,int>> GetGEMChannels();
      std::vector<std::pair<std::string,int>> GetLVChannels();
      std::pair<TLegend*,TMultiGraph*> GetGEMGraphs();
      std::pair<TLegend*,TMultiGraph*> GetLVGraphs();
      double GetApproximateProcessingTime();
      int GetNPassedType(const int type);
      int GetNVertexType(const int type);
      TString GetListOfRuns();

      //Adders.
      virtual void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition ) {assert(!"Child class must have this");};
      void AddStartDumpMarker(double time)               {  fDumpStarts.push_back(time);}
      void AddStopDumpMarker(double time)                {  fDumpStops.push_back(time); }
      void AddInjection(double time)                     {  fInjections.push_back(time);}
      void AddEjection(double time)                      {  fEjections.push_back(time);  }
      //Adders defined in .cxx
      void AddRunNumber(int runNumber);
      void AddTimeGates(int runNumber, std::vector<double> minTimes, std::vector<double> maxTimes, std::vector<double> timeZeros);
      void AddTimeGates(int runNumber, std::vector<double> minTimes, std::vector<double> maxTimes);
      void AddTimeGate(const int runNumber, const double minTime, const double maxTime, const double zeroTime);
      void AddTimeGate(const int runNumber, const double minTime, const double maxTime);
      void AddVertexEvent(int runNumber, int eventNo, int cutsResult, int vertexStatus, double x, double y, double z, double t, double eventTime, double eunTime, int numHelices, int numTracks);


      //Load data functions.
      template<typename T> void LoadFEGEMData(TFEGEMData& gemData, TTreeReader* gemReader, const char* name, double firstTime, double lastTime);
      void LoadFEGEMData(int runNumber, double firstTime, double lastTime);
      void LoadFELVData(TFELabVIEWData& labviewData, TTreeReader* labviewReader, const char* name, double firstTime, double lastTime);
      void LoadFELVData(int runNumber, double firstTime, double lastTime);
      // Hmm the add operators prevent this being a pure virtual function (ie
      // the add operator can't return TAPlot if TAPlot is an abstract class...
      // one to think about Joe)
      virtual void LoadRun(int runNumber, double firstTime, double lastTime) {};
      void LoadData();

      
      //Default members, operators, and prints.
      TAPlot(const TAPlot& object);
      TAPlot(bool zeroTime = true);//, int MVAMode = 0);
      virtual ~TAPlot();
      TAPlot& operator=(const TAPlot& rhs);
      friend TAPlot operator+(const TAPlot& lhs, const TAPlot& rhs);
      TAPlot& operator+=(const TAPlot &rhs);
      void Print(Option_t* option="") const;
      virtual void PrintFull();
      
      //Histogram functions
      void AutoTimeRange();
      template <class T> void AddHistogram(const char* keyname, T* h) //This refuses to go in the .cxx for some reason.
      { 
         fHistos.Add(h);
         fHistoPositions[keyname]=fHistos.GetEntries()-1;
      }
      void FillHistogram(const char* keyname,double x, int counts);
      void FillHistogram(const char* keyname, double x);
      void FillHistogram(const char* keyname, double x, double y);
      TH1D* GetTH1D(const char* keyname);
      void DrawHistogram(const char* keyname, const char* settings);
      TLegend* DrawLines(TLegend* legend, const char* keyname);
      TLegend* AddLegendIntegral(TLegend* legend, const char* message, const char* keyname);
      void ClearHisto();
      void SetUpHistograms();
      void PrintTimeRanges();
      void ExportCSV(std::string filename, bool PassedCutOnly = true);

   ClassDef(TAPlot, 1);
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

