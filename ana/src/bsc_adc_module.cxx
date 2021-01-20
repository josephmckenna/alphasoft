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
#include "TF1.h"

#include "TBarEvent.hh"

class BscFlags
{
public:
   bool fPrint = false;
   bool fPulser = false; // Calibration pulser run
   bool fProtoTOF = false; // TRIUMF prototype
};

class BscModule: public TARunObject
{
private:

   // Set parameters
   int pedestal_length = 100;
   int threshold = 1400; // Minimum ADC value to define start and end of pulse // Old = 800
   double amplitude_cut = 5000; // Minimum ADC value for peak height
   double adc_dynamic_range = 2.0; // ADC dynamic range of 2 volts
   double adc_conversion = adc_dynamic_range/(TMath::Power(2,15)); // Conversion factor from 15 bit adc value to volts
   const static int sample_waveforms_to_plot = 10; // Saves a number of raw pulses for inspection
   int hit_num=0;

public:
   BscFlags* fFlags;

private:

   // Initialize histograms
   TH1D* hBars = NULL;
   TH1D* hChan = NULL;
   TH1D *hBsc_Time=NULL;
   TH2D *hBsc_TimeVsBar = NULL;
   TH1D *hBsc_Amplitude = NULL;
   TH2D *hBsc_AmplitudeVsBar = NULL;
   TH2D *hBsc_AmplitudeVsChannel = NULL;
   TH2D *hBsc_SaturatedVsBar = NULL;
   TH2D *hBsc_TimeVsChannel = NULL;
   TH1D* hWave = NULL;
   TF1* sgfit = NULL;
   TH1D* hNumChan = NULL;
   TH2D* hFitAmp = NULL;
   TH2D* hFitStartTime = NULL;
   TH2D* hFitEndTime = NULL;

public:

   BscModule(TARunInfo* runinfo, BscFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef MANALYZER_PROFILER
      ModuleName="bsc adc module";
#endif
   }

   ~BscModule()
   {   }


   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("bsc")->cd();

      if (fFlags->fProtoTOF) {
         hChan = new TH1D("hChan", "Channels hit;Channel number",16,-0.5,15.5);
         hWave = new TH1D("hWave","ADC Waveform",700,0,700);
         if( !(fFlags->fPulser) ) {
            hBsc_Amplitude=new TH1D("hBsc_Amplitude", "ADC Pulse Amplitude;Amplitude (V)", 2000,0.,4.0);
            hBsc_AmplitudeVsChannel=new TH2D("hBsc_AmplitudeVsChannel", "ADC Pulse Amplitude;Channel;Amplitude (V)", 15, -0.5, 15.5, 2000,0.,4.0);
            hFitAmp = new TH2D("hFitAmp", "ADC Fit Amplitude;Real Amplitude;Fit Amplitude",2000,0,35000,2000,0,80000);
            hFitStartTime = new TH2D("hFitStartTime", "ADC interpolated waveform start time;Real Time [ns];Fit Time [ns]",200,1200,1600,200,1200,1600);
            hFitEndTime = new TH2D("hFitEndTime", "ADC interpolated waveform end time;Real Time [ns];Fit Time [ns]",400,1200,2000,400,1200,2000);
            hBsc_Time=new TH1D("hBsc_Time", "ADC Time;ADC Time [ns]", 200,0,2000);
            hBsc_TimeVsChannel=new TH2D("hBsc_TimeVsChannel", "ADC Time;Channel;ADC Time [ns]", 16,-0.5,15.5,200,0,2000);
            hNumChan = new TH1D("hNumChan", "Number of channels hit;Number of channels",7,-0.5,6.5);
         }
         if( fFlags->fPulser ) {
            hBsc_Time=new TH1D("hBsc_Time", "ADC Time;ADC Time [ns]", 200,2840,2900);
            hBsc_TimeVsChannel=new TH2D("hBsc_TimeVsChannel", "ADC Time;Channel;ADC Time [ns]", 16,-0.5,15.5,200,2840,2900);
            hNumChan = new TH1D("hNumChan", "Number of channels hit;Number of channels",17,-0.5,16.5);
            hBsc_Amplitude=new TH1D("hBsc_Amplitude", "ADC Pulse Amplitude;Amplitude (V)", 2000,0.,4.);
            hBsc_AmplitudeVsChannel=new TH2D("hBsc_AmplitudeVsChannel", "ADC Pulse Amplitude;Channel;Amplitude (V)", 15, -0.5, 15.5, 2000,0.,4.);
         }
      }
      if ( !(fFlags->fProtoTOF) ) {
         hBars = new TH1D("hBars", "Bar ends hit;Bar end number",128,-0.5,127.5);
         hBsc_Time=new TH1D("hBsc_Time", "ADC Time;ADC Time [ns]", 200,0,2000);
         hBsc_TimeVsBar=new TH2D("hBsc_TimeVsBar", "ADC Time;Bar end number;ADC Time [ns]", 128,-0.5,127.5,200,0,2000);
         hBsc_Amplitude=new TH1D("hBsc_Amplitude", "ADC Pulse Amplitude;Amplitude (V)", 2000,0.,4.);
         hBsc_AmplitudeVsBar=new TH2D("hBsc_AmplitudeVsBar", "ADC Pulse Amplitude;Bar end number;Amplitude (V)", 128, -0.5, 127.5, 2000,0.,4.);
         hBsc_SaturatedVsBar = new TH2D("hBsc_SaturatedVsBar","Count of events with saturated ADC channels;Bar end number;0=Unsaturated, 1=Saturated",128,-0.5,127.5,2,-0.5,1.5);
         hWave = new TH1D("hWave","ADC Waveform",700,0,700);
         hFitAmp = new TH2D("hFitAmp", "ADC Fit Amplitude;Real Amplitude;Fit Amplitude",2000,0,35000,2000,0,80000);
         hFitStartTime = new TH2D("hFitStartTime", "ADC interpolated waveform start time;Real Time [ns];Fit Time [ns]",200,1000,1400,200,1000,1400);
         hFitEndTime = new TH2D("hFitEndTime", "ADC interpolated waveform end time;Real Time [ns];Fit Time [ns]",400,1000,1800,400,1000,1800);
         hNumChan = new TH1D("hNumBars", "Number of bar ends hit;Number of channels",16,-0.5,15.5);
      }
      gDirectory->mkdir("SampleWaveforms");

   }

   void EndRun(TARunInfo* runinfo)
   {
      // Write histograms
      runinfo->fRoot->fOutputFile->Write();

      // Delete histograms
      delete hBars;
      delete hChan;
      delete hBsc_Time;
      delete hBsc_TimeVsBar;
      delete hBsc_TimeVsChannel;
      delete hBsc_Amplitude;
      delete hBsc_AmplitudeVsBar;
      delete hBsc_SaturatedVsBar;
      delete hBsc_AmplitudeVsChannel;
      delete hWave;
      delete hFitAmp;
      delete hFitStartTime;
      delete hFitEndTime;
      delete hNumChan;

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
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
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

      if( fFlags->fPrint )
         printf("BscModule::AnalyzeFlowEvent(...) has %d hits\n",BarEvent->GetNBars());

      flow = new AgBarEventFlow(flow, BarEvent);
      return flow;
   }

// -------------- Main function, called for each event ----------

   TBarEvent* AnalyzeBars(const Alpha16Event* data, TARunInfo* runinfo)
   {
      std::vector<Alpha16Channel*> channels = data->hits;
      TBarEvent* BarEvent = new TBarEvent();

      int counter = 0;
      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*
            int chan = ch->adc_chan;
            int bar = ch->bsc_bar;

            // Cuts out AW channels
            if( chan >= 16 ) continue; // it's AW
            if( !(fFlags->fProtoTOF) and ch->bsc_bar < 0 ) continue;

            // Calculates baseline
            double baseline(0.);
            for(int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at( b );
            baseline /= double(pedestal_length);

            // Finds maximum amplitude
            auto max_e = std::max_element(std::begin(ch->adc_samples),std::end(ch->adc_samples));
            double max = *max_e;
            int imax = max_e-ch->adc_samples.begin();
            double amp = max - baseline;

            // Finds pulse start and end
            int start_time = 0;
            int end_time = 0;
            int sample_length = int(ch->adc_samples.size());
            for (int ii=0; ii<sample_length; ii++)
               {
                  // Exit if the pulse starts by going negative, then positive (its noise)
                  if (ch->adc_samples.at(ii) - baseline < -1*threshold && start_time==0) {start_time = -1; break;}
                  // Pulse start time is the first time it goes above threshold
                  if (ch->adc_samples.at(ii) - baseline > threshold && start_time==0) start_time = ii;
                  // Pulse end time is the first time it goes back below threshold
                  if (ch->adc_samples.at(ii) - baseline < threshold && start_time!=0) { end_time = ii; break; }
               }
                  
            // Exit if there is no pulse
            if (start_time<=0 or end_time<=0) continue;

            // Exit if the pulse is too small
            if (amp<amplitude_cut) continue;

            // Count 1 pulse in the event
            counter++;

            // Plots pulse. Sets zero error for saturated pulses since fitter ignores zero error bins
            hWave->Reset();
            for (int ii=0;ii<ch->adc_samples.size();ii++)
               {
                  int bin_num = hWave->Fill(ii,ch->adc_samples.at(ii));
                  if (ch->adc_samples.at(ii) > 32000) hWave->SetBinError(bin_num,0);
                  else hWave->SetBinError(bin_num,100);
               }
            std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
            {

            if( !(fFlags->fPulser) )  // Normal run
               {

                  // Fits pulse
                  sgfit = new TF1("sgfit","[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))",start_time-1,end_time+1);
                  sgfit->SetParameters(max,imax,5,0.2);
                  sgfit->SetParLimits(0,0.9*max,100*max);
                  sgfit->SetParLimits(1,0,500);
                  sgfit->SetParLimits(2,0,100);
                  sgfit->SetParLimits(3,0,2);
                  hWave->Fit("sgfit","RQ");
      
                  // Extrapolates amplitude and interpolates start and end times
                  double fit_amp = sgfit->GetParameter(0) - baseline;
                  double maximum_time = sgfit->GetMaximumX();
                  int error_level_save = gErrorIgnoreLevel;
                  gErrorIgnoreLevel = kFatal;
                  double fit_start_time = sgfit->GetX(threshold+baseline,start_time-1,maximum_time);
                  double fit_end_time = sgfit->GetX(threshold+baseline,maximum_time,end_time+1);
                  gErrorIgnoreLevel = error_level_save;
                  double time_before_peak = fit_start_time - maximum_time;

                  // Converts amplitude to volts
                  double amp_volts = fit_amp*adc_conversion;
      
                  // Fills histograms
                  hFitAmp->Fill(amp,fit_amp);
                  hFitStartTime->Fill(start_time*10,fit_start_time*10);
                  hFitEndTime->Fill(end_time*10,fit_end_time*10);
                  hBsc_Amplitude->Fill(amp_volts);
                  if (fFlags->fProtoTOF) {
                     hBsc_AmplitudeVsChannel->Fill(chan,amp_volts);
                  }
                  if ( !(fFlags->fProtoTOF) ) {
                     hBsc_AmplitudeVsBar->Fill(bar,amp_volts);
                  }
      
                  // Fills bar event
                  int bar = ch->bsc_bar;
                  if (fFlags->fProtoTOF) {
                     BarEvent->AddADCHit(chan,amp_volts,fit_start_time*10);
                  }
                  if ( !(fFlags->fProtoTOF) ) {
                     BarEvent->AddADCHit(bar,amp_volts,fit_start_time*10);
                  }

               }

            if( fFlags->fPulser ) // Pulser run
               {

                  // Converts amplitude to volts
                  double amp_volts = amp*adc_conversion;
      
                  // Fills histograms
                  hBsc_Amplitude->Fill(amp_volts);
                  if (fFlags->fProtoTOF) {
                     hBsc_AmplitudeVsChannel->Fill(chan,amp_volts);
                  }
                  if ( !(fFlags->fProtoTOF) ) {
                     hBsc_AmplitudeVsBar->Fill(bar,amp_volts);
                  }

                  // Fills bar event
                  int bar = ch->bsc_bar;
                  if (fFlags->fProtoTOF) {
                     BarEvent->AddADCHit(chan,amp_volts,start_time*10);
                  }
                  if ( !(fFlags->fProtoTOF) ) {
                     BarEvent->AddADCHit(bar,amp_volts,start_time*10);
                  }

               }

            // Fills histograms
            hBsc_Time->Fill(start_time*10);
            if (fFlags->fProtoTOF) {
               hBsc_TimeVsChannel->Fill(chan,start_time*10);
               hChan->Fill(chan);
            }
            if ( !(fFlags->fProtoTOF) ) {
               hBsc_TimeVsBar->Fill(bar,start_time*10);
               hBars->Fill(bar);
            }

            // Copies histogram to sample histogram
            if (hit_num < sample_waveforms_to_plot)
               {
                  runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
                  gDirectory->cd("bsc/SampleWaveforms");
                  hWave->Clone(Form("Sample Waveform %d",(hit_num)));
                  hit_num++;
               }
            if( !(fFlags->fPulser) )  // Normal run
               delete sgfit;
            }

         }

      hNumChan->Fill(counter);

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
         if( args[i] == "--bscpulser")
            fFlags.fPulser = true;
         if (args[i] == "--bscProtoTOF")
            fFlags.fProtoTOF = true;
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
