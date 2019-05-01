#include <iostream>
#include <vector>
#include <map>
#include <cassert>

#include <TH1D.h>
#include <TH2D.h>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

class tdcmodule: public TARunObject
{
private:
   bool fTrace = false;
   bool fDebug = false;

   std::vector<TH1D> fhCoarseTime_0;
   std::vector<TH1D> fhFineTime_0;
   std::vector<TH1D> fhFinalTime_0;
   TH1D* fhTrigCoarseTime_fpga1_0;
   TH1D* fhTrigFineTime_fpga1_0;
   TH1D* fhTrigFinalTime_fpga1_0;
   TH1D* fhTrigCoarseTime_fpga2_0;
   TH1D* fhTrigFineTime_fpga2_0;
   TH1D* fhTrigFinalTime_fpga2_0;
   TH1D* fhTrigCoarseTime_fpga3_0;
   TH1D* fhTrigFineTime_fpga3_0;
   TH1D* fhTrigFinalTime_fpga3_0;
   std::vector<TH1D> fhCoarseTime_diff_0;
   std::vector<TH1D> fhFineTime_diff_0;
   std::vector<TH1D> fhFinalTime_diff_0;

   std::vector<TH1D> fhCoarseTime_1;
   std::vector<TH1D> fhFineTime_1;
   std::vector<TH1D> fhFinalTime_1;
   TH1D* fhTrigCoarseTime_fpga1_1;
   TH1D* fhTrigFineTime_fpga1_1;
   TH1D* fhTrigFinalTime_fpga1_1;
   TH1D* fhTrigCoarseTime_fpga2_1;
   TH1D* fhTrigFineTime_fpga2_1;
   TH1D* fhTrigFinalTime_fpga2_1;
   TH1D* fhTrigCoarseTime_fpga3_1;
   TH1D* fhTrigFineTime_fpga3_1;
   TH1D* fhTrigFinalTime_fpga3_1;
   std::vector<TH1D> fhCoarseTime_diff_1;
   std::vector<TH1D> fhFineTime_diff_1;
   std::vector<TH1D> fhFinalTime_diff_1;

   TH1D* hNhits;
   TH1D* hNuniqHits;

   TH1D* hOcc_fpga1;
   TH1D* hOcc_fpga2;
   TH1D* hOcc_fpga3;

   TH1D* hOcc;
   TH2D* hOcc_0;
   TH2D* hOcc_1;

   TH2D* hOcc_diff_1;
   TH2D* hOcc_delta_1;

   const unsigned fNfpga = 3;
   const unsigned ffpga[3]={1,2,3};
   const unsigned fNch[3]={32,48,48};

   // https://daq.triumf.ca/elog-alphag/alphag/1961
   const double epoch_freq = 97656.25; // 200MHz/(2<<11);
   const double coarse_freq = 200.0e6; // 200MHz  
  
   // linear calibration:
   // $ROOTANASYS/libAnalyzer/TRB3Decoder.hxx
   const double trb3LinearLowEnd = 17.0; 
   const double trb3LinearHighEnd = 473.0; 

public:

   tdcmodule(TARunInfo* runinfo): TARunObject(runinfo)
   {
      printf("tdcmodule::ctor!\n");
   }

   ~tdcmodule() 
   {
      printf("tdcmodule::dtor!\n");
   }
  
   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      TDirectory* dir = gDirectory->mkdir("tdc");
      dir->cd(); // select correct ROOT directory

      hNhits = new TH1D("hNhits","Number of Hits;# of Hits",100,0.,100.);
      hNuniqHits = new TH1D("hNuniqHits","Number of Unique Hits;# of Hits",100,0.,100.);
    
      hOcc = new TH1D("hOcc","Channels Occupancy",128,-0.5,127.5);
      hOcc_fpga1 = new TH1D("hOcc_fpga1","Channels Occupancy FPGA 1",65,-0.5,64.5);
      hOcc_fpga2 = new TH1D("hOcc_fpga2","Channels Occupancy FPGA 2",65,-0.5,64.5);
      hOcc_fpga3 = new TH1D("hOcc_fpga3","Channels Occupancy FPGA 3",65,-0.5,64.5);
  
      const int NbinsCoarseTime = 2000;
      const double MaxCoarseTime = 1.e4;     
      const int NbinsFineTime = 512;
      const double MaxFineTime = 512.;
      const int NbinsFinalTime = 10000;
      const double MaxFinalTime = 1.e7;

      hOcc_0 = new TH2D("hOcc_0","Channels Occupancy re0 and Final Time;Channel;Time [ps]",
                        128,-0.5,127.5,1000,0.,MaxFinalTime);
      hOcc_1 = new TH2D("hOcc_1","Channels Occupancy re1 and Final Time;Channel;Time [ps]",
                        128,-0.5,127.5,1000,0.,MaxFinalTime);

      hOcc_diff_1 = new TH2D("hOcc_diff_1","Channels Occupancy re1 and Final Time Diff With Trigger;Channel;Time [ps]",
                             128,-0.5,127.5,1000,0.,MaxFinalTime);
      hOcc_delta_1 = new TH2D("hOcc_delta_1","Channels Occupancy re1 and Final Time Delta With First Hit;Channel;Time [ps]",
                              128,-0.5,127.5,1000,0.,MaxFinalTime);

      fhTrigCoarseTime_fpga1_0 = new TH1D("hTrigCoarseTime_fpga1_0","Trigger Coarse Time FPGA 1 re0;Hit time [ns]",
                                          NbinsCoarseTime,0.,MaxCoarseTime);
      fhTrigFineTime_fpga1_0 = new TH1D("hTrigFineTime_fpga1_0","Trigger Fine Time FPGA 1 re0",
                                        NbinsFineTime,0.,MaxFineTime);
      fhTrigFinalTime_fpga1_0 = new TH1D("hTrigFinalTime_fpga1_0","Trigger Final Time FPGA 1 re0;Hit time [ps]",
                                         NbinsFinalTime,0.,MaxFinalTime);

      fhTrigCoarseTime_fpga2_0 = new TH1D("hTrigCoarseTime_fpga2_0","Trigger Coarse Time FPGA 2 re0;Hit time [ns]",
                                          NbinsCoarseTime,0.,MaxCoarseTime);
      fhTrigFineTime_fpga2_0 = new TH1D("hTrigFineTime_fpga2_0","Trigger Fine Time FPGA 2 re0",
                                        NbinsFineTime,0.,MaxFineTime);
      fhTrigFinalTime_fpga2_0 = new TH1D("hTrigFinalTime_fpga2_0","Trigger Final Time FPGA 2 re0;Hit time [ps]",
                                         NbinsFinalTime,0.,MaxFinalTime);
      
      fhTrigCoarseTime_fpga3_0 = new TH1D("hTrigCoarseTime_fpga3_0","Trigger Coarse Time FPGA 3 re0;Hit time [ns]",
                                          NbinsCoarseTime,0.,MaxCoarseTime);
      fhTrigFineTime_fpga3_0 = new TH1D("hTrigFineTime_fpga3_0","Trigger Fine Time FPGA 3 re0",
                                        NbinsFineTime,0.,MaxFineTime);
      fhTrigFinalTime_fpga3_0 = new TH1D("hTrigFinalTime_fpga3_0","Trigger Final Time FPGA 3 re0;Hit time [ps]",
                                         NbinsFinalTime,0.,MaxFinalTime);

      fhTrigCoarseTime_fpga1_1 = new TH1D("hTrigCoarseTime_fpga1_1","Trigger Coarse Time FPGA 1 re1;Hit time [ns]",
                                          NbinsCoarseTime,0.,MaxCoarseTime);
      fhTrigFineTime_fpga1_1 = new TH1D("hTrigFineTime_fpga1_1","Trigger Fine Time FPGA 1 re1",
                                        NbinsFineTime,0.,MaxFineTime);
      fhTrigFinalTime_fpga1_1 = new TH1D("hTrigFinalTime_fpga1_1","Trigger Final Time FPGA1 re1;Hit time [ps]",
                                         NbinsFinalTime,0.,MaxFinalTime);

      fhTrigCoarseTime_fpga2_1 = new TH1D("hTrigCoarseTime_fpga2_1","Trigger Coarse Time FPGA 2 re1;Hit time [ns]",
                                          NbinsCoarseTime,0.,MaxCoarseTime);
      fhTrigFineTime_fpga2_1 = new TH1D("hTrigFineTime_fpga2_1","Trigger Fine Time FPGA 2 re1",
                                        NbinsFineTime,0.,MaxFineTime);
      fhTrigFinalTime_fpga2_1 = new TH1D("hTrigFinalTime_fpga2_1","Trigger Final Time FPGA2 re1;Hit time [ps]",
                                         NbinsFinalTime,0.,MaxFinalTime);
      
      fhTrigCoarseTime_fpga3_1 = new TH1D("hTrigCoarseTime_fpga3_1","Trigger Coarse Time FPGA 3 re1;Hit time [ns]",
                                          NbinsCoarseTime,0.,MaxCoarseTime);
      fhTrigFineTime_fpga3_1 = new TH1D("hTrigFineTime_fpga3_1","Trigger Fine Time FPGA 3 re1",
                                        NbinsFineTime,0.,MaxFineTime);
      fhTrigFinalTime_fpga3_1 = new TH1D("hTrigFinalTime_fpga3_1","Trigger Final Time FPGA 3 re1;Hit time [ps]",
                                         NbinsFinalTime,0.,MaxFinalTime);
     
      for(unsigned f=0; f<fNfpga; ++f)
         {
            for(unsigned ic = 0; ic<fNch[f]; ++ic)
               {
                  int ch = ic+1;
                  ch=Channel((uint8_t&)ffpga[f],(uint8_t&)ch);
                  if( fTrace )
                     std::cout<<"Creating histos for FPGA: "<<ffpga[f]<<" ch: "<<ch<<std::endl;
                  

                  TString hname=TString::Format("hCoarseTime_%d_%02d_%03d_0",ffpga[f],ic,ch);
                  TString htitle=TString::Format("Coarse Time FPGA: %d Ch: %02d   BV Ch: %03d re0;Hit time [ns]",ffpga[f],ic,ch);
                  fhCoarseTime_0.emplace_back(hname.Data(),htitle.Data(),NbinsCoarseTime,0.,MaxCoarseTime);

                  hname=TString::Format("hFineTime_%d_%02d_%03d_0",ffpga[f],ic,ch);
                  htitle=TString::Format("Fine Time FPGA: %d FGPA Ch: %02d  BV Ch: %03d re0",ffpga[f],ic,ch);
                  fhFineTime_0.emplace_back(hname.Data(),htitle.Data(),NbinsFineTime,0.,MaxFineTime); 

                  hname=TString::Format("hFinalTime_%d_%02d_%03d_0",ffpga[f],ic,ch);
                  htitle=TString::Format("Final Time FPGA: %d FGPA Ch: %02d  BV Ch: %03d re0;Hit time [ps]",ffpga[f],ic,ch);
                  fhFinalTime_0.emplace_back(hname.Data(),htitle.Data(),NbinsFinalTime,0.,MaxFinalTime);

                  hname=TString::Format("hCoarseTime_%d_%02d_%03d_1",ffpga[f],ic,ch);
                  htitle=TString::Format("Coarse Time FPGA: %d FGPA Ch: %02d  BV Ch: %03d re1;Hit time [ns]",ffpga[f],ic,ch);
                  fhCoarseTime_1.emplace_back(hname.Data(),htitle.Data(),NbinsCoarseTime,0.,MaxCoarseTime);

                  hname=TString::Format("hFineTime_%d_%02d_%03d_1",ffpga[f],ic,ch);
                  htitle=TString::Format("Fine Time FPGA: %d FGPA Ch: %02d  BV Ch: %03d re1",ffpga[f],ic,ch);
                  fhFineTime_1.emplace_back(hname.Data(),htitle.Data(),NbinsFineTime,0.,MaxFineTime); 

                  hname=TString::Format("hFinalTime_%d_%02d_%03d_1",ffpga[f],ic,ch);
                  htitle=TString::Format("Final Time FPGA: %d FGPA Ch: %02d  BV Ch: %03d re1;Hit time [ps]",ffpga[f],ic,ch);
                  fhFinalTime_1.emplace_back(hname.Data(),htitle.Data(),NbinsFinalTime,0.,MaxFinalTime);


                  hname=TString::Format("hCoarseTime_diff_%d_%02d_%03d_0",ffpga[f],ic,ch);
                  htitle=TString::Format("Coarse Time Diff Trig FPGA: %d FGPA Ch: %02d  BV Ch: %03d re0;Hit time [ns]",ffpga[f],ic,ch);
                  fhCoarseTime_diff_0.emplace_back(hname.Data(),htitle.Data(),NbinsCoarseTime,0.,MaxCoarseTime);

                  hname=TString::Format("hFineTime_diff_%d_%02d_%03d_0",ffpga[f],ic,ch);
                  htitle=TString::Format("Fine Time Diff Trig FPGA: %d FGPA Ch: %02d  BV Ch: %03d re0",ffpga[f],ic,ch);
                  fhFineTime_diff_0.emplace_back(hname.Data(),htitle.Data(),NbinsFineTime,0.,MaxFineTime); 

                  hname=TString::Format("hFinalTime_diff_%d_%02d_%03d_0",ffpga[f],ic,ch);
                  htitle=TString::Format("Final Time Diff Trig FPGA: %d FGPA Ch: %02d  BV Ch: %03d re0;Hit time [ps]",ffpga[f],ic,ch);
                  fhFinalTime_diff_0.emplace_back(hname.Data(),htitle.Data(),NbinsFinalTime,0.,MaxFinalTime);

                  hname=TString::Format("hCoarseTime_diff_%d_%02d_%03d_1",ffpga[f],ic,ch);
                  htitle=TString::Format("Coarse Time Diff Trig FPGA: %d FGPA Ch: %02d  BV Ch: %03d re1;Hit time [ns]",ffpga[f],ic,ch);
                  fhCoarseTime_diff_1.emplace_back(hname.Data(),htitle.Data(),NbinsCoarseTime,0.,MaxCoarseTime);

                  hname=TString::Format("hFineTime_diff_%d_%02d_%03d_1",ffpga[f],ic,ch);
                  htitle=TString::Format("Fine Time Diff Trig FPGA: %d FGPA Ch: %02d  BV Ch: %03d re1",ffpga[f],ic,ch);
                  fhFineTime_diff_1.emplace_back(hname.Data(),htitle.Data(),NbinsFineTime,0.,MaxFineTime); 

                  hname=TString::Format("hFinalTime_diff_%d_%02d_%03d_1",ffpga[f],ic,ch);
                  htitle=TString::Format("Final Time Diff Trig FPGA: %d FGPA Ch: %02d  BV Ch: %03d re1;Hit time [ps]",ffpga[f],ic,ch);
                  fhFinalTime_diff_1.emplace_back(hname.Data(),htitle.Data(),NbinsFinalTime,0.,MaxFinalTime);
               }
         }
   }

   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      TDirectory* dir = gDirectory->mkdir("coarse_re0");
      dir->cd();
      for( auto it = fhCoarseTime_0.begin(); it != fhCoarseTime_0.end(); ++it) it->Write(); 
      
      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("fine_re0");
      dir->cd();
      for( auto it = fhFineTime_0.begin(); it != fhFineTime_0.end(); ++it) it->Write();
     
      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("final_re0");
      dir->cd();
      for( auto it = fhFinalTime_0.begin(); it != fhFinalTime_0.end(); ++it) it->Write();

      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("coarse_re1");
      dir->cd();
      for( auto it = fhCoarseTime_1.begin(); it != fhCoarseTime_1.end(); ++it) it->Write();
      
      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("fine_re1");
      dir->cd();
      for( auto it = fhFineTime_1.begin(); it != fhFineTime_1.end(); ++it) it->Write();

      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("final_re1");
      dir->cd();
      for( auto it = fhFinalTime_1.begin(); it != fhFinalTime_1.end(); ++it) it->Write();

      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("corse_diff_re0");
      dir->cd();
      for( auto it = fhCoarseTime_diff_0.begin(); it != fhCoarseTime_diff_0.end(); ++it) it->Write();

      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("fine_diff_re0");
      dir->cd();
      for( auto it = fhFineTime_diff_0.begin(); it != fhFineTime_diff_0.end(); ++it) it->Write();

      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("final_diff_re0");
      dir->cd();
      for( auto it = fhFinalTime_diff_0.begin(); it != fhFinalTime_diff_0.end(); ++it) it->Write();

      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("coarse_diff_re1");
      dir->cd();
      for( auto it = fhCoarseTime_diff_1.begin(); it != fhCoarseTime_diff_1.end(); ++it) it->Write();

      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("fine_diff_re1");
      dir->cd();
      for( auto it = fhFineTime_diff_1.begin(); it != fhFineTime_diff_1.end(); ++it) it->Write();

      runinfo->fRoot->fOutputFile->cd("tdc"); // select correct ROOT directory
      dir = gDirectory->mkdir("final_diff_re1");
      dir->cd();
      for( auto it = fhFinalTime_diff_1.begin(); it != fhFinalTime_diff_1.end(); ++it) it->Write();

      printf("tdcmodule::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   { }

   void ResumeRun(TARunInfo* runinfo)
   { }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      AgEvent* age = ef->fEvent;

      TdcEvent* et = age->tdc;

      if( et )
         {
            if( et->complete )
               {
                  // if( !et->error )
                  //   {
                  // 	std::cout<<"tdcmodule::AnalyzeFlowEvent  good TDC event"<<std::endl;
                  // 	FillHistos( et );
                  //   }
                  // else
                  //   std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event error"<<std::endl;
                  std::cout<<"tdcmodule::AnalyzeFlowEvent  good TDC event"<<std::endl;
                  FillHistos( et );
                  FillDiffHistos( et );
                  FinalHisto( et );
               }
            else
               std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event incomplete"<<std::endl;
         }
      else
         std::cout<<"tdcmodule::AnalyzeFlowEvent  No TDC event"<<std::endl;
      
      return flow;
   }

   void FillHistos(TdcEvent* evt)
   {
      std::vector<TdcHit*> hits = evt->hits; 
      std::map<int,double> CountsInChannel;
      for(auto it=hits.begin(); it!=hits.end(); ++it)
         {
            if( fDebug )
               std::cout<<"tdcmodule::FillHistos  hit on FPGA "<<int((*it)->fpga)
                        <<" Chan: "<<int((*it)->chan)<<std::endl;
            if( (*it)->fpga == 1 )
               {
                  hOcc_fpga1->Fill( (*it)->chan );
               }
            else if( (*it)->fpga == 2 )
               {
                  hOcc_fpga2->Fill( (*it)->chan );
               }
            else if( (*it)->fpga == 3 )
               {
                  hOcc_fpga3->Fill( (*it)->chan );
               }
            int ch = Channel( (*it)->fpga, (*it)->chan );
            if( fDebug )
               std::cout<<"tdcmodule::FillHistos  progressive channel: "<<ch<<std::endl;
            if( ch < 0 ) continue;
            double coarse_time = GetCoarseTime((*it)->epoch,(*it)->coarse_time);
            double fine_time = double((*it)->fine_time);
            double final_time = GetFinalTime((*it)->coarse_time,fine_time);
                   
            if( fTrace )
               std::cout<<"tdcmodule::FillHistos ch: "<<ch
                        <<" fpga: "<<int((*it)->fpga)<<" chan: "<<int((*it)->chan)
                        <<"  coarse time: "<<coarse_time
                        <<" ns  fine time: "<<fine_time<<" dc  final time: "<<final_time<<" ps"<<std::endl;

            // DATA CHANNELS HISTOS
            if( (*it)->rising_edge )
               {
                  ++CountsInChannel[ch];

                  fhCoarseTime_1.at(ch).Fill(coarse_time);
                  fhFineTime_1.at(ch).Fill(fine_time);
                  fhFinalTime_1.at(ch).Fill(final_time);

                  hOcc->Fill(ch);
                  hOcc_1->Fill(ch,final_time);
               }
            else
               {
                  fhCoarseTime_0.at(ch).Fill(coarse_time);
                  fhFineTime_0.at(ch).Fill(fine_time);
                  fhFinalTime_0.at(ch).Fill(final_time);

                  hOcc_0->Fill(ch,final_time);
               }
         }
      hNuniqHits->Fill( double(CountsInChannel.size()) );
      double tot_hits=0.;
      for( auto h=CountsInChannel.begin(); h!=CountsInChannel.end(); ++h)
         {
            tot_hits+=h->second;
            // std::cout<<"tdcmodule::FillHistos  ch:"<<h->first
            // 	 <<"\t cnt: "<<h->second<<" ["<<tot_hits<<"]"<<std::endl;
         }
      hNhits->Fill( tot_hits );
   }

   void FillDiffHistos(TdcEvent* evt)
   {
      std::vector<TdcHit*> hits = evt->hits;
      for(auto it=hits.begin(); it!=hits.end(); ++it)
         {
            int ch = Channel( (*it)->fpga, (*it)->chan );
            if( fDebug )
               std::cout<<"tdcmodule::FillHistos  progressive channel: "<<ch<<std::endl;
            if( ch < -1 ) continue;

            double coarse_time = GetCoarseTime((*it)->epoch,(*it)->coarse_time);
            double fine_time = double((*it)->fine_time);
            double final_time = GetFinalTime((*it)->coarse_time,fine_time);

            // TRIGGER? HISTOS
            // I count on the fact that chan 0 of each FPGA always comes first...
            double trig_coarse_time_re0, trig_fine_time_re0, trig_final_time_re0,
               trig_coarse_time_re1, trig_fine_time_re1, trig_final_time_re1;
            trig_coarse_time_re0=trig_fine_time_re0=trig_final_time_re0=trig_coarse_time_re1=trig_fine_time_re1=trig_final_time_re1=0.;
            if( ch == -1 )
               {
                  if( (*it)->rising_edge )
                     {
                        trig_coarse_time_re1 = coarse_time;
                        trig_fine_time_re1 = fine_time;
                        trig_final_time_re1 = final_time;
                        
                        if( (*it)->fpga == 1 )
                           {
                              fhTrigCoarseTime_fpga1_1->Fill( trig_coarse_time_re1 );
                              fhTrigFineTime_fpga1_1->Fill( trig_fine_time_re1 );
                              fhTrigFinalTime_fpga1_1->Fill( trig_final_time_re1 );
                           }
                        else if( (*it)->fpga == 2 )
                           {
                              fhTrigCoarseTime_fpga2_1->Fill( trig_coarse_time_re1 );
                              fhTrigFineTime_fpga2_1->Fill( trig_fine_time_re1 );
                              fhTrigFinalTime_fpga2_1->Fill( trig_final_time_re1 );
                           }
                        else if ( (*it)->fpga == 3 )
                           {
                              fhTrigCoarseTime_fpga3_1->Fill( trig_coarse_time_re1 );
                              fhTrigFineTime_fpga3_1->Fill( trig_fine_time_re1 );
                              fhTrigFinalTime_fpga3_1->Fill( trig_final_time_re1 );
                           }
                     }
                  else
                     {
                        trig_coarse_time_re0 = coarse_time;
                        trig_fine_time_re0 = fine_time;
                        trig_final_time_re0 = final_time;

                        if( (*it)->fpga == 1 )
                           {
                              fhTrigCoarseTime_fpga1_0->Fill( trig_coarse_time_re0 );
                              fhTrigFineTime_fpga1_0->Fill( trig_fine_time_re0 );
                              fhTrigFinalTime_fpga1_0->Fill( trig_final_time_re0 );
                           }
                        else if( (*it)->fpga == 2 )
                           {
                              fhTrigCoarseTime_fpga2_0->Fill( trig_coarse_time_re0 );
                              fhTrigFineTime_fpga2_0->Fill( trig_fine_time_re0 );
                              fhTrigFinalTime_fpga2_0->Fill( trig_final_time_re0 );
                           }
                        else if ( (*it)->fpga == 3 )
                           {
                              fhTrigCoarseTime_fpga3_0->Fill( trig_coarse_time_re0 );
                              fhTrigFineTime_fpga3_0->Fill( trig_fine_time_re0 );
                              fhTrigFinalTime_fpga3_0->Fill( trig_final_time_re0 );
                           }
                     }
                  if( fTrace )
                     std::cout<<"Found trigger time: "<<final_time<<" for fpga "<<int((*it)->fpga)<<" (re"<<int((*it)->rising_edge)<<")"<<std::endl;
                  continue;
               }
            
            // DATA CHANNELS HISTOS
            if( (*it)->rising_edge )
               {
                  fhCoarseTime_diff_1.at(ch).Fill( coarse_time - trig_coarse_time_re1 );
                  fhFineTime_diff_1.at(ch).Fill( fine_time - trig_fine_time_re1 );
                  fhFinalTime_diff_1.at(ch).Fill( final_time - trig_final_time_re1 );
               }
            else
               {
                  
                  fhCoarseTime_diff_0.at(ch).Fill( coarse_time - trig_coarse_time_re0 );
                  fhFineTime_diff_0.at(ch).Fill( fine_time - trig_coarse_time_re0 );
                  fhFinalTime_diff_0.at(ch).Fill( final_time - trig_coarse_time_re0 );
               }
         }
   }

   void FinalHisto(TdcEvent* evt)
   {
      std::vector<TdcHit*> hits = evt->hits;
      double trig_time1, trig_time2, trig_time3, 
         first_time1, first_time2, first_time3;
      trig_time1 = trig_time2 = trig_time3 = 
         first_time1 = first_time2 = first_time3 = 0.;
      int stat = FindTriggerTime(hits, trig_time1, trig_time2, trig_time3);
      std::cout<<"tdcmodule::FinalHistos Found "<<stat<<" triggers"<<std::endl;
      stat = FindFirstHit(hits, first_time1, first_time2, first_time3);
      std::cout<<"tdcmodule::FinalHistos First Timer "<<stat<<std::endl;
      for(auto it=hits.begin(); it!=hits.end(); ++it)
         {
            int ch = Channel( (*it)->fpga, (*it)->chan );
            if( fDebug )
               std::cout<<"tdcmodule::FinalHisto  progressive channel: "<<ch<<std::endl;
            if( ch < 0 ) continue;

            double final_time = GetFinalTime((*it)->coarse_time,(*it)->fine_time);
            if( (*it)->fpga == 1 )
               {
                  hOcc_diff_1->Fill(ch, final_time - trig_time1 );
                  hOcc_delta_1->Fill(ch, final_time - first_time1 );
               }
            else if( (*it)->fpga == 2 )
               {
                  hOcc_diff_1->Fill(ch, final_time - trig_time2 );
                  hOcc_delta_1->Fill(ch, final_time - first_time2 );
               }
            else if( (*it)->fpga == 3 ) 
               {
                  hOcc_diff_1->Fill(ch, final_time - trig_time3 );
                  hOcc_delta_1->Fill(ch, final_time - first_time3 );
               }
         }
   }

   int Channel(uint8_t& fpga, uint8_t& chan)
   {
      int ff = int(fpga);
      int ch = int(chan);

      if( ch == 0 )
         return -1;
      else if( ch > int(fNch[ff-1]) )
         return -2;

      if ( ff == 1 )
         return ch - 1;
      else if( ff == 2 )
         return ch - 1 + int(fNch[0]);
      else if( ff == 3 )
         return ch - 1 + int(fNch[0]+fNch[1]);
      else
         return -3;
   }

   double GetCoarseTime( uint32_t epoch, uint16_t coarse )
   {
      return double(epoch)/epoch_freq + double(coarse)/coarse_freq;
   }

   double GetFinalTime( uint16_t coarse, double fine )
   {
      double A = double(coarse) * 5000.,
         B = fine - trb3LinearLowEnd,
         C = trb3LinearHighEnd - trb3LinearLowEnd;
      return A - (B/C) * 5000.;
   }
   
   double GetFinalTime( uint16_t coarse, uint16_t fine )
   {
      return GetFinalTime( coarse, double(fine) );
   }

   int FindTriggerTime(std::vector<TdcHit*> hits, double& fpga1, double& fpga2, double& fpga3)
   {
      int stat=0;
      for(auto it=hits.begin(); it!=hits.end(); ++it)
         {
            if( (*it)->chan != 0 ) continue;
            if( !(*it)->rising_edge ) continue;
            double final_time = GetFinalTime((*it)->coarse_time,(*it)->fine_time);
            if( (*it)->fpga == 1 )
               {
                  fpga1 = final_time;
                  ++stat;
               }
            else if( (*it)->fpga == 2 )
               {
                  fpga2 = final_time;
                  ++stat;
               }
            else if( (*it)->fpga == 3 )
               {
                  fpga3 = final_time;
                  ++stat;
               }
            if( stat == 3 ) break;
         }
      return stat;
   }

   int FindFirstHit(std::vector<TdcHit*> hits, double& fpga1, double& fpga2, double& fpga3)
   {
      int stat=0;
      fpga1=fpga2=fpga3=9.e9;
      for(auto it=hits.begin(); it!=hits.end(); ++it)
         {
            if( !(*it)->rising_edge ) continue;
            int ch = Channel( (*it)->fpga, (*it)->chan );
            if( ch < 0 ) continue;
            double final_time = GetFinalTime((*it)->coarse_time,(*it)->fine_time);
            if( (*it)->fpga == 1 )
               {
                  fpga1 = final_time<fpga1?final_time:fpga1;
                  stat+=ch;
               }
            else if( (*it)->fpga == 2 )
               {
                  fpga2 = final_time<fpga2?final_time:fpga2;
                  stat+=(100*ch);
               }
            else if( (*it)->fpga == 3 )
               {
                  fpga3 = final_time<fpga3?final_time:fpga3;
                  stat+=(10000*ch);
               }
         }
      return stat;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("tdcmodule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
  
  
};

class TdcModuleFactory: public TAFactory
{
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("TdcModuleFactory::Init!\n");
      
      for (unsigned i=0; i<args.size(); i++) { }
   }

   void Finish()
   {
      printf("TdcModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("TdcModule::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new tdcmodule(runinfo);
   }
};

static TARegister tar(new TdcModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
