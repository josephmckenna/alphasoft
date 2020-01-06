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
   int pedestal_length = 100;
   int threshold = 1400;
   double amplitude_cut = 2000;
   int bscMap[64][4];
   std::map<int,std::vector<std::map<std::string,double>>> fBarHits; // Maps bar# to vector of hit maps. Each hit map maps feature names to values.

public:
   BscFlags* fFlags;

private:
   TH1D *hBsc_Time=NULL;
   TH2D *hBsc_TimeVsBar = NULL;
   TH1D *hBsc_Amplitude = NULL;
   TH1D *hBsc_Integral = NULL;
   TH2D *hBsc_Duration = NULL;
   TH2D* hBsc_RiseTime = NULL;
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
      hBsc_Duration=new TH2D("hBsc_Duration", "ADC Pulse Duration;Pulse Amplitude;Duration [ns]",2000,0,25000,100,0,1000);
      hBsc_RiseTime = new TH2D("hBsc_Risetime","ADC Pulse Rise Time;Pulse Amplitude;Rise Time [ns]",2000,0,35000,200,0,2000);

      // Loads Bscint map
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
      const AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;
      #ifdef _TIME_ANALYSIS_
      START_TIMER
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


            // FINDS PEAKS
            bool inpeak = false;
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
                        std::map<std::string,double> hit; // structure to store hit info
                        hit["ti"] = double(starttime); // peak start
                        hit["tf"] = double(ii); // peak end
                        hit["tavg"] = (double(ii)+double(starttime))/2.; // average of start and end times
                        double max = *std::max_element(ch->adc_samples.begin()+starttime,ch->adc_samples.begin()+ii);
                        hit["max"] = max; // maximum peak value
                        hit["integral"] = std::accumulate(ch->adc_samples.begin()+starttime,ch->adc_samples.begin()+ii,0); // peak integral
                        // Calculates rise time (time between 20% max and max)
                        int max_index = std::max_element(ch->adc_samples.begin(),ch->adc_samples.end()) - ch->adc_samples.begin();
                        int t20=-999;
                        for (int j=max_index;j>=0;j--) // starts at peak max, goes backwards
                           {
                              double chvv = ch->adc_samples.at(j);
                              if (chvv>max*0.2) t20 = j;
                              else break; // stops when channel value less than 20% peak max
                           }
                        hit["tmax"] = max_index; // time of peak max
                        hit["t20"] = t20; // time of 20% of peak max
                        fBarHits[ch->bsc_bar].push_back(hit);
                     }
               }
         }

      // FILLS HISTS
      for (int ibar=0; ibar<128; ibar++)
         {
            for (int ii=0; ii<int(fBarHits[ibar].size()); ii++)
               {
                  if (fBarHits[ibar][ii]["max"]<amplitude_cut) continue;
                  hBsc_Time->Fill(fBarHits[ibar][ii]["tavg"]*10);
                  hBsc_TimeVsBar->Fill(ibar,fBarHits[ibar][ii]["tavg"]*10);
                  hBsc_Amplitude->Fill(fBarHits[ibar][ii]["max"]);
                  hBsc_Integral->Fill(fBarHits[ibar][ii]["integral"]);
                  hBsc_Duration->Fill(fBarHits[ibar][ii]["max"],(fBarHits[ibar][ii]["tf"]-fBarHits[ibar][ii]["ti"])*10);
                  hBsc_RiseTime->Fill(fBarHits[ibar][ii]["max"],(fBarHits[ibar][ii]["tmax"]-fBarHits[ibar][ii]["t20"])*10);
               }
        }

      // FILLS BAR EVENT
      TBarEvent* BarEvent = new TBarEvent();
      for (int ibar=0; ibar<64; ibar++)
         {
            for (auto hit: fBarHits[ibar]) // Adds each hit on bottom bar
               {
                  if (hit["max"]<amplitude_cut) continue;
                  BarEvent->AddADCHit(ibar,-999.,hit["max"],-999.,hit["ti"],-999,hit["integral"],-999.,hit["tmax"]-hit["t20"]);
                  ////BarEvent->AddADCHit(ibar,-999.,hit["max"],-999.,hit["tavg"],-999,hit["integral"],-999.,hit["tmax"]-hit["t20"]);
               }
            for (auto hit: fBarHits[ibar+64]) // Adds each hit on top bar
               {
                  if (hit["max"]<amplitude_cut) continue;
                  BarEvent->AddADCHit(ibar+64,hit["max"],-999.,hit["ti"],-999,hit["integral"],-999.,hit["tmax"]-hit["t20"],-999.);
                  ////BarEvent->AddADCHit(ibar+64,hit["max"],-999.,hit["tavg"],-999,hit["integral"],-999.,hit["tmax"]-hit["t20"],-999.);
               }

         }


      return BarEvent;
   }

   void ResetBarHits()
   {
      fBarHits.clear();
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
