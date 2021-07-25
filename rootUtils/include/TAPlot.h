
#ifndef _TALPHAPLOT_
#define _TALPHAPLOT_


#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TLegend.h"
#include "TreeGetters.h"
#include "TStoreGEMEvent.h"
#include "TStoreLabVIEWEvent.h"
#include <algorithm>
#include <iostream>
#include "AlphaColourWheel.h"
#include "TMultiGraph.h"
#include "TTimeStamp.h"
#include "AnalysisReportGetters.h"
#include <numeric>

#define SCALECUT 0.6

//Basic internal containers:

//SVD / TPC vertex
class TVertexEvents: public TObject
{
   public:
      std::vector<int> fRunNumbers; // I don't get set yet...
      std::vector<int> fEventNos;
      std::vector<int> fCutsResults;
      std::vector<int> fVertexStatuses;
      std::vector<double> fXVertex;
      std::vector<double> fYVertex;
      std::vector<double> fZVertex;
      std::vector<double> fTimes; //Plot time (based off offical time)
      std::vector<double> fEventTimes; //TPC time stamp
      std::vector<double> fRunTimes; //Official Time
      std::vector<int> fNumHelices; // helices used for vertexing
      std::vector<int> fNumTracks; // reconstructed (good) helices

      //Copy and assign operators
      TVertexEvents()
      {
      }
      ~TVertexEvents()
      {
      }
      //Basic copy constructor.
      TVertexEvents(const TVertexEvents& vertexEvents) : TObject(vertexEvents)
      {
         //Making deep copies of the vectors, we require loop method as the vectors are already initialised earlier.
         for(size_t i = 0; i < vertexEvents.fXVertex.size(); i++)
         {
            fRunNumbers.push_back(vertexEvents.fRunNumbers[i]);
            fEventNos.push_back(vertexEvents.fEventNos[i]);
            fCutsResults.push_back(vertexEvents.fCutsResults[i]);
            fVertexStatuses.push_back(vertexEvents.fVertexStatuses[i]);
            fXVertex.push_back(vertexEvents.fXVertex[i]);
            fYVertex.push_back(vertexEvents.fYVertex[i]);
            fZVertex.push_back(vertexEvents.fZVertex[i]);
            fTimes.push_back(vertexEvents.fTimes[i]);
            fEventTimes.push_back(vertexEvents.fEventTimes[i]);
            fRunTimes.push_back(vertexEvents.fRunTimes[i]);
            fNumHelices.push_back(vertexEvents.fNumHelices[i]);
            fNumTracks.push_back(vertexEvents.fNumTracks[i]);
         }
      }
      //Assignment operator.
      TVertexEvents& operator=(const TVertexEvents& rhs)
      {
         //Making deep copies of the vectors, we require loop method as the vectors are already initialised earlier.
         for(size_t i = 0; i < rhs.fXVertex.size(); i++)
         {
            this->fRunNumbers.push_back(rhs.fRunNumbers[i]);
            this->fEventNos.push_back(rhs.fEventNos[i]);
            this->fCutsResults.push_back(rhs.fCutsResults[i]);
            this->fVertexStatuses.push_back(rhs.fVertexStatuses[i]);
            this->fXVertex.push_back(rhs.fXVertex[i]);
            this->fYVertex.push_back(rhs.fYVertex[i]);
            this->fZVertex.push_back(rhs.fZVertex[i]);
            this->fTimes.push_back(rhs.fTimes[i]);
            this->fEventTimes.push_back(rhs.fEventTimes[i]);
            this->fRunTimes.push_back(rhs.fRunTimes[i]);
            this->fNumHelices.push_back(rhs.fNumHelices[i]);
            this->fNumTracks.push_back(rhs.fNumTracks[i]);
         }
         return *this;
      }
      //=+ Operator.
      TVertexEvents operator+=(const TVertexEvents &rhs) 
      {
         std::cout << "TAVertexEvents += operator" << std::endl;
         this->fRunNumbers       .insert(this->fRunNumbers.end(),      rhs.fRunNumbers.begin(),     rhs.fRunNumbers.end());
         this->fEventNos         .insert(this->fEventNos.end(),        rhs.fEventNos.begin(),       rhs.fEventNos.end());
         this->fCutsResults      .insert(this->fCutsResults.end(),     rhs.fCutsResults.begin(),    rhs.fCutsResults.end()); 
         this->fVertexStatuses   .insert(this->fVertexStatuses.end(),  rhs.fVertexStatuses.begin(), rhs.fVertexStatuses.end());    
         this->fXVertex               .insert(this->fXVertex.end(),              rhs.fXVertex.begin(),             rhs.fXVertex.end());
         this->fYVertex               .insert(this->fYVertex.end(),              rhs.fYVertex.begin(),             rhs.fYVertex.end());
         this->fZVertex               .insert(this->fZVertex.end(),              rhs.fZVertex.begin(),             rhs.fZVertex.end());
         this->fTimes               .insert(this->fTimes.end(),              rhs.fTimes.begin(),             rhs.fTimes.end());
         this->fEventTimes       .insert(this->fEventTimes.end(),      rhs.fEventTimes.begin(),     rhs.fEventTimes.end());
         this->fRunTimes         .insert(this->fRunTimes.end(),        rhs.fRunTimes.begin(),       rhs.fRunTimes.end());
         this->fNumHelices         .insert(this->fNumHelices.end(),        rhs.fNumHelices.begin(),       rhs.fNumHelices.end());
         this->fNumTracks          .insert(this->fNumTracks.end(),         rhs.fNumTracks.begin(),        rhs.fNumTracks.end());
         return *this;
      }
      ClassDef(TVertexEvents, 1);
};

class TTimeWindows : public TObject
{
   private:
      bool isSorted = false;
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fMinTime;
      std::vector<double> fMaxTime;
      std::vector<double> fZeroTime;
      
      //Standard ctor and dtor
      TTimeWindows()
      {
      }
      ~TTimeWindows()
      {
      }
      //Copy ctor
      TTimeWindows(const TTimeWindows& timeWindows) : TObject(timeWindows)
      {
         std::cout << "TTimeWindows copy ctor." << std::endl;
         //Deep copy
         for(size_t i = 0; i<timeWindows.fMinTime.size(); i++)
         {
            fRunNumber.push_back(timeWindows.fRunNumber[i]);
            fMinTime.push_back(timeWindows.fMinTime[i]);
            fMaxTime.push_back(timeWindows.fMaxTime[i]);
            fZeroTime.push_back(timeWindows.fZeroTime[i]);
         }
      }
      TTimeWindows& operator=(const TTimeWindows& rhs)
      {
         std::cout << "TTimeWindows = operator." << std::endl;
         for(size_t i = 0; i<rhs.fMinTime.size(); i++)
         {
            this->fRunNumber.push_back(rhs.fRunNumber[i]);
            this->fMinTime.push_back(rhs.fMinTime[i]);
            this->fMaxTime.push_back(rhs.fMaxTime[i]);
            this->fZeroTime.push_back(rhs.fZeroTime[i]);
         }
         return *this;
      }
      friend TTimeWindows operator+(const TTimeWindows& lhs, const TTimeWindows& rhs)
      {
         std::cout << "TTimeWindows addition operator" << std::endl;
         TTimeWindows outputPlot(lhs); //Create new from copy

         //Vectors- need concacting
         outputPlot.fRunNumber.insert(outputPlot.fRunNumber.end(), rhs.fRunNumber.begin(), rhs.fRunNumber.end() );
         outputPlot.fMinTime.insert(outputPlot.fMinTime.end(), rhs.fMinTime.begin(), rhs.fMinTime.end() );
         outputPlot.fMaxTime.insert(outputPlot.fMaxTime.end(), rhs.fMaxTime.begin(), rhs.fMaxTime.end() );
         outputPlot.fZeroTime.insert(outputPlot.fZeroTime.end(), rhs.fZeroTime.begin(), rhs.fZeroTime.end() );
         return outputPlot;
      }
      TTimeWindows operator+=(const TTimeWindows &rhs) 
      {
         std::cout << "TTimeWindows += operator" << std::endl;
         this->fRunNumber.insert(this->fRunNumber.end(), rhs.fRunNumber.begin(), rhs.fRunNumber.end() );
         this->fMinTime.insert(this->fMinTime.end(), rhs.fMinTime.begin(), rhs.fMinTime.end() );
         this->fMaxTime.insert(this->fMaxTime.end(), rhs.fMaxTime.begin(), rhs.fMaxTime.end() );
         this->fZeroTime.insert(this->fZeroTime.end(), rhs.fZeroTime.begin(), rhs.fZeroTime.end() );
         return *this;
      }
      void AddTimeWindow(int runNumber, double minTime, double maxTime, double timeZero)
      {
         fRunNumber.push_back(runNumber);
         fMinTime.push_back(minTime);
         fMaxTime.push_back(maxTime);
         fZeroTime.push_back(timeZero);
         if(maxTime >= 0)
         {
            assert(maxTime>minTime);
            assert(timeZero<maxTime);
         }
         assert(timeZero>=minTime);
      }

      int GetValidWindowNumber(double time)
      {
         //Checks whether vector is sorted and sorts if not.
         if(!isSorted)
         {
            SortTimeWindows();
         }
         //Check where t sits in tmin.
         const size_t tmaxSize = fMaxTime.size();
         for (size_t j = 0; j < tmaxSize; j++)
         {
            //If inside the time window
            if ( time > fMinTime[j] )
            {
               const double currentTMax = fMaxTime[j];
               if(time < currentTMax || currentTMax < 0)
               {
                  return j;
               }
            }
         }
         //If we get down here just return error.
         return -2;
      }
      template <class T>
      static std::vector<T> reorder(std::vector<T>& nums, std::vector<size_t>& index)
      {
         const int kNumsSize = nums.size();
         std::vector<T> newest(kNumsSize);
         newest.resize(kNumsSize);
         for (int i = 0; i < kNumsSize; i++)
         {
            newest[i] = nums[ index[i] ] ;
         }
         nums.clear();
         return newest;  
      }
      void SortTimeWindows()
      {
         assert( fMinTime.size() == fMaxTime.size() );
         const size_t minTimeSize = fMinTime.size();
         //Create vectors needed for sorting.
         std::vector<std::pair<double,double>> zipped(minTimeSize);
         std::vector<size_t> idx(minTimeSize);
         iota(idx.begin(),idx.end(),0);


         //Zip tmin and index vector together.
         zipped.resize(minTimeSize);
         for(size_t i=0; i < minTimeSize; ++i)
         {
            zipped[i]=std::make_pair(fMinTime[i], idx[i]);
         }

         //Sort based on first (tmin) keeping idx in the same location
         std::sort(std::begin(zipped), std::end(zipped), 
                   [&](const std::pair<double,double>& lhs, const std::pair<double,double>& rhs)
                   {
                      return lhs.first < rhs.first;
                   }
         );
         
         //Unzip back into vectors.
         for(size_t i=0; i < minTimeSize; i++)
         {
            fMinTime[i] = zipped[i].first;
            idx[i] = zipped[i].second;
         }

         fRunNumber = reorder<int>(fRunNumber,idx);
         fMaxTime = reorder<double>(fMaxTime,idx);
         fZeroTime = reorder<double>(fZeroTime,idx);
  
         isSorted = true;
      }
      ClassDef(TTimeWindows, 1);
};

//Generic feLabVIEW / feGEM data inside a time window
class TEnvDataPlot: public TObject
{
   public:
      std::vector<double> fTimes;
      std::vector<double> fRunTimes;
      std::vector<double> fData;
   public:
      TEnvDataPlot()
      {
      }
      ~TEnvDataPlot()
      {
      }
      TEnvDataPlot(const TEnvDataPlot& envDataPlot) : TObject(envDataPlot)
      {
         fTimes = envDataPlot.fTimes;
         fRunTimes = envDataPlot.fRunTimes;
         fData = envDataPlot.fData;
      }
      TEnvDataPlot operator=(const TEnvDataPlot rhs)
      {
         this->fTimes = rhs.fTimes;
         this->fRunTimes = rhs.fRunTimes;
         this->fData = rhs.fData;

         return *this;
      }
      void AddPoint(double time, double runTime, double data)
      {
         fTimes.push_back(time);
         fRunTimes.push_back(runTime);
         fData.push_back(data);
      }
      TGraph* GetGraph(bool zeroTime)
      {
         if (zeroTime)
            return new TGraph(fData.size(),fTimes.data(),fData.data());
         else
            return new TGraph(fData.size(),fRunTimes.data(),fData.data());
      }
      ClassDef(TEnvDataPlot, 1);
};

//Collection of feLabVIEW / feGEM data with the same name (the same source)
class TEnvData: public TObject 
{
   public:
      std::string fVariableName;
      std::string fLabel;
      int fArrayNumber;
   private:
      std::vector<TEnvDataPlot*> fPlots;
   public:
      std::string GetVariable() { return fVariableName + "[" + std::to_string(fArrayNumber) + "]";}
      std::string GetVariableID() { return fVariableName + "_" + std::to_string(fArrayNumber);}
      std::string GetLabel() { return fLabel; }
      TEnvData()
      {
         fArrayNumber = -1;
      }
      ~TEnvData()
      {
      }
      TEnvData(const TEnvData& envData) : TObject(envData)
      {
         fVariableName = envData.fVariableName;
         fLabel = envData.fLabel;
         fArrayNumber = envData.fArrayNumber;
         for (TEnvDataPlot* plots: envData.fPlots)
            fPlots.push_back(new TEnvDataPlot(*plots));

      }
      TEnvData operator=(const TEnvData rhs)
      {
         this->fVariableName = rhs.fVariableName;
         this->fLabel = rhs.fLabel;
         this->fArrayNumber = rhs.fArrayNumber;
         for (TEnvDataPlot* plots: rhs.fPlots)
            fPlots.push_back(new TEnvDataPlot(*plots));
         return *this;
      }
      std::pair<double,double> GetMinMax()
      {
         double min = -1E99;
         double max = 1E99;
         for (auto& plot: fPlots)
         {
            for (double& data: plot->fData)
            {
               if (data < min)
                  min = data;
               if (data > max )
                  max = data;
            }
         }
         return std::pair<double,double>(min,max);
      }
      TEnvDataPlot* GetPlot(size_t index)
      {
         while (index>=fPlots.size())
         {
            TEnvDataPlot* graph = new TEnvDataPlot();
            fPlots.push_back(graph);
         }
         return fPlots.at(index);
      }
      TGraph* BuildGraph(int index, bool zeroTime)
      {
         TGraph* graph = GetPlot(index)->GetGraph(zeroTime);
         graph->SetNameTitle(
            GetVariable().c_str(),
            std::string( GetLabel() + "; t [s];").c_str()
            );
         return graph;
      }
      ClassDef(TEnvData, 1);
};

//Specialise the above for feGEM
class TFEGEMData: public TEnvData
{
   public:
      template<typename T>
      void AddGEMEvent(TStoreGEMData<T>* gemEvent, TTimeWindows& timeWindows)
      {
         double time = gemEvent->GetRunTime();
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //Hopefully now better than On^2
         int index = timeWindows.GetValidWindowNumber(time);
         if(index>=0)
         {
            TEnvDataPlot* plot = GetPlot(index);
            plot->AddPoint(time - timeWindows.fZeroTime[index], time, (double)gemEvent->GetArrayEntry(fArrayNumber));
         }
         return;
      }
      static std::string CombinedName(const std::string& category, const std::string& varName)
      {
         return std::string(category + "\\" + varName);
      }
      ClassDef(TFEGEMData, 1);
};

//Specialise the above for feLabVIEW
class TFELabVIEWData: public TEnvData
{
   public:
      void AddLVEvent(int runNumber, TStoreLabVIEWEvent* labviewEvent, TTimeWindows& timeWindows)
      {
         double time=labviewEvent->GetRunTime();
         double runStart = Get_A2Analysis_Report(runNumber).GetRunStartTime();
         time = labviewEvent->GetMIDAS_TIME() - runStart;
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //Hopefully now better than On^2
         int index = timeWindows.GetValidWindowNumber(time);
         if(index>=0)
         {
            TEnvDataPlot* plot = GetPlot(index);
            plot->AddPoint(time - timeWindows.fZeroTime[index], time, labviewEvent->GetArrayEntry(fArrayNumber));
         }
         return;
      }
      ClassDef(TFELabVIEWData, 1);
};

class TAPlot: public TObject
{
   protected:
      
      std::string fCanvasTitle; //Used to give the TCanvas a title
      int fMVAMode; //Default 0;
      int fNumBins; // 100;
      int fDrawStyle; //Switch between colour modes
      int fLegendDetail; // = 1;
      int fTotalVert;
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
      int                  GetNVerticies()          {  return fTotalVert;   }
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
      void LoadFELVData(int runNumber, TFELabVIEWData& labviewData, TTreeReader* labviewReader, const char* name, double firstTime, double lastTime);
      void LoadFELVData(int runNumber, double firstTime, double lastTime);
      virtual void LoadRun(int runNumber, double firstTime, double lastTime) {};
      void LoadData(bool verbose = false);

      
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

