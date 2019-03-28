#ifndef __TAGPlot__
#define __TAGPlot__

#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStoreEvent.hh"
#include "TChrono_Event.h"
#include "TStoreHelix.hh"

#include "TSpacePoint.hh"
#include "TFitHelix.hh"

#include "TLatex.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TLine.h"
#include <fstream>

#include "TStyle.h"
#include "chrono_module.h"
#include "RootUtils/RootUtils.h"


//using namespace TRootUtils;
//extern Bool_t TRootUtils::gApplyCuts;


//extern Bool_t gApplyCuts;
//extern Double_t grfcut;

struct VertexEvent {
        Int_t runNumber; // I don't get set yet...
        Int_t EventNo;
        Int_t CutsResult;
        Int_t VertexStatus;
        //TVector3* vtx;
        Double_t x;
        Double_t y;
        Double_t z;
        Double_t t; //Plot time (based off offical time)
        Double_t EventTime; //TPC time stamp
        Double_t RunTime; //Official Time

        Int_t nHelices; // helices used for vertexing
        Int_t NBars;
        Int_t nTracks; // reconstructed (good) helices
};

struct HelixEvent {
  Double_t pT;
  Double_t pZ;
  Double_t pTot;

  Double_t parD;
  Double_t Curvature;
  
  Int_t nPoints;
};

struct SpacePointEvent{
  Double_t x, y, z, r, p;
  // p = phi in degrees
};

struct ChronoPlotEvent {
   Int_t runNumber; // I don't get set yet...
   //Int_t Clock;
   Double_t t;
   Double_t RunTime;
   Double_t OfficialTime;
   Int_t Counts;
   ChronoChannel Chrono_Channel;
};

class TAGPlot : public TObject
{
private:
  Int_t MVAMode; //Default 0;
  Int_t Nbin; // 100;
  Int_t DrawStyle; //Switch between colour modes
  
  Int_t gLegendDetail; // = 1;
  Bool_t gApplyCuts;
  Double_t grfcut;
  Int_t BarMultiplicityCut;
  
  Double_t TMin;
  Double_t TMax;
  
  Double_t fTotalTime;
  Int_t fTotalVert;
  
  bool fVerbose;
  
  //Hold historams in a vector so that saved TAGPlot objects can be 
  //backwards and forwards compatable
  //std::vector<TH1D*> TH1D_HISTO;
  //std::vector<TH2D*> TH2D_HISTO;
  TObjArray HISTOS;
  std::map<std::string,int> HISTO_POSITION;
  
  //Detector Chrono channels
  ChronoChannel top;
  ChronoChannel bottom;
  ChronoChannel sipmad;
  ChronoChannel sipmcf;
  ChronoChannel TPC_TRIG;
    
  //Beam injection/ ejection markers:
  ChronoChannel Beam_Injection;
  ChronoChannel Beam_Ejection;
  std::vector<Double_t> Ejections;
  std::vector<Double_t> Injections;
  
  //Dump marks:
  Int_t CATStart;
  Int_t CATStop;
  Int_t RCTStart;
  Int_t RCTStop;
  Int_t ATMStart;
  Int_t ATMStop;
  std::vector<Double_t> DumpStarts;
  std::vector<Double_t> DumpStops;
  
  //List of all SIS channels above,used to loop over them
  //(Set in SetSISChannel(runNumber))
  std::vector<ChronoChannel> ChronoChannels;
  
  std::vector<Int_t> Runs;

  Float_t gTMVAVarFloat[100];
  Int_t gTMVAVarInt[100];
  //TMVA::Reader *reader;

  //Double_t gZcutMax;
  //Double_t gZcutMin;
  //Double_t grfcut;
  std::vector<VertexEvent> VertexEvents;
  std::vector<ChronoPlotEvent> ChronoPlotEvents;

  std::vector<HelixEvent> HelixEvents;
  std::vector<HelixEvent> UsedHelixEvents;
  std::vector<SpacePointEvent> SpacePointHelixEvents;
  std::vector<SpacePointEvent> SpacePointUsedHelixEvents;
  bool fPlotTracks;

public:


  
  
  void SetCutsOn() { gApplyCuts = kTRUE; }
  void SetCutsOff() { gApplyCuts = kFALSE; }
  //Setters
  void SetBinNumber(Int_t bin) { Nbin = bin; }
  void SetMVAMode(Int_t Mode) { MVAMode = Mode; }

  UInt_t GetVertexEventEntries() { return VertexEvents.size(); }
  UInt_t GetChronoPlotEventEntreis() { return ChronoPlotEvents.size(); }

  void AddStoreEvent(TStoreEvent *event, Double_t OfficialTimeStamp, Double_t StartOffset = 0.);

  void AddToTAGPlot(TAGPlot *ialphaplot);
  void AddToTAGPlot(TString file="plot.root");
  TAGPlot* LoadTAGPlot(TString file="plot.root");

  void AddChronoEvent(TChrono_Event *event, double official_time, Double_t StartOffset);

  Int_t AddEvents(Int_t runNumber, char *description, Int_t repetition = 1, Double_t Toffset = 0, Bool_t zeroTime = kTRUE);
  Int_t AddEvents(Int_t runNumber, Double_t tmin, Double_t tmax, Double_t Toffset = 0., Bool_t zeroTime = kTRUE);

  void SetChronoChannels(Int_t runNumber);
  void AutoTimeRange();
  void SetTimeRange(Double_t tmin_, Double_t tmax_);
  Double_t GetTimeRangeMax() {return TMax; }
  Double_t GetTimeRangeMin() {return TMin; }
  
  Double_t GetSilEventMaxT();
  Double_t GetSilEventMinT();
  Double_t GetChronoPlotEventMaxT();
  Double_t GetChronoPlotEventMinT();
  
  void SetBarMultiplicityCut(Int_t cut) { BarMultiplicityCut=cut; }
  Int_t GetBarMultiplicityCut()         { return BarMultiplicityCut; }
  
  
  void SetUpHistograms();
  void PrintTimeRange() { std::cout << "Tmin: " << TMin << " Tmax: " << TMax << std::endl; }

  void SetPlotTracks() {fPlotTracks=true;}

  Double_t GetTotalTime() const { return fTotalTime; }
  Int_t GetTotalVertices() const { return fTotalVert; }

  void FillHisto();
  TObjArray GetHisto() {return HISTOS;}
  std::map<std::string,int> GetHistoPosition() {return HISTO_POSITION;}
  void ClearHisto();
  TCanvas *Canvas(TString Name = "cVTX");

  void ProcessHelices(const TObjArray*);
  void ProcessUsedHelices(const TObjArray*);
  void SetupTrackHistos();
  void FillTrackHisto();
  TCanvas* DrawTrackHisto(TString Name);

  // default class member functions
  TAGPlot(Bool_t ApplyCuts = kTRUE, Int_t MVAMode = 0);

  void SetVerbose(bool v) {fVerbose=v;}
  
  void ExportCSV(TString filename, Bool_t PassedCutOnly=kFALSE);
  virtual ~TAGPlot();
  using TObject::Print;
  virtual void Print();
  using TObject::Draw;
  virtual void Draw(Option_t *option="");

  ClassDef(TAGPlot, 1)
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
