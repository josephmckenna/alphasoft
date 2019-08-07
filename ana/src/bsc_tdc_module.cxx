#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "AnalysisTimer.h"

#include "TBarEvent.hh"


class tdcmodule: public TARunObject
{
private:

   // Constante value declaration
   int pedestal_length = 100;
   int threshold = 1400;

   // https://daq.triumf.ca/elog-alphag/alphag/1961
   const double epoch_freq = 97656.25; // 200MHz/(2<<11);
   const double coarse_freq = 200.0e6; // 200MHz

   const double coarse_range = 10240026.0; // ~10.24 us
   const double fine_range = 5000.0; // 5 ns

   // linear calibration:
   // $ROOTANASYS/libAnalyzer/TRB3Decoder.hxx
   const double trb3LinearLowEnd = 17.0;
   const double trb3LinearHighEnd = 473.0;

   // Container declaration
   std::map<int,std::vector<BarHit>> ADCHits;
   std::map<int,std::vector<double>> TDCHits;
   double firstHit[128][5];
   int adcHits[64]={0};
   int bscTdcMap[64][5];
   double tdcTimeDiff[64]={0};
   double time_top[64]={0};
   double time_bot[64]={0};
   double adcAmpTop[64]={0};
   double adcAmpBot[64]={0};
   double adcIntegralTop[64]={0};
   double adcIntegralBot[64]={0};
   

   //Histogramm declaration
   TH2D *hTdcTime = NULL;
   TH2D *hTimeDiff = NULL;
   TH2D *hTdcZed = NULL;
   TH1D *hTdcMissedEvent = NULL;
   TH2D *hTdcTimeTopVsBot = NULL;
   TH2D *hTdcTopVsAmpTop = NULL;
   TH2D *hTdcBotVsAmpTop = NULL;
   TH2D *hTdcTopVsAmpBot = NULL;
   TH2D *hTdcBotVsAmpBot = NULL;
   TH2D *hTdcTimeDiffVsAmpTop = NULL;
   TH2D *hTdcTimeDiffVsAmpBot = NULL;
   TH2D *hTdcTimeDiffVsIntegralTop = NULL;
   TH2D *hTdcTimeDiffVsIntegralBot = NULL;
   TH2D *hHitsADCVsTDC = NULL;
   TH2D *hHitsTDCTopVsBot = NULL;

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
      gDirectory->mkdir("bsc_tdc_module")->cd();

      // Histogramm declaration
      hTdcTime=new TH2D("hTdcTime","Time measured on TDC minus trigger time;Channel;Time [ps]",128,-0.5,127.5,2000,-1600000,-1100000);
      hTimeDiff=new TH2D("hTimeDiff","Time difference per bar;Bar;Time [ps]",64,-0.5,63.5,6000,-60000,60000);
      hTdcZed=new TH2D("hTdcZed","Zed of the events;Bar;Zed [m]",64,-0.5,63.5,6000,-3,3);
      hTdcMissedEvent=new TH1D("hTdcMissedEvent", "Event missed by TDC;Bar;",64,-0.5,63.5);
      hTdcTimeTopVsBot = new TH2D("hTdcTimeTopVsBot","Time from trigger measured by top and bottom;Time Top minus trigger time [ps];Time Bottom minus trigger time [ps]",2000,-1600000.,-1100000,2000,-1600000.,-1100000.);
      hTdcTopVsAmpTop = new TH2D("hTdcTopVsAmpTop","Top TDC time vs Top ADC amplitude;Time minus trigger time [ps];ADC pulse amplitute",2000,-1600000,-1100000,2000,0.,20000.);
      hTdcBotVsAmpTop = new TH2D("hTdcBotVsAmpTop","Bot TDC time vs Top ADC amplitude;Time minus trigger time [ps];ADC pulse amplitute",2000,-1600000,-1100000,2000,0.,20000.);
      hTdcTopVsAmpBot = new TH2D("hTdcTopVsAmpBot","Top TDC time vs Bot ADC amplitude;Time minus trigger time [ps];ADC pulse amplitute",2000,-1600000,-1100000,2000,0.,20000.);
      hTdcBotVsAmpBot = new TH2D("hTdcBotVsAmpBot","Bot TDC time vs Bot ADC amplitude;Time minus trigger time [ps];ADC pulse amplitute",2000,-1600000,-1100000,2000,0.,20000.);
      hTdcTimeDiffVsAmpTop = new TH2D("hTdcTimeDiffVsAmpTop","Time difference vs adc amplitude;Time difference [ps];ADC Top pulse amplitute",2000,-60000,60000,2000,0.,20000.);
      hTdcTimeDiffVsAmpBot = new TH2D("hTdcTimeDiffVsAmpBot","Time difference vs adc amplitude;Time difference [ps];ADC Bot pulse amplitute",2000,-60000,60000,2000,0.,20000.);
      hTdcTimeDiffVsIntegralTop = new TH2D("hTdcTimeDiffVsIntegralTop","Time difference vs adc peak integral;Time difference [ps];ADC Top pulse integral",2000,-60000,60000,2000,0.,200000.);
      hTdcTimeDiffVsIntegralBot = new TH2D("hTdcTimeDiffVsIntegralBot","Time difference vs adc peak integral;Time difference [ps];ADC Bot pulse integral",2000,-60000,60000,2000,0.,200000.);
      hHitsADCVsTDC = new TH2D("hHitsADCVsTDC","Number of hits on one bar;ADC Hits;TDC Hits",10,-0.5,9.5,10,-0.5,9.5);
      hHitsTDCTopVsBot = new TH2D("hHitsTDCTopVsBot","Number of TDC hits;Bot Hits;Top Hits",10,-0.5,9.5,10,-0.5,9.5);

      // Load Bscint tdc map
      TString mapfile=getenv("AGRELEASE");
      mapfile+="/ana/bscint/";
      mapfile+="bscint_tdc.map";
      std::ifstream fbscMap(mapfile.Data());
      if(fbscMap)
         {
            std::string comment;
            getline(fbscMap, comment);
            for(int bar_ind=0; bar_ind<64; bar_ind++)
               {
                  fbscMap >> bscTdcMap[bar_ind][0] >> bscTdcMap[bar_ind][1] >> bscTdcMap[bar_ind][2] >> bscTdcMap[bar_ind][3] >> bscTdcMap[bar_ind][4];
               }
            fbscMap.close();
         }
   }

   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
      delete hTimeDiff;
      delete hTdcTime;
      delete hTdcZed;
      delete hTdcMissedEvent;
      delete hTdcTimeTopVsBot;
      delete hTdcTopVsAmpTop;
      delete hTdcBotVsAmpTop;
      delete hTdcTopVsAmpBot;
      delete hTdcBotVsAmpBot;
      delete hTdcTimeDiffVsAmpTop;
      delete hTdcTimeDiffVsAmpBot;
      delete hTdcTimeDiffVsIntegralTop;
      delete hTdcTimeDiffVsIntegralBot;
      delete hHitsADCVsTDC;
      delete hHitsTDCTopVsBot;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   // Main function
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {

      // Unpack Event flow
      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif
      AgEvent* age = ef->fEvent;

      // Unpack tdc data from event
      TdcEvent* tdc = age->tdc;
      
      for(int ii=0; ii<128; ii++)
         {
            for(int jj=0; jj<5; jj++)
               firstHit[ii][jj]=0;
         }

      ResetADCHits();
      ResetTDCHits();

      for(int ii=0; ii<64; ii++)
         {
            adcHits[ii]=0;
            tdcTimeDiff[ii]=0;
            adcAmpTop[ii]=0;
            adcAmpBot[ii]=0;
            adcIntegralTop[ii]=0;
            adcIntegralBot[ii]=0;
            time_top[ii]=0;
            time_bot[ii]=0;
         }


      if( tdc )
         {
            if( tdc->complete )
               {
                  //std::cout<<"tdcmodule::AnalyzeFlowEvent  good TDC event"<<std::endl;

                  // Add function here !!!
                  cleanHits(tdc); //feed firstHit tab
                  getAdcHits(flow); //feed adcHits tab
                  getADCHits(flow);
                  getTDCHits(tdc);
                  getTdcTime(tdc);
                  compare(tdc);

                  flow=feedFlow(flow);

               }
            else
               std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event incomplete"<<std::endl;
         }
      else
         std::cout<<"tdcmodule::AnalyzeFlowEvent  No TDC event"<<std::endl;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bsc_tdc_module",timer_start);
      #endif
      return flow;
   }


   // Secondary function

   TAFlowEvent* feedFlow(TAFlowEvent* flow)
   {
      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>();
      if( !bef ) return flow;
      TBarEvent *barEvt=bef->BarEvent;
      std::vector<BarHit>* flowAdcHits=barEvt->GetBars();
      

      //      double ZedTdc=0;// --> unused variable -- AC 29/5/2019

      for(int ii=0; ii<int(flowAdcHits->size()); ii++)
         {
            int barID=flowAdcHits->at(ii).GetBar();
            //ZedTdc=getZedTdc(*tdcTimeDiff[barID]);
            //flowAdcHits->at(ii).SetZedTdc(ZedTdc);

            // Check
            //
            //std::cout<<"---------------------> TDC Zed calculation gave "<<Zed<<std::endl;
            if (time_top[barID] && time_bot[barID])
            {
               flowAdcHits->at(ii).SetTDCHit(barID, time_top[barID], time_bot[barID]);
               //flowAdcHits->at(ii).Print();
               double Zed=flowAdcHits->at(ii).GetTDCZed();
               hTdcZed->Fill(barID,Zed);
            }
         }

      return flow;
   }

   void compare(TdcEvent* tdc)
   {
      for (int bar=0;bar<64;bar++)
         {
            int nTDCTop = TDCHits[bar+64].size();
            int nTDCBot = TDCHits[bar].size();
            int nADC = ADCHits[bar].size();
            hHitsTDCTopVsBot->Fill(nTDCTop,nTDCBot);
            hHitsADCVsTDC->Fill(nADC,nTDCBot);
            if ( nADC==nTDCBot && nTDCBot==nTDCTop ) // All hits detected
               {
                  for (int ii=0; ii<nADC; ii++)
                     {
                        double trig_time=FindTriggerTime(tdc->hits,bar);
                        double tdc_bot = TDCHits[bar].at(ii) - trig_time;
                        double tdc_top = TDCHits[bar+64].at(ii) - trig_time;
                        double diff_time = TDCHits[bar].at(ii) - TDCHits[bar+64].at(ii);
                        // FILLS HISTOS
                        hTdcTime->Fill(bar+64, tdc_top);
                        hTdcTime->Fill(bar, tdc_bot);
                        hTdcTimeTopVsBot->Fill(tdc_top,tdc_bot);
                        hTimeDiff->Fill(bar, diff_time);
                        hTdcTopVsAmpTop->Fill(tdc_top, ADCHits[bar].at(ii).GetAmpTop());
                        hTdcBotVsAmpTop->Fill(tdc_bot, ADCHits[bar].at(ii).GetAmpTop());
                        hTdcTopVsAmpBot->Fill(tdc_top, ADCHits[bar].at(ii).GetAmpBot());
                        hTdcBotVsAmpBot->Fill(tdc_bot, ADCHits[bar].at(ii).GetAmpBot());
                        hTdcTimeDiffVsAmpTop->Fill(diff_time, ADCHits[bar].at(ii).GetAmpTop());
                        hTdcTimeDiffVsAmpBot->Fill(diff_time, ADCHits[bar].at(ii).GetAmpBot());
                        hTdcTimeDiffVsIntegralTop->Fill(diff_time, ADCHits[bar].at(ii).GetIntegralTop());
                        hTdcTimeDiffVsIntegralBot->Fill(diff_time, ADCHits[bar].at(ii).GetIntegralBot());
                     }
               }
            else // Mismatch in hits detected
               {
                  // Do something maybe
                  hTdcMissedEvent->Fill(bar);
               }
         }
   }

    void getTdcTime(TdcEvent* tdc)
   {
      std::vector<TdcHit*> hits = tdc->hits;

      for(int bar=0; bar<64; bar ++)
         {
            tdcTimeDiff[bar]=0;

            if(adcHits[bar]==1)
               {
                  if(firstHit[bar][3]<0 || firstHit[bar+64][3]<0)
                     {
                        //std::cout<<"-------------------> Event missed by the TDC"<<std::endl;
                     }
                  else
                     {
                        double final_time_top=firstHit[bar][3];
                        double final_time_bot=firstHit[bar+64][3];

                        double trig_time=FindTriggerTime(hits,bar);

                        time_top[bar]=final_time_top-trig_time;
                        time_bot[bar]=final_time_bot-trig_time;

                        double diff_time=time_bot[bar]-time_top[bar]; // GS swapped top and bot
                        tdcTimeDiff[bar]=diff_time;

                        //std::cout<<"-------------------> Event on bar "<<bar<<" time top is "<<time_top<<" and time bot is "<<time_bot<<" and trigger is "<<trig_time<<" diff time is "<<diff_time<<"Final time top = "<<final_time_top<<" et final time bot = "<<final_time_bot<<std::endl;
                     }
               }
         }


   }

    double FindTriggerTime(std::vector<TdcHit*> hits, int bar)
   {
      double trig_time=0;
      int tdc_fpga=bscTdcMap[bar][1]-1;
      for (auto hit: hits)
         {
            if ( hit->chan != 0 ) continue;
            if ( ! hit->rising_edge ) continue;
            if ( hit->fpga > tdc_fpga ) break;
            if ( hit->fpga==tdc_fpga ) trig_time = GetFinalTime(hit->epoch,hit->coarse_time,hit->fine_time);
         }
      return trig_time;
   }


   void getADCHits(TAFlowEvent* flow)
   {

      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>();
      if( !bef ) return;
      TBarEvent *barEvt=bef->BarEvent;
      std::vector<BarHit>* flowAdcHits=barEvt->GetBars();

      // Sorts ADC hits into a separate vector for each bar
      ResetADCHits();
      for (auto hit: *flowAdcHits)
         {
            int barID = hit.GetBar();
            ADCHits[barID].push_back(hit);
         }

   }

   void getAdcHits(TAFlowEvent* flow)
   {

      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>();
      if( !bef ) return;
      TBarEvent *barEvt=bef->BarEvent;
      std::vector<BarHit>* flowAdcHits=barEvt->GetBars();

      //Reset adcHits tab
      for(int ii=0; ii<64; ii++) {
         adcHits[ii]=0;
         adcAmpTop[ii]=0;
         adcAmpBot[ii]=0;
         adcIntegralTop[ii]=0;
         adcIntegralBot[ii]=0;
      }

      for(int ii=0; ii<int(flowAdcHits->size()); ii++)
         {
            int barID=flowAdcHits->at(ii).GetBar();
            adcHits[barID]=1;
            adcAmpTop[barID]=flowAdcHits->at(ii).GetAmpTop();
            adcAmpBot[barID]=flowAdcHits->at(ii).GetAmpBot();
            adcIntegralTop[barID]=flowAdcHits->at(ii).GetIntegralTop();
            adcIntegralBot[barID]=flowAdcHits->at(ii).GetIntegralBot();
            //std::cout<<"---------------------------->ADC hit on bar "<<barID<<std::endl;
         }
   }


   void getTDCHits(TdcEvent* tdc)
    {
      std::vector<TdcHit*> hits = tdc->hits;
      ResetTDCHits();
      for (auto hit: hits)
         {
            if (hit->rising_edge==1 && int(hit->chan)>0)
               {
                  int barID = fpga2barID(int(hit->fpga),int(hit->chan));
                  double final_time = GetFinalTime(hit->epoch,hit->coarse_time,hit->fine_time);
                  TDCHits[barID].push_back(final_time);
               }
         }
    }


  void cleanHits(TdcEvent* tdc)
   {
      std::vector<TdcHit*> hits = tdc->hits;

      int fpga=-1;
      int chan=-1;
      int bar=-1;
      // Reset firstHit tab
      for(int ii=0; ii<128; ii++)
         {
            for(int jj=0; jj<5; jj++)
               firstHit[ii][jj]=-1;
         }

      for(auto it=hits.begin(); it!=hits.end(); ++it)
         {
            if(int((*it)->fpga)==fpga && int((*it)->chan)==chan)
            { /* Not the first hit */ }
            else if(int((*it)->chan)==0 ||((*it)->rising_edge)==0 )
               {/*  */}
            else
               {
                  //Load new fpga and chan value
                  fpga=int((*it)->fpga);
                  chan=int((*it)->chan);

                  //Find bar ID
                  bar=fpga2barID(fpga,chan);

                  //Get Time Value
                  double coarse_time = GetCoarseTime((*it)->epoch,(*it)->coarse_time);
                  double fine_time = double((*it)->fine_time);
                  double final_time = GetFinalTime((*it)->epoch,(*it)->coarse_time,(*it)->fine_time);
                  //double final_time = GetFinalTime(coarse_time,fine_time);

                  //Feed "firstHit" tab
                  firstHit[bar][0]=double(bar);
                  firstHit[bar][1]=coarse_time;
                  firstHit[bar][2]=fine_time;
                  firstHit[bar][3]=final_time;
                  //std::cout<< "------------------------> first hit on bar ID="<<*firstHit[bar][0]<< " and coarse-time ="<<*firstHit[bar][1]<<"ns  Final time = "<<final_time<<" ps"<<" fine time = "<<fine_time<<std::endl;

               }

         }
   }


   int fpga2barID(int fpga, int chan)
   {
      int bar = -1;
      if(chan==0)
         return -1;
      else
         for(bar=0; bar<64; bar ++)
            {
               if(fpga==bscTdcMap[bar][1]-1)
                  {
                     if(chan== (bscTdcMap[bar][2]-1)*16+bscTdcMap[bar][3]+1)
                        return bar+64; //top side
                     else if(chan== (bscTdcMap[bar][2]-1)*16+bscTdcMap[bar][4]+1)
                        return bar; //bot side
                  }
            }
      return -1;
   }


   double GetCoarseTime( uint32_t epoch, uint16_t coarse )
   {
      return double(epoch)/epoch_freq + double(coarse)/coarse_freq;
   }

   double GetFinalTime( uint32_t epoch, uint16_t coarse, uint16_t fine )
   {
      double B = double(fine) - trb3LinearLowEnd;
      double C = trb3LinearHighEnd - trb3LinearLowEnd;
      return double(epoch)*coarse_range +  double(coarse)*fine_range - (B/C) * fine_range;
   }

   void ResetADCHits()
      {
         for (int ii=0; ii<64; ii++) ADCHits.clear();
      }
   void ResetTDCHits()
      {
         for (int ii=0; ii<64; ii++) TDCHits.clear();
      }


};

class tdcModuleFactory: public TAFactory
{
public:
   void Help()
   {   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("tdcModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) { }
   }

   void Finish()
   {
      printf("tdcModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("tdcModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new tdcmodule(runinfo);
   }
};

static TARegister tar(new tdcModuleFactory);


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
