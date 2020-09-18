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
   //int threshold = 800; // Minimum ADC value to define start and end of pulse
   int threshold = 1400; // Minimum ADC value to define start and end of pulse
   double amplitude_cut = 5000; // Minimum ADC value for peak height
   const static int sample_waveforms_to_plot = 30; // Saves a number of raw pulses for inspection
   int bscMap[64][4];
   int hit_num=0;

public:
   BscFlags* fFlags;

private:
   TH1D* hChan = NULL;
   TH1D* hCount = NULL;
   TH1D *hBsc_Time=NULL;
   TH2D *hBsc_TimeVsChannel = NULL;
   TH1D *hBsc_Amplitude = NULL;
   TH2D *hBsc_AmplitudeVsChannel = NULL;
   TH1D *hBsc_Max = NULL;
   TH1D *hBsc_Integral = NULL;
   TH1D *hBsc_Duration = NULL;
   TH1D *hBsc_Baseline = NULL;
   TH2D *hBsc_BaselineVsChannel = NULL;
   TH1D *hBsc_Saturated = NULL;
   TH2D *hBsc_SaturatedVsChannel = NULL;
   TH1D* hSampleWaveforms[sample_waveforms_to_plot] = {NULL};
   TH1D* hWave = NULL;
   TH2D* hFitAmp = NULL;
   TH2D* hFitStartTime = NULL;
   TH2D* hFitEndTime = NULL;
   TH1D* hNumChan = NULL;
   TH1D* hTimeDiffA = NULL;
   TH1D* hTimeDiffB = NULL;
   TH2D* hTimeDiffBothBars = NULL;
   TH1D* hTimeDiffDiff = NULL;
   TH1D* hTOF = NULL;
   TH1D* hTimeSum = NULL;
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

      hChan = new TH1D("hChan", "Channels hit;Channel number",16,-0.5,15.5);
      hCount = new TH1D("hCount", "Counting statistics;0=Total hits,1=BarA hits,2=BarB hits,3=4 end coincidences,4=top end coincidences,5=bottom end coincidence",6,-0.5,5.5);
      hBsc_Time=new TH1D("hBsc_Time", "ADC Time;ADC Time [ns]", 200,0,2000);
      hBsc_TimeVsChannel=new TH2D("hBsc_TimeVsChannel", "ADC Time;Channel;ADC Time [ns]", 16,-0.5,15.5,200,0,2000);
      hBsc_Amplitude=new TH1D("hBsc_Amplitude", "ADC Pulse Amplitude;Amplitude", 2000,0.,50000.);
      hBsc_AmplitudeVsChannel=new TH2D("hBsc_AmplitudeVsChannel", "ADC Pulse Amplitude;Channel;Amplitude", 15, -0.5, 15.5, 2000,0.,50000.);
      hBsc_Max=new TH1D("hBsc_Max", "ADC Pulse Maximum Value;Maximum", 2000,0.,35000.);
      hBsc_Integral=new TH1D("hBsc_Integral", "ADC Pulse Integral;Integral", 2000,0.,700000.);
      hBsc_Duration=new TH1D("hBsc_Duration", "ADC Pulse Duration;Integral", 1000,0.,1000.);
      hBsc_Baseline=new TH1D("hBsc_Baseline", "ADC Baseline;Baseline", 2000,-3000.,3000.);
      hBsc_BaselineVsChannel=new TH2D("hBsc_BaselineVsChannel", "ADC Baseline;Channel;Baseline", 16,-0.5,15.5,2000,-3000.,3000.);
      hBsc_Saturated = new TH1D("hBsc_Saturated","Count of events with saturated ADC channels;0=Unsaturated, 1=Saturated",2,-0.5,1.5);
      hBsc_SaturatedVsChannel = new TH2D("hBsc_SaturatedVsChannel","Count of events with saturated ADC channels;Channel;0=Unsaturated, 1=Saturated",16,-0.5,15.5,2,-0.5,1.5);
      hWave = new TH1D("hWave","ADC Waveform",700,0,700);
      hFitAmp = new TH2D("hFitAmp", "ADC Fit Amplitude;Real Amplitude;Fit Amplitude",2000,0,35000,2000,0,80000);
      hFitStartTime = new TH2D("hFitStartTime", "ADC interpolated waveform start time;Real Time [ns];Fit Time [ns]",200,1200,1600,200,1200,1600);
      hFitEndTime = new TH2D("hFitEndTime", "ADC interpolated waveform end time;Real Time [ns];Fit Time [ns]",400,1200,2000,400,1200,2000);
      hNumChan = new TH1D("hNumChan", "Number of channels hit;Number of channels",7,-0.5,6.5);
      hTimeDiffA = new TH1D("hTimeDiffA","ADC time diff between top and bottom ends;Time difference [ns]",100,-50,50);
      hTimeDiffB = new TH1D("hTimeDiffB","ADC time diff between top and bottom ends;Time difference [ns]",100,-50,50);
      hTimeDiffBothBars = new TH2D("hTimeDiffBothBars","ADC time diff between top and bottom ends of bar A, compared to bar B;Time difference bar A [ns];Time difference bar B [ns]",100,-50,50,100,-50,50);
      hTimeDiffDiff = new TH1D("hTimeDiffDiff","Difference between bar A and bar B time differences;Time difference difference [ns]",100,-50,50);
      hTOF = new TH1D("hTOF","Time of flight;TOF [ns]",100,-50,50);
      hTimeSum = new TH1D("hTimeSum","Sum of ADC time of both ends of bar;Time sum [ns]",200,2000,3000);

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
      delete hChan;
      delete hCount;
      delete hBsc_Time;
      delete hBsc_TimeVsChannel;
      delete hBsc_Amplitude;
      delete hBsc_AmplitudeVsChannel;
      delete hBsc_Max;
      delete hBsc_Integral;
      delete hBsc_Duration;
      delete hBsc_Baseline;
      delete hBsc_BaselineVsChannel;
      delete hBsc_Saturated;
      delete hBsc_SaturatedVsChannel;
      delete hWave;
      delete hFitAmp;
      delete hFitStartTime;
      delete hFitEndTime;
      delete hNumChan;
      delete hTimeDiffA;
      delete hTimeDiffB;
      delete hTimeDiffBothBars;
      delete hTimeDiffDiff;
      delete hTOF;
      delete hTimeSum;
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
      TBarEvent* BarEvent = new TBarEvent();
      std::map<int,bool> chans_hit;
      chans_hit[0]=false;
      chans_hit[5]=false;
      chans_hit[15]=false;
      chans_hit[9]=false;
      std::map<int,double> adc_times;

      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*
            int chan = ch->adc_chan;

            // CHANNEL CUTS
            if( chan >= 16 ) continue; // it's AW
            if( chan!=0 and chan!=5 and chan!=9 and chan!=15 ) continue; //wrong channel
            //if( ch->bsc_bar < 0 ) continue;

            // CALCULATE BASELINE
            double baseline(0.);
            for(int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at( b );
            baseline /= double(pedestal_length);


            // FINDS PEAK
            int start_time = 0;
            int end_time = 0;
            int sample_length = int(ch->adc_samples.size());
            double max = 0;
            double amp = 0;
            for (int ii=0; ii<sample_length; ii++)
               {
                  double chv = ch->adc_samples.at(ii) - baseline;
                  if (chv>threshold && start_time==0) { start_time=ii; }
                  if (chv>amp) amp=chv;
                  if (chv<threshold && start_time!=0) { end_time=ii; break; }
               }
            max = amp + baseline;
            if (start_time==0 or end_time==0) continue;

            int imax=0;
            while (ch->adc_samples.at(imax) < 0.99*max and imax<sample_length-1) imax++;

            // AMPLITUE CUT
            if (amp<amplitude_cut) continue;

            // FITS TO FIND MAXIMUM AND INTERPOLATE HIT TIME
            hWave->Reset();
            for (int ii=0;ii<ch->adc_samples.size();ii++)
               {
                  int bin_num = hWave ->Fill(ii,ch->adc_samples.at(ii));
                  if (ch->adc_samples.at(ii) > 32000) hWave->SetBinError(bin_num,0);
                  else hWave->SetBinError(bin_num,100);
               }
            TF1 *sgfit = new TF1("sgfit","[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))",start_time-1,end_time+1);
            sgfit->SetParameters(max,imax,5,0.2);
            sgfit->SetParLimits(0,0.9*max,100*max);
            sgfit->SetParLimits(1,0,500);
            sgfit->SetParLimits(2,0,100);
            sgfit->SetParLimits(3,0,2);
            hWave->Fit("sgfit","RQ");
            double fit_amp = sgfit->GetParameter(0) - baseline;
            double maximum_time = sgfit->GetMaximumX();
            double fit_start_time = sgfit->GetX(threshold+baseline,start_time-1,maximum_time);
            double fit_end_time = sgfit->GetX(threshold+baseline,maximum_time,end_time+1);
            double fit_integral = sgfit->Integral(fit_start_time,fit_end_time)-baseline*(fit_end_time-fit_start_time);
            delete sgfit;

            // CHANNEL IS HIT
            chans_hit[chan]=true;
            adc_times[chan]=fit_start_time*10;


            // PLOTS SAMPLE WAVEFORMS
            if (hit_num < sample_waveforms_to_plot)
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
                          std::cout<<"GS sample waveform #"<<hit_num<<" on channel "<<chan<<std::endl;
                          // Fits with skewed gaussian
                          TF1 *sgf = new TF1("sgf","[0]*exp(-0.5*pow((x-[1])/([2]+(x<[1])*[3]*(x-[1])),2))",start_time-1,end_time+1);
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
            hChan->Fill(chan);
            hBsc_Time->Fill(start_time*10);
            hBsc_TimeVsChannel->Fill(chan,start_time*10);
            hFitAmp->Fill(amp,fit_amp);
            hFitStartTime->Fill(start_time*10,fit_start_time*10);
            hFitEndTime->Fill(end_time*10,fit_end_time*10);
            hBsc_Amplitude->Fill(fit_amp);
            hBsc_AmplitudeVsChannel->Fill(chan,fit_amp);
            hBsc_Max->Fill(max);
            hBsc_Baseline->Fill(baseline);
            hBsc_BaselineVsChannel->Fill(chan,baseline);
            hBsc_Integral->Fill(fit_integral);
            hBsc_Duration->Fill((fit_end_time-fit_start_time)*10);
            if (max>32000) {
               hBsc_Saturated->Fill(1);
               hBsc_SaturatedVsChannel->Fill(chan,1);
            }
            else {
               hBsc_Saturated->Fill(0);
               hBsc_SaturatedVsChannel->Fill(chan,0);
            }

            // FILLS BAR EVENT
            int bar = ch->bsc_bar;
            if (max > 32000) BarEvent->AddADCHit(bar,fit_amp,start_time,fit_integral);
            else BarEvent->AddADCHit(bar,amp,start_time,fit_integral);
         }

      // MULTIPLE CHANNEL COINCIDENCE
      bool barA_hit = chans_hit[9] and chans_hit[15];
      if (barA_hit) {
         hCount->Fill(1);
      }
      bool barB_hit = chans_hit[5] and chans_hit[0];
      if (barB_hit) {
         hCount->Fill(2);
      }
      bool coincidence = chans_hit[0] and chans_hit[5] and chans_hit[15] and chans_hit[9];
      if (coincidence) {
         hCount->Fill(3);

         double time_diff_A = adc_times[9]-adc_times[15];
         double time_sum_A = adc_times[9]+adc_times[15];
         double time_diff_B = adc_times[5]-adc_times[0];
         double time_sum_B = adc_times[5]+adc_times[0];
         double TOF = time_sum_B/2. - time_sum_A/2.;

         hTimeDiffA->Fill(time_diff_A);
         hTimeSum->Fill(time_sum_A);
         hTimeDiffB->Fill(time_diff_B);
         hTimeSum->Fill(time_sum_B);
         hTimeDiffBothBars->Fill(time_diff_A,time_diff_B);
         hTimeDiffDiff->Fill(time_diff_B-time_diff_A);
         hTOF->Fill(TOF);

      }

      hNumChan->Fill(chans_hit[0]+chans_hit[5]+chans_hit[15]+chans_hit[9]);
      hCount->Fill(0);



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
