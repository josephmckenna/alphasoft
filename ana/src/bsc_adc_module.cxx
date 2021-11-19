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
#include "TH2D.h"
#include "TTree.h"
#include "TF1.h"

#include "TBarEvent.hh"

#include"fitBV.hh"
#include"Minuit2/FunctionMinimum.h"
#include"Minuit2/VariableMetricMinimizer.h"


class BscFlags
{
public:
   bool fPrint = false;
   bool fDiag = false;
   bool fPulser = false; // Calibration pulser run
   bool fProtoTOF = false; // TRIUMF prototype
   bool fFitAll = false; // If false, fits only saturated waveforms
   bool fBackground = false;
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

// Initialize histograms
   TH1D * hBars               = NULL;
   TH1D * hBsc_Time           = NULL;
   TH2D * hBsc_TimeVsBar      = NULL;
   TH1D * hBsc_Amplitude      = NULL;
   TH2D * hBsc_AmplitudeVsBar = NULL;
   TH2D * hBsc_SaturatedVsBar = NULL;
   TH1D * hWave               = NULL;
   TH2D * hFitAmp             = NULL;
   TH2D * hFitStartTime       = NULL;
   TH2D * hFitEndTime         = NULL;
   TH1D * hNumBars            = NULL;
   TTree *tBSC                = NULL;

   int    event;
   std::vector<int>     bar_id;                 ///< bar/SiPM
   std::vector<int>     bar_tini;               ///< t start
   std::vector<int>     bar_tend;               ///< t end
   std::vector<int>     bar_base;               ///< amplitude baseline
   std::vector<double>  bar_ampl;                ///< signal amplitude (max - baseline)
   std::vector<double>  bar_vmax;               ///< maximum value (in V)
   std::vector<int>     bar_tmax;               ///< time of maximum
   std::vector<bool>    bar_pair_flag;          ///< is bar/SiPM coupled? (read by the two ends)
   std::vector<bool>    bar_ampl_flag;          ///< is the amplitude above threshold?
   std::vector<bool>    bar_time_flag;          ///< is the time in the correct window?
   std::vector<std::vector<int> > bar_waveforms;///< waveforms (all bins)
   std::vector<double>  fit_ampl;               ///< [from fit] signal amplitude (p0-baseline) 
   std::vector<double>     fit_par0;               ///< [from fit] t end
   std::vector<double>     fit_par1;               ///< [from fit] t start
   std::vector<double>     fit_par2;               ///< [from fit] t end
   std::vector<double>     fit_par3;               ///< [from fit] t end
   
   //std::vector<int>     fit_tmax;               ///< [from fit] time of maximum

   void ResetBarVectors()
   {
      bar_id.clear();
      bar_tini.clear();
      bar_tend.clear();
      bar_base.clear();
      bar_ampl.clear();
      bar_vmax.clear();
      bar_tmax.clear();
      bar_pair_flag.clear();
      bar_ampl_flag.clear();
      bar_time_flag.clear();
      std::vector<std::vector<int> >::iterator it;
      for (it = bar_waveforms.begin(); it != bar_waveforms.end(); it++) {
         (*it).clear();
      }
      bar_waveforms.clear();
      fit_par0.clear();
      fit_par1.clear();
      fit_par2.clear();
      fit_par3.clear();
      fit_ampl.clear();
      //fit_tmax.clear();
   }

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
      if (fFlags->fDiag) {
         gDirectory->mkdir("bsc")->cd();
         hBars     = new TH1D("hBars", "Bar ends hit;Bar end number", 128, -0.5, 127.5);
         hBsc_Time = new TH1D("hBsc_Time", "ADC Time;ADC Time [ns]", 200, 0, 2000);
         hBsc_TimeVsBar =
            new TH2D("hBsc_TimeVsBar", "ADC Time;Bar end number;ADC Time [ns]", 128, -0.5, 127.5, 200, 0, 2000);
         hBsc_Amplitude      = new TH1D("hBsc_Amplitude", "ADC Pulse Amplitude;Amplitude", 2000, 0., 50000.);
         hBsc_AmplitudeVsBar = new TH2D("hBsc_AmplitudeVsBar", "ADC Pulse Amplitude;Bar end number;Amplitude", 128, -0.5,
                                        127.5, 2000, 0., 50000.);
         hBsc_SaturatedVsBar = new TH2D(
            "hBsc_SaturatedVsBar", "Count of events with saturated ADC channels;Bar end number;0=Unsaturated, 1=Saturated",
            128, -0.5, 127.5, 2, -0.5, 1.5);
         hWave   = new TH1D("hWave", "ADC Waveform", 700, 0, 700);
         hFitAmp = new TH2D("hFitAmp", "ADC Fit Amplitude;Real Amplitude;Fit Amplitude", 2000, 0, 35000, 2000, 0, 80000);
         hFitStartTime = new TH2D("hFitStartTime", "ADC interpolated waveform start time;Real Time [ns];Fit Time [ns]",
                                  200, 1000, 1400, 200, 1000, 1400);
         hFitEndTime   = new TH2D("hFitEndTime", "ADC interpolated waveform end time;Real Time [ns];Fit Time [ns]", 400,
                                1000, 1800, 400, 1000, 1800);
         hNumBars      = new TH1D("hNumBars", "Number of bar ends hit;Number of channels", 16, -0.5, 15.5);
         tBSC          = new TTree("tBSC", "BSC output tree");
         // event number
         // Set object pointer
         // Set branch addresses and branch pointers


         tBSC->Branch("event", &event);
         // info about the scintillator bars (BCS) that fired (bar == SiPM channel)
         tBSC->Branch("bar_id", "std::vector<int>", &bar_id);
         tBSC->Branch("bar_tini", "std::vector<int>", &bar_tini);
         tBSC->Branch("bar_tend", "std::vector<int>", &bar_tend);
         tBSC->Branch("bar_base", "std::vector<int>", &bar_base);
         tBSC->Branch("bar_ampl", "std::vector<double>", &bar_ampl);
         tBSC->Branch("bar_vmax", "std::vector<double>", &bar_vmax);
         tBSC->Branch("bar_tmax", "std::vector<int>", &bar_tmax);
         tBSC->Branch("bar_pair_flag", "std::vector<bool>", &bar_pair_flag);
         tBSC->Branch("bar_ampl_flag", "std::vector<bool>", &bar_ampl_flag);
         tBSC->Branch("bar_time_flag", "std::vector<bool>", &bar_time_flag);
         tBSC->Branch("bar_waveforms",&bar_waveforms);
         tBSC->Branch("fit_ampl", "std::vector<double>", &fit_ampl);
         tBSC->Branch("fit_par0", "std::vector<double>", &fit_par0);
         tBSC->Branch("fit_par1", "std::vector<double>", &fit_par1);
         tBSC->Branch("fit_par2", "std::vector<double>", &fit_par2);
         tBSC->Branch("fit_par3", "std::vector<double>", &fit_par3);
         //tBSC->Branch("fit_tmax", "std::vector<int>", &fit_tmax);


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

      ///< Filling the tBSC TTree
      event = e->counter;
      tBSC->Fill();

      if( fFlags->fPrint )
         printf("BscModule::AnalyzeFlowEvent(...) has %d hits\n",BarEvent->GetNBars());

      flow = new AgBarEventFlow(flow, BarEvent);
      return flow;
   }

// -------------- Main function, called for each event ----------

   TBarEvent* AnalyzeBars(const Alpha16Event* data, TARunInfo* runinfo)
   {
      ResetBarVectors();

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
                  //if (ch->adc_samples.at(ii) - baseline < -1*threshold && start_time==0) {start_time = -1; break;}
                  // Pulse start time is the first time it goes above threshold
                  if (ch->adc_samples.at(ii) - baseline > threshold && start_time==0) start_time = ii;
                  // Pulse end time is the first time it goes back below threshold
                  if (ch->adc_samples.at(ii) - baseline < threshold && start_time!=0) { end_time = ii; break; }
               }

            
            if(!(fFlags->fBackground))
            {
               // Exit if there is no pulse
               if (start_time<=0 or end_time<=0) continue; 
               //    bar_time_flag.push_back(0); 
               // else 
               //    bar_time_flag.push_back(1); 

               // Exit if the pulse is too small
               if (amp<amplitude_cut)  continue;
               //    bar_ampl_flag.push_back(0); 
               // else 
               //    bar_ampl_flag.push_back(1);
            }else{
               if (start_time>0 && end_time>0) continue; 
               if (amp>amplitude_cut)  continue;
            }

            // Exit if bad bar
            if (bar<0 or bar>128) continue;

            // Count 1 pulse in the event
            counter++;

            double amp_raw = amp;  
            double amp_fit=-999; 
            double par0 = -999;
            double par1 = -999;
            double par2 = -999;
            double par3 = -999;
            if(start_time>0 && end_time>0 && amp>=amplitude_cut)    
            {
               if( !(fFlags->fPulser) )  // Normal run
                  {
                     // Converts amplitude to volts
                     amp_fit=amp;
                     double amp_volts = amp*adc_conversion;
                     double amp_volts_raw = amp*adc_conversion;

                     // Sets weights of saturated bins to zero
                     std::vector<double> weights(ch->adc_samples.size(),1.);
                     bool saturated = false;
                     for (int ii=0;ii<ch->adc_samples.size();ii++) {
                        if (ch->adc_samples.at(ii) > 32750) {
                           weights[ii] = 0.;
                           saturated = true;
                        }
                     }

                     if (fFlags->fFitAll or saturated) {

                        // Creates fitting FCN
                        SkewGaussFcn theFCN(ch->adc_samples, weights);

                        // Initial fitting parameters
                        std::vector<double> init_params = {max, double(imax), (end_time-start_time)/4., 0.2};
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
                        par0 = the_state.Value(0);
                        par1 = the_state.Value(1);
                        par2 = the_state.Value(2);
                        par3 = the_state.Value(3);

                        if (fFlags->fDiag) {
                           if (!(sample_plotted.at(bar)))
                              hSampleWaveformFits[bar]->SetParameters(the_state.Value(0),the_state.Value(1),the_state.Value(2),the_state.Value(3));
                        }

                        // Saves fit voltage
                        amp_fit = new_fit_amp;
                     }

                     // Copies histogram to sample histogram
                     if (fFlags->fDiag) {
                        if (!(sample_plotted.at(bar)))
                           {
                              for (int ii=0;ii<ch->adc_samples.size();ii++)
                                 { hSampleWaveforms[bar]->Fill(ii,ch->adc_samples.at(ii)); }
                              if (saturated) {
                                 hSampleWaveformFits[bar]->SetRange(start_time-1,end_time+1);
                                 hSampleWaveforms[bar]->GetListOfFunctions()->Add(hSampleWaveformFits[bar]);
                              }
                              sample_plotted.at(bar)=true;
                           }
                     }

                     // Writes bar event
                     if (saturated) {
                        if (fFlags->fProtoTOF) BarEvent->AddADCHit(chan,amp_volts,amp_volts_raw,start_time*10);
                        if ( !(fFlags->fProtoTOF) ) BarEvent->AddADCHit(bar,amp_volts,amp_volts_raw,start_time*10);
                     }
                     if (!saturated) {
                        if (fFlags->fProtoTOF) BarEvent->AddADCHit(chan,amp_volts_raw,start_time*10);
                        if ( !(fFlags->fProtoTOF) ) BarEvent->AddADCHit(bar,amp_volts_raw,start_time*10);
                     }


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

            std::vector<int> wave;
            for (int ii = 0; ii < ch->adc_samples.size(); ii++) {
               wave.push_back(ch->adc_samples.at(ii));
            }

            ///< ###############################################
            ///< FILLING THE BSC TTREE 
            ///< ###############################################
            ///< Check if there is also the corresponding "top/bottom" pair
            bool is_paired = false;
            for(unsigned int kk=0; kk<bar_id.size(); kk++) {
               if(bar==(bar_id[kk]+64)||bar==(bar_id[kk]-64)) {
                  is_paired = true;
                  bar_pair_flag[kk] = true;
                  break;
               }
            }
            ///< Flags
            bar_pair_flag.push_back(is_paired);
            bar_id.push_back(bar);
            bar_base.push_back(baseline);
            bar_tini.push_back(start_time*10);
            bar_tend.push_back(end_time*10);
            bar_ampl.push_back(amp_raw); //amp_volts_raw
            bar_vmax.push_back(max);
            bar_tmax.push_back(imax*10); ///< To have the time in ns (??? - to be checked)
            bar_waveforms.push_back(wave);
            ///< Fit values
            fit_ampl.push_back(amp_fit);
            fit_par0.push_back(par0);//(fit_start_time*10); 
            fit_par1.push_back(par1);//(fit_start_time*10);
            fit_par2.push_back(par2);//(fit_end_time*10);
            fit_par3.push_back(par3);//(fit_end_time*10);
   
            // Fills histograms
            hBars->Fill(bar);
            hBsc_Time->Fill(start_time * 10);
            hBsc_TimeVsBar->Fill(bar, start_time * 10);
            if(amp_fit>=0){
               hFitAmp->Fill(amp_raw, amp_fit);
               hBsc_Amplitude->Fill(amp_fit);
               hBsc_AmplitudeVsBar->Fill(bar, amp_fit);
            }
            hBsc_SaturatedVsBar->Fill(bar, (max > 32000));
            //hFitStartTime->Fill(start_time * 10, fit_start_time * 10);
            //hFitEndTime->Fill(end_time * 10, fit_end_time * 10);
            
         }
      
      hNumBars->Fill(counter);
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
      printf("\t--bscdiag\t\t\tenables analysis histograms\n");
      printf("\t--bscpulser\t\t\tanalyze run with calibration pulser data instead of cosmics/hbar data\n");
      printf("\t--bscProtoTOF\t\t\tanalyze run with with TRIUMF prototype instead of full BV\n");
      printf("\t--bscprint\t\t\tverbose mode\n");
      printf("\t--bscfitall\t\t\tfits all bsc adc waveforms instead of only saturated waveforms\n");
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
         if (args[i] == "--bscdiag")
            fFlags.fDiag = true;
         if( args[i] == "--bscpulser")
            fFlags.fPulser = true;
         if (args[i] == "--bscProtoTOF")
            fFlags.fProtoTOF = true;
         if( args[i] == "--anasettings" ) 
            json=args[i+1];
         if( args[i] == "--bscfitall")
            fFlags.fFitAll = true;
         if( args[i] == "--bscbkg")
            fFlags.fBackground = true;
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
