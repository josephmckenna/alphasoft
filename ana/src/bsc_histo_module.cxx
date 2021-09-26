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
   double c = 0.2998; // m/ns
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
   TH2D* hTdcSingleChannelHitTime2d;
   TH1D* hTdcTimeVsCh0;
   TH2D* hTdcTimeVsCh02d;
   std::map<int,TH2D*> hTdcOffsetByRTM;

   // Bars
   TH1D* hBarMultiplicity;
   TH1D* hBarOccupancy;
   TH2D* hBarCorrelation;
   TH1D* hTopBotDiff;
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
   TH1D* hTwoBarExpectedTOF;
   TH2D* hTwoBarExpectedTOFvsTOF;



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
         hTdcMultiplicity = new TH1D("hTdcMultiplicity","TDC channel multiplicity;Number of TDC channels hit",17,-0.5,16.5);
         hTdcSingleChannelMultiplicity = new TH1D("hTdcSingleChannelMultiplicity","Number of TDC hits on one bar end;Number of TDC hits",10,-0.5,9.5);
         hTdcSingleChannelMultiplicity2d = new TH2D("hTdcSingleChannelMultiplicity2d","Number of TDC hits on one bar end;Channel number;Number of TDC hits",16,-0.5,15.5,10,-0.5,9.5);
         hTdcSingleChannelHitTime = new TH1D("hTdcSingleChannelHitTime","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,400);
         hTdcSingleChannelHitTime2d = new TH2D("hTdcSingleChannelHitTime2d","Time of subsequent hits on same channel;Channel number;Time of subsequent hits after first hit (ns)",16,-0.5,15.5,1000,0,400);
         if (fFlags->fPulser) {
            hTdcTimeVsCh0 = new TH1D("hTdcTimeVsCh0","TDC time with reference to channel forty;TDC time minus ch40 time (ns)",2000,-5,5);
            hTdcTimeVsCh02d = new TH2D("hTdcTimeVsCh02d","TDC time with reference to channel forty;Channel number;TDC time minus ch40 time (ns)",16,-0.5,15.5,2000,-5,5);
         }
         gDirectory->cd("..");

         // Bar
         gDirectory->mkdir("bar_histos")->cd();
         hBarOccupancy = new TH1D("hBarOccupancy","Bar occupancy;Bar number;Counts",2,-0.5,1.5);
         if ( !(fFlags->fPulser) ) {
            hBarMultiplicity = new TH1D("hBarMultiplicity","Bar multiplicity;Number of bars hit",3,-0.5,2.5);
            hTopBotDiff = new TH1D("hTopBotDiff","Top vs bottom time difference;TDC top time minus TDC bottom time (ns)",200,-10,10);
            hTopBotDiff2d = new TH2D("hTopBotDiff2d","Top vs bottom time difference;Bar number;TDC top time minus TDC bottom time (ns)",2,-0.5,1.5,200,-10,10);
            hZed = new TH1D("hZed","Zed position of bar hit from TDC;Zed position from centre (m)",200,-2,2);
            hZed2d = new TH2D("hZed2d","Zed position of bar hit from TDC;Bar number;Zed position from centre (m)",2,-0.5,1.5,200,-2,2);
            hTwoBarTOF = new TH1D("hTwoBarTOF","Time of flight between two bars;TOF (ns)",200,-20,20);
            hTwoBarDZed = new TH1D("hTwoBarDZed","Zed difference between two bars;Delta Zed (m)",200,-2,2);
         }
         gDirectory->cd("..");

      }

      if ( !(fFlags->fProtoTOF) ) { // full barrel veto

         // ADC
         gDirectory->mkdir("adc_histos")->cd();
         hAdcOccupancy = new TH1D("hAdcOccupancy","ADC channel occupancy;Channel number;Counts",128,-0.5,127.5);
         hAdcCorrelation = new TH2D("hAdcCorrelation","ADC channel correlation;Channel number;Channel number",128,-0.5,127.5,128,-0.5,127.5);
         hAdcMultiplicity = new TH1D("hAdcMultiplicity","ADC hit multiplicity;Number of ADC channels hit",129,-0.5,128.5);
         hAdcAmp = new TH1D("hAdcAmp","ADC pulse amplitude;Amplitude (volts)",200,0.,4.);
         hAdcAmp2d = new TH2D("hAdcAmp2d","ADC pulse amplitude;Channel number;Amplitude (volts)",128,-0.5,127.5,200,0.,4.);
         hAdcFitAmp = new TH1D("hAdcFitAmp","ADC pulse amplitude from fit;Amplitude from fit (volts)",200,0.,4.);
         hAdcFitAmp2d = new TH2D("hAdcFitAmp2d","ADC pulse amplitude from fit;Channel number;Amplitude from fit (volts)",128,-0.5,127.5,200,0.,4.);
         hAdcFitting = new TH2D("hAdcFitting","ADC pulse amplitude fit vs. measured;Amplitude (Volts);Amplitude from fit (volts)",200,0.,4.,200,0.,4.);
         hAdcTime = new TH1D("hAdcTime","ADC pulse start time;ADC pulse start time [ns]",3000,0,3000);
         hAdcTime2d = new TH2D("hAdcTime2d","ADC pulse start time;Channel number;ADC pulse start time [ns]",128,-0.5,127.5,3000,0,3000);
         gDirectory->cd("..");

         // TDC
         gDirectory->mkdir("tdc_histos")->cd();
         hAdcTdcOccupancy = new TH1D("hAdcTdcOccupancy","Channel occupancy after TDC matching;Channel number",128,-0.5,127.5);
         hTdcOccupancy = new TH1D("hTdcOccupancy","TDC channel occupancy;Channel number",128,-0.5,127.5);
         hTdcCoincidence = new TH1D("hTdcCoincidence","TDC hits with corresponing hit on other end;Channel number",128,-0.5,127.5);
         hTdcCorrelation = new TH2D("hTdcCorrelation","TDC channel correlation;Channel number;Channel number",128,-0.5,127.5,128,-0.5,127.5);
         hTdcMultiplicity = new TH1D("hTdcMultiplicity","TDC channel multiplicity;Number of TDC channels hit",129,-0.5,128.5);
         hTdcSingleChannelMultiplicity = new TH1D("hTdcSingleChannelMultiplicity","Number of TDC hits on one bar end;Number of TDC hits",10,-0.5,9.5);
         hTdcSingleChannelMultiplicity2d = new TH2D("hTdcSingleChannelMultiplicity2d","Number of TDC hits on one bar end;Channel number;Number of TDC hits",128,-0.5,127.5,10,-0.5,9.5);
         hTdcSingleChannelHitTime = new TH1D("hTdcSingleChannelHitTime","Time of subsequent hits on same channel;Time of subsequent hits after first hit (ns)",1000,0,400);
         hTdcSingleChannelHitTime2d = new TH2D("hTdcSingleChannelHitTime2d","Time of subsequent hits on same channel;Channel number;Time of subsequent hits after first hit (ns)",128,-0.5,127.5,1000,0,400);
         if (fFlags->fPulser) {
            hTdcTimeVsCh0 = new TH1D("hTdcTimeVsCh0","TDC time with reference to channel forty;TDC time minus ch40 time (ns)",2000,-35,35);
            hTdcTimeVsCh02d = new TH2D("hTdcTimeVsCh02d","TDC time with reference to channel forty;Channel number;TDC time minus ch40 time (ns)",128,-0.5,127.5,2000,-35,35);
            gDirectory->mkdir("offsets")->cd();
            for (int rtm=0;rtm<8;rtm++) {
               TString hname = TString::Format("hOffsetRTM%d",rtm);
               TString htitle = TString::Format("Time offset from channel 0 - RTM %d;Channel number;TDC time minus ch0 time (ns)",rtm);
               hTdcOffsetByRTM[rtm] = new TH2D(hname.Data(),htitle.Data(),16,-0.5,15.5,2000,-35.,35.);
            }
            gDirectory->cd("..");
         }
         gDirectory->cd("..");

         // Bar
         gDirectory->mkdir("bar_histos")->cd();
         hBarOccupancy = new TH1D("hBarOccupancy","Bar occupancy;Bar number;Counts",64,-0.5,63.5);
         if ( !(fFlags->fPulser) ) {
            hBarMultiplicity = new TH1D("hBarMultiplicity","Bar multiplicity;Number of bars hit",65,-0.5,64.5);
            hBarCorrelation = new TH2D("hBarCorrelation","Bar correlation;Bar number;Bar number",64,-0.5,63.5,64,-0.5,63.5);
            hTopBotDiff = new TH1D("hTopBotDiff","Top vs bottom time difference;TDC top time minus TDC bottom time (ns)",200,-30,30);
            hTopBotDiff2d = new TH2D("hTopBotDiff2d","Top vs bottom time difference;Bar number;TDC top time minus TDC bottom time (ns)",64,-0.5,63.5,200,-30,30);
            hZed = new TH1D("hZed","Zed position of bar hit from TDC;Zed position from centre (m)",200,-3,3);
            hZed2d = new TH2D("hZed2d","Zed position of bar hit from TDC;Bar number;Zed position from centre (m)",64,-0.5,63.5,200,-2,2);
            hTwoBarTOF = new TH1D("hTwoBarTOF","TOF for events with N=2 bars;TOF (ns)",200,-20,20);
            hTwoBarTOF2d = new TH2D("hTwoBarTOF2d","TOF for events with N=2 bars;Bar number;TOF (ns)",64,-0.5,63.5,200,-20,20);
            hNBarTOF = new TH1D("hNBarTOF","TOF for any permutation of two hits;TOF (ns)",200,-20,20);
            hNBarTOF2d = new TH2D("hNBarTOF2d","TOF for any permutation of two hits;Bar number;TOF (ns)",64,-0.5,63.5,200,-20,20);
            hTwoBarDPhi = new TH1D("hTwoBarDPhi","Angular separation for events with N=2 bars;Delta phi (degrees)",32,0,180);
            hNBarDPhi = new TH1D("hNBarDPhi","Angular separation for any permutation of two hits;Delta phi (degrees)",32,0,180);
            hTwoBarDZed = new TH1D("hTwoBarDZed","Zed separation for events with N=2 bars;Delta zed (m)",200,-6,6);
            hNBarDZed = new TH1D("hNBarDZed","Zed separation for any permutation of two hits;Delta zed (m)",200,-6,6);
           hTwoBarExpectedTOF = new TH1D("hTwoBarExpectedTOF","Geometric distance/speed of light for events with N=2 bars;(Distance between hits)/c (ns)",200,0,30);
           hTwoBarExpectedTOFvsTOF = new TH2D("hTwoBarExpectedTOFvsTOF","Geometric distance/speed of light for events with N=2 bars;(Distance between hits)/c (ns);Measured TOF (ns)",200,0,30,200,0,30);
         }
         gDirectory->cd("..");
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
     
      
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      if (endhits.size()==0) {
         if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent no endhits\n");
         return flow;
      }
      std::vector<SimpleTdcHit*> tdchits = barEvt->GetTdcHits();
      if (tdchits.size()==0) {
         if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent no tdc hits\n");
         return flow;
      }

      if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent start\n");

      AdcHistos(endhits);
      TdcHistos(endhits);
      DirectTdcHistos(tdchits);

      std::vector<BarHit*> barhits = barEvt->GetBars();
      if (barhits.size()==0) {
         if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent no barhits\n");
         return flow;
      }
      BarHistos(barhits);
      TOFHistos(barhits);

      ++fCounter;
      if( fFlags->fPrint ) printf("BscHistoModule::AnalyzeFlowEvent complete\n");
      return flow;
   }

   void AdcHistos(std::vector<EndHit*> endhits)
   {
      hAdcMultiplicity->Fill(endhits.size());
      for (EndHit* endhit: endhits) {
         int bar = endhit->GetBar();
         hAdcOccupancy->Fill(bar);
         hAdcTime->Fill(endhit->GetADCTime());
         hAdcTime2d->Fill(bar,endhit->GetADCTime());
         hAdcAmp->Fill(endhit->GetAmpRaw());
         hAdcAmp2d->Fill(bar,endhit->GetAmpRaw());
         hAdcFitAmp->Fill(endhit->GetAmp());
         hAdcFitAmp2d->Fill(bar,endhit->GetAmp());
         hAdcFitting->Fill(endhit->GetAmpRaw(),endhit->GetAmp());
         for (EndHit* endhit2: endhits) {
            hAdcCorrelation->Fill(bar,endhit2->GetBar());
         }
      }
   }

   void TdcHistos(std::vector<EndHit*> endhits)
   {
      double ch0 = 0;
      for (EndHit* endhit: endhits) {
         if (!(endhit->IsTDCMatched())) continue;
         int end = endhit->GetBar();
         if (end==pulser_reference_chan) ch0 = endhit->GetTDCTime();
         hAdcTdcOccupancy->Fill(end);
      }
      if (fFlags->fPulser and ch0!=0) {
         for (EndHit* endhit: endhits) {
            if (endhit->GetBar()==0) continue;
            hTdcTimeVsCh0->Fill(1e9*(endhit->GetTDCTime()-ch0));
            hTdcTimeVsCh02d->Fill(endhit->GetBar(),1e9*(endhit->GetTDCTime()-ch0));
         }
      }
      if (fFlags->fPulser and !(fFlags->fProtoTOF)) {
         for (EndHit* endhit: endhits) {
            int end = endhit->GetBar();
            if (end==pulser_reference_chan) continue;
            //if (end%8==0 and end<63) continue;
            int rtm = (end/8)%8;
            int chan = end%8+(end/64)*8;
            hTdcOffsetByRTM[rtm]->Fill(chan,1e9*(endhit->GetTDCTime()-ch0));
         }
      }
   }

   void DirectTdcHistos(std::vector<SimpleTdcHit*> tdchits)
   {
      int max_chan=128;
      if (fFlags->fProtoTOF) max_chan=16;
      std::vector<int> counts(max_chan,0);
      std::vector<double> t0(max_chan,0);
      for (SimpleTdcHit* tdchit: tdchits) {
         int bar = tdchit->GetBar();
         double time = tdchit->GetTime();
         hTdcOccupancy->Fill(bar);
         counts[bar]++;
         if (t0[bar]!=0) {
            hTdcSingleChannelHitTime->Fill(1e9*(time-t0[bar]));
            hTdcSingleChannelHitTime2d->Fill(bar,1e9*(time-t0[bar]));
         }
         if (t0[bar]==0) t0[bar] = time;
         if (!(fFlags->fProtoTOF)) {
            bool matched = false;
            for (SimpleTdcHit* tdchit2: tdchits) {
               if (tdchit2->GetBar()!=(bar+64) and tdchit2->GetBar()!=(bar-64)) continue;
               double time2 = tdchit2->GetTime();
               if (TMath::Abs(time-time2)>25*1e-9) continue;
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
            hTdcCorrelation->Fill(bar,bar2,counts[bar]*counts[bar2]);
         }
      }
      hTdcMultiplicity->Fill(bars);
   }

   void BarHistos(std::vector<BarHit*> barhits)
   {
      for (BarHit* barhit: barhits) {
         hBarOccupancy->Fill(barhit->GetBar());
      }
      if (fFlags->fPulser) return;
      hBarMultiplicity->Fill(barhits.size());
      for (BarHit* barhit: barhits) {
         int bar = barhit->GetBar();
         hTopBotDiff->Fill(1e9*(barhit->GetTDCTop()-barhit->GetTDCBot()));
         hTopBotDiff2d->Fill(bar,1e9*(barhit->GetTDCTop()-barhit->GetTDCBot()));
         hZed->Fill(barhit->GetTDCZed());
         hZed2d->Fill(bar,barhit->GetTDCZed());
         if (TMath::Abs(barhit->GetTDCZed())>3) {
            if (fFlags->fPrint) printf("BscHistoModule: weird bar hit: bar %d zed %f top %f bot %f diff %f \n",bar,barhit->GetTDCZed(),barhit->GetTDCTop(),barhit->GetTDCBot(),barhit->GetTDCTop()-barhit->GetTDCBot());
         }
         if (!(fFlags->fProtoTOF)) {
            for (BarHit* barhit2: barhits) {
               hBarCorrelation->Fill(bar,barhit2->GetBar());
            }
         }
      }
   }

   void TOFHistos(std::vector<BarHit*> barhits)
   {
      if (barhits.size()<2) return;
      if (fFlags->fPulser) return;
      if (barhits.size()==2) {
         hTwoBarTOF->Fill(1e9*(barhits[0]->GetAverageTDCTime()-barhits[1]->GetAverageTDCTime()));
         double x0,y0,x1,y1,z0,z1;
         barhits[0]->GetXY(x0,y0);
         barhits[1]->GetXY(x1,y1);
         z0 = barhits[0]->GetTDCZed();
         z1 = barhits[1]->GetTDCZed();
         hTwoBarDZed->Fill(z0-z1);
         if (!(fFlags->fProtoTOF)) {
            hTwoBarTOF2d->Fill(barhits[0]->GetBar(),1e9*(barhits[0]->GetAverageTDCTime()-barhits[1]->GetAverageTDCTime()));
            hTwoBarTOF2d->Fill(barhits[1]->GetBar(),1e9*(barhits[1]->GetAverageTDCTime()-barhits[0]->GetAverageTDCTime()));
            double angle = (180/TMath::Pi())*TMath::Abs(barhits[0]->GetPhi()-barhits[1]->GetPhi());
            if (angle>180) angle = 360 - angle;
            hTwoBarDPhi->Fill(angle);
            double expTOF = TMath::Sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)+(z0-z1)*(z0-z1))/c;
            hTwoBarExpectedTOF->Fill(expTOF);
            hTwoBarExpectedTOFvsTOF->Fill(expTOF,1e9*TMath::Abs(barhits[0]->GetAverageTDCTime()-barhits[1]->GetAverageTDCTime()));
         }
      }
      if (!(fFlags->fProtoTOF)) {
         for (BarHit* barhit: barhits) {
            for (BarHit* barhit2: barhits) {
               double TOF = 1e9*(barhit->GetAverageTDCTime()-barhit2->GetAverageTDCTime());
               if (TOF==0) continue;
               hNBarTOF->Fill(TOF);
               hNBarTOF2d->Fill(barhit->GetBar(),TOF);
               double angle = (180/TMath::Pi())*TMath::Abs(barhit->GetPhi()-barhit2->GetPhi());
               if (angle>180) angle = 360 - angle;
               hNBarDPhi->Fill(angle);
               hNBarDZed->Fill(barhit->GetTDCZed()-barhit2->GetTDCZed());
            }
         }
      }
   }


};


class BscHistoModuleFactory: public TAFactory
{
public:
   BscHistoFlags fFlags;
   
public:
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
