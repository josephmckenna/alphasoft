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

#include"fitBV.hh"
#include"Minuit2/FunctionMinimum.h"
#include"Minuit2/VariableMetricMinimizer.h"


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

   std::map<int,TH1D*> hSampleWaveforms;
   std::map<int,TF1*> hSampleWaveformFits;

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
      gDirectory->mkdir("SampleWaveforms")->cd();
      for (int ii=0;ii<128;ii++)
         {
            TString hname = TString::Format("hSampleBar%d",ii);
            TString htitle = TString::Format("Sample ADC Waveform Channel %d",ii);
            hSampleWaveforms[ii] = new TH1D(hname,htitle,700,0,700);
            sample_plotted.push_back(false);
            if (!(fFlags->fPulser)) {
               TString fitname = TString::Format("fitSampleBar%d",ii);
               hSampleWaveformFits[ii] = new TF1(fitname,"[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))");
            }
         }
      gDirectory->cd("..");

   }

   void EndRun(TARunInfo* runinfo)
   {
      // Write histograms
      runinfo->fRoot->fOutputFile->Write();
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

            {

            if( !(fFlags->fPulser) )  // Normal run
               {

                  // Sets weights of saturated bins to zero
                  std::vector<double> weights(ch->adc_samples.size(),1.);
                  for (int ii=0;ii<ch->adc_samples.size();ii++)
                     if (ch->adc_samples.at(ii) > 32750) weights[ii] = 0.;

                  // Creates fitting FCN
                  SkewGaussFcn theFCN(ch->adc_samples, weights);

                  // Initial fitting parameters
                  std::vector<double> init_params = {max, double(imax), 5, 0.2};
                  std::vector<double> init_errors = {1000, 2, 0.05, 0.002};

                  // Creates minimizer
                  ROOT::Minuit2::VariableMetricMinimizer theMinimizer; 
                  theMinimizer.Builder().SetPrintLevel(0);
                  int error_level_save = gErrorIgnoreLevel;
                  gErrorIgnoreLevel = kFatal;

                  // Minimize to fit waveform
                  ROOT::Minuit2::FunctionMinimum min = theMinimizer.Minimize(theFCN, init_params, init_errors);
                  gErrorIgnoreLevel = error_level_save;

                  // Gets minimized parameters
                  ROOT::Minuit2::MnUserParameterState the_state = min.UserState();
                  double new_fit_amp = the_state.Value(0) - baseline;

                  // Converts amplitude to volts
                  double amp_volts = new_fit_amp*adc_conversion;
                  double amp_volts_raw = amp*adc_conversion;

                  // Copies histogram to sample histogram
                  if (!(sample_plotted.at(bar)))
                     {
                        for (int ii=0;ii<ch->adc_samples.size();ii++)
                           { hSampleWaveforms[bar]->Fill(ii,ch->adc_samples.at(ii)); }
                        hSampleWaveformFits[bar]->SetParameters(the_state.Value(0),the_state.Value(1),the_state.Value(2),the_state.Value(3));
                        hSampleWaveformFits[bar]->SetRange(start_time-1,end_time+1);
                        hSampleWaveforms[bar]->GetListOfFunctions()->Add(hSampleWaveformFits[bar]);
                        sample_plotted.at(bar)=true;
                     }
      
                  // Writes bar event
                  if (fFlags->fProtoTOF) BarEvent->AddADCHit(chan,amp_volts,amp_volts_raw,start_time*10);
                  if ( !(fFlags->fProtoTOF) ) BarEvent->AddADCHit(bar,amp_volts,amp_volts_raw,start_time*10);

               }

            if( fFlags->fPulser ) // Pulser run
               {

                  // Converts amplitude to volts
                  double amp_volts = amp*adc_conversion;
      
                  // Writes bar event
                  if (fFlags->fProtoTOF) BarEvent->AddADCHit(chan,amp_volts,start_time*10);
                  if ( !(fFlags->fProtoTOF) ) BarEvent->AddADCHit(bar,amp_volts,start_time*10);
                  // Copies histogram to sample histogram
                  if (!(sample_plotted.at(bar)))
                     {
                        for (int ii=0;ii<ch->adc_samples.size();ii++)
                           { hSampleWaveforms[bar]->Fill(ii,ch->adc_samples.at(ii)); }
                        sample_plotted.at(bar)=true;
                     }
               }
            }
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
