
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
#include <chrono>
   
#define SCALECUT 0.6

//Basic internal containers:

//SVD / TPC vertex
class VertexEvent
{
   public:
      int runNumber; // I don't get set yet...
      int EventNo;
      int CutsResult;
      int VertexStatus;
      double x;
      double y;
      double z;
      double t; //Plot time (based off offical time)
      double EventTime; //TPC time stamp
      double RunTime; //Official Time
      int nHelices; // helices used for vertexing
      int nTracks; // reconstructed (good) helices

      //LMG - Copy and assign operators
      VertexEvent()
      {
      }
      ~VertexEvent()
      {
      }
      VertexEvent(const VertexEvent& m_VertexEvent)
      {
         runNumber        = m_VertexEvent.runNumber;
         EventNo          = m_VertexEvent.EventNo;
         CutsResult       = m_VertexEvent.CutsResult;
         VertexStatus     = m_VertexEvent.VertexStatus;
         x                = m_VertexEvent.x;
         y                = m_VertexEvent.y;
         z                = m_VertexEvent.z;
         t                = m_VertexEvent.t;
         EventTime        = m_VertexEvent.EventTime;
         RunTime          = m_VertexEvent.RunTime;
         nHelices         = m_VertexEvent.nHelices;
         nTracks          = m_VertexEvent.nTracks;
      }
      VertexEvent operator=(const VertexEvent m_VertexEvent)
      {
         this->runNumber        = m_VertexEvent.runNumber;
         this->EventNo          = m_VertexEvent.EventNo;
         this->CutsResult       = m_VertexEvent.CutsResult;
         this->VertexStatus     = m_VertexEvent.VertexStatus;
         this->x                = m_VertexEvent.x;
         this->y                = m_VertexEvent.y;
         this->z                = m_VertexEvent.z;
         this->t                = m_VertexEvent.t;
         this->EventTime        = m_VertexEvent.EventTime;
         this->RunTime          = m_VertexEvent.RunTime;
         this->nHelices         = m_VertexEvent.nHelices;
         this->nTracks          = m_VertexEvent.nTracks;
         return *this;
      }
};

//Any time window
class TimeWindow
{
   public:
   int runNumber;
   double tmin;
   double tmax;
   double tzero;
   TimeWindow()
   {
      runNumber = -1;
      tmin = -1;
      tmax = -1;
      tzero = -1;
   }
   TimeWindow(int _runNumber, double _tmin, double _tmax, double _tzero)
   {
      runNumber = _runNumber;
      tmin = _tmin;
      tmax = _tmax;
      tzero = _tzero;
      assert(tmax>tmin);
      assert(tzero>=tmin);
      assert(tzero<tmax);
   }
   void print()
   {
      std::cout << "RunNo:"<<runNumber<<"\ttmin:" << tmin << "\ttmax:" << tmax << "\ttzero"<< tzero<< std::endl;
   }
   TimeWindow(const TimeWindow& m_TimeWindow)
   {
      runNumber = m_TimeWindow.runNumber;
      tmin = m_TimeWindow.tmin;
      tmax = m_TimeWindow.tmax;
      tzero = m_TimeWindow.tzero;
   }
   TimeWindow operator=(const TimeWindow m_TimeWindow)
   {
      this->runNumber = m_TimeWindow.runNumber;
      this->tmin = m_TimeWindow.tmin;
      this->tmax = m_TimeWindow.tmax;
      this->tzero = m_TimeWindow.tzero;

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
   feENVdataPlot()
   {
   }
   ~feENVdataPlot()
   {
   }
};

//Collection of feLabVIEW / feGEM data with the same name (the same source)
class feENVdata {
   public:
   //#public:
   std::string name;
   std::string title;
   int array_number;
   private:
   //One TGraph per run per window (size of Windows)
   std::vector<feENVdataPlot*> plots;
   public:
   std::string GetName() { return name + "[" + std::to_string(array_number) + "]";}
   std::string GetNameID() { return name + "_" + std::to_string(array_number);}
   std::string GetTitle() { return title; }
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
};

//Specialise the above for feGEM
class feGEMdata: public feENVdata
{
   public:
   template<typename T>
   void AddGEMEvent(TStoreGEMData<T>* GEMEvent,const std::vector<TimeWindow>& timewindows)
   {
      double time=GEMEvent->GetRunTime();
      //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
      //for (auto& window: timewindows)
      for (size_t i=0; i<timewindows.size(); i++)
      {
         auto& window = timewindows[i];
         //If inside the time window
         if ( (time > window.tmin && time < window.tmax) ||
         //Or if after tmin and tmax is invalid (-1)
            (time > window.tmin && window.tmax<0) )
         {
            feENVdataPlot* plot = GetPlot(i);
            plot->AddPoint(time, time - window.tzero, (double)GEMEvent->GetArrayEntry(array_number));
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
   void AddLVEvent(TStoreLabVIEWEvent* LVEvent,const std::vector<TimeWindow>& timewindows)
   {
      double time=LVEvent->GetRunTime();
      //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
      //for (auto& window: timewindows)
      for (size_t i=0; i<timewindows.size(); i++)
      {
         auto& window = timewindows[i];
         //If inside the time window
         if ( (time > window.tmin && time < window.tmax) ||
         //Or if after tmin and tmax is invalid (-1)
            (time > window.tmin && window.tmax<0) )
         {
            feENVdataPlot* plot = GetPlot(i);
            plot->AddPoint( time, time - window.tzero, LVEvent->GetArrayEntry(array_number));
         }
      }
      return;
   }
};


class TAPlot: public TObject
{

//Warning LMG changed to protected from private, now to public
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
   std::vector<TimeWindow> TimeWindows; //check dupes - fatal error
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

   std::vector<VertexEvent> VertexEvents;
   std::vector<int> Runs; //check dupes - ignore copies. AddRunNumber

   std::vector<feGEMdata> feGEM;
   std::vector<feLVdata> feLV;
   std::chrono::high_resolution_clock::time_point ObjectConstructionTime;
   std::chrono::high_resolution_clock::time_point DataLoadedTime;
  

public:
   //Use a time axis that counts from Zero of a dump window (default true)
   const bool ZeroTimeAxis;

   void SetTAPlotTitle(const std::string& _title)
   {
      title=_title;
   }
   std::string GetTAPlotTitle()
   {
      return title;
   }

   void SetGEMChannel(const std::string& name, int ArrayEntry, std::string title="")
   {
      for (auto& d: feGEM)
      {
         if (d.array_number!= ArrayEntry)
            continue;
         if (d.name!=name)
            continue;
         std::cout<<"GEM Channel "<< name.c_str()<< "["<<ArrayEntry<<"] already registered"<<std::endl;
      }
      feGEMdata new_entry;
      new_entry.name = name;
      if (title.size() == 0)
         new_entry.title = name;
      else
         new_entry.title = title;
      new_entry.array_number = ArrayEntry;
      
      feGEM.push_back(new_entry);
   }

   void SetGEMChannel(const std::string& Category, const std::string& Varname, int ArrayEntry, std::string title="")
   {
      //Perhaps this line should be a function used everywhere
      std::string name = feGEMdata::CombinedName(Category, Varname);
      return SetGEMChannel(name,ArrayEntry,title);
   }

   std::vector<std::pair<std::string,int>> GetGEMChannels()
   {
      std::vector<std::pair<std::string,int>> channels;
      for (auto& f: feGEM)
         channels.push_back({f.GetName(),f.array_number});
      return channels;
   }

   std::pair<TLegend*,TMultiGraph*> GetGEMGraphs()
   {
       TMultiGraph *feGEMmg = NULL;
      TLegend* legend = NULL;
      if (feLV.size()==0)
         return {legend,feGEMmg};
      feGEMmg = new TMultiGraph();
      legend = new TLegend(0.1,0.7,0.48,0.9);
      //For each unique variable being logged
      int ColourOffset = 0;
      for (auto& f: feGEM)
      {
         std::map<std::string,TGraph*> unique_labels;
         const std::vector<int> UniqueRuns = GetArrayOfRuns();
         for (size_t i=0; i< TimeWindows.size(); i++)
         {
            size_t ColourID=0;
            for ( ; ColourID< UniqueRuns.size(); ColourID++)
            {
               if (TimeWindows.at(i).runNumber == UniqueRuns.at(ColourID))
                  break;
            }
            TGraph* graph = f.BuildGraph(i,ZeroTimeAxis);
            graph->SetLineColor(GetColour(ColourID + ColourOffset));
            graph->SetMarkerColor(GetColour(ColourID + ColourOffset));
            unique_labels[f.GetName()] = graph;
            //if (i==0)
            //   legend->AddEntry(graph,f.GetName().c_str());
            if (graph->GetN())
               feGEMmg->Add(graph);
         }
         for (auto& a: unique_labels)
         {
            legend->AddEntry(a.second,a.first.c_str());
         }
         ColourOffset++;
      }
      return {legend,feGEMmg};
   }

   bool HaveGEMData()
   {
      return (feGEM.size() != 0);
   }

   void SetLVChannel(const std::string& name, int ArrayEntry, std::string title="")
   {
      for (auto& d: feLV)
      {
         if (d.array_number!= ArrayEntry)
            continue;
         if (d.name!=name)
            continue;
         std::cout<<"LV Channel "<< name.c_str()<< "["<<ArrayEntry<<"] already registered"<<std::endl;
      }
      feLVdata new_entry;
      new_entry.name = name;
      if (title.size() == 0)
         new_entry.title = name;
      else
         new_entry.title = title;
      new_entry.array_number = ArrayEntry;
      
      feLV.push_back(new_entry);
   }

   std::vector<std::pair<std::string,int>> GetLVChannels()
   {
      std::vector<std::pair<std::string,int>> channels;
      for (auto& f: feGEM)
         channels.push_back({f.GetName(),f.array_number});
      return channels;
   }

   std::pair<TLegend*,TMultiGraph*> GetLVGraphs()
   {
      TMultiGraph *feLVmg = NULL;
      TLegend* legend = NULL;
      if (feLV.size()==0)
         return {legend,feLVmg};
      feLVmg = new TMultiGraph();
      legend = new TLegend(0.1,0.7,0.48,0.9);
      //For each unique variable being logged
      int ColourOffset = 0;
      for (auto& f: feLV)
      {
         std::map<std::string,TGraph*> unique_labels;

         const std::vector<int> UniqueRuns = GetArrayOfRuns();
         for (size_t i=0; i< TimeWindows.size(); i++)
         {
            size_t ColourID=0;
            for ( ; ColourID< UniqueRuns.size(); ColourID++)
            {
               if (TimeWindows.at(i).runNumber == UniqueRuns.at(ColourID))
                  break;
            }
            TGraph* graph = f.BuildGraph(i,ZeroTimeAxis);
            graph->SetLineColor(GetColour(ColourID + ColourOffset));
            graph->SetMarkerColor(GetColour(ColourID + ColourOffset));
            unique_labels[f.GetName()] = graph;
            
            //if (i==0)
            //   legend->AddEntry(graph,f.GetName().c_str());
            //Add the graph only if there is data in it
            if (graph->GetN())
               feLVmg->Add(graph);
         }
         for (auto& a: unique_labels)
         {
            legend->AddEntry(a.second,a.first.c_str());
         }
         ColourOffset++;
      }
      return {legend,feLVmg};
   }
   
   bool HaveLVData()
   {
      return (feGEM.size() != 0);
   }

   template<typename T> void LoadfeGEMData(feGEMdata& f, TTreeReader* feGEMReader, const char* name, double first_time, double last_time);
   void LoadfeGEMData(int RunNumber, double first_time, double last_time);

   void LoadfeLVData(feLVdata& f, TTreeReader* feLVReader, const char* name, double first_time, double last_time);
   void LoadfeLVData(int RunNumber, double first_time, double last_time);

   double GetApproximateProcessingTime()
   {
      if ( DataLoadedTime == std::chrono::high_resolution_clock::from_time_t(0) )
         LoadingDataLoadingDone();
      std::chrono::duration<double> d = 
         std::chrono::duration_cast<std::chrono::seconds>( DataLoadedTime - ObjectConstructionTime );
      return d.count();
   }
   
   void LoadingDataLoadingDone()
   {
      DataLoadedTime = std::chrono::high_resolution_clock::now();
   }
   // default class member functions
   TAPlot(const TAPlot& m_TAPlot);
   TAPlot(bool zerotime = true);//, int MVAMode = 0);
   virtual ~TAPlot();
   void Print(Option_t *option="") const;
   int GetNBins() const { return Nbin; }
   void SetTimeFactor(double t) { tFactor=t; }
   double GetTimeFactor() const { return tFactor; }
   void AddVertexEvent(VertexEvent e)
   {
      VertexEvents.push_back(e);
   }
   const std::vector<TimeWindow> GetTimeWindows() { return TimeWindows; }
   const std::vector<VertexEvent> GetVertexEvents() { return VertexEvents; }
   void AddRunNumber(int runNumber)
   {
      for (auto& no: Runs)
      {
         if (no==runNumber)
         {
            return;
         }
      }
      Runs.push_back(runNumber);
   }

   void SetCutsOn() { fApplyCuts = kTRUE; }
   void SetCutsOff() { fApplyCuts = kFALSE; }
   bool GetCutsSettings() const { return fApplyCuts; }
   //Setters
   void SetBinNumber(int bin) { Nbin = bin; }
   void SetMVAMode(int Mode) { MVAMode = Mode; }
   
   size_t GetNVertexEvents() { return VertexEvents.size(); }
   int GetNVerticies()
   {
      return fTotalVert;
   }
   int GetNPassedType(const int type)
   {
      int n=0;
      for (auto& event: VertexEvents)
      {
         if (event.CutsResult&type)
            n++;
      }
      return n;
   }
   int GetNPassedCuts()
   {
      return GetNPassedType(1);
   }
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
   double GetMaxDumpLength() const { return MaxDumpLength; }
   double GetFirstTmin() const     { return FirstTmin;     }
   double GetLastTmax() const      { return LastTmax;      }
   double GetBiggestTzero() const  { return BiggestTzero;  }
 
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
   void FillHistogram(const char* keyname,double x, int counts)
   {
      if (HISTO_POSITION.count(keyname))
         ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Fill(x,counts);
   }
   void FillHistogram(const char* keyname, double x)
   {
      if (HISTO_POSITION.count(keyname))
         ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Fill(x);
   }
   void FillHistogram(const char* keyname, double x, double y)
   {
      if (HISTO_POSITION.count(keyname))
         ((TH2D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Fill(x,y);
   }
   TH1D* GetTH1D(const char* keyname)
   {
      if (HISTO_POSITION.count(keyname))
         return (TH1D*)HISTOS.At(HISTO_POSITION.at(keyname));
      return NULL;
   }
   void DrawHistogram(const char* keyname, const char* settings)
   {
      if (HISTO_POSITION.count(keyname))
         ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Draw(settings);
      else
         std::cout<<"Warning: Histogram"<< keyname << "not found"<<std::endl;
   }
   TLegend* DrawLines(TLegend* legend, const char* keyname)
   {
      double max = ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->GetMaximum();
      if (!legend)
         legend = new TLegend(1, 0.7, 0.55, .95); //, "NDC NB");
      for (UInt_t i = 0; i < Injections.size(); i++)
      {
         TLine *l = new TLine(Injections[i]*tFactor, 0., Injections[i]*tFactor, max );
         l->SetLineColor(6);
         l->Draw();
         if (i == 0)
            legend->AddEntry(l, "AD fill", "l");
      }
      for (UInt_t i = 0; i < Ejections.size(); i++)
      {
         TLine *l = new TLine(Ejections[i]*tFactor, 0., Ejections[i]*tFactor, max );
         l->SetLineColor(7);
         l->Draw();
         if (i == 0)
            legend->AddEntry(l, "Beam to ALPHA", "l");
      }
      for (UInt_t i = 0; i < DumpStarts.size(); i++)
      {
         if (DumpStarts.size() > 4) continue; //Don't draw dumps if there are lots
         TLine *l = new TLine(DumpStarts[i]*tFactor, 0., DumpStarts[i]*tFactor, max );
         //l->SetLineColor(7);
         l->SetLineColorAlpha(kGreen, 0.35);
         //l->SetFillColorAlpha(kGreen,0.35);
         l->Draw();
         if (i == 0)
            legend->AddEntry(l, "Dump Start", "l");
      }
      for (UInt_t i = 0; i < DumpStops.size(); i++)
      {
         if (DumpStops.size() > 4) continue; //Don't draw dumps if there are lots
         TLine *l = new TLine(DumpStops[i]*tFactor, 0., DumpStops[i]*tFactor, max );
         //l->SetLineColor(7);
         l->SetLineColorAlpha(kRed, 0.35);
         l->Draw();
         if (i == 0)
            legend->AddEntry(l, "Dump Stop", "l");
      }
      legend->Draw();
      return legend;
   }
   TLegend* AddLegendIntegral(TLegend* legend, const char* message,const char* keyname)
   {
      char line[201];
      if (!legend)
         legend = new TLegend(1, 0.7, 0.55, .95); //, "NDC NB");
      snprintf(line, 200, message, ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Integral());
      legend->AddEntry((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)), line, "f");
      legend->SetFillColor(kWhite);
      legend->SetFillStyle(1001);
      legend->Draw();
      return legend;
   }
   void AddStartDumpMarker(double t)
   {
      DumpStarts.push_back(t);
   }
   void AddStopDumpMarker(double t)
   {
      DumpStops.push_back(t);
   }
   void AddInjection(double t)
   {
      Injections.push_back(t);
   }
   void AddEjection(double t)
   {
      Ejections.push_back(t);
   }
   TString GetListOfRuns()
   {
      TString runs_string="";
      std::sort(Runs.begin(), Runs.end());
      for (size_t i = 0; i < Runs.size(); i++)
      {
         //std::cout <<"Run: "<<Runs[i] <<std::endl;
         if (i > 0)
            runs_string += ",";
         runs_string += Runs[i];
      }
      return runs_string;
   }
   const std::vector<int> GetArrayOfRuns() { return Runs; }
   void SetUpHistograms();
   void PrintTimeRanges()
   {
      for (auto& w: TimeWindows)
         w.print();
   }
   

   //virtual void FillHisto();
   TObjArray GetHisto() {return HISTOS;}
   std::map<std::string,int> GetHistoPosition() {return HISTO_POSITION;}
   void ClearHisto();
   //TCanvas *Canvas(TString Name = "cVTX");


   void SetVerbose(bool v) {fVerbose=v;}
  
   //virtual void ExportCSV(TString filename, Bool_t PassedCutOnly=kFALSE);

   virtual void PrintFull()
   {
      std::cout << "===========================" << std::endl;
      std::cout << "Printing TAPlot located at " << this << std::endl;
      std::cout << "===========================" << std::endl;
      std::cout << "Title is " << title << std::endl;

      std::cout << "MVAMode = " << MVAMode << std::endl;
      std::cout << "Nbin = " << Nbin << std::endl;
      std::cout << "DrawStyle = " << DrawStyle << std::endl;

      std::cout << "gLegendDetail = " << gLegendDetail << std::endl;
      std::cout << "fApplyCuts = " << fApplyCuts << std::endl;
      std::cout << "fClassifierCut = " << fClassifierCut << std::endl;

      std::cout << "FirstTmin = " << FirstTmin << std::endl;
      std::cout << "LastTmax = " << LastTmax << std::endl;
      std::cout << "BiggestTzero = " << BiggestTzero << std::endl;
      std::cout << "MaxDumpLength = " << MaxDumpLength << std::endl;


      std::cout << "Printing TimeWindows at " << &TimeWindows << std::endl;
      std::cout << "TimeWindows.size() = " << TimeWindows.size() << std::endl;
      std::cout << "First time window info:" << std::endl;
      std::cout << "TimeWindows[0].runNumber = " << TimeWindows.at(0).runNumber << std::endl;
      std::cout << "TimeWindows[0].tmax = " << TimeWindows.at(0).tmax << std::endl;
      std::cout << "TimeWindows[0].tmin = " << TimeWindows.at(0).tmin << std::endl;
      std::cout << "TimeWindows[0].tzero = " << TimeWindows.at(0).tzero << std::endl;

      std::cout << "fTotalTime = " << fTotalTime << std::endl;
      std::cout << "fTotalVert = " << fTotalVert << std::endl;


      std::cout << "Printing HISTOS at " << &HISTOS << std::endl;
      std::cout << "HISTOS.size() = " << HISTOS.GetSize() << std::endl;
      std::cout << "First HISTOS info:" << std::endl;
      std::cout << "HISTOS[0] = " << HISTOS.At(0) << std::endl;

      std::cout << "Printing HISTO_POSITION at " << &HISTO_POSITION << std::endl;
      std::cout << "HISTO_POSITION.size() = " << HISTO_POSITION.size() << std::endl;
      std::cout << "First HISTO_POSITION info:" << std::endl;
      std::cout << "HISTO_POSITION[0] string = " << HISTO_POSITION.begin()->first << std::endl;
      std::cout << "HISTO_POSITION[0] int = " << HISTO_POSITION.begin()->second << std::endl;
  
      std::cout << "Printing Ejections at " << &Ejections << std::endl;
      std::cout << "Ejections.size() = " << Ejections.size() << std::endl;
      std::cout << "First Ejections info:" << std::endl;
      if(Ejections.size() > 0)
      {
         std::cout << "Ejections[0] = " << Ejections.at(0) << std::endl;
      }
      else { std::cout << "Empty sorry, moving on to next member." << std::endl; }

      std::cout << "Printing Injections at " << &Injections << std::endl;
      std::cout << "Injections.size() = " << Injections.size() << std::endl;
      std::cout << "First Injections info:" << std::endl;
      if(Injections.size() > 0)
      {
         std::cout << "Injections[0] = " << Injections.at(0) << std::endl;
      }
      else { std::cout << "Empty sorry, moving on to next member." << std::endl; }

      std::cout << "Printing DumpStarts at " << &DumpStarts << std::endl;
      std::cout << "DumpStarts.size() = " << DumpStarts.size() << std::endl;
      std::cout << "First DumpStarts info:" << std::endl;
      if(DumpStarts.size() > 0)
      {
         std::cout << "DumpStarts[0] = " << DumpStarts.at(0) << std::endl;
      }
      else { std::cout << "Empty sorry, moving on to next member." << std::endl; }

      std::cout << "Printing DumpStops at " << &DumpStops << std::endl;
      std::cout << "DumpStops.size() = " << DumpStops.size() << std::endl;
      std::cout << "First DumpStops info:" << std::endl;
      if(DumpStops.size() > 0)
      {
         std::cout << "DumpStops[0] = " << DumpStops.at(0) << std::endl;
      }
      else { std::cout << "Empty sorry, moving on to next member." << std::endl; }

      std::cout << "Printing VertexEvents at " << &VertexEvents << std::endl;
      std::cout << "VertexEvents.size() = " << VertexEvents.size() << std::endl;
      std::cout << "First VertexEvents info:" << std::endl;
      if(VertexEvents.size() > 0)
      {
         std::cout << "VertexEvents[0].runNumber = " << VertexEvents.at(0).runNumber << std::endl;
         std::cout << "VertexEvents[0].x = " << VertexEvents.at(0).x << std::endl;
         std::cout << "VertexEvents[0].y = " << VertexEvents.at(0).y << std::endl;
         std::cout << "VertexEvents[0].z = " << VertexEvents.at(0).z << std::endl;
         std::cout << "VertexEvents[0].t = " << VertexEvents.at(0).t << std::endl;
      }
      else { std::cout << "Empty sorry, moving on to next member." << std::endl; }

      std::cout << "Printing Runs at " << &Runs << std::endl;
      std::cout << "Runs.size() = " << Runs.size() << std::endl;
      std::cout << "First Runs info:" << std::endl;
      if(Runs.size() > 0)
      {
         std::cout << "Runs[0] = " << Runs.at(0) << std::endl;
      }
      else { std::cout << "Empty sorry, moving on to next member." << std::endl; }
      //Leaving out for now, might add later.
      //std::vector<feGEMdata> feGEM;
      //std::vector<feLVdata> feLV;
      //std::chrono::high_resolution_clock::time_point ObjectConstructionTime;
      //std::chrono::high_resolution_clock::time_point DataLoadedTime;

      std::cout << "===========================" << std::endl;
      std::cout << std::endl << std::endl << std::endl;
   }

   TAPlot& operator=(const TAPlot& m_TAPlot)
   {
      std::cout << "TAPlot = operator" << std::endl;
      this->title = m_TAPlot.title ;
      this->MVAMode = m_TAPlot.MVAMode ;
      this->Nbin = m_TAPlot.Nbin ; 
      this->DrawStyle = m_TAPlot.DrawStyle ;
      this->gLegendDetail = m_TAPlot.gLegendDetail ; 
      this->fApplyCuts = m_TAPlot.fApplyCuts ;
      this->fClassifierCut = m_TAPlot.fClassifierCut ;
      this->FirstTmin = m_TAPlot.FirstTmin ;
      this->LastTmax = m_TAPlot.LastTmax ;
      this->BiggestTzero = m_TAPlot.BiggestTzero ;
      this->MaxDumpLength = m_TAPlot.MaxDumpLength ;
      
      this->fTotalTime = m_TAPlot.fTotalTime ;
      this->fTotalVert = m_TAPlot.fTotalVert ;
      this->fVerbose = m_TAPlot.fVerbose ;
      this->tFactor = m_TAPlot.tFactor ;

      for(int i=0;i<m_TAPlot.TimeWindows.size();i++)
         this->TimeWindows.push_back(m_TAPlot.TimeWindows.at(i));

      for(int i=0;i<m_TAPlot.Ejections.size();i++)
         this->Ejections.push_back(m_TAPlot.Ejections.at(i));
      
      for(int i=0;i<m_TAPlot.Injections.size();i++)
         this->Injections.push_back(m_TAPlot.Injections.at(i));
      
      for(int i=0;i<m_TAPlot.DumpStarts.size();i++)
         this->DumpStarts.push_back(m_TAPlot.DumpStarts.at(i));
      
      for(int i=0;i<m_TAPlot.DumpStops.size();i++)
         this->DumpStops.push_back(m_TAPlot.DumpStops.at(i));
      
      for(int i=0;i<m_TAPlot.VertexEvents.size();i++)
         this->VertexEvents.push_back(m_TAPlot.VertexEvents.at(i));
      
      for(int i=0;i<m_TAPlot.Runs.size();i++)
         this->Runs.push_back(m_TAPlot.Runs.at(i));
      
      for(int i=0;i<m_TAPlot.feGEM.size();i++)
         this->feGEM.push_back(m_TAPlot.feGEM.at(i));
      
      for(int i=0;i<m_TAPlot.feLV.size();i++)
         this->feLV.push_back(m_TAPlot.feLV.at(i));

      this->ObjectConstructionTime = m_TAPlot.ObjectConstructionTime ;
      this->DataLoadedTime = m_TAPlot.DataLoadedTime ;

      return *this;
   }

   virtual TAPlot& operator+=(const TAPlot &plotB) 
   {
      std::cout << "TAPlot += operator" << std::endl;

      //Vectors- need concating
      this->Ejections.insert(this->Ejections.end(), plotB.Ejections.begin(), plotB.Ejections.end() );
      std::cout << "How about here?" << std::endl;
      this->Injections.insert(this->Injections.end(), plotB.Injections.begin(), plotB.Injections.end() );
      this->DumpStarts.insert(this->DumpStarts.end(), plotB.DumpStarts.begin(), plotB.DumpStarts.end() );
      this->DumpStops.insert(this->DumpStops.end(), plotB.DumpStops.begin(), plotB.DumpStops.end() );
      this->TimeWindows.insert(this->TimeWindows.end(), plotB.TimeWindows.begin(), plotB.TimeWindows.end() );
      this->VertexEvents.insert(this->VertexEvents.end(), plotB.VertexEvents.begin(), plotB.VertexEvents.end() );
      this->Runs.insert(this->Runs.end(), plotB.Runs.begin(), plotB.Runs.end() );//check dupes - ignore copies. AddRunNumber
      this->feGEM.insert(this->feGEM.end(), plotB.feGEM.begin(), plotB.feGEM.end() );
      this->feLV.insert(this->feLV.end(), plotB.feLV.begin(), plotB.feLV.end() );

      //Strings
      this->title+= ", ";
      this->title+=plotB.title;

      //Unsure - Just keep as is, no need to be overwritten.
      //this->MVAMode                = plotB.MVAMode;
      //this->Nbin                   = plotB.Nbin;
      //this->DrawStyle              = plotB.DrawStyle;
      //this->gLegendDetail          = plotB.gLegendDetail;
      //this->fApplyCuts             = plotB.fApplyCuts;
      //Needs help - copy from A
      //this->fClassifierCut         = plotB.fClassifierCut;
      //this->fVerbose               = plotB.fVerbose;
   
      //All doubles
      this->FirstTmin              = (this->FirstTmin < plotB.FirstTmin)?this->FirstTmin:plotB.FirstTmin;
      this->LastTmax               = (this->LastTmax > plotB.LastTmax)?this->LastTmax:plotB.LastTmax;
      this->BiggestTzero           = (this->BiggestTzero > plotB.BiggestTzero)?this->BiggestTzero:plotB.BiggestTzero;
      this->MaxDumpLength          = (this->MaxDumpLength > plotB.MaxDumpLength)?this->MaxDumpLength:plotB.MaxDumpLength;
      this->fTotalTime             = (this->fTotalTime > plotB.fTotalTime)?this->fTotalTime:plotB.fTotalTime;
      this->ObjectConstructionTime = (this->ObjectConstructionTime < plotB.ObjectConstructionTime)?this->ObjectConstructionTime:plotB.ObjectConstructionTime;
      this->DataLoadedTime         = (this->DataLoadedTime > plotB.DataLoadedTime)?this->DataLoadedTime:plotB.DataLoadedTime;
      this->tFactor                = (this->tFactor < plotB.tFactor)?this->tFactor:plotB.tFactor;


      //Total vertices?
      this->fTotalVert             += plotB.fTotalVert;

      //Need a new addition overload- no for loop to 
      //this->HISTOS                 = plotB.HISTOS + plotb.HISTOS;
      std::cout << "How about down here?" << std::endl;
      for(int i = 0; i < plotB.HISTOS.GetSize(); i++)
      {
         this->HISTOS.Add(plotB.HISTOS.At(i));
      }
      std::cout << "And after this?" << std::endl;
      
      this->HISTO_POSITION.insert( plotB.HISTO_POSITION.begin(), plotB.HISTO_POSITION.end() );

      return *this;
   }

   friend TAPlot operator+(const TAPlot& plotA, const TAPlot& plotB)
   {
      std::cout << "TAPlot addition operator" << std::endl;
      TAPlot outputplot(plotA); //Create new from copy
      //outputplot = this; //?? Maybe we want to copy this way

      //Vectors- need concating
      std::cout << "Are we getting here ok?" << std::endl;
      outputplot.Ejections.insert(outputplot.Ejections.end(), plotB.Ejections.begin(), plotB.Ejections.end() );
      std::cout << "How about here?" << std::endl;
      outputplot.Injections.insert(outputplot.Injections.end(), plotB.Injections.begin(), plotB.Injections.end() );
      outputplot.DumpStarts.insert(outputplot.DumpStarts.end(), plotB.DumpStarts.begin(), plotB.DumpStarts.end() );
      outputplot.DumpStops.insert(outputplot.DumpStops.end(), plotB.DumpStops.begin(), plotB.DumpStops.end() );
      outputplot.TimeWindows.insert(outputplot.TimeWindows.end(), plotB.TimeWindows.begin(), plotB.TimeWindows.end() );
      outputplot.VertexEvents.insert(outputplot.VertexEvents.end(), plotB.VertexEvents.begin(), plotB.VertexEvents.end() );
      outputplot.Runs.insert(outputplot.Runs.end(), plotB.Runs.begin(), plotB.Runs.end() );//check dupes - ignore copies. AddRunNumber
      outputplot.feGEM.insert(outputplot.feGEM.end(), plotB.feGEM.begin(), plotB.feGEM.end() );
      outputplot.feLV.insert(outputplot.feLV.end(), plotB.feLV.begin(), plotB.feLV.end() );

      //Strings
      outputplot.title+= ", ";
      outputplot.title+=plotB.title;

      //Unsure - Just keep as is, no need to be overwritten.
      //outputplot.MVAMode                = plotB.MVAMode;
      //outputplot.Nbin                   = plotB.Nbin;
      //outputplot.DrawStyle              = plotB.DrawStyle;
      //outputplot.gLegendDetail          = plotB.gLegendDetail;
      //outputplot.fApplyCuts             = plotB.fApplyCuts;
      //Needs help - copy from A
      //outputplot.fClassifierCut         = plotB.fClassifierCut;
      //outputplot.fVerbose               = plotB.fVerbose;
   
      //All doubles
      outputplot.FirstTmin              = (outputplot.FirstTmin < plotB.FirstTmin)?outputplot.FirstTmin:plotB.FirstTmin;
      outputplot.LastTmax               = (outputplot.LastTmax > plotB.LastTmax)?outputplot.LastTmax:plotB.LastTmax;
      outputplot.BiggestTzero           = (outputplot.BiggestTzero > plotB.BiggestTzero)?outputplot.BiggestTzero:plotB.BiggestTzero;
      outputplot.MaxDumpLength          = (outputplot.MaxDumpLength > plotB.MaxDumpLength)?outputplot.MaxDumpLength:plotB.MaxDumpLength;
      outputplot.fTotalTime             = (outputplot.fTotalTime > plotB.fTotalTime)?outputplot.fTotalTime:plotB.fTotalTime;
      outputplot.ObjectConstructionTime = (outputplot.ObjectConstructionTime < plotB.ObjectConstructionTime)?outputplot.ObjectConstructionTime:plotB.ObjectConstructionTime;
      outputplot.DataLoadedTime         = (outputplot.DataLoadedTime > plotB.DataLoadedTime)?outputplot.DataLoadedTime:plotB.DataLoadedTime;
      outputplot.tFactor                = (outputplot.tFactor < plotB.tFactor)?outputplot.tFactor:plotB.tFactor;


      //Total vertices?
      outputplot.fTotalVert             += plotB.fTotalVert;

      //Need a new addition overload- no for loop to 
      //outputplot.HISTOS                 = plotB.HISTOS + plotb.HISTOS;
      std::cout << "How about down here?" << std::endl;
      for(int i = 0; i < plotB.HISTOS.GetSize(); i++)
      {
         outputplot.HISTOS.Add(plotB.HISTOS.At(i));
      }
      std::cout << "And after this?" << std::endl;
      
      outputplot.HISTO_POSITION.insert( plotB.HISTO_POSITION.begin(), plotB.HISTO_POSITION.end() );

      return outputplot;

      //Updated this whole lot:

   }

   //Lets try again with this lot.
   /*const TAPlot operator+(const TAPlot &other) const 
   {
      TAPlot result(*this);     // Make a copy of myself.  Same as MyClass result(*this);
      result += other;            // Use += to add other to the copy.
      return result;              // All done!
   }*/

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

