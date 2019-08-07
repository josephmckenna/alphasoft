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
#include "AnalysisTimer.h"

#include "TBarEvent.hh"

class BscFlags
{
public:
   bool fPrint = false;
};

class BscModule: public TARunObject
{
private:
   float time_trig = 0;
   int fCounter = 0;
   int pedestal_length = 100;
   int threshold = 1400;
   int bscMap[64][4];
   TBarEvent* BarEvent;

   std::map<int,std::vector<std::map<std::string,double>>> fBarHits;

public:
   BscFlags* fFlags;

private:
   TH1D *hBsc_Zed=NULL;
   TH1D *hBsc_TimeVsTop = NULL;
   TH1D *hBsc_TimeVsBot =NULL;
   TH1D *hBsc_TimeTopAndBot =NULL;
   TH2D *hBsc_TimeVsBar = NULL;
   TH1D *hBsc_TimeDiff = NULL;
   TH1D *hBsc_Amplitude = NULL;
   TH1D *hBsc_Integral = NULL;
   TH1D *hBsc_Duration = NULL;
   TH2D *hBsc_nHitsTopVsBot = NULL;
public:

   BscModule(TARunInfo* runinfo, BscFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      ResetBarHits();
   }

   ~BscModule()
   {   }


   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("bsc")->cd();

      hBsc_Zed=new TH1D("hBsc_Zed", "Altitude from ADC time difference", 46000,-2300.,2300.);
      hBsc_TimeVsTop=new TH1D("hBsc_TimeVsTop", "ADC time on TOP", 700,0,700);
      hBsc_TimeVsBot=new TH1D("hBsc_TimeVsBot", "ADC Time on BOT", 700,0,700);
      hBsc_TimeTopAndBot=new TH1D("hBsc_TimeAndBot", "ADC Time on TOP and BOT", 700,0,700);
      hBsc_TimeVsBar=new TH2D("hBsc_TimeVsBar", "ADC Time on TOP and BOT", 700,0,700, 63, 0, 63);
      hBsc_TimeDiff=new TH1D("hBsc_TimeDiff", "ADC Time Diff", 20,-10,10);
      hBsc_nHitsTopVsBot = new TH2D("hBsc_nHitsTopVsBot","Number of Hits on Bar Top and Bottom per bar per event",10,-0.5,9.5,10,-0.5,9.5);
      hBsc_Amplitude=new TH1D("hBsc_Amplitude", "ADC Pulse Amplitude", 2000,0.,200000.);
      hBsc_Integral=new TH1D("hBsc_Integral", "ADC Pulse Integral", 2000,0.,2000000.);
      hBsc_Duration=new TH1D("hBsc_Duration", "ADC Pulse Duration", 100,0,100);

      //Chargement Bscint map
      TString mapfile=getenv("AGRELEASE");
      mapfile+="/ana/bscint/";
      mapfile+="bscint.map";
      std::ifstream fbscMap(mapfile.Data());
      if(fbscMap)
         {
            std::string comment;
            getline(fbscMap, comment);
            for(int bar_ind=0; bar_ind<64; bar_ind++)
               {
                  fbscMap >> bscMap[bar_ind][0] >> bscMap[bar_ind][1] >> bscMap[bar_ind][2] >> bscMap[bar_ind][3];
               }
            fbscMap.close();
         }
   }

   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
      delete hBsc_Zed;
      delete hBsc_TimeVsTop;
      delete hBsc_TimeVsBot;
      delete hBsc_TimeTopAndBot;
      delete hBsc_TimeVsBar;
      delete hBsc_TimeDiff;
      delete hBsc_nHitsTopVsBot;
      delete hBsc_Amplitude;
      delete hBsc_Integral;
      delete hBsc_Duration;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }


   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Bscint Module: Analyze, run %d, counter %d\n", runinfo->fRunNo, fCounter);
      const AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif      
      const AgEvent* e = ef->fEvent;
      const Alpha16Event* data = e->a16;

      if( !data )
         {
            std::cout<<"BscModule::AnalyzeFlowEvent(...) No Alpha16Event in AgEvent # "
                     <<e->counter<<std::endl;
            return flow;
         }

      if( fFlags->fPrint )
         printf("BscModule::AnalyzeFlowEvent(...) Event number is : %d \n", data->counter);


      TBarEvent* BarEvent = AnalyzeBars(data);
      BarEvent->SetID(e->counter);
      BarEvent->SetRunTime(e->time);

      std::cout<<"BscModule::AnalyzeFlowEvent(...) has "<<BarEvent->GetNBars()<<" hits"<<std::endl;

      flow = new AgBarEventFlow(flow, BarEvent);

#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bscint_adc_module",timer_start);
#endif
      return flow;
   }


   TBarEvent* AnalyzeBars(const Alpha16Event* data)
   {
      std::vector<Alpha16Channel*> channels = data->hits;
      ResetBarHits();

      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*
            if( ch->adc_chan >= 16 ) continue; // it's AW
            if( ch->bsc_bar < 0 )
               {
                  //std::cerr<<"BscModule::AnalyzeBars() Error Bar number"<<std::endl;
                  continue;
               }

            // CALCULATE BASELINE
            double baseline(0.);
            for(int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at( b );
            baseline /= double(pedestal_length);


            // FILLS fBarHits WITH FEATURES OF EACH ADC PULSE
            bool inpeak = false;
            double peakmax = 0.;
            double integral = 0.;
            int starttime = 0;
            int sample_length = int(ch->adc_samples.size());
            for (int ii=0; ii<sample_length; ii++)
               {
                  double chv = ch->adc_samples.at(ii) - baseline;
                  if (!inpeak && chv > threshold) // start of new peak
                     {
                        inpeak = true;
                        peakmax = chv;
                        integral = 0.;
                        starttime = ii;
                     }
                  if (inpeak)
                     {
                        if (chv < threshold || ii+1==sample_length) // end of peak
                           {
                              inpeak = false;
                              std::map<std::string,double> hit;
                              hit["ti"] = double(starttime);
                              hit["tf"] = double(ii);
                              hit["max"] = peakmax;
                              hit["integral"] = integral;
                              fBarHits[ch->bsc_bar].push_back(hit);
                           }
                        else
                           {
                              if (chv > peakmax) peakmax = chv;
                              integral += chv;
                           }
                     }
               }
         }

      // MERGES HITS SEPERATED BY <5 ADC BINS
      for (int ii=0; ii<128; ii++)
         {
            for (int ihit=int(fBarHits[ii].size())-1; ihit>0; ihit--) 
               {
                  if (fBarHits[ii][ihit]["ti"]-fBarHits[ii][ihit-1]["tf"]<5)
                     {
                        fBarHits[ii][ihit-1]["tf"] = fBarHits[ii][ihit]["tf"];
                        fBarHits[ii][ihit-1]["max"] = std::max(fBarHits[ii][ihit-1]["max"],fBarHits[ii][ihit]["max"]);
                        fBarHits[ii][ihit-1]["integral"] = fBarHits[ii][ihit-1]["integral"] + fBarHits[ii][ihit]["integral"];
                        fBarHits[ii].erase(fBarHits[ii].begin() + ihit);
                     }
               }
         }

      // FILLS HISTS AND BAR EVENT
      TBarEvent* BarEvent = new TBarEvent();
      for (int ibar=0; ibar<64; ibar++)
         {
            hBsc_nHitsTopVsBot->Fill(fBarHits[ibar].size(),fBarHits[ibar+64].size());
            if (fBarHits[ibar].size() == fBarHits[ibar+64].size()) // Proceed only if same number of hits on top and bottom
               {
                  for (int ii=0; ii<int(fBarHits[ibar].size()); ii++)
                     {
                        double time_bot = fBarHits[ibar][ii]["ti"];
                        double time_top = fBarHits[ibar+64][ii]["ti"];
                        double max_bot = fBarHits[ibar][ii]["max"];
                        double max_top = fBarHits[ibar+64][ii]["max"];
                        double duration_bot = fBarHits[ibar][ii]["tf"] - fBarHits[ibar][ii]["ti"];
                        double duration_top = fBarHits[ibar+64][ii]["tf"] - fBarHits[ibar+64][ii]["ti"];
                        double integral_bot = fBarHits[ibar][ii]["integral"];
                        double integral_top = fBarHits[ibar+64][ii]["integral"];
                        // Fills hists
                        hBsc_TimeVsBot->Fill(time_bot);
                        hBsc_TimeVsTop->Fill(time_top);
                        hBsc_TimeTopAndBot->Fill(time_bot);
                        hBsc_TimeTopAndBot->Fill(time_top);
                        hBsc_TimeVsBar->Fill(time_bot,ibar);
                        hBsc_TimeVsBar->Fill(time_top,ibar);
                        hBsc_TimeDiff->Fill(time_bot-time_top);
                        hBsc_Amplitude->Fill(max_top);
                        hBsc_Integral->Fill(integral_top);
                        hBsc_Duration->Fill(duration_top);
                        hBsc_Amplitude->Fill(max_bot);
                        hBsc_Integral->Fill(integral_bot);
                        hBsc_Duration->Fill(duration_bot);
                        // Get Zed
                        double speed=TMath::C()*1.e-6;
                        double cFactor=1.58;
                        double ZedADC=((speed/cFactor) * double(time_bot-time_top)*10.)*0.5;
                        hBsc_Zed->Fill(ZedADC);
                        // Fills BarEvent
                        BarEvent->AddADCHit(ibar,max_top,max_bot, time_top, time_bot, ZedADC);
                     }
               }
         }


      return BarEvent;
   }

   void ResetBarHits()
   {
      for (int ii=0; ii<128; ii++) fBarHits.clear();
   }

};


class BscModuleFactory: public TAFactory
{
public:
   BscFlags fFlags;
public:
   void Help()
   {   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("BscModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--bscprint")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("BscModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("BscModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new BscModule(runinfo, &fFlags);
   }
};

static TARegister tar(new BscModuleFactory);


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
