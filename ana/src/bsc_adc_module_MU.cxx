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
#include "TTree.h"
#include "TBranch.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"

#include "TBarEvent.hh"

class BscFlags {
public:
   bool fPrint = false;
};

class BscModule : public TARunObject {
private:
   // Set parameters
   int              pedestal_length          = 100;
   int              threshold                = 1400; // Minimum ADC value to define start and end of pulse // Old = 800
   double           amplitude_cut            = 5000; // Minimum ADC value for peak height
   const static int sample_waveforms_to_plot = 10;   // Saves a number of raw pulses for inspection
   int              hit_num                  = 0;

public:
   BscFlags *fFlags;

private:
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
   std::vector<int>     bar_id;
   std::vector<int>     bar_tini;
   std::vector<int>     bar_tend;
   std::vector<int>     bar_base;
   std::vector<double>  bar_amp;
   std::vector<double>  bar_vmax;
   std::vector<int>     bar_tmax;
   std::vector<bool>    bar_pair_flag;
   std::vector<bool>    bar_amp_flag;
   std::vector<bool>    bar_time_flag;
   std::vector<std::vector<int> > bar_waveforms; //! int

   void ResetBarVectors()
   {
      bar_id.clear();
      bar_tini.clear();
      bar_tend.clear();
      bar_base.clear();
      bar_amp.clear();
      bar_vmax.clear();
      bar_tmax.clear();
      bar_pair_flag.clear();
      bar_amp_flag.clear();
      bar_time_flag.clear();
      std::vector<std::vector<int> >::iterator it;
      for (it = bar_waveforms.begin(); it != bar_waveforms.end(); it++) {
         (*it).clear();
      }
      bar_waveforms.clear();
   }

public:
   BscModule(TARunInfo *runinfo, BscFlags *flags) : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName = "bsc adc module MU";
#endif
   }

   ~BscModule() {}

   void BeginRun(TARunInfo *runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("bsc_MU")->cd();

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
      tBSC->Branch("bar_amp", "std::vector<double>", &bar_amp);
      tBSC->Branch("bar_vmax", "std::vector<double>", &bar_vmax);
      tBSC->Branch("bar_tmax", "std::vector<int>", &bar_tmax);
      tBSC->Branch("bar_pair_flag", "std::vector<bool>", &bar_pair_flag);
      tBSC->Branch("bar_amp_flag", "std::vector<bool>", &bar_amp_flag);
      tBSC->Branch("bar_time_flag", "std::vector<bool>", &bar_time_flag);
      tBSC->Branch("bar_waveforms",&bar_waveforms);

      // tBSC->Branch("bar_waveforms", &bar_waveforms);

      gDirectory->mkdir("SampleWaveforms");
   }

   void EndRun(TARunInfo *runinfo)
   {

      // Write histograms
      runinfo->fRoot->fOutputFile->Write();
      // runinfo->fRoot->fOutputFile->cd();
      // tBSC->Write();
      // Delete histograms
      delete hBars;
      delete hBsc_Time;
      delete hBsc_TimeVsBar;
      delete hBsc_Amplitude;
      delete hBsc_AmplitudeVsBar;
      delete hBsc_SaturatedVsBar;
      delete hWave;
      delete hFitAmp;
      delete hFitStartTime;
      delete hFitEndTime;
      delete hNumBars;
      delete tBSC;
   }

   void PauseRun(TARunInfo *runinfo) { printf("PauseRun, run %d\n", runinfo->fRunNo); }

   void ResumeRun(TARunInfo *runinfo) { printf("ResumeRun, run %d\n", runinfo->fRunNo); }

   TAFlowEvent *AnalyzeFlowEvent(TARunInfo *runinfo, TAFlags *flags, TAFlowEvent *flow)
   {
      if (fFlags->fPrint) printf("BscModule::Analyze, run %d\n", runinfo->fRunNo);

      const AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent) {
#ifdef HAVE_MANALYZER_PROFILER
         *flags |= TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
#ifdef _TIME_ANALYSIS_
      START_TIMER
#endif
      const AgEvent *     e    = ef->fEvent;
      const Alpha16Event *data = e->a16;

      if (!data) {
         std::cout << "BscModule::AnalyzeFlowEvent(...) No Alpha16Event in AgEvent # " << e->counter << std::endl;
         return flow;
      }

      if (fFlags->fPrint) printf("BscModule::AnalyzeFlowEvent(...) Event number is : %d \n", data->counter);

      // std::cout << e->counter << ") "; // MU
      TBarEvent *BarEvent = AnalyzeBars(data, runinfo);
      BarEvent->SetID(e->counter);
      BarEvent->SetRunTime(e->time);

      // std::cout << std::endl;
      ///< Filling the tBSC TTree
      event = e->counter;
      tBSC->Fill();

      if (fFlags->fPrint) printf("BscModule::AnalyzeFlowEvent(...) has %d hits\n", BarEvent->GetNBars());

      flow = new AgBarEventFlow(flow, BarEvent);

#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow = new AgAnalysisReportFlow(flow, "bscint_adc_module", timer_start);
#endif

      return flow;
   }

   // -------------- Main function ----------

   TBarEvent *AnalyzeBars(const Alpha16Event *data, TARunInfo *runinfo)
   {

      std::vector<Alpha16Channel *> channels = data->hits;
      TBarEvent *                   BarEvent = new TBarEvent();

      int num_bars = 0;
      ResetBarVectors(); ///< Empty the bar vectors (holding the id of the bars that are in the data)
      for (unsigned int i = 0; i < channels.size(); ++i) { ///< Loop over the channels (bar/SiPM that fired?)
         auto &ch = channels.at(i); // Alpha16Channel*

         // Cuts out adc channels and bad bar numbers
         if (ch->adc_chan >= 16) continue; // it's AW
         int bar = ch->bsc_bar;
         if (bar < 0) continue;

         // Calculates baseline
         double baseline(0.);
         for (int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at(b);
         baseline /= double(pedestal_length);
         // Finds maximum amplitude
         auto   max_e = std::max_element(std::begin(ch->adc_samples), std::end(ch->adc_samples));
         double max   = *max_e;
         int    imax  = max_e - ch->adc_samples.begin();
         double amp   = max - baseline;

         // Finds pulse start and end
         int start_time    = 0;
         int end_time      = 0;
         int sample_length = int(ch->adc_samples.size());
         for (int ii = 0; ii < sample_length; ii++) {
            // Exit if the pulse starts by going negative, then positive (its noise)
            if (ch->adc_samples.at(ii) - baseline < -1 * threshold && start_time == 0) {
               start_time = -1;
               break;
            }
            // Pulse start time is the first time it goes above threshold
            if (ch->adc_samples.at(ii) - baseline > threshold && start_time == 0) start_time = ii;
            // Pulse end time is the first time it goes back below threshold
            if (ch->adc_samples.at(ii) - baseline < threshold && start_time != 0) {
               end_time = ii;
               break;
            }
         }

         // Exit if there is no pulse
         // Exit if the pulse is too small
         if (start_time <= 0 or end_time <= 0) continue;
         if (amp < amplitude_cut) continue;

         // Count 1 pulse in event
         num_bars++;
         // std::cout << bar << " "; // MU

         // Plots pulse. Sets zero error for saturated pulses since fitter ignores zero error bins
         hWave->Reset();
         std::vector<int> wave;
         for (int ii = 0; ii < ch->adc_samples.size(); ii++) {
            int bin_num = hWave->Fill(ii, ch->adc_samples.at(ii));
            wave.push_back(ch->adc_samples.at(ii));
            if (ch->adc_samples.at(ii) > 32000)
               hWave->SetBinError(bin_num, 0);
            else
               hWave->SetBinError(bin_num, 100);
         }
         ///< Check if there is also the corresponding "top/bottom" pair
         bool is_paired = false;
         for(auto & id : bar_id) {
            if(bar==(id+64)||bar==(id-64)) is_paired = true;
         }
         bar_id.push_back(bar);
         bar_base.push_back(baseline);
         bar_tini.push_back(start_time*10);
         bar_tend.push_back(end_time*10);
         bar_amp.push_back(amp);
         bar_vmax.push_back(max);
         bar_tmax.push_back(imax*10); ///< To have the time in ns (??? - to be checked)
         ///< Flags
         bar_pair_flag.push_back(is_paired);
         if (start_time <= 0 or end_time <= 0) bar_time_flag.push_back(0); else bar_time_flag.push_back(1);
         if (amp < amplitude_cut) bar_amp_flag.push_back(0); else bar_amp_flag.push_back(1);
         bar_waveforms.push_back(wave);
         ///< ##########################
         ///< FITTING THE PULSE 
         ///< ##########################
         // Fits pulse
         double fit_amp;
         double maximum_time;
         double fit_start_time;
         double fit_end_time;
         {
            // Root's fitting routines are often not thread safe, lock globally
#ifdef MODULE_MULTITHREAD
            std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
#endif
            TString fname = TString::Format("sgfit_%d_%d", data->counter, i);
            TF1 *   sgfit =
               new TF1(fname, "[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))", start_time - 1, end_time + 1);
            // TF1 *sgfit = new TF1(fname,sgfunc,start_time-1,end_time+1,4);
            sgfit->SetParameters(max, imax, 5, 0.2);
            sgfit->SetParLimits(0, 0.9 * max, 100 * max);
            sgfit->SetParLimits(1, 0, 500);
            sgfit->SetParLimits(2, 0, 100);
            sgfit->SetParLimits(3, 0, 2);

            // hWave->Fit("sgfit","RQ0");
            hWave->Fit(fname, "RQ0");
            // hWave->Fit(sgfit,"RQ0");
            // Extrapolates amplitude and interpolates start and end times
            fit_amp              = sgfit->GetParameter(0) - baseline;
            maximum_time         = sgfit->GetMaximumX();
            int error_level_save = gErrorIgnoreLevel;
            gErrorIgnoreLevel    = kFatal;
            fit_start_time       = sgfit->GetX(threshold + baseline, start_time - 1, maximum_time);
            fit_end_time         = sgfit->GetX(threshold + baseline, maximum_time, end_time + 1);
            gErrorIgnoreLevel    = error_level_save;

            // Copies histogram to sample histogram
            if (hit_num < sample_waveforms_to_plot) {
               runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
               gDirectory->cd("bsc_MU/SampleWaveforms");
               hWave->Clone(Form("Sample Waveform %d", (hit_num)));
               hit_num++;
            }
            delete sgfit;
         }

         // Fills histograms
         hBars->Fill(bar);
         hBsc_Time->Fill(start_time * 10);
         hBsc_TimeVsBar->Fill(bar, start_time * 10);
         hFitAmp->Fill(amp, fit_amp);
         hFitStartTime->Fill(start_time * 10, fit_start_time * 10);
         hFitEndTime->Fill(end_time * 10, fit_end_time * 10);
         hBsc_Amplitude->Fill(fit_amp);
         hBsc_AmplitudeVsBar->Fill(bar, fit_amp);
         hBsc_SaturatedVsBar->Fill(bar, (max > 32000));

         // Fills bar event
         BarEvent->AddADCHit(bar, fit_amp, fit_start_time * 10);
      } ///< Loop over the channels (bar/SiPM that fired?)

      hNumBars->Fill(num_bars);
      return BarEvent;
   }

   static double sgfunc(double *x, double *p)
   {
      // "[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))"
      double xx   = x[0];
      double skew = 0.;
      if (xx > p[1]) skew = p[3] * (xx - p[1]);
      double t = (xx - p[1]) / (p[2] + skew);
      return p[0] * exp(-0.5 * pow(t, 2.));
   }
};

class BscModuleFactory : public TAFactory {
public:
   BscFlags fFlags;

public:
   void Help() {}
   void Usage() { Help(); }
   void Init(const std::vector<std::string> &args)
   {
      printf("BscModuleFactory::Init!\n");

      for (unsigned i = 0; i < args.size(); i++) {
         if (args[i] == "--bscprint") fFlags.fPrint = true;
      }
   }

   void Finish() { printf("BscModuleFactory::Finish!\n"); }

   TARunObject *NewRunObject(TARunInfo *runinfo)
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
