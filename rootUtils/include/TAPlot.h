
#ifndef _TALPHAPLOT_
#define _TALPHAPLOT_


#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TCanvas.h"


#define SCALECUT 0.6
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
   // default class member functions
   TAPlot();//, int MVAMode = 0);
   virtual ~TAPlot();
   void Print();
   const int GetNBins() { return Nbin; }
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
   virtual void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition ) =0;
   
   virtual void LoadRun(int runNumber) {};
   void LoadData();

   void AutoTimeRange();
   const int GetMaxDumpLength() { return MaxDumpLength; }
   const int GetFirstTmin()     { return FirstTmin;     }
   const int GetLastTmax()      { return LastTmax;      }
 
  //void SetTimeRange(double tmin_, double tmax_);
   template <class T>
   void AddHistogram(const char* keyname, T* h)
   {
      HISTOS.Add(h);
      HISTO_POSITION[keyname]=HISTOS.GetEntries()-1;
   }
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

