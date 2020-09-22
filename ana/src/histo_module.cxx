#include "AgFlow.h"
#include "RecoFlow.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

#include "SignalsType.hh"
#include <set>
#include <iostream>

#include "AnalysisTimer.h"
class HistoFlags
{
public:
   bool fDiag=false;

public:
   HistoFlags() // ctor
   { }

   ~HistoFlags() // dtor
   { }
};

class HistoModule: public TARunObject
{
public:
   HistoFlags* fFlags = NULL;
   int fCounter;
   bool fTrace = false;
   //bool fTrace = true;

   double fCoincTime; // time window to match electrodes
   double fpc_timecut; // isolate pc hits

private:

   bool diagnostics;

   // anodes  
   TH1D* hNhitBot;
   TH1D* hNhitTop;
   TH1D* hOccBot;
   TH1D* hOccTop;
   TH1D* hAvgRMSBot;
   TH1D* hAvgRMSTop;

   TH1D* hAmpBot;
   TH1D* hAmpTop;
   TH1D* hErrTop;
   TH1D* hTimeBot;
   TH1D* hTimeTop;

   TH2D* hTimeAmpBot;
   TH2D* hTimeAmpTop;

   TH2D* hTimeBotChan;
   TH2D* hTimeTopChan;
   TH2D* hAmpBotChan;
   TH2D* hAmpTopChan;

   TH1D* hAwOccSec;
   TH1D* hAwOccIsec;

   // adc wf study
   TH2D* hAdcAmp;
   TProfile* hAdcAmp_prox;
   std::map<int,TH2D*> hAdcTimeAmp;
   TH2D* hAdcRange;
   TProfile* hAdcRange_prox;
   std::map<int,TH2D*> hAdcTimeRange;

   TH1D* hAdcWfAmp;
   TH1D* hAdcWfRange;

   // pads
   TH1D* hNhitPad;
   TH1D* hOccRow;
   TH1D* hOccCol;
   TH2D* hOccPad;
   TH1D* hAvgRMSPad;

   TH1D* hAmpPad;
   TH1D* hErrPad;
   TH1D* hTimePad;

   TH2D* hTimeAmpPad;

   TH2D* hTimePadCol;
   TH2D* hTimePadRow;
   TH2D* hAmpPadCol;
   TH2D* hAmpPadRow;

   // pwb wf study
   TH2D* hPwbAmp;
   TProfile* hPwbAmp_prox;
   //   std::map<int,TH2D*> hPwbTimeAmp;
   TH2D* hPwbRange;
   TProfile* hPwbRange_prox;
   //   std::map<int,TH2D*> hPwbTimeRange;

   TH1D* hPwbWfAmp;
   TH1D* hPwbWfRange;

   std::map<int,TProfile*> hPwbTimeAmp_prox;
   std::map<int,TH2D*> hPwbTimeColRowAmp;
   std::map<int,TProfile*> hPwbTimeRange_prox;
   std::map<int,TH2D*> hPwbTimeColRowRange;

   // match AW*PAD
   TH2D* hawcol;
   TH2D* hamprow_timecolcut;

   TH1D* hNmatch;

   TH2D* hawcol_time;
   TH2D* hawcol_sector_time;
   TH2D* hawcol_deltat_sec;

   TH2D* hawcol_match;
   TH2D* hawcol_match_amp;
   TH2D* hawcol_match_time;

   TH2D* hawamp_match_aw_amp;
   TH2D* hawamp_match_amp;
   TH2D* hawamp_match_aw;
   TH2D* hawamp_match_amp_pc;

   // matching AW
   TH1D* hAwOcc_match;
   TH1D* hAwOccSec_match;
   TH1D* hAwOccIsec_match;
   // matching PADS
   TH2D* hOccPad_match;

   // signals spacepoints
   TH1D* hNsp;

   TH1D* hAWspAmp;
   TH1D* hAWspTime;
   TH1D* hAwspOcc;
   TH1D* hAwspOccSec;
   TH1D* hAwspOccIsec;
   TH2D* hAWspTimeAmp;

   TH2D* hOccSpPad;
   TH1D* hOccSpRow;
   TH1D* hOccSpCol;

   TH1D* hAmpSpPad;
   TH1D* hTimeSpPad;
   TH2D* hTimeAmpSpPad;

   TH2D* hTimeSpPadCol;
   TH2D* hTimeSpPadRow;
   TH2D* hAmpSpPadCol;
   TH2D* hAmpSpPadRow;

   padmap* pmap;

public:
   HistoModule(TARunInfo* runinfo, HistoFlags* f):TARunObject(runinfo),
                                                  fFlags(f),fCounter(0),
                                                  fCoincTime(20.),fpc_timecut(300.) // ns

   {
      diagnostics=f->fDiag;
   }

   ~HistoModule(){}

   void BeginRun(TARunInfo* runinfo)
   {
      #ifdef HAVE_CXX11_THREADS
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      #endif
      if(!diagnostics) return;
      printf("HistoModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      if( !gDirectory->cd("awdeconv") )
         gDirectory->mkdir("awdeconv")->cd();
      //gDirectory->pwd();
      
      hNhitBot = new TH1D("hNhitBot","Number of Hits Bottom;N",500,0.,5000.);
      hNhitTop = new TH1D("hNhitTop","Number of Hits Top;N",500,0.,5000.);
      hOccBot = new TH1D("hOccBot","Occupancy per AW Bottom",256,0.,256.);
      hOccBot->SetMinimum(0.);
      hOccTop = new TH1D("hOccTop","Occupancy per AW Top",256,0.,256.);
      hOccTop->SetMinimum(0.);

      hAmpBot = new TH1D("hAmpBot","Reconstructed Avalanche Size Bottom",200,0.,2000.);
      hAmpTop = new TH1D("hAmpTop","Reconstructed Avalanche Size Top",200,0.,2000.);
      hErrTop = new TH1D("hErrTop","Reconstructed Avalanche Error Size Top",200,0.,100.);
      hTimeBot = new TH1D("hTimeBot","Reconstructed Avalanche Time Bottom",375,0.,6000.);
      hTimeTop = new TH1D("hTimeTop","Reconstructed Avalanche Time Top",375,0.,6000.);
      hTimeAmpBot = new TH2D("hTimeAmpBot","Reconstructed Avalanche Time Vs Size - Bottom",60,0.,6000.,50,0.,2000.);
      hTimeAmpTop = new TH2D("hTimeAmpTop","Reconstructed Avalanche Time Vs Size - Top",60,0.,6000.,50,0.,2000.);

      hTimeBotChan = new TH2D("hTimeBotChan","Reconstructed Avalanche Time Vs Bottom Channel",256,0.,256.,37,0.,6000.);
      hTimeTopChan = new TH2D("hTimeTopChan","Reconstructed Avalanche Time Vs Top Channel",256,0.,256.,37,0.,6000.);
      hAmpBotChan = new TH2D("hAmpBotChan","Reconstructed Avalanche Size Vs Bottom Channel",256,0.,256.,500,0.,2000.);
      hAmpTopChan = new TH2D("hAmpTopChan","Reconstructed Avalanche Size Vs Top Channel",256,0.,256.,500,0.,2000.);

      hAwOccSec = new TH1D("hAwOccSec","Number of TOP AW hits per Pad Sector;N",32,0.,32.);
      hAwOccSec->SetMinimum(0.);
      hAwOccIsec = new TH1D("hAwOccIsec","Number of TOP AW hits Inside Pad Sector;N",8,0.,8.);
      hAwOccIsec->SetMinimum(0.);

      gDirectory->mkdir("adcwf")->cd();
      hAdcAmp = new TH2D("hAdcAmp","Maximum WF Amplitude Vs Channel",256,0.,256.,1000,0.,17000.);
      hAdcAmp_prox = new TProfile("hAdcAmp_prox","Average Maximum WF Amplitude Vs Channel;AW;ADC",
                                  256,0.,256.,0.,17000.);
      hAdcAmp_prox->SetMinimum(0.);
      hAdcRange = new TH2D("hAdcRange","WF Range Vs Channel",256,0.,256.,1000,0.,18000.);
      hAdcRange_prox = new TProfile("hAdcRange_prox","Average WF Range Vs Channel;AW;ADC",
                                    256,0.,256.,0.,18000.);
      hAdcRange_prox->SetMinimum(0.);

      hAdcWfAmp = new TH1D("hAdcWfAmp","ADC WF amp",1000,-1000.,17000.);
      hAdcWfRange = new TH1D("hAdcWfRange","ADC WF amp",1000,-1000.,18000.);

      gDirectory->mkdir("adc32")->cd();
      for( int i=0; i<256; ++i)
         {
            TString hname = TString::Format("hadcampch%03d",i);
            TString htitle = TString::Format("Maximum WF Amplitude Vs Time AW: %d;Time [ns];Amplitude [a.u.]",i);
            hAdcTimeAmp[i] = new TH2D(hname.Data(),htitle.Data(),600,0.,6000.,1000,0.,17000.);
         }
      for( int i=0; i<256; ++i)
         {
            TString hname = TString::Format("hadcrangech%03d",i);
            TString htitle = TString::Format("Maximum WF Amplitude Vs Time AW: %d;Time [ns];Amplitude [a.u.]",i);
            hAdcTimeRange[i] = new TH2D(hname.Data(),htitle.Data(),600,0.,6000.,1000,0.,18000.);
         }
  

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      // pads histograms
      if( !gDirectory->cd("paddeconv") )
         gDirectory->mkdir("paddeconv")->cd();

      hNhitPad = new TH1D("hNhitPad","Number of Hits Pad;N",500,0.,5000.);
      hOccRow = new TH1D("hOccRow","Number of Hits Pad Rows;N",576,0.,576.);
      hOccRow->SetMinimum(0.);
      hOccCol = new TH1D("hOccCol","Number of Hits Pad Cols;N",32,0.,32.);
      hOccCol->SetMinimum(0.);
      hOccPad = new TH2D("hOccPad","Number of Hits Pads;row;sec;N",576,0.,576.,32,0.,32.);

      hAmpPad = new TH1D("hAmpPad","Reconstructed Avalanche Size Pad",200,0.,10000.);
      hErrPad = new TH1D("hErrPad","Reconstructed Avalanche Error Size Pad",200,0.,100.);
      hTimePad = new TH1D("hTimePad","Reconstructed Avalanche Time Pad",375,0.,6000.);

      hTimeAmpPad = new TH2D("hTimeAmpPad","Reconstructed Avalanche Time Vs Size - Pad",300,0.,6000.,100,0.,5100.);
      
      hTimePadCol = new TH2D("hTimePadCol","Reconstructed Avalanche Time Vs Pad Cols;sec;time [ns]",32,0.,32.,40,0.,6000.);
      hTimePadRow = new TH2D("hTimePadRow","Reconstructed Avalanche Time Vs Pad Rows;row;time [ns]",576,0.,576,40,0.,6000.);
      hAmpPadCol = new TH2D("hAmpPadCol","Reconstructed Avalanche Size Vs Pad Cols;sec;amp",32,0.,32.,500,0.,5000.);
      hAmpPadRow = new TH2D("hAmpPadRow","Reconstructed Avalanche Size Vs Pad Rows;row;amp",576,0.,576,500,0.,5000.);

      gDirectory->mkdir("pwbwf")->cd();
      hPwbAmp = new TH2D("hPwbAmp","Maximum WF Amplitude Vs Channel",32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,1000,0.,4200.);
      hPwbAmp_prox = new TProfile("hPwbAmp_prox","Average Maximum WF Amplitude Vs Channel;Pad;PWB",
                                  32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,0.,4200.);
      hPwbAmp_prox->SetMinimum(0.);
      hPwbRange = new TH2D("hPwbRange","WF Range Vs Channel",32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,1000,0.,5100.);
      hPwbRange_prox = new TProfile("hPwbRange_prox","Average WF Range Vs Channel;Pad;PWB",
                                    32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,0.,5100.);
      hPwbRange_prox->SetMinimum(0.);

      hPwbWfAmp = new TH1D("hPwbWfAmp","PWB WF amp",500,-100.,4200.);
      hPwbWfRange = new TH1D("hPwbWfRange","PWB WF amp",500,-100.,5100.);
      // gDirectory->mkdir("pwb")->cd();
      // for( int i=0; i<32*576; ++i)
      //    {
      //       TString hname = TString::Format("hpwbampch%03d",i);
      //       TString htitle = TString::Format("Maximum WF Amplitude Vs Time AW: %d;Time [ns];Amplitude [a.u.]",i);
      //       hPwbTimeAmp[i] = new TH2D(hname.Data(),htitle.Data(),600,0.,6000.,300,0.,3000.);
      //    }
      // for( int i=0; i<32*576; ++i)
      //    {
      //       TString hname = TString::Format("hpwbrangech%03d",i);
      //       TString htitle = TString::Format("Maximum WF Amplitude Vs Time AW: %d;Time [ns];Amplitude [a.u.]",i);
      //       hPwbTimeRange[i] = new TH2D(hname.Data(),htitle.Data(),600,0.,6000.,300,0.,3000.);
      //    }
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->cd("paddeconv");
      gDirectory->mkdir("pwbtime")->cd();
      for(int t=0; t<10; ++t)
         {
            TString hname = TString::Format("hPwbTimeAmp_prox%d",t);
            TString htitle = TString::Format("Average Maximum WF Amplitude Vs Channel for t~%d us;Pad;PWB",t);
            hPwbTimeAmp_prox[t] = new TProfile(hname.Data(),htitle.Data(),32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,0.,5000.);
            hPwbTimeAmp_prox[t]->SetMinimum(0.);

            hname = TString::Format("hPwbTimeColRowAmp%d",t);
            htitle = TString::Format("Maximum WF Amplitude Vs Pad for t~%d us;row;sec;N",t);
            hPwbTimeColRowAmp[t] = new TH2D(hname.Data(),htitle.Data(),576,0.,576.,32,0.,32.);
            
            hname = TString::Format("hPwbTimeRange_prox%d",t);
            htitle = TString::Format("Average WF Range  Vs Channel for t~%d us;Pad;PWB",t);
            hPwbTimeRange_prox[t] = new TProfile(hname.Data(),htitle.Data(),32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,0.,5000.);
            hPwbTimeRange_prox[t]->SetMinimum(0.);

            hname = TString::Format("hPwbTimeColRowRange%d",t);
            htitle = TString::Format("WF Range Vs Pad for t~%d us;row;sec;N",t);
            hPwbTimeColRowRange[t] = new TH2D(hname.Data(),htitle.Data(),576,0.,576.,32,0.,32.);
         }

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      // match
      if( !gDirectory->cd("match_el") )
         gDirectory->mkdir("match_el")->cd();
      gDirectory->pwd();

      hawcol = new TH2D("hawcol",
                        "Match Electrodes;AW;PAD COL",
                        256,0.,256.,32,0.,32.);

      hawcol_time = new TH2D("hawcol_time","AW vs PAD Time;AW [ns];PAD [ns]",375,0.,6000.,375,0.,6000.);
      hawcol_sector_time = new TH2D("hawcol_sector_time","AW vs PAD Time with Matching Sector;AW [ns];PAD [ns]",375,0.,6000.,375,0.,6000.);
      hawcol_deltat_sec = new TH2D("hawcol_deltat_sec","AW vs PAD col with Matching Time;AW;PAD COL",256,0.,256.,32,0.,32.);
      
      hawcol_match = new TH2D("hawcol_match",
                              "Match Electrodes Time && Sector Cut;AW;PAD COL",
                              256,0.,256.,32,0.,32.);
      hawcol_match_amp = new TH2D("hawcol_match_amp",
                                  "Amplitude of Matching Electrodes Time && Sector Cut;AW;PAD COL",
                                  200,0.,2000.,200,0.,10000.);   
      hawcol_match_time = new TH2D("hawcol_match_time",
                                   "Time of Matching Electrodes Time && Sector Cut;AW [ns];PAD [ns]",
                                   375,0.,6000.,375,0.,6000.);  

      hamprow_timecolcut = new TH2D("hamprow_timecolcut",
                                    "Pad Amplitude By Row - Matched Electrodes by Time && Sector Cut;PAD ROW",
                                    576,0.,576.,300,0.,6000.);

      hawamp_match_aw_amp = new TH2D("hawamp_match_aw_amp",
                                     "AW amplitude vs Pad Row and AW number;PAD ROW;AW;AW AMP",
                                     576,0.,576.,256,0.,256.);
      hawamp_match_amp = new TH2D("hawamp_match_amp",
                                  "AW amplitude vs Pad Row;PAD ROW;AW AMP",
                                  576,0.,576.,200,0.,2000.);
      hawamp_match_aw = new TH2D("hawamp_match_aw",
                                 "AW hit vs Pad Row and AW number;PAD ROW;AW",
                                 576,0.,576.,256,0.,256.);
      hawamp_match_amp_pc = new TH2D("hawamp_match_amp_pc",
                                     "AW amplitude vs Pad Row in the Proportional Region;PAD ROW;AW AMP",
                                     576,0.,576.,200,0.,2000.);

      hNmatch = new TH1D("hNmatch","Number of AW*PAD matches",500,0.,5000.);

      // Matching AW
      hAwOcc_match = new TH1D("hAwOcc_match","Occupancy per AW Top Matching",256,0.,256.);
      hAwOcc_match->SetMinimum(0.);
      hAwOccSec_match = new TH1D("hAwOccSec_match","Number of TOP AW Matching hits per Pad Sector;N",32,0.,32.);
      hAwOccSec_match->SetMinimum(0.);
      hAwOccIsec_match = new TH1D("hAwOccIsec_match","Number of TOP AW Matching hits Inside Pad Sector;N",8,0.,8.);
      hAwOccIsec_match->SetMinimum(0.);
      // Matching PAD
      hOccPad_match = new TH2D("hOccPad_match","Number of Hits Pads Matching;row;sec;N",576,0.,576.,32,0.,32.);

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      if( !gDirectory->cd("sigpoints") )
         gDirectory->mkdir("sigpoints")->cd();
      gDirectory->pwd();
      
      hNsp = new TH1D("hNsigsp","Number of Signal Spacepoints",500,0.,5000.);

      hAWspAmp = new TH1D("hAwspAmp","Reconstructed Avalanche Size Top AW W/ Spacepoint",200,0.,2000.);
      hAWspAmp->SetMinimum(0.);
      hAWspTime = new TH1D("hAwspTime","Reconstructed Avalanche Time Top AW W/ Spacepoint",375,0.,6000.);
      hAwspOcc = new TH1D("hAwspOcc","Occupancy per AW Top W/ Spacepoint",256,0.,256.);
      hAwspOcc->SetMinimum(0.);
      hAwspOccSec = new TH1D("hAwspOccSec","Number of TOP AW hits per Pad Sector W/ Spacepoint;N",32,0.,32.);
      hAwspOccSec->SetMinimum(0.);
      hAwspOccIsec = new TH1D("hAwspOccIsec","Number of TOP AW hits Inside Pad Sector W/ Spacepoint;N",8,0.,8.);
      hAwspOccIsec->SetMinimum(0.);
      hAWspTimeAmp = new TH2D("hAWspTimeAmp","Reconstructed Avalanche Time Vs Size - Top W/ Spacepoint",
                              60,0.,6000.,50,0.,2000.);

      hOccSpPad = new TH2D("hOccSpPad","Number of Hits Pads W/ Spacepoint;row;sec;N",576,0.,576.,32,0.,32.);
      hOccSpRow = new TH1D("hOccSpPadRow","Number of Hits Pad Rows W/ Spacepoint;N",576,0.,576.);
      hOccSpRow->SetMinimum(0.);
      hOccSpCol = new TH1D("hOccSpPadSec","Number of Hits Pad Cols W/ Spacepoint;N",32,0.,32.);
      hOccSpCol->SetMinimum(0.);

      hAmpSpPad = new TH1D("hAmpSpPad","Reconstructed Avalanche Size Pad W/ Spacepoint",200,0.,10000.);
      hAmpSpPad->SetMinimum(0.);
      hTimeSpPad = new TH1D("hTimeSpPad","Reconstructed Avalanche Time Pad W/ Spacepoint",375,0.,6000.);
      hTimeAmpSpPad = new TH2D("hTimeAmpSpPad","Reconstructed Avalanche Time Vs Size - Pad W/ Spacepoint",
                               300,0.,6000.,100,0.,5100.);

      hTimeSpPadCol = new TH2D("hTimeSpPadSec","Reconstructed Avalanche Time Vs Pad Secs W/ Spacepoint;sec;time [ns]",
                               32,0.,32.,40,0.,6000.);
      hTimeSpPadRow = new TH2D("hTimeSpPadRow","Reconstructed Avalanche Time Vs Pad Rows; W/ Spacepointrow;time [ns]",
                               576,0.,576,40,0.,6000.);
      hAmpSpPadCol = new TH2D("hAmpSpPadSec","Reconstructed Avalanche Size Vs Pad Secs W/ Spacepoint;sec;amp",
                              32,0.,32.,500,0.,5000.);
      hAmpSpPadRow = new TH2D("hAmpSpPadRow","Reconstructed Avalanche Size Vs Pad Rows W/ Spacepoint;row;amp",
                              576,0.,576,500,0.,5000.);
      
      pmap = new padmap;
   }

   void EndRun(TARunInfo* runinfo)
   {
      if(!diagnostics) return;
      delete pmap;
      printf("HistoModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
      // pwbmap.close();
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {      
      if(!diagnostics) return flow;

      const AgEventFlow* ef = flow->Find<AgEventFlow>();
     
      if (!ef || !ef->fEvent)
         return flow;
     
      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) return flow;

      if( fTrace )
         {
            if( SigFlow->adc32max )
               printf("HistoModule::Analyze, ADC # signals %d\n", 
                      int(SigFlow->adc32max->size()));
            else
               printf("HistoModule::Analyze, NO ADC signals\n");

            if( SigFlow->pwbMax )
               printf("HistoModule::Analyze, PWB # signals %d\n", 
                      int(SigFlow->pwbMax->size()));
            else
               printf("HistoModule::Analyze, NO PWB signals\n");

            if( SigFlow->awSig )
               printf("HistoModule::Analyze, AW # signals %d\n", 
                      int(SigFlow->awSig->size()));
            else
               printf("HistoModule::Analyze, NO AW signals\n");

            if( SigFlow->pdSig )
               printf("HistoModule::Analyze, PAD # signals %d\n",
                      int(SigFlow->pdSig->size()));
            else
               printf("HistoModule::Analyze, NO PAD signals\n");
            
            if( SigFlow->matchSig )
               printf("HistoModule::Analyze, SP # %d\n",
                      int(SigFlow->matchSig->size()));
            else
               printf("HistoModule::Analyze, NO SP matches\n");
         }

      // if( !SigFlow->awSig ) return flow;
      // if( SigFlow->awSig->size() == 0 ) return flow;
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif   

      if( SigFlow->adc32max )
         {

            if( fTrace )
               printf("HistoModule::AnalyzeFlowEvent, ADC Diagnostic start\n");
            //      ADCdiagnostic(&SigFlow->adc32max,&SigFlow->adc32range);
            ADCdiagnostic(SigFlow->adc32max);
         }

      if( SigFlow->pwbMax )
         {
            if( fTrace )
               printf("HistoModule::AnalyzeFlowEvent, PWB Diagnostic start\n");
            //      PWBdiagnostic(&SigFlow->pwbMax,&SigFlow->pwbRange);
            PWBdiagnostic(SigFlow->pwbMax);
         }

      if( fTrace )
         printf("HistoModule::AnalyzeFlowEvent, Analysis Diagnostic start\n");

      if( SigFlow->awSig )
         AWdiagnostic(SigFlow->awSig);

      if( SigFlow->pdSig )
         PADdiagnostic(SigFlow->pdSig);
      
      if( SigFlow->pdSig && SigFlow->awSig )
         MatchDiagnostic(SigFlow->awSig,SigFlow->pdSig);
         
      if( SigFlow->matchSig )
         SigSpacePointsDiagnostic( SigFlow->matchSig );

      ++fCounter;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"histo_module",timer_start);
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   void AWdiagnostic(std::vector<signal> *sanode)
   {
      int nbot=0,ntop=0;
      for( auto iSig=sanode->begin(); iSig!=sanode->end(); ++iSig )
         { 
            if( iSig->sec )
               {
                  hOccBot->Fill(iSig->idx);
                  hAmpBot->Fill(iSig->height);
                  hTimeBot->Fill(iSig->t);
                  hTimeAmpBot->Fill(iSig->t,iSig->height);
                  hTimeBotChan->Fill(iSig->idx,iSig->t);
                  hAmpBotChan->Fill(iSig->idx,iSig->height);
                  ++nbot;
               }
            else
               {
                  hOccTop->Fill(iSig->idx);
                  hAmpTop->Fill(iSig->height);
                  hErrTop->Fill(iSig->errh);
                  hTimeTop->Fill(iSig->t);
                  hTimeAmpTop->Fill(iSig->t,iSig->height);
                  hTimeTopChan->Fill(iSig->idx,iSig->t);
                  hAmpTopChan->Fill(iSig->idx,iSig->height);

                  hAwOccSec->Fill(iSig->idx/8);
                  hAwOccIsec->Fill(iSig->idx%8);

                  ++ntop;
               }
         }
      hNhitBot->Fill(nbot);
      hNhitTop->Fill(ntop);
      if( fTrace )
      std::cout<<"HistoModule::AWdiagnostic # hit top: "<<ntop<<" bot: "<<nbot<<std::endl;
   }

   void ADCdiagnostic(std::vector<signal> *wfamp/*, std::vector<signal> *wfrange*/)
   {
      if( wfamp->size() > 0 )
         {
            for( auto sig = wfamp->begin(); sig!=wfamp->end(); ++sig )
               {
                  hAdcAmp->Fill(sig->idx,sig->height);
                  hAdcAmp_prox->Fill(sig->idx,sig->height);
                  hAdcTimeAmp[sig->idx]->Fill(sig->t,sig->height);
                  hAdcWfAmp->Fill(sig->height);
               }
         }
      // if( wfrange->size() > 0 )
      //    {
      //       for( auto sig = wfrange->begin(); sig!=wfrange->end(); ++sig )
      //          {
      //             hAdcRange->Fill(sig->idx,sig->height);
      //             hAdcRange_prox->Fill(sig->idx,sig->height);
      //             hAdcTimeRange[sig->idx]->Fill(sig->t,sig->height);
      //             hAdcWfRange->Fill(sig->height);
      //          }
      //    }
   }

   void PWBdiagnostic(std::vector<signal> *wfamp/*, std::vector<signal> *wfrange*/)
   {
      if( wfamp->size() > 0 )
         {
            for( auto sig = wfamp->begin(); sig!=wfamp->end(); ++sig )
               {
                  double pad_index = double(pmap->index(sig->sec,sig->idx));
                  hPwbAmp->Fill(pad_index,sig->height);
                  hPwbAmp_prox->Fill(pad_index,sig->height);
                  //                  hPwbTimeAmp[pad_index]->Fill(sig->t,sig->height);
                  
                  hPwbWfAmp->Fill(sig->height);
   
                  int time = int(1.e-3*sig->t-1.6);
                  if( time < 0 ) continue;
                  //std::cout<<"HistoModule::PWBdiagnostic amp time: "<<time<<" us\tsig time: "<<sig->t<<" ns"<<std::endl;
                  if( time >= 10 )
                     {
                        std::cerr<<"HistoModule::PWBdiagnostic ERROR amp time: "<<time<<" us"<<std::endl;
                        continue;
                     }
                  hPwbTimeAmp_prox[time]->Fill(pad_index,sig->height);
                  hPwbTimeColRowAmp[time]->Fill(sig->idx,sig->sec,sig->height);
               }
         }
      // if( wfrange->size() > 0 )
      //    {
      //       for( auto sig = wfrange->begin(); sig!=wfrange->end(); ++sig )
      //          {
      //             double pad_index = double(pmap->index(sig->sec,sig->idx));
      //             hPwbRange->Fill(pad_index,sig->height);
      //             hPwbRange_prox->Fill(pad_index,sig->height);
      //             //                  hPwbTimeRange[pad_index]->Fill(sig->t,sig->height);

      //             hPwbWfRange->Fill(sig->height);

      //             int time = int(1.e-3*sig->t-1.6);
      //             if( time < 0 ) continue;
      //             if( time >= 10 )
      //                {
      //                   std::cerr<<"HistoModule::PWBdiagnostic ERROR range time: "<<time<<" us"<<std::endl;
      //                   continue;
      //                }
      //             hPwbTimeRange_prox[time]->Fill(pad_index,sig->height);
      //             hPwbTimeColRowRange[time]->Fill(sig->idx,sig->sec,sig->height);
      //          }
      //    }
   }

   void PADdiagnostic(std::vector<signal> *spad)
   {
      int nhit=0;
      //std::cout<<"HistoModule::PADdiagnostic()"<<std::endl;
      for( auto iSig=spad->begin(); iSig!=spad->end(); ++iSig )
         { 
            hOccCol->Fill(iSig->sec);
            hOccRow->Fill(iSig->idx);
            hOccPad->Fill(iSig->idx,iSig->sec);
            hAmpPad->Fill(iSig->height);
            hErrPad->Fill(iSig->errh);
            hTimePad->Fill(iSig->t);
            hTimeAmpPad->Fill(iSig->t,iSig->height);

            hTimePadCol->Fill(iSig->sec,iSig->t);
            hAmpPadCol->Fill(iSig->sec,iSig->height);
            hTimePadRow->Fill(iSig->idx,iSig->t);
            hAmpPadRow->Fill(iSig->idx,iSig->height);
            ++nhit;
            // std::cout<<"\t"<<nhit<<" "<<iSig->sec<<" "<<iSig->i<<" "<<iSig->height<<" "<<iSig->t<<std::endl;
         }
      hNhitPad->Fill(nhit);
      if( fTrace )
      std::cout<<"HistoModule::PADdiagnostic # hit: "<<nhit<<std::endl;
   }

   void MatchDiagnostic(std::vector<signal>* awsignals, 
                        std::vector<signal>* padsignals)
   {
      std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(), 
                                                         awsignals->end());
      std::multiset<signal, signal::timeorder> pad_bytime(padsignals->begin(), 
                                                          padsignals->end());
      int Nmatch=0;
      for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
         {
            short sector = short(iaw->idx/8);
            if( fTrace )
               std::cout<<"HistoModule::Match aw: "<<iaw->idx
                        <<" t: "<<iaw->t<<" pad sector: "<<sector<<std::endl;
            for( auto ipd=pad_bytime.begin(); ipd!=pad_bytime.end(); ++ipd )
               {
                  bool tmatch=false;
                  bool pmatch=false;

                  hawcol->Fill(iaw->idx,ipd->sec);
                  hawcol_time->Fill( iaw->t , ipd->t );

                  double delta = fabs( iaw->t - ipd->t );
                  if( delta < fCoincTime ) 
                     {
                        tmatch=true;
                        hawcol_deltat_sec->Fill(iaw->idx,ipd->sec);
                     }
                  
                  if( sector == ipd->sec ) 
                     {
                        pmatch=true;
                        hawcol_sector_time->Fill( iaw->t , ipd->t );
                     }

                  if( tmatch && pmatch ) 
                     {
                        hawcol_match->Fill(iaw->idx,ipd->sec);
                        hawcol_match_amp->Fill(iaw->height,ipd->height);
                        hawcol_match_time->Fill(iaw->t,ipd->t);
                        hamprow_timecolcut->Fill(ipd->idx,ipd->height);

                        hawamp_match_aw_amp->Fill(ipd->idx,iaw->idx,iaw->height);
                        hawamp_match_amp->Fill(ipd->idx,iaw->height);
                        hawamp_match_aw->Fill(ipd->idx,iaw->idx);

                        if( iaw->t < fpc_timecut )
                           hawamp_match_amp_pc->Fill(ipd->idx,iaw->height);

                        hAwOcc_match->Fill(iaw->idx);
                        hAwOccSec_match->Fill(sector);
                        hAwOccIsec_match->Fill(iaw->idx%8);

                        hOccPad_match->Fill(ipd->idx,ipd->sec);

                        ++Nmatch;
                        if( fTrace )
                           std::cout<<"\t"<<Nmatch<<")  pad col: "<<ipd->sec<<" pad row: "<<ipd->idx<<std::endl;
                     }
               }
         }
      if( fTrace )
         std::cout<<"HistoModule::Match Number of Matches: "<<Nmatch<<std::endl;
      if( Nmatch ) hNmatch->Fill( double(Nmatch) );
   }

   void SigSpacePointsDiagnostic( std::vector< std::pair<signal,signal> >* sp )
   {
      hNsp->Fill(sp->size());

      for(auto& ip: *sp )
         {
            hAWspAmp->Fill(ip.first.height);
            hAWspTime->Fill(ip.first.t);
            hAwspOcc->Fill(ip.first.idx);
            hAwspOccSec->Fill(ip.first.idx/8);
            hAwspOccIsec->Fill(ip.first.idx%8);
            hAWspTimeAmp->Fill(ip.first.t,ip.first.height);
            
            hOccSpPad->Fill(ip.second.idx,ip.second.sec);
            hOccSpRow->Fill(ip.second.idx);
            hOccSpCol->Fill(ip.second.sec);
            
            hAmpSpPad->Fill(ip.second.height);
            hTimeSpPad->Fill(ip.second.t);
            hTimeAmpSpPad->Fill(ip.second.t,ip.second.height);
            
            hTimeSpPadCol->Fill(ip.second.sec,ip.second.t);
            hTimeSpPadRow->Fill(ip.second.idx,ip.second.t);
            hAmpSpPadCol->Fill(ip.second.sec,ip.second.height);
            hAmpSpPadRow->Fill(ip.second.idx,ip.second.height);
         }
   }

};


class HistoModuleFactory: public TAFactory
{
public:
   HistoFlags fFlags;
   
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("HistoModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if( args[i] == "--diag" )
            fFlags.fDiag = true;
      }
   }

   void Finish()
   {
      printf("HistoModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("HistoModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HistoModule(runinfo, &fFlags);
   }
};

static TARegister tar(new HistoModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
