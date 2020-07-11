
#ifndef _TALPHAPLOT_
#define _TALPHAPLOT_


#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TCanvas.h"
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
struct TimeWindow {
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

   void AddVertexEvent(VertexEvent e)
   {
      VertexEvents.push_back(e);
   }
   const std::vector<TimeWindow> GetTimeWindows() { return TimeWindows; }
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
 
  //void SetTimeRange(double tmin_, double tmax_);


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

