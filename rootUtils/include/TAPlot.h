
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
#include "TColor.h"
#include "TMultiGraph.h"
   
#define SCALECUT 0.6

class AlphaColourWheel
{
   private:
      std::vector<int> colour_list{
         kRed+2, kMagenta+3, kBlue +1, kCyan +1, kGreen + 3, kYellow + 3,
         kOrange, kPink - 3, kViolet -2, kAzure - 3, kTeal -6, kSpring +8
      };
      int position;
   public:
      AlphaColourWheel() { position = 0;}
      EColor GetNewColour()
      {
         ++position;
         if ( position > colour_list.size())
            position=0;
         return (EColor)colour_list[position];
      }
      EColor GetCurrentColour()
      {
         return (EColor)colour_list[position];
      }
};


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




struct TimeWindow
{
   int runNumber;
   double tmin;
   double tmax;
   void print()
   {
      std::cout << "RunNo:"<<runNumber<<"\ttmin:" << tmin << "\ttmax:" << tmax << std::endl;
   }
};

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

class feENVdata {
   public:
   //#public:
   std::string name;
   std::string title;
   int array_number;
   //One TGraph per run per window (size of Windows)
   std::vector<feENVdataPlot*> plots;

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
   feENVdataPlot* GetPlot(int i)
   {
      while (i>=plots.size())
      {
         plots.push_back(new feENVdataPlot());
      }
      return plots.at(i);
   }
};

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
            plot->AddPoint(time - window.tmin, time, (double)GEMEvent->GetArrayEntry(array_number));
         }
      }
      return;
   }
};


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
            plot->AddPoint( time - window.tmin, time,LVEvent->GetArrayEntry(array_number));
         }
      }
      return;
   }
};


class TAPlot: public TObject
{

private:
   int MVAMode; //Default 0;
   int Nbin; // 100;
   int DrawStyle; //Switch between colour modes

   int gLegendDetail; // = 1;
   Bool_t fApplyCuts;
   double fClassifierCut;

   double FirstTmin;
   double LastTmax;
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

   int CATStart;
   int CATStop;
   int RCTStart;
   int RCTStop;
   int ATMStart;
   int ATMStop;
   std::vector<double> DumpStarts;
   std::vector<double> DumpStops;

   std::vector<VertexEvent> VertexEvents;
   std::vector<int> Runs;

public:
   std::vector<feGEMdata> feGEM;
   std::vector<feLVdata> feLV;
   TMultiGraph *feGEMmg;
   TMultiGraph *feLVmg;
   
   static std::string CombinedName(const std::string& Category, const std::string& Varname)
   {
      return std::string(Category + "\\" + Varname);
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
      std::string name = CombinedName(Category, Varname);
      return SetGEMChannel(name,ArrayEntry,title);
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

   template<typename T> void LoadfeGEMData(feGEMdata& f, TTreeReader* feGEMReader, const char* name, double first_time, double last_time);
   void LoadfeGEMData(int RunNumber, double first_time, double last_time);

   void LoadfeLVData(feLVdata& f, TTreeReader* feLVReader, const char* name, double first_time, double last_time);
   void LoadfeLVData(int RunNumber, double first_time, double last_time);

   // default class member functions
   TAPlot();//, int MVAMode = 0);
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

   void AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax);
   void AddTimeGate(const int runNumber, const double tmin, const double tmax);
   virtual void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition ) =0;
   //?virtual void AddDumpGates(int runNumber, std::vector<TA2Spill*> spills ) =0;
   //If spills are from one run, it is faster to call the function above
   //?virtual void AddDumpGates(std::vector<TA2Spill*> spills ) =0;
   virtual void LoadRun(int runNumber, double first_time, double last_time) {};
   void LoadData();

   void AutoTimeRange();
   int GetMaxDumpLength() const { return MaxDumpLength; }
   int GetFirstTmin() const     { return FirstTmin;     }
   int GetLastTmax() const      { return LastTmax;      }
 
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

