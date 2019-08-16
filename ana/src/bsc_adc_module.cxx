#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <numeric>

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
   double amplitude_cut = 3000;
   int bscMap[64][4];
   TBarEvent* BarEvent;
   bool merge_hits = true; // Merge hits which are close together
   int hit_counter = 0;

   std::map<int,std::vector<std::map<std::string,double>>> fBarHits;

public:
   BscFlags* fFlags;

private:
   TH1D *hBsc_Time=NULL;
   TH2D *hBsc_TimeVsBar = NULL;
   TH1D *hBsc_Amplitude = NULL;
   TH1D *hBsc_Integral = NULL;
   TH1D *hBsc_Duration = NULL;
   TH2D* hBsc_RiseTime = NULL;
   TH1D* hPulse22 = NULL;
   TH1D* hPulse33 = NULL;
   TH1D* hPulse44 = NULL;
   TH1D* hPulse55 = NULL;
   TH1D* hPulse66 = NULL;
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

      hBsc_Time=new TH1D("hBsc_Time", "ADC Time;ADC Time [ns]", 700,0,7000);
      hBsc_TimeVsBar=new TH2D("hBsc_TimeVsBar", "ADC Time;Bar;ADC Time [ns]", 128,0,128,700,0,7000);
      hBsc_Amplitude=new TH1D("hBsc_Amplitude", "ADC Pulse Amplitude;Amplitude", 2000,0.,35000.);
      hBsc_Integral=new TH1D("hBsc_Integral", "ADC Pulse Integral;Integral", 2000,0.,700000.);
      hBsc_Duration=new TH1D("hBsc_Duration", "ADC Pulse Duration;Duration [ns]", 100,0,1000);
      hBsc_RiseTime = new TH2D("hBsc_Risetime","ADC Pulse Rise Time;Pulse Amplitude;Rise Time [ns]",2000,0,35000,200,0,2000);
      hPulse22 = new TH1D("hPulse22","ADC Pulse 22;Time [ns];ADC value minus threshold",700,0,7000);
      hPulse33 = new TH1D("hPulse33","ADC Pulse 33;Time [ns];ADC value minus threshold",700,0,7000);
      hPulse44 = new TH1D("hPulse44","ADC Pulse 44;Time [ns];ADC value minus threshold",700,0,7000);
      hPulse55 = new TH1D("hPulse55","ADC Pulse 55;Time [ns];ADC value minus threshold",700,0,7000);
      hPulse66 = new TH1D("hPulse66","ADC Pulse 66;Time [ns];ADC value minus threshold",700,0,7000);

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
      delete hBsc_Time;
      delete hBsc_TimeVsBar;
      delete hBsc_Amplitude;
      delete hBsc_Integral;
      delete hBsc_Duration;
      delete hBsc_RiseTime;
      delete hPulse22;
      delete hPulse33;
      delete hPulse44;
      delete hPulse55;
      delete hPulse66;
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
            if( ch->bsc_bar < 0 ) continue;

            // CALCULATE BASELINE
            double baseline(0.);
            for(int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at( b );
            baseline /= double(pedestal_length);


            // FILLS fBarHits WITH FEATURES OF EACH ADC PULSE
            bool inpeak = false;
            bool ishit = false;
            int starttime = 0;
            int sample_length = int(ch->adc_samples.size());
            for (int ii=0; ii<sample_length; ii++)
               {
                  double chv = ch->adc_samples.at(ii) - baseline;
                  if (!inpeak && chv>threshold) // start of new peak
                     {
                        inpeak = true;
                        starttime = ii;
                     }
                  if (inpeak && (chv<threshold || ii+1==sample_length)) // end of peak
                     {
                        inpeak = false;
                        hit_counter++;
                        std::map<std::string,double> hit;
                        hit["ti"] = double(starttime);
                        hit["tf"] = double(ii);
                        double max = *std::max_element(ch->adc_samples.begin()+starttime,ch->adc_samples.begin()+ii);
                        hit["max"] = max;
                        hit["integral"] = std::accumulate(ch->adc_samples.begin()+starttime,ch->adc_samples.begin()+ii,0);
                        // Calculates rise time
                        int max_index = std::max_element(ch->adc_samples.begin(),ch->adc_samples.end()) - ch->adc_samples.begin();
                        int t80=-999;
                        int t20=-999;
                        for (int j=max_index;j>=0;j--)
                           {
                              double chvv = ch->adc_samples.at(j);
                              if (chvv>max*0.8) t80 = j;
                              if (chvv>max*0.2) t20 = j;
                              else break;
                           }
                        hit["t80"] = t80;
                        hit["t20"] = t20;
                        fBarHits[ch->bsc_bar].push_back(hit);
                        ishit = true;
                     }
               }
            // Saves some random adc pulses to histograms
            if (hit_counter==22 && ishit)
               for (int ii=0;ii<sample_length;ii++)
                     hPulse22->Fill(ii*10,ch->adc_samples.at(ii)-baseline);
            if (hit_counter==33 && ishit)
               for (int ii=0;ii<sample_length;ii++)
                     hPulse33->Fill(ii*10,ch->adc_samples.at(ii)-baseline);
            if (hit_counter==44 && ishit)
               for (int ii=0;ii<sample_length;ii++)
                     hPulse44->Fill(ii*10,ch->adc_samples.at(ii)-baseline);
            if (hit_counter==55 && ishit)
               for (int ii=0;ii<sample_length;ii++)
                     hPulse55->Fill(ii*10,ch->adc_samples.at(ii)-baseline);
            if (hit_counter==66 && ishit)
               for (int ii=0;ii<sample_length;ii++)
                     hPulse66->Fill(ii*10,ch->adc_samples.at(ii)-baseline);
         }

      // MERGES HITS SEPERATED BY <5 ADC BINS
      if (merge_hits)
         {
            for (int ibar=0; ibar<128; ibar++)
               {
                  for (int ii=int(fBarHits[ibar].size())-1; ii>0; ii--) 
                     {
                        if (fBarHits[ibar][ii]["ti"]-fBarHits[ibar][ii-1]["tf"]<5)
                           {
                              fBarHits[ibar][ii-1]["tf"] = fBarHits[ibar][ii]["tf"];
                              fBarHits[ibar][ii-1]["max"] = std::max(fBarHits[ibar][ii-1]["max"],fBarHits[ibar][ii]["max"]);
                              fBarHits[ibar][ii-1]["integral"] = fBarHits[ibar][ii-1]["integral"] + fBarHits[ibar][ii]["integral"];
                              fBarHits[ibar].erase(fBarHits[ibar].begin() + ii);
                           }
                     }
               }
        }

      // FILLS HISTS
      for (int ibar=0; ibar<128; ibar++)
         {
            for (int ii=0; ii<int(fBarHits[ibar].size()); ii++)
               {
                  if (fBarHits[ibar][ii]["max"]<amplitude_cut) continue;
                  hBsc_Time->Fill(fBarHits[ibar][ii]["ti"]*10);
                  hBsc_TimeVsBar->Fill(ibar,fBarHits[ibar][ii]["ti"]*10);
                  hBsc_Amplitude->Fill(fBarHits[ibar][ii]["max"]);
                  hBsc_Integral->Fill(fBarHits[ibar][ii]["integral"]);
                  hBsc_Duration->Fill((fBarHits[ibar][ii]["tf"]-fBarHits[ibar][ii]["ti"])*10);
                  hBsc_RiseTime->Fill(fBarHits[ibar][ii]["max"],(fBarHits[ibar][ii]["t80"]-fBarHits[ibar][ii]["t20"])*10);
               }
        }

      // FILLS BAR EVENT
      TBarEvent* BarEvent = new TBarEvent();
      for (int ibar=0; ibar<64; ibar++)
         {
            for (auto hit: fBarHits[ibar]) // Adds each hit on bottom bar
               {
                  if (hit["max"]<amplitude_cut) continue;
                  //if (hit["max"]>32750) continue; // Cut overflow events
                  BarEvent->AddADCHit(ibar,-999.,hit["max"],-999.,hit["t20"],-999,hit["integral"],-999.,hit["t80"]-hit["t20"]);
               }
            for (auto hit: fBarHits[ibar+64]) // Adds each hit on top bar
               {
                  if (hit["max"]<amplitude_cut) continue;
                  //if (hit["max"]>32750) continue; // Cut overflow events
                  BarEvent->AddADCHit(ibar+64,hit["max"],-999.,hit["20"],-999,hit["integral"],-999.,hit["t80"]-hit["t20"],-999.);
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
