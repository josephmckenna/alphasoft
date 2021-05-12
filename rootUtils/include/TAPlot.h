
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
      TVertexEvents(const TVertexEvents& kVertexEvents)
      {
         //Making deep copies of the vectors, we require loop method as the vectors are already initialised earlier.
         for(int i = 0; i < kVertexEvents.fXVertex.size(); i++)
         {
            fRunNumbers.push_back(kVertexEvents.fRunNumbers[i]);
            fEventNos.push_back(kVertexEvents.fEventNos[i]);
            fCutsResults.push_back(kVertexEvents.fCutsResults[i]);
            fVertexStatuses.push_back(kVertexEvents.fVertexStatuses[i]);
            fXVertex.push_back(kVertexEvents.fXVertex[i]);
            fYVertex.push_back(kVertexEvents.fYVertex[i]);
            fZVertex.push_back(kVertexEvents.fZVertex[i]);
            fTimes.push_back(kVertexEvents.fTimes[i]);
            fEventTimes.push_back(kVertexEvents.fEventTimes[i]);
            fRunTimes.push_back(kVertexEvents.fRunTimes[i]);
            fNumHelices.push_back(kVertexEvents.fNumHelices[i]);
            fNumTracks.push_back(kVertexEvents.fNumTracks[i]);
         }
      }
      //Assignment operator.
      TVertexEvents& operator=(const TVertexEvents& kVertexEvents)
      {
         //Making deep copies of the vectors, we require loop method as the vectors are already initialised earlier.
         for(int i = 0; i < kVertexEvents.fXVertex.size(); i++)
         {
            this->fRunNumbers.push_back(kVertexEvents.fRunNumbers[i]);
            this->fEventNos.push_back(kVertexEvents.fEventNos[i]);
            this->fCutsResults.push_back(kVertexEvents.fCutsResults[i]);
            this->fVertexStatuses.push_back(kVertexEvents.fVertexStatuses[i]);
            this->fXVertex.push_back(kVertexEvents.fXVertex[i]);
            this->fYVertex.push_back(kVertexEvents.fYVertex[i]);
            this->fZVertex.push_back(kVertexEvents.fZVertex[i]);
            this->fTimes.push_back(kVertexEvents.fTimes[i]);
            this->fEventTimes.push_back(kVertexEvents.fEventTimes[i]);
            this->fRunTimes.push_back(kVertexEvents.fRunTimes[i]);
            this->fNumHelices.push_back(kVertexEvents.fNumHelices[i]);
            this->fNumTracks.push_back(kVertexEvents.fNumTracks[i]);
         }
         return *this;
      }
      //=+ Operator.
      TVertexEvents operator+=(const TVertexEvents &kVertexEventsA) 
      {
         std::cout << "TAVertexEvents += operator" << std::endl;
         this->fRunNumbers       .insert(this->fRunNumbers.end(),      kVertexEventsA.fRunNumbers.begin(),     kVertexEventsA.fRunNumbers.end());
         this->fEventNos         .insert(this->fEventNos.end(),        kVertexEventsA.fEventNos.begin(),       kVertexEventsA.fEventNos.end());
         this->fCutsResults      .insert(this->fCutsResults.end(),     kVertexEventsA.fCutsResults.begin(),    kVertexEventsA.fCutsResults.end()); 
         this->fVertexStatuses   .insert(this->fVertexStatuses.end(),  kVertexEventsA.fVertexStatuses.begin(), kVertexEventsA.fVertexStatuses.end());    
         this->fXVertex               .insert(this->fXVertex.end(),              kVertexEventsA.fXVertex.begin(),             kVertexEventsA.fXVertex.end());
         this->fYVertex               .insert(this->fYVertex.end(),              kVertexEventsA.fYVertex.begin(),             kVertexEventsA.fYVertex.end());
         this->fZVertex               .insert(this->fZVertex.end(),              kVertexEventsA.fZVertex.begin(),             kVertexEventsA.fZVertex.end());
         this->fTimes               .insert(this->fTimes.end(),              kVertexEventsA.fTimes.begin(),             kVertexEventsA.fTimes.end());
         this->fEventTimes       .insert(this->fEventTimes.end(),      kVertexEventsA.fEventTimes.begin(),     kVertexEventsA.fEventTimes.end());
         this->fRunTimes         .insert(this->fRunTimes.end(),        kVertexEventsA.fRunTimes.begin(),       kVertexEventsA.fRunTimes.end());
         this->fNumHelices         .insert(this->fNumHelices.end(),        kVertexEventsA.fNumHelices.begin(),       kVertexEventsA.fNumHelices.end());
         this->fNumTracks          .insert(this->fNumTracks.end(),         kVertexEventsA.fNumTracks.begin(),        kVertexEventsA.fNumTracks.end());
         return *this;
      }
};

class TTimeWindows : public TObject
{
   private:
      bool isSorted = false;
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fTMin;
      std::vector<double> fTmax;
      std::vector<double> fTZero;
      
      //Standard ctor and dtor
      TTimeWindows()
      {
      }
      ~TTimeWindows()
      {
      }
      //Copy ctor
      TTimeWindows(const TTimeWindows& kTimeWindows)
      {
         std::cout << "TTimeWindows copy ctor." << std::endl;
         //Deep copy
         for(int i = 0; i<kTimeWindows.fTMin.size(); i++)
         {
            fRunNumber.push_back(kTimeWindows.fRunNumber[i]);
            fTMin.push_back(kTimeWindows.fTMin[i]);
            fTmax.push_back(kTimeWindows.fTmax[i]);
            fTZero.push_back(kTimeWindows.fTZero[i]);
         }
      }
      TTimeWindows& operator=(const TTimeWindows& kTimeWindows)
      {
         std::cout << "TTimeWindows = operator." << std::endl;
         for(int i = 0; i<kTimeWindows.fTMin.size(); i++)
         {
            this->fRunNumber.push_back(kTimeWindows.fRunNumber[i]);
            this->fTMin.push_back(kTimeWindows.fTMin[i]);
            this->fTmax.push_back(kTimeWindows.fTmax[i]);
            this->fTZero.push_back(kTimeWindows.fTZero[i]);
         }
         return *this;
      }
      friend TTimeWindows operator+(const TTimeWindows& kTimeWindowsA, const TTimeWindows& kTimeWindowsB)
      {
         std::cout << "TTimeWindows addition operator" << std::endl;
         TTimeWindows outputPlot(kTimeWindowsA); //Create new from copy

         //Vectors- need concacting
         outputPlot.fRunNumber.insert(outputPlot.fRunNumber.end(), kTimeWindowsB.fRunNumber.begin(), kTimeWindowsB.fRunNumber.end() );
         outputPlot.fTMin.insert(outputPlot.fTMin.end(), kTimeWindowsB.fTMin.begin(), kTimeWindowsB.fTMin.end() );
         outputPlot.fTmax.insert(outputPlot.fTmax.end(), kTimeWindowsB.fTmax.begin(), kTimeWindowsB.fTmax.end() );
         outputPlot.fTZero.insert(outputPlot.fTZero.end(), kTimeWindowsB.fTZero.begin(), kTimeWindowsB.fTZero.end() );
         return outputPlot;
      }
      TTimeWindows operator+=(const TTimeWindows &kTimeWindowsA) 
      {
         std::cout << "TTimeWindows += operator" << std::endl;
         this->fRunNumber.insert(this->fRunNumber.end(), kTimeWindowsA.fRunNumber.begin(), kTimeWindowsA.fRunNumber.end() );
         this->fTMin.insert(this->fTMin.end(), kTimeWindowsA.fTMin.begin(), kTimeWindowsA.fTMin.end() );
         this->fTmax.insert(this->fTmax.end(), kTimeWindowsA.fTmax.begin(), kTimeWindowsA.fTmax.end() );
         this->fTZero.insert(this->fTZero.end(), kTimeWindowsA.fTZero.begin(), kTimeWindowsA.fTZero.end() );
         return *this;
      }
      void AddTimeWindow(int runNumber, double minTime, double maxTime, double timeZero)
      {
         fRunNumber.push_back(runNumber);
         fTMin.push_back(minTime);
         fTmax.push_back(maxTime);
         fTZero.push_back(timeZero);
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
         const size_t kTMaxSize = fTmax.size();
         for (int j = 0; j < kTMaxSize; j++)
         {
            //If inside the time window
            if ( time > fTMin[j] )
            {
               const double kCurrentTMax = fTmax[j];
               if(time < kCurrentTMax || kCurrentTMax < 0)
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
         assert( fTMin.size() == fTmax.size() );
         const int kTMinSize = fTMin.size();
         //Create vectors needed for sorting.
         std::vector<std::pair<double,double>> zipped(kTMinSize);
         std::vector<size_t> idx(kTMinSize);
         iota(idx.begin(),idx.end(),0);


         //Zip tmin and index vector together.
         zipped.resize(kTMinSize);
         for(size_t i=0; i < kTMinSize; ++i)
         {
            zipped[i]=std::make_pair(fTMin[i], idx[i]);
         }

         //Sort based on first (tmin) keeping idx in the same location
         std::sort(std::begin(zipped), std::end(zipped), 
         [&](const std::pair<double,double>& kPairA, const std::pair<double,double>& kPairB)
         {return kPairA.first < kPairB.first;} );
         
         //Unzip back into vectors.
         for(size_t i=0; i < kTMinSize; i++)
         {
            fTMin[i] = zipped[i].first;
            idx[i] = zipped[i].second;
         }

         fRunNumber = reorder<int>(fRunNumber,idx);
         fTmax = reorder<double>(fTmax,idx);
         fTZero = reorder<double>(fTZero,idx);
  
         isSorted = true;
      }
};

//Generic feLabVIEW / feGEM data inside a time window
class TFEENVDataPlot: public TObject //Joe - Technically I think this is right (begins with T, followed by acronym) but I'm not actually sure what "feENV" means.
{
   public:
      std::vector<double> fTimes;
      std::vector<double> fRunTimes;
      std::vector<double> fData;
   public:
      TFEENVDataPlot()
      {
      }
      ~TFEENVDataPlot()
      {
      }
      TFEENVDataPlot(const TFEENVDataPlot& kTFEENVDataPlot)
      {
         fTimes = kTFEENVDataPlot.fTimes;
         fRunTimes = kTFEENVDataPlot.fRunTimes;
         fData = kTFEENVDataPlot.fData;
      }
      TFEENVDataPlot operator=(const TFEENVDataPlot kTFEENVDataPlot)
      {
         this->fTimes = kTFEENVDataPlot.fTimes;
         this->fRunTimes = kTFEENVDataPlot.fRunTimes;
         this->fData = kTFEENVDataPlot.fData;

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
};

//Collection of feLabVIEW / feGEM data with the same name (the same source)
class TFEENVData: public TObject 
{
   public:
      std::string fName;
      std::string fTitle;
      int fArrayNumber;
   private:
      std::vector<TFEENVDataPlot*> fPlots;
   public:
      std::string GetName() { return fName + "[" + std::to_string(fArrayNumber) + "]";}
      std::string GetNameID() { return fName + "_" + std::to_string(fArrayNumber);}
      std::string GetTitle() { return fTitle; }
      TFEENVData()
      {
         fArrayNumber = -1;
      }
      ~TFEENVData()
      {
      }
      TFEENVData(const TFEENVData& kTFEENVData)
      {
         fName = kTFEENVData.fName;
         fTitle = kTFEENVData.fTitle;
         fArrayNumber = kTFEENVData.fArrayNumber;
         for (TFEENVDataPlot* plots: kTFEENVData.fPlots)
            fPlots.push_back(new TFEENVDataPlot(*plots));

      }
      TFEENVData operator=(const TFEENVData kTFEENVData)
      {
         this->fName = kTFEENVData.fName;
         this->fTitle = kTFEENVData.fTitle;
         this->fArrayNumber = kTFEENVData.fArrayNumber;
         for (TFEENVDataPlot* plots: kTFEENVData.fPlots)
            fPlots.push_back(new TFEENVDataPlot(*plots));
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
      TFEENVDataPlot* GetPlot(size_t index)
      {
         while (index>=fPlots.size())
         {
            TFEENVDataPlot* graph = new TFEENVDataPlot();
            fPlots.push_back(graph);
         }
         return fPlots.at(index);
      }
      TGraph* BuildGraph(int index, bool zeroTime)
      {
         TGraph* graph = GetPlot(index)->GetGraph(zeroTime);
         graph->SetNameTitle(GetName().c_str(),std::string( GetTitle() + "; t [s];").c_str());
         return graph;
      }
};

//Specialise the above for feGEM
class TFEGEMData: public TFEENVData
{
   public:
      template<typename T>
      void AddGEMEvent(TStoreGEMData<T>* GEMEvent, TTimeWindows& timeWindows)
      {
         double time = GEMEvent->GetRunTime();
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //Hopefully now better than On^2
         int index = timeWindows.GetValidWindowNumber(time);
         if(index>=0)
         {
            TFEENVDataPlot* plot = GetPlot(index);
            plot->AddPoint(time, time - timeWindows.fTZero[index], (double)GEMEvent->GetArrayEntry(fArrayNumber));
         }
         return;
      }
      static std::string CombinedName(const std::string& category, const std::string& varName)
      {
         return std::string(category + "\\" + varName);
      }
};

//Specialise the above for feLabVIEW
class TFELVData: public TFEENVData
{
   public:
      void AddLVEvent(TStoreLabVIEWEvent* LVEvent, TTimeWindows& timeWindows)
      {
         double time=LVEvent->GetRunTime();
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //Hopefully now better than On^2
         int index = timeWindows.GetValidWindowNumber(time);
         if(index>=0)
         {
            TFEENVDataPlot* plot = GetPlot(index);
            plot->AddPoint( time, time - timeWindows.fTZero[index], LVEvent->GetArrayEntry(fArrayNumber));
         }
         return;
      }
};

class TAPlot: public TObject
{
   protected:
      //Used to give the TCanvas a title
      std::string fTitle;
      
      int fMVAMode; //Default 0;
      int fNumBins; // 100;
      int fDrawStyle; //Switch between colour modes

      int fLegendDetail; // = 1;
      Bool_t fApplyCuts;
      double fClassifierCut;

      double fFirstTMin;
      double fLastTMax;
      double fBiggestTZero;
      double fMaxDumpLength;
      TTimeWindows fTimeWindows;
      double fTotalTime;
      int fTotalVert;

      bool fVerbose;
      //Time scaling factor (used to switch between seconds and milliseconds)
      Double_t fTimeFactor=1.;

      //Hold historams in a vector so that saved TAGPlot objects can be 
      //backwards and forwards compatable
      TObjArray fHistos;
      std::map<std::string,int> fHistoPositions;
   
   
      std::vector<double> fEjections;
      std::vector<double> fInjections;

      std::vector<double> fDumpStarts;
      std::vector<double> fDumpStops;

      TVertexEvents fVertexEvents;
      std::vector<int> Runs; //check dupes - ignore copies. AddRunNumber

      std::vector<TFEGEMData> feGEM;

      std::vector<TFELVData> feLV;
      TTimeStamp ObjectConstructionTime{0};
      TTimeStamp DataLoadedTime{0};
   

   public:
      //Use a time axis that counts from Zero of a dump window (default true)
      const bool ZeroTimeAxis;

      void SetTAPlotTitle(const std::string& _title)     {  fTitle=_title;  }
      std::string GetTAPlotTitle()                       {  return fTitle;  }
      bool HaveGEMData()                                 {  return (feGEM.size() != 0);   }
      bool HaveLVData()                                  {  return (feGEM.size() != 0);   }
      void LoadingDataLoadingDone()                      {  DataLoadedTime = TTimeStamp(); }
      int GetNBins() const                               {  return fNumBins; }
      void SetTimeFactor(double t)                       {  fTimeFactor=t; }
      double GetTimeFactor() const                       {  return fTimeFactor; }
      void AddVertexEvent(int runNumber, int EventNo, int CutsResult, int VertexStatus, double x, double y, double z, double t, double EventTime, double RunTime, int nHelices, int nTracks)                 
      {
         fVertexEvents.fRunNumbers.push_back(runNumber);
         fVertexEvents.fEventNos.push_back(EventNo);
         fVertexEvents.fCutsResults.push_back(CutsResult);
         fVertexEvents.fVertexStatuses.push_back(VertexStatus);
         fVertexEvents.fXVertex.push_back(x);
         fVertexEvents.fYVertex.push_back(y);
         fVertexEvents.fZVertex.push_back(z);
         fVertexEvents.fTimes.push_back(t);
         fVertexEvents.fEventTimes.push_back(EventTime);
         fVertexEvents.fRunTimes.push_back(RunTime);
         fVertexEvents.fNumHelices.push_back(nHelices);
         fVertexEvents.fNumTracks.push_back(nTracks);
      }
      TTimeWindows* GetTimeWindows()                     {  return &fTimeWindows; }
      const TVertexEvents* GetVertexEvents()             {  return &fVertexEvents; }
      void SetCutsOn()                                   {  fApplyCuts = kTRUE; }
      void SetCutsOff()                                  {  fApplyCuts = kFALSE; }
      bool GetCutsSettings() const                       {  return fApplyCuts; }
      void SetBinNumber(int bin)                         {  fNumBins = bin; }
      void SetMVAMode(int Mode)                          {  fMVAMode = Mode; }
      size_t GetNVertexEvents()                          {  return fVertexEvents.fXVertex.size(); }
      int GetNVerticies()                                {  return fTotalVert;   }
      double GetMaxDumpLength() const                    {  return fMaxDumpLength; }
      double GetFirstTmin() const                        {  return fFirstTMin;     }
      double GetLastTmax() const                         {  return fLastTMax;      }
      double GetBiggestTzero() const                     {  return fBiggestTZero;  }
      int GetNPassedCuts()                               {  return GetNPassedType(1);  }
      void AddStartDumpMarker(double t)                  {  fDumpStarts.push_back(t);}
      void AddStopDumpMarker(double t)                   {  fDumpStops.push_back(t); }
      void AddInjection(double t)                        {  fInjections.push_back(t);}
      void AddEjection(double t)                         {  fEjections.push_back(t);  }

      void SetGEMChannel(const std::string& name, int ArrayEntry, std::string title="");
      
      void SetGEMChannel(const std::string& Category, const std::string& Varname, int ArrayEntry, std::string title="");

      std::vector<std::pair<std::string,int>> GetGEMChannels();

      std::pair<TLegend*,TMultiGraph*> GetGEMGraphs();

      void SetLVChannel(const std::string& name, int ArrayEntry, std::string title="");

      std::vector<std::pair<std::string,int>> GetLVChannels();

      std::pair<TLegend*,TMultiGraph*> GetLVGraphs();

      template<typename T> void LoadfeGEMData(TFEGEMData& f, TTreeReader* feGEMReader, const char* name, double first_time, double last_time);
      void LoadfeGEMData(int RunNumber, double first_time, double last_time);

      void LoadfeLVData(TFELVData& f, TTreeReader* feLVReader, const char* name, double first_time, double last_time);
      void LoadfeLVData(int RunNumber, double first_time, double last_time);

      double GetApproximateProcessingTime();
      
      // default class member functions
      TAPlot(const TAPlot& m_TAPlot);
      TAPlot(bool zerotime = true);//, int MVAMode = 0);
      virtual ~TAPlot();
      void Print(Option_t *option="") const;

      void AddRunNumber(int runNumber);

      int GetNPassedType(const int type);
      
      //virtual void AddToTAPlot(TString file);
      //virtual TAPlot* LoadTAPlot(TString file);
      void AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax, std::vector<double> tzero);
      void AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax);
      void AddTimeGate(const int runNumber, const double tmin, const double tmax, const double tzero);
      void AddTimeGate(const int runNumber, const double tmin, const double tmax);
      virtual void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition ) {assert(!"Child class must have this");};
      //?virtual void AddDumpGates(int runNumber, std::vector<TA2Spill*> spills ) =0;
      //If spills are from one run, it is faster to call the function above
      //?virtual void AddDumpGates(std::vector<TA2Spill*> spills ) =0;
      virtual void LoadRun(int runNumber, double first_time, double last_time) {};
      void LoadData();

      void AutoTimeRange();
   
      //void SetTimeRange(double tmin_, double tmax_);

      template <class T>
      void AddHistogram(const char* keyname, T* h)
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

      TLegend* AddLegendIntegral(TLegend* legend, const char* message,const char* keyname);
      
      TString GetListOfRuns();

      const std::vector<int> GetArrayOfRuns() { return Runs; }
      void SetUpHistograms();
      void PrintTimeRanges();

      //virtual void FillHisto();
      TObjArray GetHisto() {return fHistos;}
      std::map<std::string,int> GetHistoPosition() {return fHistoPositions;}
      void ClearHisto();
      //TCanvas *Canvas(TString Name = "cVTX");

      void SetVerbose(bool v) {fVerbose=v;}
   
      //virtual void ExportCSV(TString filename, Bool_t PassedCutOnly=kFALSE);

      virtual void PrintFull();

      TAPlot& operator=(const TAPlot& m_TAPlot);
      friend TAPlot operator+(const TAPlot& plotA, const TAPlot& plotB);
      TAPlot& operator+=(const TAPlot &plotB);

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

