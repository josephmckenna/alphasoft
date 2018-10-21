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

#include "TLatex.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TLine.h"
#include <fstream>

#include "TStyle.h"

#include "RootUtils/RootUtils.h"

#define VERTEX_HISTO_Z 0
#define VERTEX_HISTO_R 1
#define VERTEX_HISTO_XY 2
#define VERTEX_HISTO_ZR 3
#define VERTEX_HISTO_ZT 4
#define VERTEX_HISTO_T 5
#define VERTEX_HISTO_IO32 6
#define VERTEX_HISTO_IO32_NOTBUSY 7
#define VERTEX_HISTO_ATOM_OR 8
#define VERTEX_HISTO_PHI 9
#define VERTEX_HISTO_ZPHI 10
#define VERTEX_HISTO_RDENS 11
#define VERTEX_HISTO_TMVA 14 // this must always be the last in the list
#define VERTEX_HISTO_VF48 12
#define VERTEX_HISTO_TPHI 13


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
};

struct ChronoPlotEvent {
   Int_t runNumber; // I don't get set yet...
   //Int_t Clock;
   Double_t t;
   Double_t RunTime;
   Int_t Counts;
   Int_t Chrono_Channel;
   Int_t Chrono_Board;
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
  
  
  Double_t TMin;
  Double_t TMax;
  
  
  
  //Hold historams in a vector so that saved TAGPlot objects can be 
  //backwards and forwards compatable
  //std::vector<TH1D*> TH1D_HISTO;
  //std::vector<TH2D*> TH2D_HISTO;
  TObjArray HISTOS;
  std::map<TString,int> HISTO_POSITION;
  
  //Detector SIS channels
  Int_t trig;
  Int_t trig_nobusy;
  Int_t atom_or;
  
  //Beam injection/ ejection markers:
  Int_t Beam_Injection;
  Int_t Beam_Ejection;
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
  std::vector<Int_t> SISChannels;
  
  std::vector<Int_t> Runs;

  Float_t gTMVAVarFloat[100];
  Int_t gTMVAVarInt[100];
  //TMVA::Reader *reader;

  //Double_t gZcutMax;
  //Double_t gZcutMin;
  //Double_t grfcut;
  std::vector<VertexEvent> VertexEvents;
  std::vector<ChronoPlotEvent> ChronoPlotEvents;

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

  void AddChronoEvent(TChrono_Event *event, Double_t StartOffset);

  void AddEvents(Int_t runNumber, char *description, Int_t repetition = 1, Double_t Toffset = 0, Bool_t zeroTime = kTRUE);
  void AddEvents(Int_t runNumber, Double_t tmin, Double_t tmax, Double_t Toffset = 0., Bool_t zeroTime = kTRUE);

  void SetChronoChannels(Int_t runNumber);
  void AutoTimeRange();
  void SetTimeRange(Double_t tmin_, Double_t tmax_);
  Double_t GetTimeRangeMax() {return TMax; }
  Double_t GetTimeRangeMin() {return TMin; }
  
  Double_t GetSilEventMaxT();
  Double_t GetSilEventMinT();
  Double_t GetChronoPlotEventMaxT();
  Double_t GetChronoPlotEventMinT();
  
  void SetUpHistograms();
  void PrintTimeRange() { std::cout << "Tmin: " << TMin << " Tmax: " << TMax << std::endl; }


  void FillHisto();
  TObjArray GetHisto();
  void ClearHisto();
  TCanvas *Canvas(TString Name = "cVTX");

  // default class member functions
  TAGPlot(Bool_t ApplyCuts = kTRUE, Int_t MVAMode = 0);
  
  void ExportCSV(TString filename, Bool_t PassedCutOnly=kFALSE);
  virtual ~TAGPlot();
  using TObject::Print;
  virtual void Print();
  using TObject::Draw;
  virtual void Draw(Option_t *option="");

  ClassDef(TAGPlot, 1)
};

#endif
