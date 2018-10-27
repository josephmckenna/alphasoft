#include "AgFlow.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

#include "SignalsType.h"
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
   TH1D* hTimeBot;
   TH1D* hTimeTop;

   TH2D* hTimeAmpBot;
   TH2D* hTimeAmpTop;

   TH2D* hTimeBotChan;
   TH2D* hTimeTopChan;
   TH2D* hAmpBotChan;
   TH2D* hAmpTopChan;

   // adc wf study
   TH2D* hAdcAmp;
   TProfile* hAdcAmp_prox;
   std::map<int,TH2D*> hAdcTimeAmp;
   TH2D* hAdcRange;
   TProfile* hAdcRange_prox;
   std::map<int,TH2D*> hAdcTimeRange;

   // pads
   TH1D* hNhitPad;
   TH1D* hOccRow;
   TH1D* hOccCol;
   TH2D* hOccPad;
   TH1D* hAvgRMSPad;

   TH1D* hAmpPad;
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

public:
   HistoModule(TARunInfo* runinfo, HistoFlags* f):TARunObject(runinfo),
                                                  fFlags(f),fCounter(0),
                                                  fCoincTime(16.),fpc_timecut(300.) // ns

   {
      diagnostics=f->fDiag;
   }

   ~HistoModule(){}

   void BeginRun(TARunInfo* runinfo)
   {
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
      // hTimeBot = new TH1D("hTimeBot","Reconstructed Avalanche Time Bottom",375,0.,6000.);
      // hTimeTop = new TH1D("hTimeTop","Reconstructed Avalanche Time Top",375,0.,6000.);
      hTimeBot = new TH1D("hTimeBot","Reconstructed Avalanche Time Bottom",375,0.,6000.);
      hTimeTop = new TH1D("hTimeTop","Reconstructed Avalanche Time Top",375,0.,6000.);
      hTimeAmpBot = new TH2D("hTimeAmpBot","Reconstructed Avalanche Time Vs Size - Bottom",60,0.,6000.,50,0.,2000.);
      hTimeAmpTop = new TH2D("hTimeAmpTop","Reconstructed Avalanche Time Vs Size - Top",60,0.,6000.,50,0.,2000.);

      hTimeBotChan = new TH2D("hTimeBotChan","Reconstructed Avalanche Time Vs Bottom Channel",256,0.,256.,37,0.,6000.);
      hTimeTopChan = new TH2D("hTimeTopChan","Reconstructed Avalanche Time Vs Top Channel",256,0.,256.,37,0.,6000.);
      hAmpBotChan = new TH2D("hAmpBotChan","Reconstructed Avalanche Size Vs Bottom Channel",256,0.,256.,500,0.,2000.);
      hAmpTopChan = new TH2D("hAmpTopChan","Reconstructed Avalanche Size Vs Top Channel",256,0.,256.,500,0.,2000.);

      hAdcAmp = new TH2D("hAdcAmp","Maximum WF Amplitude Vs Channel",256,0.,256.,1000,0.,3000.);
      hAdcAmp_prox = new TProfile("hAdcAmp_prox","Average Maximum WF Amplitude Vs Channel;AW;ADC",
                                  256,0.,256.,0.,5000.);
      hAdcAmp_prox->SetMinimum(0.);
      hAdcRange = new TH2D("hAdcRange","WF Range Vs Channel",256,0.,256.,1000,0.,3000.);
      hAdcRange_prox = new TProfile("hAdcRange_prox","Average WF Range Vs Channel;AW;ADC",
                                    256,0.,256.,0.,5000.);
      hAdcRange_prox->SetMinimum(0.);
      gDirectory->mkdir("adc32")->cd();
      for( int i=0; i<256; ++i)
         {
            TString hname = TString::Format("hadcampch%03d",i);
            TString htitle = TString::Format("Maximum WF Amplitude Vs Time AW: %d;Time [ns];Amplitude [a.u.]",i);
            hAdcTimeAmp[i] = new TH2D(hname.Data(),htitle.Data(),600,0.,6000.,300,0.,3000.);
         }
      for( int i=0; i<256; ++i)
         {
            TString hname = TString::Format("hadcrangech%03d",i);
            TString htitle = TString::Format("Maximum WF Amplitude Vs Time AW: %d;Time [ns];Amplitude [a.u.]",i);
            hAdcTimeRange[i] = new TH2D(hname.Data(),htitle.Data(),600,0.,6000.,300,0.,3000.);
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
      hOccPad = new TH2D("hOccPad","Number of Hits Pads;N",576,0.,576.,32,0.,32.);

      hAmpPad = new TH1D("hAmpPad","Reconstructed Avalanche Size Pad",200,0.,10000.);
      hTimePad = new TH1D("hTimePad","Reconstructed Avalanche Time Pad",375,0.,6000.);

      hTimeAmpPad = new TH2D("hTimeAmpPad","Reconstructed Avalanche Time Vs Size - Pad",40,0.,6000.,20,0.,10000.);
      
      hTimePadCol = new TH2D("hTimePadCol","Reconstructed Avalanche Time Vs Pad Cols",32,0.,32.,40,0.,6000.);
      hTimePadRow = new TH2D("hTimePadRow","Reconstructed Avalanche Time Vs Pad Rows",576,0.,576,40,0.,6000.);
      hAmpPadCol = new TH2D("hAmpPadCol","Reconstructed Avalanche Size Vs Pad Cols",32,0.,32.,20,0.,10000.);
      hAmpPadRow = new TH2D("hAmpPadRow","Reconstructed Avalanche Size Vs Pad Rows",576,0.,576,20,0.,10000.);

      hPwbAmp = new TH2D("hPwbAmp","Maximum WF Amplitude Vs Channel",32*576,0.,_padcol*_padrow,1000,0.,3000.);
      hPwbAmp_prox = new TProfile("hPwbAmp_prox","Average Maximum WF Amplitude Vs Channel;Pad;PWB",
                                  32*576,0.,_padcol*_padrow,0.,5000.);
      hPwbAmp_prox->SetMinimum(0.);
      hPwbRange = new TH2D("hPwbRange","WF Range Vs Channel",32*576,0.,_padcol*_padrow,1000,0.,3000.);
      hPwbRange_prox = new TProfile("hPwbRange_prox","Average WF Range Vs Channel;Pad;PWB",
                                    32*576,0.,_padcol*_padrow,0.,5000.);
      hPwbRange_prox->SetMinimum(0.);
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

   }

   void EndRun(TARunInfo* runinfo)
   {
      if(!diagnostics) return;
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
            printf("HistoModule::Analyze, AW # signals %d\n", 
                   int(SigFlow->awSig.size()));
            printf("HistoModule::Analyze, PAD # signals %d\n",
                   int(SigFlow->pdSig.size()));
            printf("HistoModule::Analyze, SP # %d\n",
                   int(SigFlow->matchSig.size()));
         }

      if( SigFlow->awSig.size() == 0 ) return flow;

      AWdiagnostic(&SigFlow->awSig);

      ADCdiagnostic(&SigFlow->adc32max,&SigFlow->adc32range);

      if( SigFlow->pdSig.size() > 0 )
         {
            
            PADdiagnostic(&SigFlow->pdSig);
            
            MatchDiagnostic(&SigFlow->awSig,&SigFlow->pdSig);

         }

      PWBdiagnostic(&SigFlow->pwbMax,&SigFlow->pwbRange);

      ++fCounter;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"histo_module");
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
                  hTimeTop->Fill(iSig->t);
                  hTimeAmpTop->Fill(iSig->t,iSig->height);
                  hTimeTopChan->Fill(iSig->idx,iSig->t);
                  hAmpTopChan->Fill(iSig->idx,iSig->height);
                  ++ntop;
               }
         }
      hNhitBot->Fill(nbot);
      hNhitTop->Fill(ntop);
      if( fTrace )
      std::cout<<"HistoModule::AWdiagnostic # hit top: "<<ntop<<" bot: "<<nbot<<std::endl;
   }

   void ADCdiagnostic(std::vector<signal> *wfamp, std::vector<signal> *wfrange)
   {
      if( wfamp->size() > 0 )
         {
            for( auto sig = wfamp->begin(); sig!=wfamp->end(); ++sig )
               {
                  hAdcAmp->Fill(sig->idx,sig->height);
                  hAdcAmp_prox->Fill(sig->idx,sig->height);
                  hAdcTimeAmp[sig->idx]->Fill(sig->t,sig->height);
               }
         }
      if( wfrange->size() > 0 )
         {
            for( auto sig = wfrange->begin(); sig!=wfrange->end(); ++sig )
               {
                  hAdcRange->Fill(sig->idx,sig->height);
                  hAdcRange_prox->Fill(sig->idx,sig->height);
                  hAdcTimeRange[sig->idx]->Fill(sig->t,sig->height);
               }
         }
   }

  void PWBdiagnostic(std::vector<signal> *wfamp, std::vector<signal> *wfrange)
   {
      if( wfamp->size() > 0 )
         {
            for( auto sig = wfamp->begin(); sig!=wfamp->end(); ++sig )
               {
                  double pad_index = double(sig->sec) + _padcol * double(sig->idx);
                  hPwbAmp->Fill(pad_index,sig->height);
                  hPwbAmp_prox->Fill(pad_index,sig->height);
                  //                  hPwbTimeAmp[pad_index]->Fill(sig->t,sig->height);
               }
         }
      if( wfrange->size() > 0 )
         {
            for( auto sig = wfrange->begin(); sig!=wfrange->end(); ++sig )
               {
                  double pad_index = double(sig->sec) + _padcol * double(sig->idx);
                  hPwbRange->Fill(pad_index,sig->height);
                  hPwbRange_prox->Fill(pad_index,sig->height);
                  //                  hPwbTimeRange[pad_index]->Fill(sig->t,sig->height);
               }
         }
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
