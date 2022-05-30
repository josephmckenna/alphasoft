#include "AgFlow.h"
#include "RecoFlow.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TGraph.h"

#include "TBarEvent.hh"
#include <set>
#include <iostream>

class BscHistoFlags
{
public:
   bool fPrint=false;
   bool fPulser=false;
   bool fProtoTOF = false;
   bool fBscDiag = false;
   bool fWriteOffsetFile = false;
   bool fRecOff = false;

public:
   BscHistoFlags() // ctor
   { }

   ~BscHistoFlags() // dtor
   { }
};

class BscHistoModule: public TARunObject
{
public:
   BscHistoFlags* fFlags = NULL;
   int fCounter;

private:

   bool diagnostics;
   double c = 2.99792e-1; // m/ns
   double refrac = 1.93; // From protoTOF tests with time walk correction applied
   double factor = c/refrac * 0.5;
   int pulser_reference_chan = 40;

   // Container declaration
   int BVTdcMap[64][7];
   int protoTOFTdcMap[16][4];

   // TDC constants
   const double epoch_freq = 97656.25; // 200MHz/(2<<11); KO+Thomas approved right frequency
   const double coarse_freq = 200.0e6; // 200MHz
      // linear calibration:
      // $ROOTANASYS/libAnalyzer/TRB3Decoder.hxx
   const double trb3LinearLowEnd = 17.0;
   const double trb3LinearHighEnd = 450.0;

   // ADC
   TH1D* hAdcOccupancy;
   TH2D* hAdcCorrelation;
   TH1D* hAdcTime;
   TH2D* hAdcTime2d;
   TH1D* hAdcMultiplicity;
   TH1D* hAdcAmp;
   TH2D* hAdcAmp2d;
   TH1D* hAdcFitAmp;
   TH2D* hAdcFitAmp2d;
   TH2D* hAdcFitting;

   // TDC
   TH1D* hAdcTdcOccupancy;
   TH1D* hTdcOccupancy;
   TH1D* hTdcCoincidence;
   TH2D* hTdcCorrelation;
   TH1D* hTdcMultiplicity;
   TH1D* hTdcSingleChannelMultiplicity;
   TH2D* hTdcSingleChannelMultiplicity2d;
   TH1D* hTdcSingleChannelHitTime;
   TH1D* hTdcSingleChannelHitTime100;
   TH1D* hTdcSingleChannelHitTime10000;
   TH1D* hTdcSingleChannelHitTime1000000;
   TH2D* hTdcSingleChannelHitTime2d;
   TH1D* hTdcTimeVsCh0;
   TH2D* hTdcTimeVsCh02d;
   TH1D* hFineTimeCounter;
   TH1D* hFineTime;
   TH2D* hFineTimeCounter2d;
   TH2D* hFineTime2d;

   // Bars
   TH1D* hBarMultiplicity;
   TH1D* hBarOccupancy;
   TH1D* hTopTime;
   TH1D* hBotTime;
   TH2D* hBarCorrelation;
   TH1D* hTopBotDiff;
   TH1D* hTopBotDiff100;
   TH1D* hTopBotDiff10000;
   TH1D* hTopBotDiff1000000;
   TH2D* hTopBotDiff2d;
   TH1D* hZed;
   TH2D* hZed2d;
   TH1D* hTwoBarTOF;
   TH2D* hTwoBarTOF2d;
   TH1D* hNBarTOF;
   TH2D* hNBarTOF2d;
   TH1D* hTwoBarDPhi;
   TH1D* hNBarDPhi;
   TH1D* hTwoBarDZed;
   TH1D* hNBarDZed;
   TH2D* hTwoBarDPhiDZed;
   TH2D* hNBarDPhiDZed;
   TH1D* hTwoBarExpectedTOF;
   TH2D* hTwoBarExpectedTOFvsTOF;
   TH1D* hNBarExpectedTOF;
   TH2D* hNBarExpectedTOFvsTOF;
   TH1D* hTwoBarExpectedTOFminusTOF;
   TH1D* hNBarExpectedTOFminusTOF;

   // Matching
   TH1D* hTPCMatched;
   TH1D* hMatchingDZ;
   TH2D* hMatchingDZbyBar;
   TH2D* hMatchingDZbyZed;
   TH1D* hMatchingDPhi;
   TH2D* hMatchingDPhibyBar;
   TH1D* hMatchingD;
   TH2D* hMatchingDbyBar;
   TH1D* hMatchedZ;
   TH1D* hMatchedZTPC;
   std::vector<TH2D*> hAmpVsZed;
   std::vector<TH2D*> hLnAmpVsZed;



public:
   BscHistoModule(TARunInfo* runinfo, BscHistoFlags* f)
      :  TARunObject(runinfo), fFlags(f), fCounter(0)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="Bsc Histo Module";
#endif
      diagnostics=f->fBscDiag;
   }

   ~BscHistoModule(){}

   void BeginRun(TARunInfo* runinfo)
   {
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      if(!diagnostics) return;
      printf("BscHistoModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      if( !gDirectory->cd("bsc_histo_module") )
         gDirectory->mkdir("bsc_histo_module")->cd();

      // Loads BV and protoTOF maps
      TString protoTOFmapfile=getenv("AGRELEASE");
      protoTOFmapfile+="/ana/bscint/protoTOF.map";
      TString BVmapfile=getenv("AGRELEASE");
      BVmapfile+="/ana/bscint/BV.map";
      std::ifstream fBVMap(BVmapfile.Data());
      std::ifstream fProtoTOFMap(protoTOFmapfile.Data());
      if(fBVMap) {
         std::string comment; getline(fBVMap, comment);
         for(int i=0; i<64; i++) {
            fBVMap >> BVTdcMap[i][0] >> BVTdcMap[i][1] >> BVTdcMap[i][2] >> BVTdcMap[i][3] >> BVTdcMap[i][4] >> BVTdcMap[i][5] >> BVTdcMap[i][6];
         }
         fBVMap.close();
      }
      if(fProtoTOFMap) {
         std::string comment; getline(fProtoTOFMap, comment);
         for(int i=0; i<16; i++) {
            fProtoTOFMap >> protoTOFTdcMap[i][0] >> protoTOFTdcMap[i][1] >> protoTOFTdcMap[i][2] >> protoTOFTdcMap[i][3];
         }
         fProtoTOFMap.close();
      }

      if (fFlags->fProtoTOF) { // two bar prototype

         // ADC
         gDirectory->mkdir("adc_histos")->cd();
         hAdcOccupancy = new TH1D("hAdcOccupancy","ADC channel occupancy;Channel number",16,-0.5,15.5);
         hAdcCorrelation = new TH2D("hAdcCorrelation","ADC channel correlation;Channel number;Channel number",16,-0.5,15.5,16,-0.5,15.5);
         hAdcMultiplicity = new TH1D("hAdcMultiplicity","ADC channel multiplicity;Number of ADC channels hit",17,-0.5,16.5);
         hAdcAmp = new TH1D("hAdcAmp","ADC pulse amplitude;Amplitude (volts)",200,0.,4.);
         hAdcAmp2d = new TH2D("hAdcAmp2d","ADC pulse amplitude;Channel number;Amplitude (volts)",16,-0.5,15.5,200,0.,4.);
         hAdcFitAmp = new TH1D("hAdcFitAmp","ADC pulse amplitude from fit;Amplitude from fit (volts)",200,0.,4.);
         hAdcFitAmp2d = new TH2D("hAdcFitAmp2d","ADC pulse amplitude from fit;Channel number;Amplitude from fit (volts)",16,-0.5,15.5,200,0.,4.);
         hAdcFitting = new TH2D("hAdcFitting","ADC pulse amplitude fit vs. measured;Amplitude (Volts);Amplitude from fit (volts)",200,0.,4.,200,0.,4.);
         hAdcTime = new TH1D("hAdcTime","ADC pulse start time;ADC pulse start time [ns]",3000,0,3000);
         hAdcTime2d = new TH2D("hAdcTime2d","ADC pulse start time;Channel number;ADC pulse start time [ns]",16,-0.5,15.5,3000,0,3000);
         gDirectory->cd("..");

         // TDC
         gDirectory->mkdir("tdc_histos")->cd();
         hAdcTdcOccupancy = new TH1D("hAdcTdcOccupancy","Channel occupancy after TDC matching;Channel number",16,-0.5,15.5);
         hTdcOccupancy = new TH1D("hTdcOccupancy","TDC channel occupancy;Channel number",16,-0.5,15.5);
         hTdcCorrelation = new TH2D("hTdcCorrelation","TDC channel correlation;Channel number;Channel number",16,-0.5,15.5,16,-0.5,15.5);
         hFineTime = new TH1D("hFineTime","Fine time;Fine time (s)",200,-1e-9,6e-9);
         hFineTime2d = new TH2D("hFineTime2d","Fine time;TDC channel number;Fine time (s)",16,-0.5,15.5,200,-1e-9,6e-9);
         hFineTimeCounter = new TH1D("hFineTimeCounter","Fine time counter;Fine time counter",1024,0,1024);
         hFineTimeCounter2d = new TH2D("hFineTimeCounter2d","Fine time counter;TDC channel number;Fine time counter",16,-0.5,15.5,1024,0,1024);
         hTdcMultiplicity = new TH1D("hTdcMultiplicity","TDC channel multiplicity;Number of TDC channels hit",17,-0.5,16.5);
         hTdcSingleChannelMultiplicity = new TH1D("hTdcSingleChannelMultiplicity","Number of TDC hits on one bar end;Number of TDC hits",201,-0.5,200.5);
         hTdcSingleChannelMultiplicity2d = new TH2D("hTdcSingleChannelMultiplicity2d","Number of TDC hits on one bar end;Channel number;Number of TDC hits",16,-0.5,15.5,201,-0.5,200.5);
         hTdcSingleChannelHitTime = new TH1D("hTdcSingleChannelHitTime","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,400);
         hTdcSingleChannelHitTime100 = new TH1D("hTdcSingleChannelHitTime100","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,40000);
         hTdcSingleChannelHitTime10000 = new TH1D("hTdcSingleChannelHitTime10000","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,4000000);
         hTdcSingleChannelHitTime1000000 = new TH1D("hTdcSingleChannelHitTime1000000","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,400000000);
         hTdcSingleChannelHitTime2d = new TH2D("hTdcSingleChannelHitTime2d","Time of subsequent hits on same channel;Channel number;Time of subsequent hits after first hit (ns)",16,-0.5,15.5,100,0,400);
         if (fFlags->fPulser) {
            hTdcTimeVsCh0 = new TH1D("hTdcTimeVsCh0","TDC time with reference to channel forty;TDC time minus ch40 time (ns)",2000,-5,5);
            hTdcTimeVsCh02d = new TH2D("hTdcTimeVsCh02d","TDC time with reference to channel forty;Channel number;TDC time minus ch40 time (ns)",16,-0.5,15.5,2000,-5,5);
         }
         gDirectory->cd("..");

         // Bar
         gDirectory->mkdir("bar_histos")->cd();
         hBarOccupancy = new TH1D("hBarOccupancy","Bar occupancy;Bar number;Counts",2,-0.5,1.5);
         hTopTime = new TH1D("hTopTime","TDC top time;Time (s)",300,-1000,6000);
         hBotTime = new TH1D("hBotTime","TDC bottom time;Time (s)",300,-1000,6000);
         if ( !(fFlags->fPulser) ) {
            hBarMultiplicity = new TH1D("hBarMultiplicity","Bar multiplicity;Number of bars hit",3,-0.5,2.5);
            hTopBotDiff = new TH1D("hTopBotDiff","Top vs bottom time difference;TDC bottom time minus TDC top time (ns)",200,-10,10);
            hTopBotDiff100 = new TH1D("hTopBotDiff100","Top vs bottom time difference;TDC bottom time minus TDC top time (ns)",200,-1000,1000);
            hTopBotDiff10000 = new TH1D("hTopBotDiff10000","Top vs bottom time difference;TDC bottom time minus TDC top time (ns)",200,-100000,100000);
            hTopBotDiff1000000 = new TH1D("hTopBotDiff1000000","Top vs bottom time difference;TDC bottom time minus TDC top time (ns)",200,-10000000,10000000);
            hTopBotDiff2d = new TH2D("hTopBotDiff2d","Top vs bottom time difference;Bar number;TDC bottom time minus TDC top time (ns)",2,-0.5,1.5,200,-10,10);
            hZed = new TH1D("hZed","Zed position of bar hit from TDC;Zed position from centre (m)",200,-2,2);
            hZed2d = new TH2D("hZed2d","Zed position of bar hit from TDC;Bar number;Zed position from centre (m)",2,-0.5,1.5,200,-2,2);
            hTwoBarTOF = new TH1D("hTwoBarTOF","Time of flight between two bars;TOF (ns)",200,0,20);
            hTwoBarDZed = new TH1D("hTwoBarDZed","Zed difference between two bars;Delta Zed (m)",200,-2,2);
         }
         gDirectory->cd("..");

      }

      if ( !(fFlags->fProtoTOF) ) { // full barrel veto

         // ADC
         gDirectory->mkdir("adc_histos")->cd();
         hAdcOccupancy = new TH1D("hAdcOccupancy","ADC channel occupancy;Channel number;Counts",128,-0.5,127.5);
         hAdcCorrelation = new TH2D("hAdcCorrelation","ADC channel correlation;Channel number;Channel number",128,-0.5,127.5,128,-0.5,127.5);
         hAdcMultiplicity = new TH1D("hAdcMultiplicity","ADC hit multiplicity;Number of ADC channels hit",41,-0.5,40.5);
         hAdcAmp = new TH1D("hAdcAmp","ADC pulse amplitude;Amplitude (volts)",200,0.,2.);
         hAdcAmp2d = new TH2D("hAdcAmp2d","ADC pulse amplitude;Channel number;Amplitude (volts)",128,-0.5,127.5,200,0.,3.);
         hAdcFitAmp = new TH1D("hAdcFitAmp","ADC pulse amplitude from fit;Amplitude from fit (volts)",200,0.,3.);
         hAdcFitAmp2d = new TH2D("hAdcFitAmp2d","ADC pulse amplitude from fit;Channel number;Amplitude from fit (volts)",128,-0.5,127.5,200,0.,3.);
         hAdcFitting = new TH2D("hAdcFitting","ADC pulse amplitude fit vs. measured;Amplitude (Volts);Amplitude from fit (volts)",200,0.,3.,200,0.,3.);
         hAdcTime = new TH1D("hAdcTime","ADC pulse start time;ADC pulse start time [ns]",3000,0,3000);
         hAdcTime2d = new TH2D("hAdcTime2d","ADC pulse start time;Channel number;ADC pulse start time [ns]",128,-0.5,127.5,3000,0,3000);
         gDirectory->cd("..");

         // TDC
         gDirectory->mkdir("tdc_histos")->cd();
         hAdcTdcOccupancy = new TH1D("hAdcTdcOccupancy","Channel occupancy after TDC matching;Channel number",128,-0.5,127.5);
         hTdcOccupancy = new TH1D("hTdcOccupancy","TDC channel occupancy;Channel number",128,-0.5,127.5);
         hTdcCoincidence = new TH1D("hTdcCoincidence","TDC hits with corresponing hit on other end;Channel number",128,-0.5,127.5);
         hTdcCorrelation = new TH2D("hTdcCorrelation","TDC channel correlation;Channel number;Channel number",128,-0.5,127.5,128,-0.5,127.5);
         hFineTime = new TH1D("hFineTime","Fine time;Fine time (s)",200,-1e-9,6e-9);
         hFineTime2d = new TH2D("hFineTime2d","Fine time;TDC channel number;Fine time (s)",128,-0.5,127.5,200,-1e-9,6e-9);
         hFineTimeCounter = new TH1D("hFineTimeCounter","Fine time counter;Fine time counter",1024,0,1024);
         hFineTimeCounter2d = new TH2D("hFineTimeCounter2d","Fine time counter;TDC channel number;Fine time counter",128,-0.5,127.5,1024,0,1024);
         hTdcMultiplicity = new TH1D("hTdcMultiplicity","TDC channel multiplicity;Number of TDC channels hit",81,-0.5,80.5);
         hTdcSingleChannelMultiplicity = new TH1D("hTdcSingleChannelMultiplicity","Number of TDC hits on one bar end;Number of TDC hits",11,-0.5,10.5);
         hTdcSingleChannelMultiplicity2d = new TH2D("hTdcSingleChannelMultiplicity2d","Number of TDC hits on one bar end;Channel number;Number of TDC hits",128,-0.5,127.5,11,-0.5,10.5);
         hTdcSingleChannelHitTime = new TH1D("hTdcSingleChannelHitTime","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,400);
         hTdcSingleChannelHitTime100 = new TH1D("hTdcSingleChannelHitTime10","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,40000);
         hTdcSingleChannelHitTime10000 = new TH1D("hTdcSingleChannelHitTime10000","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,4000000);
         hTdcSingleChannelHitTime1000000 = new TH1D("hTdcSingleChannelHitTime1000000","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,400000000);
         hTdcSingleChannelHitTime2d = new TH2D("hTdcSingleChannelHitTime2d","Time of subsequent hits on same channel;Channel number;Time of subsequent hits after first hit (ns)",128,-0.5,127.5,1000,0,400);
         if (fFlags->fPulser) {
            hTdcTimeVsCh0 = new TH1D("hTdcTimeVsCh0","TDC time with reference to channel forty;TDC time minus ch40 time (ns)",2000,-35,35);
            hTdcTimeVsCh02d = new TH2D("hTdcTimeVsCh02d","TDC time with reference to channel forty;Channel number;TDC time minus ch40 time (ns)",128,-0.5,127.5,2000,-35,35);
         }
         gDirectory->cd("..");

         // Bar
         gDirectory->mkdir("bar_histos")->cd();
         hBarOccupancy = new TH1D("hBarOccupancy","Bar occupancy;Bar number;Counts",64,-0.5,63.5);
         hTopTime = new TH1D("hTopTime","TDC top time;Time (s)",300,-1000,6000);
         hBotTime = new TH1D("hBotTime","TDC bottom time;Time (s)",300,-1000,6000);
         if ( !(fFlags->fPulser) ) {
            hBarMultiplicity = new TH1D("hBarMultiplicity","Bar multiplicity;Number of bars hit",65,-0.5,64.5);
            hBarCorrelation = new TH2D("hBarCorrelation","Bar correlation;Bar number;Bar number",64,-0.5,63.5,64,-0.5,63.5);
            hTopBotDiff = new TH1D("hTopBotDiff","Top vs bottom time difference;TDC bottom time minus TDC top time (ns)",200,-30,30);
            hTopBotDiff100 = new TH1D("hTopBotDiff100","Top vs bottom time difference;TDC bottom time minus TDC top time (ns)",200,-3000,3000);
            hTopBotDiff10000 = new TH1D("hTopBotDiff10000","Top vs bottom time difference;TDC bottom time minus TDC top time (ns)",200,-300000,300000);
            hTopBotDiff1000000 = new TH1D("hTopBotDiff1000000","Top vs bottom time difference;TDC bottom time minus TDC top time (ns)",200,-30000000,30000000);
            hTopBotDiff2d = new TH2D("hTopBotDiff2d","Top vs bottom time difference;Bar number;TDC bottom time minus TDC top time (ns)",64,-0.5,63.5,200,-30,30);
            hZed = new TH1D("hZed","Zed position of bar hit from TDC;Zed position from centre (m)",200,-3,3);
            hZed2d = new TH2D("hZed2d","Zed position of bar hit from TDC;Bar number;Zed position from centre (m)",64,-0.5,63.5,200,-2,2);
            hTwoBarTOF = new TH1D("hTwoBarTOF","TOF for events with N=2 bars;TOF (ns)",200,0,10);
            hTwoBarTOF2d = new TH2D("hTwoBarTOF2d","TOF for events with N=2 bars;Bar number;TOF (ns)",64,-0.5,63.5,200,0,10);
            hNBarTOF = new TH1D("hNBarTOF","TOF for any permutation of two hits;TOF (ns)",200,0,10);
            hNBarTOF2d = new TH2D("hNBarTOF2d","TOF for any permutation of two hits;Bar number;TOF (ns)",64,-0.5,63.5,200,0,10);
            hTwoBarDPhi = new TH1D("hTwoBarDPhi","Angular separation for events with N=2 bars;Delta phi (bars)",65,-0.25,32.25);
            hNBarDPhi = new TH1D("hNBarDPhi","Angular separation for any permutation of two hits;Delta phi (bars)",65,-0.25,32.25);
            hTwoBarDZed = new TH1D("hTwoBarDZed","Zed separation for events with N=2 bars;Delta zed (m)",200,-6,6);
            hNBarDZed = new TH1D("hNBarDZed","Zed separation for any permutation of two hits;Delta zed (m)",200,-6,6);
            hTwoBarDPhiDZed = new TH2D("hTwoBarDPhiDZed","Angular separation vs zed separation for events with N=2 bars;Delta phi (bars);Delta zed (m)",33,-0.5,32.5,200,-6,6);
            hNBarDPhiDZed = new TH2D("hNBarDPhiDZed","Angular separation vs zed separation for any permutation of two hits;Delta phi (bars);Delta zed (m)",33,-0.5,32.5,200,-6,6);
           hTwoBarExpectedTOF = new TH1D("hTwoBarExpectedTOF","Geometric distance/speed of light for events with N=2 bars;(Distance between hits)/c (ns)",200,0,10);
           hTwoBarExpectedTOFvsTOF = new TH2D("hTwoBarExpectedTOFvsTOF","Geometric distance/speed of light for events with N=2 bars;(Distance between hits)/c (ns);Measured TOF (ns)",200,0,10,200,0,10);
           hNBarExpectedTOF = new TH1D("hNBarExpectedTOF","Geometric distance/speed of light for all permutations;(Distance between hits)/c (ns)",200,0,10);
           hNBarExpectedTOFvsTOF = new TH2D("hNBarExpectedTOFvsTOF","Geometric distance/speed of light for all permutations;(Distance between hits)/c (ns);Measured TOF (ns)",200,0,10,200,0,10);
           hTwoBarExpectedTOFminusTOF = new TH1D("hTwoBarExpectedTOFminusTOF","Geometric distance/speed of light minus TOF for events with N=2 bars;TOF - (Distance between hits)/c (ns)",200,-5,5);
           hNBarExpectedTOFminusTOF = new TH1D("hNBarExpectedTOFminusTOF","Geometric distance/speed of light minus TOF for all permutations;TOF - (Distance between hits)/c (ns)",200,-5,5);
         }
         gDirectory->cd("..");

         // TPC matching
         if ( !(fFlags->fPulser) ) {
           gDirectory->mkdir("tpc_matching_histos")->cd();
           hTPCMatched = new TH1D("hTPCMatched","Number of hits sucessfully matched to tracks;Bar number;Counts",64,-0.5,63.5);
           hMatchingDZ = new TH1D("hMatchingDZ","Zed distance between BV and TPC hit;Delta Zed (m);Counts",200,-2,2);
           hMatchingDZbyBar = new TH2D("hMatchingDZbyBar","Zed distance between BV and TPC hit;Bar number;Delta Zed (m)",64,-0.5,63.5,200,-2,2);
           hMatchingDZbyZed = new TH2D("hMatchingDZbyZed","Zed distance between BV and TPC hit;Zed position;Delta Zed (m)",200,-3,3,200,-2,2);
           hMatchingDPhi = new TH1D("hMatchingDPhi","Phi distance between BV and TPC hit;Delta Phi (rad);Counts",200,-2,2);
           hMatchingDPhibyBar = new TH2D("hMatchingDPhibyBar","Phi distance between BV and TPC hit;Bar number;Delta Phi (rad)",64,-0.5,63.5,200,-2,2);
           hMatchingD = new TH1D("hMatchingD","Geometric distance between BV and TPC hit;Geometric distance (m);Counts",200,0,2);
           hMatchingDbyBar = new TH2D("hMatchingDbyBar","Geometric distance between BV and TPC hit;Bar number;Geometric distance (m)",64,-0.5,63.5,200,0,2);
           hMatchedZ = new TH1D("hMatchedZ","Zed of matched hits from BV;Zed (m)",200,-3,3);
           hMatchedZTPC = new TH1D("hMatchedZTPC","Zed of matched hits from TPC;Zed (m)",200,-3,3);
           gDirectory->mkdir("amp_vs_zed")->cd();
           for (int i=0;i<64;i++) {
              hAmpVsZed.push_back(new TH2D(Form("hAmpVsZed%d",i),Form("Amplitude vs TPC zed, channel %d;Zed from TPC (m);Amplitude from fit (volts)",i),200,-3,3,200,0,4));
              hLnAmpVsZed.push_back(new TH2D(Form("hLnAmpVsZed%d",i),Form("Amplitude vs TPC zed, channel %d;Zed from TPC (m);Natural logarithm of amplitude from fit",i),200,-3,3,200,-3,1.5));
           }
           for (int i=64;i<128;i++) {
              hAmpVsZed.push_back(new TH2D(Form("hAmpVsZed%d",i),Form("Amplitude vs TPC zed, channel %d;-1 times Zed from TPC (m);Amplitude from fit (volts)",i),200,-3,3,200,0,4));
              hLnAmpVsZed.push_back(new TH2D(Form("hLnAmpVsZed%d",i),Form("Amplitude vs TPC zed, channel %d;-1 times Zed from TPC (m);Natural logarithm of amplitude from fit",i),200,-3,3,200,-3,1.5));
           }
           gDirectory->cd("..");
           gDirectory->cd("..");
         }

      }


   }

   void EndRun(TARunInfo* runinfo)
   {
      if( fFlags->fPrint ) printf("BscHistoModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
      if (fFlags->fWriteOffsetFile and fFlags->fPulser) {
         if (!hTdcTimeVsCh02d) {
            if (fFlags->fPrint) printf("BscHistoModule can't save TDC offsets, offset histogram not found\n");
            return;
         }
         if (fFlags->fPrint) printf("BscHistoModule calculating TDC channel offsets");
         TH1D* hTdcTimeQuantiles = hTdcTimeVsCh02d->QuantilesX(0.5,"hTdcTimeQuantiles");
         TString Ofilename=getenv("AGRELEASE");
         if (!(fFlags->fProtoTOF)) Ofilename+="/ana/bscint/BVoffsets.calib";
         if (fFlags->fProtoTOF) Ofilename+="/ana/bscint/protoTOFoffsets.calib";
         if (fFlags->fPrint) printf("BscHistoModule saving TDC channel offset file as %s\n",Ofilename.Data());
         std::ofstream Ofile;
         Ofile.open(Ofilename);
         Ofile<<"Bar end number | Offset(ns)\n";
			int n_ch = 128;
         if (fFlags->fProtoTOF) n_ch = 16;
         for (int ch=0;ch<n_ch;ch++) { // First channel is reference channel
            if (ch==pulser_reference_chan) {
               Ofile<<ch<<"\t0\n";
               continue;
            }
            Ofile<<ch<<"\t"<<hTdcTimeQuantiles->GetBinContent(ch+1)<<"\n"; // Bin 0 is underflow, labelling starts at 1
         }
         Ofile.close();
         delete hTdcTimeQuantiles;
         if (fFlags->fPrint) printf("BscHistoModule TDC channel offsets saved");
      }
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if( fFlags->fPrint ) printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if( fFlags->fPrint ) printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {      
      if( (!diagnostics) and (!(fFlags->fWriteOffsetFile and fFlags->fPulser)) )
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      const AgEventFlow* ef = flow->Find<AgEventFlow>();
     
      if (!ef || !ef->fEvent)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
           
      AgBarEventFlow *bef = flow->Find<AgBarEventFlow>();
      if( !bef || !bef->BarEvent)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      TBarEvent* barEvt = bef->BarEvent;
      AgEvent* agEvt = ef->fEvent;

      TdcEvent* tdc = agEvt->tdc;

      if (!tdc || !tdc->complete)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         if (fFlags->fPrint) printf("bsc_histo_module: TDC event incomplete or missing\n");
         return flow;
      }
     
      
      std::vector<TBarEndHit*> endhits = barEvt->GetEndHits();
      if (endhits.size()==0) {
         if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent no endhits\n");
         return flow;
      }
      std::vector<TBarSimpleTdcHit*> tdchits = barEvt->GetTdcHits();
      if (tdchits.size()==0) {
         if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent no tdc hits\n");
         return flow;
      }

      if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent start\n");

      if( fFlags->fPrint ) printf("Filling ADC histos\n");
      AdcHistos(endhits);
      if( fFlags->fPrint ) printf("Filling TDC histos\n");
      TdcHistos(endhits);
      if( fFlags->fPrint ) printf("Filling more TDC histos\n");
      DirectTdcHistos(tdchits);

      std::vector<TBarHit*> barhits = barEvt->GetBars();
      if (barhits.size()==0) {
         if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent no barhits\n");
         return flow;
      }
      if( fFlags->fPrint ) printf("Filling bar histos\n");
      BarHistos(barhits);
      if( fFlags->fPrint ) printf("Filling TOF histos\n");
      TOFHistos(barhits);
      if( fFlags->fPrint ) printf("Filling BV/TPC matching histos\n");
      MatchingHistos(barhits);

      ++fCounter;
      if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent complete\n");
      return flow;
   }

   void AdcHistos(const std::vector<TBarEndHit*> endhits)
   {
      hAdcMultiplicity->Fill(endhits.size());
      for (TBarEndHit* endhit: endhits) {
         int bar = endhit->GetBar();
         hAdcOccupancy->Fill(bar);
         hAdcTime->Fill(endhit->GetADCTime());
         hAdcTime2d->Fill(bar,endhit->GetADCTime());
         hAdcAmp->Fill(endhit->GetAmpRaw());
         hAdcAmp2d->Fill(bar,endhit->GetAmpRaw());
         hAdcFitAmp->Fill(endhit->GetAmp());
         hAdcFitAmp2d->Fill(bar,endhit->GetAmp());
         hAdcFitting->Fill(endhit->GetAmpRaw(),endhit->GetAmp());
         for (TBarEndHit* endhit2: endhits) {
            int bar2 = endhit2->GetBar();
            if (bar==bar2) continue;
            hAdcCorrelation->Fill(bar,bar2);
         }
      }
   }

   void TdcHistos(const std::vector<TBarEndHit*> endhits)
   {
      double ch0 = 0;
      for (TBarEndHit* endhit: endhits) {
         if (!(endhit->IsTDCMatched())) continue;
         int end = endhit->GetBar();
         if (end==pulser_reference_chan) {
            ch0 = endhit->GetTDCTime();
         }
         hAdcTdcOccupancy->Fill(end);
      }
      if (fFlags->fPulser and ch0!=0) {
         for (TBarEndHit* endhit: endhits) {
            if (endhit->GetBar()==0) continue;
            hTdcTimeVsCh0->Fill(1e9*(endhit->GetTDCTime()-ch0));
            hTdcTimeVsCh02d->Fill(endhit->GetBar(),1e9*(endhit->GetTDCTime()-ch0));
         }
      }
   }

   void DirectTdcHistos(const std::vector<TBarSimpleTdcHit*> tdchits)
   {
      int max_chan=128;
      if (fFlags->fProtoTOF) max_chan=16;
      std::vector<int> counts(max_chan,0);
      std::vector<double> t0(max_chan,0);
      for (const TBarSimpleTdcHit* tdchit: tdchits) {
         int bar = tdchit->GetBar();
         double time = tdchit->GetTime();
         int fine_count = tdchit->GetFineTimeCount();
         double fine_time = tdchit->GetFineTime();
         hTdcOccupancy->Fill(bar);
         hFineTime->Fill(fine_time);
         hFineTime2d->Fill(bar,fine_time);
         hFineTimeCounter->Fill(fine_count);
         hFineTimeCounter2d->Fill(bar,fine_count);
         counts[bar]++;
         if (t0[bar]!=0) {
            hTdcSingleChannelHitTime->Fill(1e9*(time-t0[bar]));
            hTdcSingleChannelHitTime100->Fill(1e9*(time-t0[bar]));
            hTdcSingleChannelHitTime10000->Fill(1e9*(time-t0[bar]));
            hTdcSingleChannelHitTime1000000->Fill(1e9*(time-t0[bar]));
            hTdcSingleChannelHitTime2d->Fill(bar,1e9*(time-t0[bar]));
         }
         if (t0[bar]==0) t0[bar] = time;
         if (!(fFlags->fProtoTOF)) {
            bool matched = false;
            for (TBarSimpleTdcHit* tdchit2: tdchits) {
               if (tdchit2->GetBar()!=(bar+64) and tdchit2->GetBar()!=(bar-64)) continue;
               double time2 = tdchit2->GetTime();
               if (TMath::Abs(time-time2)>50*1e-9) continue;
               matched = true;
            }
            if (matched) hTdcCoincidence->Fill(bar);
         }
      }

      int bars=0;
      for (int bar=0;bar<max_chan;bar++) {
         if (counts[bar]>0) {
            bars++;
            hTdcSingleChannelMultiplicity->Fill(counts[bar]);
            hTdcSingleChannelMultiplicity2d->Fill(bar,counts[bar]);
         }
         for (int bar2=0;bar2<max_chan;bar2++) {
            if (bar==bar2) continue;
            hTdcCorrelation->Fill(bar,bar2,counts[bar]*counts[bar2]);
         }
      }
      hTdcMultiplicity->Fill(bars);
   }

   void BarHistos(const std::vector<TBarHit*> barhits)
   {
      for (TBarHit* barhit: barhits) {
         hBarOccupancy->Fill(barhit->GetBar());
         hTopTime->Fill(barhit->GetTDCTop());
         hBotTime->Fill(barhit->GetTDCBot());
      }
      if (fFlags->fPulser) return;
      hBarMultiplicity->Fill(barhits.size());
      for (TBarHit* barhit: barhits) {
         int bar = barhit->GetBar();
         hTopBotDiff->Fill(1e9*(barhit->GetTDCBot()-barhit->GetTDCTop()));
         hTopBotDiff100->Fill(1e9*(barhit->GetTDCBot()-barhit->GetTDCTop()));
         hTopBotDiff10000->Fill(1e9*(barhit->GetTDCBot()-barhit->GetTDCTop()));
         hTopBotDiff1000000->Fill(1e9*(barhit->GetTDCBot()-barhit->GetTDCTop()));
         hTopBotDiff2d->Fill(bar,1e9*(barhit->GetTDCBot()-barhit->GetTDCTop()));
         hZed->Fill(barhit->GetTDCZed());
         hZed2d->Fill(bar,barhit->GetTDCZed());
         if (TMath::Abs(barhit->GetTDCZed())>3) {
            if (fFlags->fPrint) printf("BscHistoModule: weird bar hit: bar %d zed %f top %f bot %f diff %f \n",bar,barhit->GetTDCZed(),barhit->GetTDCTop(),barhit->GetTDCBot(),barhit->GetTDCTop()-barhit->GetTDCBot());
         }
         if (!(fFlags->fProtoTOF)) {
            for (TBarHit* barhit2: barhits) {
               int bar2 = barhit2->GetBar();
               if (bar==bar2) continue;
               hBarCorrelation->Fill(bar,bar2);
            }
         }
      }
   }

   void TOFHistos(const std::vector<TBarHit*> barhits)
   {
      if (barhits.size()<2) return;
      if (fFlags->fPulser) return;
      if (barhits.size()==2) {
         if (!(fFlags->fProtoTOF)) {
            if (barhits[0]->IsTPCMatched() and barhits[0]->IsTPCMatched() or (fFlags->fRecOff)) {
               double TOF = 1e9*(barhits[0]->GetAverageTDCTime()-barhits[1]->GetAverageTDCTime());
               TBarHit* hit0 = barhits[0];
               TBarHit* hit1 = barhits[1];
               if (TOF<=0) { hit1 = barhits[0]; hit0 = barhits[1]; }
               TOF = TMath::Abs(TOF);
               hTwoBarTOF->Fill(TOF);
               double x0,y0,x1,y1,z0,z1;
               hit0->GetXY(x0,y0);
               hit1->GetXY(x1,y1);
               z0 = hit0->GetTDCZed();
               z1 = hit1->GetTDCZed();
               hTwoBarDZed->Fill(z0-z1);
               if (!(fFlags->fProtoTOF)) {
                  hTwoBarTOF2d->Fill(hit0->GetBar(),TOF);
                  hTwoBarTOF2d->Fill(hit1->GetBar(),TOF);
                  double angle =TMath::Abs(hit0->GetBar()-hit1->GetBar());
                  if (angle>32) angle = 64 - angle;
                  hTwoBarDPhi->Fill(angle);
                  hTwoBarDPhiDZed->Fill(angle,z0-z1);
                  double expTOF = TMath::Sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)+(z0-z1)*(z0-z1))/c;
                  hTwoBarExpectedTOF->Fill(expTOF);
                  hTwoBarExpectedTOFvsTOF->Fill(expTOF,TOF);
                  hTwoBarExpectedTOFminusTOF->Fill(TOF-expTOF);
               }
            }
         }
      }
      if (!(fFlags->fProtoTOF)) {
         for (TBarHit* barhit: barhits) {
            for (TBarHit* barhit2: barhits) {
               if ((!(barhit->IsTPCMatched())) and (!(fFlags->fRecOff))) continue;
               if ((!(barhit2->IsTPCMatched())) and (!(fFlags->fRecOff))) continue;
               double TOF = 1e9*(barhit->GetAverageTDCTime()-barhit2->GetAverageTDCTime());
               if (TOF<=0) continue;
               hNBarTOF->Fill(TOF);
               hNBarTOF2d->Fill(barhit->GetBar(),TOF);
               double angle = TMath::Abs(barhit->GetBar()-barhit2->GetBar());
               if (angle>32) angle = 64 - angle;
               hNBarDPhi->Fill(angle);
               double x0,y0,x1,y1,z0,z1;
               barhit->GetXY(x0,y0);
               barhit2->GetXY(x1,y1);
               z0 = barhit->GetTDCZed();
               z1 = barhit2->GetTDCZed();
               hNBarDZed->Fill(z0-z1);
               hNBarDPhiDZed->Fill(angle,z0-z1);
               double expTOF = TMath::Sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)+(z0-z1)*(z0-z1))/c;
               hNBarExpectedTOF->Fill(expTOF);
               hNBarExpectedTOFvsTOF->Fill(expTOF,1e9*TMath::Abs(barhit->GetAverageTDCTime()-barhit2->GetAverageTDCTime()));
               hNBarExpectedTOFminusTOF->Fill((1e9*TMath::Abs(barhit->GetAverageTDCTime()-barhit2->GetAverageTDCTime()))-expTOF);
            }
         }
      }
   }
   void MatchingHistos(const std::vector<TBarHit*> barhits)
   {
      if (fFlags->fPulser) return;
      if (fFlags->fProtoTOF) return;
      if (fFlags->fRecOff) return;
      for (TBarHit* barhit: barhits) {
         if (!(barhit->IsTPCMatched())) continue;
         TVector3 tpc_point = barhit->GetTPC();
         TVector3 bv_point = Get3VectorBV(barhit);
         double dz = GetDZ(tpc_point,bv_point);
         double dphi = GetDPhi(tpc_point,bv_point);
         double diff = GetGeometricDistance(tpc_point,bv_point);
         int bar = barhit->GetBar();
         double z = barhit->GetTDCZed();
         hTPCMatched->Fill(bar);
         hMatchingDZ->Fill(dz/1000);
         hMatchingDZbyBar->Fill(bar,dz/1000);
         hMatchingDZbyZed->Fill(z,dz/1000);
         hMatchingDPhi->Fill(dphi);
         hMatchingDPhibyBar->Fill(bar,dphi);
         hMatchingD->Fill(diff/1000);
         hMatchingDbyBar->Fill(bar,diff/1000);
         hMatchedZ->Fill(tpc_point.z()/1000);
         hMatchedZTPC->Fill(z);
         hAmpVsZed[bar]->Fill(tpc_point.z()/1000,barhit->GetAmpBot());
         hAmpVsZed[bar+64]->Fill(-1*tpc_point.z()/1000,barhit->GetAmpTop());
         hLnAmpVsZed[bar]->Fill(tpc_point.z()/1000,TMath::Log(barhit->GetAmpBot()));
         hLnAmpVsZed[bar+64]->Fill(-1*tpc_point.z()/1000,TMath::Log(barhit->GetAmpTop()));

      }
   }

   // Helper functions

   double GetDZ(TVector3 p1, TVector3 p2)
   {
      return p2.z() - p1.z();
   }
   double GetDPhi(TVector3 p1, TVector3 p2)
   {
      return p2.DeltaPhi(p1);
   }
   TVector3 Get3VectorBV(TBarHit* hit)
   {
      double xbv,ybv;
      hit->GetXY(xbv,ybv);
      double dt = (hit->GetTDCBot() - hit->GetTDCTop())*1e9;
      double zbv = factor*dt;
      TVector3 bv_point = TVector3(xbv*1000,ybv*1000,zbv*1000); // to mm
      return bv_point;
   }
   double GetGeometricDistance(TVector3 p1, TVector3 p2)
   {
      TVector3 diff = p2-p1;
      return diff.Mag();
   }


};


class BscHistoModuleFactory: public TAFactory
{
public:
   BscHistoFlags fFlags;
   
public:
   void Help()
   {  
      printf("BscHistoModuleFactory::Help\n");
      printf("\t--bscdiag\t\t\tenables analysis histograms\n");
      printf("\t--bscpulser\t\t\tanalyze run with calibration pulser data instead of cosmics/hbar data\n");
      printf("\t--bscProtoTOF\t\t\tanalyze run with with TRIUMF prototype instead of full BV\n");
      printf("\t--bscprint\t\t\tverbose mode\n");
      printf("\t--bscWriteOffsetFile\t\t\twhen used on calibration pulser data, saves a calibration file to correct the tdc channel-by-channel time offsets\n");
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("BscHistoModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if( args[i] == "--bscdiag" )
            fFlags.fBscDiag = true;
         if (args[i] == "--bscprint")
            fFlags.fPrint = true;
         if( args[i] == "--bscpulser" )
            fFlags.fPulser = true;
         if( args[i] == "--bscProtoTOF" )
            fFlags.fProtoTOF = true;
         if( args[i] == "--bscWriteOffsetFile" )
            fFlags.fWriteOffsetFile = true;
         if( args[i] == "--recoff" )
            fFlags.fRecOff = true;

      }
   }

   void Finish()
   {
      printf("BscHistoModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("BscHistoModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new BscHistoModule(runinfo, &fFlags);
   }
};

static TARegister tar(new BscHistoModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
