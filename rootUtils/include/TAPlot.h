
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
struct VertexEvent
{
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
      assert(tzero>tmin);
      assert(tzero<tmax);
   }
   void print()
   {
      std::cout << "RunNo:"<<runNumber<<"\ttmin:" << tmin << "\ttmax:" << tmax << "\ttzero"<< tzero<< std::endl;
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

private:
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
   std::vector<TimeWindow> TimeWindows;
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
   std::vector<int> Runs;

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
      if ( DataLoadedTime.time_since_epoch().Count() == 0 )
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
   virtual void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition ) =0;
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

