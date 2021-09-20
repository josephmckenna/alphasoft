#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include "AnaSettings.hh"
#include "json.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <numeric>

#include "TMath.h"
#include "TH1D.h"
#include "TF1.h"

#include "TBarEvent.hh"

class BscFlags
{
public:
   bool fPrint = false;
   bool fPulser = false; // Calibration pulser run
   bool fProtoTOF = false; // TRIUMF prototype
   AnaSettings* ana_settings=0;
};

class BscModule: public TARunObject
{
private:

   // Set parameters
   int pedestal_length = 100;
   int threshold; // Minimum ADC value to define start and end of pulse
   double amplitude_cut; // Minimum ADC value for peak height
   double adc_dynamic_range = 2.0; // ADC dynamic range of 2 volts
   double adc_conversion = adc_dynamic_range/(TMath::Power(2,15)); // Conversion factor from 15 bit adc value to volts
   std::vector<bool> sample_plotted;

public:
   BscFlags* fFlags;

private:

   TH1D* hWave = NULL;
   TF1* sgfit = NULL;

public:

   BscModule(TARunInfo* runinfo, BscFlags* flags)
      : TARunObject(runinfo), fFlags(flags),
        threshold(flags->ana_settings->GetInt("BscModule","pulse_threshold")),
        amplitude_cut(flags->ana_settings->GetDouble("BscModule","ADCamplitude"))
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="bsc adc module";
#endif
      
   }

   ~BscModule()
   {   }


   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("bsc")->cd();
      hWave = new TH1D("hWave","ADC Waveform",700,0,700);
      gDirectory->mkdir("SampleWaveforms");
      for (int i=0;i<128;i++) sample_plotted.push_back(false);

   }

   void EndRun(TARunInfo* runinfo)
   {
      // Write histograms
      runinfo->fRoot->fOutputFile->Write();
      delete hWave;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if( fFlags->fPrint ) printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if( fFlags->fPrint ) printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }


   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      const AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
#ifdef HAVE_MANALYZER_PROFILER
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

            // Exit if bad bar
            if (bar<0 or bar>128) continue;

            // Count 1 pulse in the event
            counter++;

            std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
            {

            if( !(fFlags->fPulser) )  // Normal run
               {

                  // Plots pulse. Sets zero error for saturated pulses since fitter ignores zero error bins
                  hWave->Reset();
                  for (int ii=0;ii<ch->adc_samples.size();ii++)
                     {
                        int bin_num = hWave->Fill(ii,ch->adc_samples.at(ii));
                        if (ch->adc_samples.at(ii) > 32000) hWave->SetBinError(bin_num,0);
                        else hWave->SetBinError(bin_num,100);
                     }
                  // Fits pulse
                  sgfit = new TF1("sgfit","[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))",start_time-1,end_time+1);
                  sgfit->SetParameters(max,imax,5,0.2);
                  sgfit->SetParLimits(0,0.9*max,100*max);
                  sgfit->SetParLimits(1,0,500);
                  sgfit->SetParLimits(2,0,100);
                  sgfit->SetParLimits(3,0,2);
                  hWave->Fit("sgfit","RQ0");
                  // Copies histogram to sample histogram
                  if (!(sample_plotted.at(bar)))
                     {
                        runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
                        gDirectory->cd("bsc/SampleWaveforms");
                        hWave->Clone(Form("Sample Ch  %d",(bar)));
                        sample_plotted.at(bar)=true;
                     }
      
                  // Extrapolates amplitude and interpolates start and end times
                  double fit_amp = sgfit->GetParameter(0) - baseline;
                  double maximum_time = sgfit->GetMaximumX();
                  int error_level_save = gErrorIgnoreLevel;
                  gErrorIgnoreLevel = kFatal;
                  double fit_start_time = sgfit->GetX(threshold+baseline,start_time-1,maximum_time);
                  gErrorIgnoreLevel = error_level_save;
                  delete sgfit;

                  // Converts amplitude to volts
                  double amp_volts = fit_amp*adc_conversion;
                  double amp_volts_raw = amp*adc_conversion;

      
                  // Writes bar event
                  //int bar = ch->bsc_bar;
                  if (fFlags->fProtoTOF) BarEvent->AddADCHit(chan,amp_volts,amp_volts_raw,fit_start_time*10);
                  if ( !(fFlags->fProtoTOF) ) BarEvent->AddADCHit(bar,amp_volts,amp_volts_raw,fit_start_time*10);

               }

            if( fFlags->fPulser ) // Pulser run
               {

                  // Converts amplitude to volts
                  double amp_volts = amp*adc_conversion;
      
                  // Writes bar event
                  //int bar = ch->bsc_bar;
                  if (fFlags->fProtoTOF) BarEvent->AddADCHit(chan,amp_volts,start_time*10);
                  if ( !(fFlags->fProtoTOF) ) BarEvent->AddADCHit(bar,amp_volts,start_time*10);
                  // Copies histogram to sample histogram
                  if (!(sample_plotted.at(bar)))
                     {
                        hWave->Reset();
                        for (int ii=0;ii<ch->adc_samples.size();ii++)
                           {
                              int bin_num = hWave->Fill(ii,ch->adc_samples.at(ii));
                           }
                        runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
                        gDirectory->cd("bsc/SampleWaveforms");
                        hWave->Clone(Form("Sample Waveform Channel %d",(bar)));
                        sample_plotted.at(bar)=true;
                     }
                  delete sgfit;
               }
            }
         }
      return BarEvent;
   }

   static double sgfunc(double* x, double* p)
    {
            // "[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))"
            double xx=x[0];
            double skew=0.;
            if( xx>p[1] ) skew = p[3]*(xx-p[1]);
            double t=(xx-p[1])/(p[2]+skew);
            return p[0]*exp(-0.5*pow(t,2.));
    }

};


class BscModuleFactory: public TAFactory
{
public:
   BscFlags fFlags;
public:
   void Help()
   {   
      printf("BscModuleFactory::Help\n");
      printf("\t--anasettings /path/to/settings.json\t\t load the specified analysis settings\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("BscModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
         if (args[i] == "--bscprint")
            fFlags.fPrint = true;
         if( args[i] == "--bscpulser")
            fFlags.fPulser = true;
         if (args[i] == "--bscProtoTOF")
            fFlags.fProtoTOF = true;
         if( args[i] == "--anasettings" ) 
            json=args[i+1];
      }
      fFlags.ana_settings=new AnaSettings(json.Data());
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
