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
   int threshold = 800; // Minimum ADC value to define start and end of pulse
   //int threshold = 1400; // Minimum ADC value to define start and end of pulse
   double amplitude_cut = 2000; // Minimum ADC value for peak height
   const static int sample_waveforms_to_plot = 10; // Saves a number of raw pulses for inspection
   int bscMap[64][4];
   int hit_num=0;

public:
   BscFlags* fFlags;

private:
   TH1D *hBsc_Time=NULL;
   TH2D *hBsc_TimeVsBar = NULL;
   TH1D *hBsc_Amplitude = NULL;
   TH1D *hBsc_Max = NULL;
   TH2D *hBsc_MaxVsBar = NULL;
   TH1D *hBsc_Integral = NULL;
   TH2D *hBsc_Duration = NULL;
   TH2D *hBsc_Slope = NULL;
   TH1D *hBsc_Baseline = NULL;
   TH2D *hBsc_BaselineVsBar = NULL;
   TH1D *hBsc_Saturated = NULL;
   TH1D* hSampleWaveforms[sample_waveforms_to_plot] = {NULL};
   TH1D* hWave = NULL;
   TH2D* hFitAmp = NULL;
public:

   BscModule(TARunInfo* runinfo, BscFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {

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
      hBsc_Max=new TH1D("hBsc_Max", "ADC Pulse Maximum Value;Maximum", 2000,0.,35000.);
      hBsc_MaxVsBar=new TH2D("hBsc_MaxVsBar", "ADC Pulse Maximum Value;Bar;Maximum", 128,0,128,2000,0.,35000.);
      hBsc_Integral=new TH1D("hBsc_Integral", "ADC Pulse Integral;Integral", 2000,0.,700000.);
      hBsc_Duration=new TH2D("hBsc_Duration", "ADC Pulse Duration;Pulse Amplitude;Duration [ns]",2000,0,35000,100,0,1000);
      hBsc_Slope=new TH2D("hBsc_Slope", "ADC Pulse Slope;Pulse Amplitude;Slope",2000,0,35000,2000,0,20000);
      hBsc_Baseline=new TH1D("hBsc_Baseline", "ADC Baseline;Baseline", 2000,-3000.,3000.);
      hBsc_BaselineVsBar=new TH2D("hBsc_BaselineVsBar", "ADC Baseline;Bar;Baseline", 128,0,128,2000,-3000.,3000.);
      hBsc_Saturated = new TH1D("hBsc_Saturated","Count of events with saturated ADC channels;0=Unsaturated, 1=Saturated",2,-0.5,1.5);
      hWave = new TH1D("hWave","ADC Waveform",700,0,700);
      hFitAmp=new TH2D("hFitAmp", "ADC Fit Amplitude;Measured Amplitude;Fit Amplitude",2000,0,35000,2000,0,80000);

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
      delete hBsc_Max;
      delete hBsc_MaxVsBar;
      delete hBsc_Integral;
      delete hBsc_Duration;
      delete hBsc_Slope;
      delete hBsc_Baseline;
      delete hBsc_BaselineVsBar;
      delete hBsc_Saturated;
      delete hWave;
      delete hFitAmp;
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


      TBarEvent* BarEvent = AnalyzeBars(data, runinfo);
      BarEvent->SetID(e->counter);
      BarEvent->SetRunTime(e->time);

      std::cout<<"BscModule::AnalyzeFlowEvent(...) has "<<BarEvent->GetNBars()<<" hits"<<std::endl;

      flow = new AgBarEventFlow(flow, BarEvent);

#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bscint_adc_module",timer_start);
#endif

      return flow;
   }

// -------------- Main function ----------

   TBarEvent* AnalyzeBars(const Alpha16Event* data, TARunInfo* runinfo)
   {
      std::vector<Alpha16Channel*> channels = data->hits;
      bool saturated = false;
      TBarEvent* BarEvent = new TBarEvent();

      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*
            if( ch->adc_chan >= 16 ) continue; // it's AW
            if( ch->bsc_bar < 0 ) continue;

            // CALCULATE BASELINE
            double baseline(0.);
            for(int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at( b );
            baseline /= double(pedestal_length);


            // FINDS PEAK
            int starttime = 0;
            int endtime = 0;
            int sample_length = int(ch->adc_samples.size());
            double max = 0;
            double amp = 0;
            double integral = 0;
            for (int ii=0; ii<sample_length; ii++)
               {
                  double chv = ch->adc_samples.at(ii) - baseline;
                  if (chv>threshold && starttime==0) { starttime=ii; }
                  if (chv>amp) amp=chv;
                  if (chv>threshold) integral+=chv;
                  if (chv<threshold && starttime!=0) { endtime=ii; break; }
               }
            max = amp + baseline;
            if (starttime==0 or endtime==0) continue;

            int imax=0;
            while (ch->adc_samples.at(imax) < 0.99*max and imax<sample_length-1) imax++;
            double slope;
            if (imax<starttime+2 or imax==sample_length-1) slope = 0;
            else slope = (ch->adc_samples.at(imax-1) - ch->adc_samples.at(starttime)) / ( (imax-1) - starttime);

            // CUTS
            if (amp<amplitude_cut) continue;

            // CHECKS FOR SATURATION
            if ( max > 32000 ) saturated = true;
                  

            // FITS TO FIND MAXIMUM
            hWave->Reset();
            for (int ii=0;ii<ch->adc_samples.size();ii++)
               {
                  int bin_num = hWave ->Fill(ii,ch->adc_samples.at(ii));
                  if (ch->adc_samples.at(ii) > 32000) hWave->SetBinError(bin_num,0);
                  else hWave->SetBinError(bin_num,100);
               }
            TF1 *sgfit = new TF1("sgfit","[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))",starttime-1,endtime+1);
            sgfit->SetParameters(max,imax,5,0.2);
            sgfit->SetParLimits(0,0.9*max,100*max);
            sgfit->SetParLimits(1,0,500);
            sgfit->SetParLimits(2,0,100);
            sgfit->SetParLimits(3,0,2);
            hWave->Fit("sgfit","RQ");
            double fit_amp = sgfit->GetParameter(0) - baseline;
            delete sgfit;


            // PLOTS SAMPLE WAVEFORMS
            if (hit_num < sample_waveforms_to_plot and max>32000)
               {
                  if (hSampleWaveforms[hit_num] == NULL) 
                     {
                          int length = int(ch->adc_samples.size());
                          runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
                          gDirectory->cd("bsc");
                          hSampleWaveforms[hit_num] = new TH1D(Form("hSampleWaveform%d",(hit_num)),"ADC Waveform",length,0,length);
                          for (int jj=0; jj<length; jj++)
                             {
                                hSampleWaveforms[hit_num]->Fill(jj,ch->adc_samples.at(jj));
                                if (ch->adc_samples.at(jj) > 32000)
                                   {
                                      int bin_num = hSampleWaveforms[hit_num]->GetXaxis()->FindBin(jj);
                                      hSampleWaveforms[hit_num]->SetBinError(bin_num,0);
                                   }
                                else
                                   {
                                      int bin_num = hSampleWaveforms[hit_num]->GetXaxis()->FindBin(jj);
                                      hSampleWaveforms[hit_num]->SetBinError(bin_num,100);
                                   }
                             }
                          // Fits with skewed gaussian
                          TF1 *sgf = new TF1("sgf","[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))",starttime-1,endtime+1);
                          sgf->SetParameters(max,imax,5,0.2);
                          sgf->SetParLimits(0,0.9*max,100*max);
                          sgf->SetParLimits(1,0,500);
                          sgf->SetParLimits(2,0,100);
                          sgf->SetParLimits(3,0,2);
                          hSampleWaveforms[hit_num]->Fit("sgf","RQ");
                          hit_num++;
                       }
               }

            // FILLS HISTS
            int bar = ch->bsc_bar;
            hBsc_Time->Fill(starttime*10);
            hBsc_TimeVsBar->Fill(bar,starttime*10);
            hBsc_Amplitude->Fill(amp);
            hBsc_Max->Fill(max);
            hBsc_MaxVsBar->Fill(bar,max);
            hBsc_Baseline->Fill(baseline);
            hBsc_BaselineVsBar->Fill(bar,baseline);
            hBsc_Integral->Fill(integral);
            hBsc_Duration->Fill(amp,(endtime-starttime)*10);
            hBsc_Slope->Fill(amp,slope);
            hFitAmp->Fill(amp,fit_amp);

            // FILLS BAR EVENT
            if (max > 32000) BarEvent->AddADCHit(bar,fit_amp,starttime,integral);
            else BarEvent->AddADCHit(bar,amp,starttime,integral);
         }

      if (saturated) hBsc_Saturated->Fill(1);
      else hBsc_Saturated->Fill(0);

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
