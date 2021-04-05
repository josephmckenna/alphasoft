
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
class TAVertexEvents: public TObject
{
   public:
      std::vector<int> runNumbers; // I don't get set yet...
      std::vector<int> EventNos;
      std::vector<int> CutsResults;
      std::vector<int> VertexStatuses;
      std::vector<double> xs;
      std::vector<double> ys;
      std::vector<double> zs;
      std::vector<double> ts; //Plot time (based off offical time)
      std::vector<double> EventTimes; //TPC time stamp
      std::vector<double> RunTimes; //Official Time
      std::vector<int> nHelices; // helices used for vertexing
      std::vector<int> nTracks; // reconstructed (good) helices

      //LMG - Copy and assign operators
      TAVertexEvents()
      {
      }
      ~TAVertexEvents()
      {
      }
      //Basic copy constructor.
      TAVertexEvents(const TAVertexEvents& m_VertexEvents)
      {
         runNumbers           = m_VertexEvents.runNumbers;
         EventNos             = m_VertexEvents.EventNos;
         CutsResults          = m_VertexEvents.CutsResults;
         VertexStatuses       = m_VertexEvents.VertexStatuses;
         xs                   = m_VertexEvents.xs;
         ys                   = m_VertexEvents.ys;
         zs                   = m_VertexEvents.zs;
         ts                   = m_VertexEvents.ts;
         EventTimes           = m_VertexEvents.EventTimes;
         RunTimes             = m_VertexEvents.RunTimes;
         nHelices             = m_VertexEvents.nHelices;
         nTracks              = m_VertexEvents.nTracks;
      }
      //Assignment operator.
      TAVertexEvents operator=(const TAVertexEvents m_VertexEvents)
      {
         this->runNumbers           = m_VertexEvents.runNumbers;
         this->EventNos             = m_VertexEvents.EventNos;
         this->CutsResults          = m_VertexEvents.CutsResults;
         this->VertexStatuses       = m_VertexEvents.VertexStatuses;
         this->xs                   = m_VertexEvents.xs;
         this->ys                   = m_VertexEvents.ys;
         this->zs                   = m_VertexEvents.zs;
         this->ts                   = m_VertexEvents.ts;
         this->EventTimes           = m_VertexEvents.EventTimes;
         this->RunTimes             = m_VertexEvents.RunTimes;
         this->nHelices             = m_VertexEvents.nHelices;
         this->nTracks              = m_VertexEvents.nTracks;
         return *this;
      }
      TAVertexEvents operator+=(const TAVertexEvents &plotB) 
      {
         std::cout << "TAVertexEvents += operator" << std::endl;
         this->runNumbers       .insert(this->runNumbers.end(),      plotB.runNumbers.begin(),     plotB.runNumbers.end());
         this->EventNos         .insert(this->EventNos.end(),        plotB.EventNos.begin(),       plotB.EventNos.end());
         this->CutsResults      .insert(this->CutsResults.end(),     plotB.CutsResults.begin(),    plotB.CutsResults.end()); 
         this->VertexStatuses   .insert(this->VertexStatuses.end(),  plotB.VertexStatuses.begin(), plotB.VertexStatuses.end());    
         this->xs               .insert(this->xs.end(),              plotB.xs.begin(),             plotB.xs.end());
         this->ys               .insert(this->ys.end(),              plotB.ys.begin(),             plotB.ys.end());
         this->zs               .insert(this->zs.end(),              plotB.zs.begin(),             plotB.zs.end());
         this->ts               .insert(this->ts.end(),              plotB.ts.begin(),             plotB.ts.end());
         this->EventTimes       .insert(this->EventTimes.end(),      plotB.EventTimes.begin(),     plotB.EventTimes.end());
         this->RunTimes         .insert(this->RunTimes.end(),        plotB.RunTimes.begin(),       plotB.RunTimes.end());
         this->nHelices         .insert(this->nHelices.end(),        plotB.nHelices.begin(),       plotB.nHelices.end());
         this->nTracks          .insert(this->nTracks.end(),         plotB.nTracks.begin(),        plotB.nTracks.end());
         return *this;
      }
};

class TATimeWindows : public TObject
{
   public:
      bool isSorted = false;
      std::vector<int> runNumber;
      std::vector<double> tmin;
      std::vector<double> tmax;
      std::vector<double> tzero;
      
      TATimeWindows()
      {
      }
      ~TATimeWindows()
      {
      }
      void AddTimeWindow(int _runNumber, double _tmin, double _tmax, double _tzero)
      {
         std::cout << "Adding TimeWindow: tmin = " << _tmin << ", tmax = " << _tmax << std::endl;
         runNumber.push_back(_runNumber);
         tmin.push_back(_tmin);
         tmax.push_back(_tmax);
         tzero.push_back(_tzero);
         assert(_tmax>_tmin);
         assert(_tzero>=_tmin);
         assert(_tzero<_tmax);
      }
      TATimeWindows(const TATimeWindows& m_TimeWindows)
      {
         runNumber = m_TimeWindows.runNumber;
         tmin = m_TimeWindows.tmin;
         tmax = m_TimeWindows.tmax;
         tzero = m_TimeWindows.tzero;
      }
      int GetValidWindowNumber(double t)
      {
         //Checks whether vector is sorted and sorts if not.
         if(!isSorted)
         {
            SortTimeWindows();
         }

         //Check where t sits in tmin.
         for(int i = 0; i < tmax.size(); i++)
         {
            //If inside the tmin window
            if ( t > tmin[i])
            {
               //Then check that it's within tmax or whether tmax is out of range.
               if( t < tmax[i] || tmax[i] < 0)
               {
                  //If so return index, i, for adding the event to the vector and break (no need to keep searching)
                  return i;
                  break;
               }
            } 
            //If not return -1 (error)
            else
            {
               return -1;
            }
         }
         //If we get down here just return error.
         return -1;
      }
      void PrintTheBoy()
      {
         std::cout << "tmin size = " << tmin.size() << std::endl;
         for(size_t i=0; i<tmin.size(); i++)
         {
            std::cout << "tmin[" << i << "] = " << tmin[i] << std::endl;
         }

         std::cout << "tmax size = " << tmax.size() << std::endl;
         for(size_t i=0; i<tmax.size(); i++)
         {
            std::cout << "tmax[" << i << "] = " << tmax[i] << std::endl;
         }

         std::cout << "runNumber size = " << runNumber.size() << std::endl;
         for(size_t i=0; i<runNumber.size(); i++)
         {
            std::cout << "runNumber[" << i << "] = " << runNumber[i] << std::endl;
         }

         std::cout << "tzero size = " << tzero.size() << std::endl;
         for(size_t i=0; i<tzero.size(); i++)
         {
            std::cout << "tzero[" << i << "] = " << tzero[i] << std::endl;
         }
      }
      template <class T>
      std::vector<T> reorder(std::vector<T>& nums, std::vector<size_t>& index)
      {
         int n = nums.size();
         std::vector<T> ans(n);
         for (int i = 0; i < n; i++)
         {
            ans[i]=nums[i];
         }   
         // finally return ans
         return ans;
      }
      void SortTimeWindows()
      {
         //PrintTheBoy();
         //Create vectors needed for sorting.
         std::vector<std::pair<double,double>> zipped;
         std::vector<size_t> idx(tmin.size());
         iota(idx.begin(),idx.end(),0);

         //Zip tmin and index vector together.
         for(size_t i=0; i<tmin.size(); ++i)
         {
            zipped.push_back(std::make_pair(tmin[i], idx[i]));
         }

         //Sort based on first (tmin) keeping idx in the same location
         std::sort(std::begin(zipped), std::end(zipped), 
         [&](const std::pair<double,double>& a, const std::pair<double,double>& b)
         {return a.second > b.second;} );
         
         //Unzip back into vectors.
         for(size_t i=0; i<tmin.size(); i++)
         {
            tmin[i] = zipped[i].first;
            idx[i] = zipped[i].second;
         }

         runNumber=reorder<int>(runNumber,idx);
         tmin=reorder<double>(tmin,idx);
         tmax=reorder<double>(tmax,idx);
         tzero=reorder<double>(tzero,idx);
  
         isSorted = true;
         //PrintTheBoy();
      }
      TATimeWindows operator=(const TATimeWindows m_TimeWindows)
      {
         this->runNumber = m_TimeWindows.runNumber;
         this->tmin = m_TimeWindows.tmin;
         this->tmax = m_TimeWindows.tmax;
         this->tzero = m_TimeWindows.tzero;
         return *this;
      }
      friend TATimeWindows operator+(const TATimeWindows& plotA, const TATimeWindows& plotB)
      {
         std::cout << "TATimeWindows addition operator" << std::endl;
         TATimeWindows outputplot(plotA); //Create new from copy

         //Vectors- need concacting
         outputplot.runNumber.insert(outputplot.runNumber.end(), plotB.runNumber.begin(), plotB.runNumber.end() );
         outputplot.tmin.insert(outputplot.tmin.end(), plotB.tmin.begin(), plotB.tmin.end() );
         outputplot.tmax.insert(outputplot.tmax.end(), plotB.tmax.begin(), plotB.tmax.end() );
         outputplot.tzero.insert(outputplot.tzero.end(), plotB.tzero.begin(), plotB.tzero.end() );
         return outputplot;
      }
      TATimeWindows operator+=(const TATimeWindows &plotB) 
      {
         std::cout << "TATimeWindows += operator" << std::endl;
         this->runNumber.insert(this->runNumber.end(), plotB.runNumber.begin(), plotB.runNumber.end() );
         this->tmin.insert(this->tmin.end(), plotB.tmin.begin(), plotB.tmin.end() );
         this->tmax.insert(this->tmax.end(), plotB.tmax.begin(), plotB.tmax.end() );
         this->tzero.insert(this->tzero.end(), plotB.tzero.begin(), plotB.tzero.end() );
         return *this;
      }
};

//Generic feLabVIEW / feGEM data inside a time window
class feENVdataPlot
{
   public:
      std::vector<double> t;
      std::vector<double> RunTime;
      std::vector<double> data;
   public:
      feENVdataPlot()
      {
      }
      ~feENVdataPlot()
      {
      }
      feENVdataPlot(const feENVdataPlot& m_feENVdataPlot)
      {
         t = m_feENVdataPlot.t;
         RunTime = m_feENVdataPlot.RunTime;
         data = m_feENVdataPlot.data;
      }
      feENVdataPlot operator=(const feENVdataPlot m_feENVdataPlot)
      {
         this->t = m_feENVdataPlot.t;
         this->RunTime = m_feENVdataPlot.RunTime;
         this->data = m_feENVdataPlot.data;

         return *this;
      }
      void AddPoint(double _t, double _RunTime, double _data)
      {
         t.push_back(_t);
         RunTime.push_back(_RunTime);
         data.push_back(_data);
      }
      TGraph* GetGraph(bool zerotime)
      {
         if (zerotime)
            return new TGraph(data.size(),t.data(),data.data());
         else
            return new TGraph(data.size(),RunTime.data(),data.data());
      }
};

//Collection of feLabVIEW / feGEM data with the same name (the same source)
class feENVdata {
   public:
      std::string name;
      std::string title;
      int array_number;
   private:
      std::vector<feENVdataPlot*> plots;
   public:
      std::string GetName() { return name + "[" + std::to_string(array_number) + "]";}
      std::string GetNameID() { return name + "_" + std::to_string(array_number);}
      std::string GetTitle() { return title; }
      feENVdata()
      {
         array_number = -1;
      }
      ~feENVdata()
      {
      }
      feENVdata(const feENVdata& m_feENVdata)
      {
         name = m_feENVdata.name;
         title = m_feENVdata.title;
         array_number = m_feENVdata.array_number;
         plots = m_feENVdata.plots;
      }
      feENVdata operator=(const feENVdata m_feENVdata)
      {
         this->name = m_feENVdata.name;
         this->title = m_feENVdata.title;
         this->array_number = m_feENVdata.array_number;
         this->plots = m_feENVdata.plots;
         return *this;
      }
      std::pair<double,double> GetMinMax()
      {
         double min = -1E99;
         double max = 1E99;
         for (auto& plot: plots)
         {
            for (double& d: plot->data)
            {
               if (d < min)
                  min = d;
               if (d > max )
                  max = d;
            }
         }
         return std::pair<double,double>(min,max);
      }
      feENVdataPlot* GetPlot(size_t i)
      {
         while (i>=plots.size())
         {
            feENVdataPlot* graph = new feENVdataPlot();
            plots.push_back(graph);
         }
         return plots.at(i);
      }
      TGraph* BuildGraph(int i, bool zeroTime)
      {
         TGraph* graph = GetPlot(i)->GetGraph(zeroTime);
         graph->SetNameTitle(GetName().c_str(),std::string( GetTitle() + "; t [s];").c_str());
         return graph;
      }
};

//Specialise the above for feGEM
class feGEMdata: public feENVdata
{
   public:
      template<typename T>
      void AddGEMEvent(TStoreGEMData<T>* GEMEvent,const TATimeWindows& timewindows)
      {
         double time=GEMEvent->GetRunTime();
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //for (auto& window: timewindows)
         for (size_t i=0; i<timewindows.tmax.size(); i++)
         {
            //If inside the time window
            if ( (time > timewindows.tmin.at(i) && time < timewindows.tmax.at(i)) ||
            //Or if after tmin and tmax is invalid (-1)
               (time > timewindows.tmin.at(i) && timewindows.tmax.at(i)<0) )
            {
               feENVdataPlot* plot = GetPlot(i);
               plot->AddPoint(time, time - timewindows.tzero.at(i), (double)GEMEvent->GetArrayEntry(array_number));
            }
         }
         return;
      }
      static std::string CombinedName(const std::string& Category, const std::string& Varname)
      {
         return std::string(Category + "\\" + Varname);
      }
};

//Specialise the above for feLabVIEW
class feLVdata: public feENVdata
{
   public:
      void AddLVEvent(TStoreLabVIEWEvent* LVEvent,const TATimeWindows& timewindows)
      {
         double time=LVEvent->GetRunTime();
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //for (auto& window: timewindows)
         for (size_t i=0; i<timewindows.tmax.size(); i++)
         {
            //If inside the time window
            if ( (time > timewindows.tmin.at(i) && time < timewindows.tmax.at(i)) ||
            //Or if after tmin and tmax is invalid (-1)
               (time > timewindows.tmin.at(i) && timewindows.tmax.at(i)<0) )
            {
               feENVdataPlot* plot = GetPlot(i);
               plot->AddPoint( time, time - timewindows.tzero.at(i), LVEvent->GetArrayEntry(array_number));
            }
         }
         return;
      }
};

class TAPlot: public TObject
{

   protected:
      //Used to give the TCanvas a title
      std::string title;
      
      int MVAMode; //Default 0;
      int Nbin; // 100;
      int DrawStyle; //Switch between colour modes

      int gLegendDetail; // = 1;
      Bool_t fApplyCuts;
      double fClassifierCut;

      double FirstTmin;
      double LastTmax;
      double BiggestTzero;
      double MaxDumpLength;
      TATimeWindows TimeWindows;
      double fTotalTime;
      int fTotalVert;

      bool fVerbose;
      //Time scaling factor (used to switch between seconds and milliseconds)
      Double_t tFactor=1.;

      //Hold historams in a vector so that saved TAGPlot objects can be 
      //backwards and forwards compatable
      TObjArray HISTOS;
      std::map<std::string,int> HISTO_POSITION;
   
   
      std::vector<double> Ejections;
      std::vector<double> Injections;

      std::vector<double> DumpStarts;
      std::vector<double> DumpStops;

      TAVertexEvents NewVertexEvents;
      std::vector<int> Runs; //check dupes - ignore copies. AddRunNumber

      std::vector<feGEMdata> feGEM;
      
      std::vector<feLVdata> feLV;
      TTimeStamp ObjectConstructionTime{0};
      TTimeStamp DataLoadedTime{0};
   

   public:
      //Use a time axis that counts from Zero of a dump window (default true)
      const bool ZeroTimeAxis;

      void SetTAPlotTitle(const std::string& _title)     {  title=_title;  }
      std::string GetTAPlotTitle()                       {  return title;  }
      bool HaveGEMData()                                 {  return (feGEM.size() != 0);   }
      bool HaveLVData()                                  {  return (feGEM.size() != 0);   }
      void LoadingDataLoadingDone()                      {  DataLoadedTime = TTimeStamp(); }
      int GetNBins() const                               {  return Nbin; }
      void SetTimeFactor(double t)                       {  tFactor=t; }
      double GetTimeFactor() const                       {  return tFactor; }
      void AddVertexEventByList(int runNumber, int EventNo, int CutsResult, int VertexStatus, double x, double y, double z, double t, double EventTime, double RunTime, int nHelices, int nTracks)                 
      {
         NewVertexEvents.runNumbers.push_back(runNumber);
         NewVertexEvents.EventNos.push_back(EventNo);
         NewVertexEvents.CutsResults.push_back(CutsResult);
         NewVertexEvents.VertexStatuses.push_back(VertexStatus);
         NewVertexEvents.xs.push_back(x);
         NewVertexEvents.ys.push_back(y);
         NewVertexEvents.zs.push_back(z);
         NewVertexEvents.ts.push_back(t);
         NewVertexEvents.EventTimes.push_back(EventTime);
         NewVertexEvents.RunTimes.push_back(RunTime);
         NewVertexEvents.nHelices.push_back(nHelices);
         NewVertexEvents.nTracks.push_back(nTracks);
      }
      const TATimeWindows GetTimeWindows()                 {  return TimeWindows; }
      const TAVertexEvents GetVertexEvents()             {  return NewVertexEvents; }
      void SetCutsOn()                                   {  fApplyCuts = kTRUE; }
      void SetCutsOff()                                  {  fApplyCuts = kFALSE; }
      bool GetCutsSettings() const                       {  return fApplyCuts; }
      void SetBinNumber(int bin)                         {  Nbin = bin; }
      void SetMVAMode(int Mode)                          {  MVAMode = Mode; }
      size_t GetNVertexEvents()                          {  return NewVertexEvents.xs.size(); }
      int GetNVerticies()                                {  return fTotalVert;   }
      double GetMaxDumpLength() const                    {  return MaxDumpLength; }
      double GetFirstTmin() const                        {  return FirstTmin;     }
      double GetLastTmax() const                         {  return LastTmax;      }
      double GetBiggestTzero() const                     {  return BiggestTzero;  }
      int GetNPassedCuts()                               {  return GetNPassedType(1);  }
      void AddStartDumpMarker(double t)                  {  DumpStarts.push_back(t);}
      void AddStopDumpMarker(double t)                   {  DumpStops.push_back(t); }
      void AddInjection(double t)                        {  Injections.push_back(t);}
      void AddEjection(double t)                         {  Ejections.push_back(t);  }

      void SetGEMChannel(const std::string& name, int ArrayEntry, std::string title="");
      
      void SetGEMChannel(const std::string& Category, const std::string& Varname, int ArrayEntry, std::string title="");

      std::vector<std::pair<std::string,int>> GetGEMChannels();

      std::pair<TLegend*,TMultiGraph*> GetGEMGraphs();

      void SetLVChannel(const std::string& name, int ArrayEntry, std::string title="");

      std::vector<std::pair<std::string,int>> GetLVChannels();

      std::pair<TLegend*,TMultiGraph*> GetLVGraphs();

      template<typename T> void LoadfeGEMData(feGEMdata& f, TTreeReader* feGEMReader, const char* name, double first_time, double last_time);
      void LoadfeGEMData(int RunNumber, double first_time, double last_time);

      void LoadfeLVData(feLVdata& f, TTreeReader* feLVReader, const char* name, double first_time, double last_time);
      void LoadfeLVData(int RunNumber, double first_time, double last_time);

      double GetApproximateProcessingTime();
      
      // default class member functions
      TAPlot(const TAPlot& m_TAPlot);
      TAPlot(bool zerotime = true);//, int MVAMode = 0);
      virtual ~TAPlot();
      void Print(Option_t *option="") const;

      void AddRunNumber(int runNumber);

      int GetNPassedType(const int type);
      
      void AddToTAPlot(TAPlot *ialphaplot);
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
         HISTOS.Add(h);
         HISTO_POSITION[keyname]=HISTOS.GetEntries()-1;
      }

      /*void FillfeGEMHistograms()
      {
         for (auto & p: feGEM)
         {
            TH1D* GEM_Plot = GetTH1D(p.GetName().c_str());
            std::pair<double,double> minmax = p.GetMinMax();
            GEM_Plot->SetMinimum(minmax.first);
            GEM_Plot->SetMaximum(minmax.second);
            for (int j=0; j<p.data.size(); j++)
               GEM_Plot->Fill(p.t[j],p.data[j]);
         }
      }*/

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
      TObjArray GetHisto() {return HISTOS;}
      std::map<std::string,int> GetHistoPosition() {return HISTO_POSITION;}
      void ClearHisto();
      //TCanvas *Canvas(TString Name = "cVTX");

      void SetVerbose(bool v) {fVerbose=v;}
   
      //virtual void ExportCSV(TString filename, Bool_t PassedCutOnly=kFALSE);

      virtual void PrintFull();

      TAPlot& operator=(const TAPlot& m_TAPlot);
      virtual TAPlot operator+=(const TAPlot &plotB);
      friend TAPlot operator+(const TAPlot& plotA, const TAPlot& plotB);

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

