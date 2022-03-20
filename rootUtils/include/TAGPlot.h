
#if BUILD_AG
#ifndef __TAGPlot__
#define __TAGPlot__

#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStoreEvent.hh"
#include "TChronoChannel.h"
#include "store_cb.h"
#include "TStoreHelix.hh"

#include "TSpacePoint.hh"
#include "TFitHelix.hh"

#include "TAGDetectorEvent.hh"

#include "TLatex.h"
#include "TLegend.h"
#include "TPaveText.h"

#include "TLine.h"
#include <fstream>

#include "TAPlot.h"
#include "TAGPlotChronoEvents.h"
#include "TAGPlotHelixEvents.h"
#include "TAGPlotSpacePointEvent.h"

#include "TStyle.h"
#include "IntGetters.h" // ApplyCuts



class TAGPlot : public TAPlot
{
   protected:
     std::vector<TChronoChannel> fChronoChannels;
private:

  Int_t BarMultiplicityCut;
  
  //Detector Chrono channels mapped to run number:
  std::map<int, TChronoChannel> top;
  std::map<int, TChronoChannel> bottom;
  std::map<int, TChronoChannel> sipmad;
  std::map<int, TChronoChannel> sipmcf;
  std::map<int, TChronoChannel> TPC_TRIG;

   //Dump marker Chronobox channels mapped to run number:
   std::map<int, TChronoChannel> fCATStart;
   std::map<int, TChronoChannel> fCATStop;
   std::map<int, TChronoChannel> fRCTStart;
   std::map<int, TChronoChannel> fRCTStop;
   std::map<int, TChronoChannel> fATMStart;
   std::map<int, TChronoChannel> fATMStop;

   //Beam injection/ ejection markers mapped to run number:
   std::map<int, TChronoChannel> fBeamInjection;
   std::map<int, TChronoChannel> fBeamEjection;

public:
   TAGPlotChronoPlotEvents fChronoEvents;

   // default class member functions
   TAGPlot(bool zeroTime = true);
   TAGPlot(double zMin, double zMax,bool zeroTime = true);
   TAGPlot(double zMin, double zMax, int barCut, bool zeroTime = true);
   TAGPlot(const TAPlot& object);
   TAGPlot(const TAGPlot& object);

   virtual ~TAGPlot();
   void Reset();
   TAGPlot& operator=(const TAGPlot& rhs);
   TAGPlot& operator+=(const TAGPlot& rhs);
   friend TAGPlot& operator+(const TAGPlot& lhs, const TAGPlot& rhs);

   //Setters and getters
   void SetChronoChannels(Int_t runNumber);

   void AddAGDetectorEvent(const TAGDetectorEvent& event);
   void AddChronoEvent(const TCbFIFOEvent& event, const std::string& board);
protected:
   void AddEvent(const TAGDetectorEvent& event, const double timeOffset = 0);
   void AddEvent(const TCbFIFOEvent& event, const TChronoChannel& channel, const double timeOffset = 0);
public:
   void AddDumpGates(const int runNumber, const std::vector<std::string> description, std::vector<int> dumpIndex );
   void AddDumpGates(const int runNumber, const std::vector<TAGSpill> spills );
   //If spills are from one run, it is faster to call the function above
   void AddDumpGates(const std::vector<TAGSpill> spills );

   void LoadRun(const int runNumber, const double tmin, const double tmax);
protected:
   void LoadChronoEvents(const int runNumber, const double firstTime, const double lastTime);
public:
   void SetUpHistograms();

   void FillHisto(bool applyCuts=true, int mode=0);
protected:
   void FillChronoHistograms();
   void FillVertexHistograms(bool applyCuts,int mode);
public:
   TCanvas *DrawVertexCanvas(const char* name = "cVTX",bool applyCuts=false, int mode=0);

   void SetBarMultiplicityCut(Int_t cut) { BarMultiplicityCut=cut; }
   Int_t GetBarMultiplicityCut()         { return BarMultiplicityCut; }
public:
      void ExportCSV(const std::string filename, Bool_t PassedCutOnly=kFALSE);
   

  ClassDef(TAGPlot, 1)
};

#endif
#endif
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
