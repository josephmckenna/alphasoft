#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

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
   int threshold = 1400; // Minimum ADC value to define start and end of pulse
   double amplitude_cut = 2000; // Minimum ADC value for peak height
   const static int sample_waveforms_to_plot = 0; // Saves a number of raw pulses for inspection
   int bscMap[64][4];

public:
   BscFlags* fFlags;

private:
   TH1D *hBsc_Time=NULL;
   TH2D *hBsc_TimeVsBar = NULL;
   TH1D *hBsc_Amplitude = NULL;
   TH1D *hBsc_Integral = NULL;
   TH2D *hBsc_Duration = NULL;
   TH1D* hSampleWaveforms[sample_waveforms_to_plot];
public:

   BscModule(TARunInfo* runinfo, BscFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      ModuleName="bsc adc module";
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
      for (auto i=0;i<sample_waveforms_to_plot;i++) hSampleWaveforms[i] = new TH1D(Form("hSampleWaveform%d",(i)),"ADC Waveform",700,0,700);

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
      for (auto i=0;i<sample_waveforms_to_plot;i++) delete hSampleWaveforms[i];
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
      if( fFlags->fPrint ) printf("BscModule::Analyze, run %d\n", runinfo->fRunNo);
         
      const AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
   
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

      if( fFlags->fPrint )
         printf("BscModule::AnalyzeFlowEvent(...) has %d hits\n",BarEvent->GetNBars());

      flow = new AgBarEventFlow(flow, BarEvent);
      return flow;
   }

// -------------- Main function ----------

   TBarEvent* AnalyzeBars(const Alpha16Event* data)
   {
      std::vector<Alpha16Channel*> channels = data->hits;
      int hit_num = 0;
      TBarEvent* BarEvent = new TBarEvent();

      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*
            if( ch->adc_chan >= 16 ) continue; // it's AW
            if( ch->bsc_bar < 0 ) continue;

            // PLOTS SAMPLE WAVEFORMS
            if (hit_num < sample_waveforms_to_plot)
               {
                  int length = int(ch->adc_samples.size());
                  for (int ii=0; ii<length; ii++) hSampleWaveforms[hit_num]->Fill(ii,ch->adc_samples.at(ii));
                  hit_num++;
               }

            // CALCULATE BASELINE
            double baseline(0.);
            for(int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at( b );
            baseline /= double(pedestal_length);


            // FINDS PEAK
            int starttime = 0;
            int endtime = 0;
            int sample_length = int(ch->adc_samples.size());
            double max = 0;
            double integral = 0;
            for (int ii=0; ii<sample_length; ii++)
               {
                  double chv = ch->adc_samples.at(ii) - baseline;
                  if (chv>threshold && starttime==0) { starttime=ii; }
                  if (chv>max) max=chv;
                  if (chv>threshold) integral+=chv;
                  if (chv<threshold && starttime!=0) { endtime=ii; break; }
               }
            if (starttime==0 or endtime==0) continue;

            // CUTS
            if (max<amplitude_cut) continue;

            // FILLS HISTS
            int bar = ch->bsc_bar;
            hBsc_Time->Fill(starttime*10);
            hBsc_TimeVsBar->Fill(bar,starttime*10);
            hBsc_Amplitude->Fill(max);
            hBsc_Integral->Fill(integral);
            hBsc_Duration->Fill(max,(endtime-starttime)*10);

            // FILLS BAR EVENT
            BarEvent->AddADCHit(bar,max,starttime,integral);
         }

      return BarEvent;
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
