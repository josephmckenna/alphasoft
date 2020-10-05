#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "AnalysisTimer.h"

#include "TBarEvent.hh"


class tdcmodule: public TARunObject
{
private:

   // Constant value declaration
   const double max_adc_tdc_diff_t = 40e-9; // s, maximum allowed time between ADC time and matched TDC time
   const double max_top_bot_diff_t = 1e-6; // s, maximum allowed time between top TDC time and matched bot TDC time
      // https://daq.triumf.ca/elog-alphag/alphag/1961
   const double epoch_freq = 97656.25; // 200MHz/(2<<11); KO+Thomas approved right frequency
   const double coarse_freq = 200.0e6; // 200MHz
      // linear calibration:
      // $ROOTANASYS/libAnalyzer/TRB3Decoder.hxx
   const double trb3LinearLowEnd = 17.0;
   const double trb3LinearHighEnd = 473.0;

   // Container declaration
   int bscTdcMap[64][5];
   

   //Histogramm declaration
   TH2D* hTdcAdcBar = NULL;
   TH2D* hTdcAdcTime = NULL;
   TH2D* hMatchedTime = NULL;
   TH1D* hNTdcHits = NULL;
   TH1D* hNMatchedHits = NULL;
   TH2D* hMatchedDelta = NULL;
   TH2D* hDiffAdc = NULL;
   TH2D* hDiffTdc = NULL;
   TH1D* hTOFADC = NULL;
   TH1D* hTOFTDC = NULL;

   // Counter initialization
   int c_adc = 0;
   int c_tdc = 0;
   int c_adctdc = 0;
   int c_topbot = 0;

public:

   tdcmodule(TARunInfo* runinfo): TARunObject(runinfo)
   {
      printf("tdcmodule::ctor!\n");
   }

   ~tdcmodule()
   {
      printf("tdcmodule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("bsc_tdc_module")->cd();

      // Histogramm declaration
      hTdcAdcTime = new TH2D("hTdcAdcTime","adc vs tdc time;adc time;tdc time",250,1000,1500,200,-2.0e-6,0);
      hTdcAdcBar = new TH2D("hTdcAdcBar","Hits on each bar;adc bar;tdc bar",128,-0.5,127.5,128,-0.5,127.5);
      hMatchedTime = new TH2D("hMatchedTime","adc vs tdc time for matched hit on correct channel;adc time;tdc time",250,1000,1500,200,-2.0e-6,-1.2e-6);
      hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",100,-0.5,99.5);
      hNMatchedHits = new TH1D("hNMatchedHits","Number of TDC hits in channel corresponding to ADC hit;Number of tdc hits",30,-0.5,29.5);
      hMatchedDelta = new TH2D("hMatchedDelta","Time difference between covnverted adc time and tdc time for matched hit on correct channel;Bar end number;Delta t [s]",128,-0.5,127.5,200,-40e-9,40e-9);
      hDiffAdc = new TH2D("hDiffAdc","ADC time difference between ends;Bar number;Time [s]",64,-0.5,63.5,200,-50e-9,50e-9);
      hDiffTdc = new TH2D("hDiffTdc","TDC time difference between ends;Bar number;Time [s]",64,-0.5,63.5,200,-50e-9,50e-9);
      hTOFADC = new TH1D("hTOFADC","Time of flight calculated using ADC;Time of flight [s]",200,-1000e-9,1000e-9);
      hTOFTDC = new TH1D("hTOFTDC","Time of flight calculated using TDC;Time of flight [s]",200,-20e-9,20e-9);


      // Load Bscint tdc map
      TString mapfile=getenv("AGRELEASE");
      mapfile+="/ana/bscint/";
      mapfile+="bscint_tdc.map";
      std::ifstream fbscMap(mapfile.Data());
      if(fbscMap)
         {
            std::string comment;
            getline(fbscMap, comment);
            for(int bar_ind=0; bar_ind<64; bar_ind++)
               {
                  fbscMap >> bscTdcMap[bar_ind][0] >> bscTdcMap[bar_ind][1] >> bscTdcMap[bar_ind][2] >> bscTdcMap[bar_ind][3] >> bscTdcMap[bar_ind][4];
               }
            fbscMap.close();
         }
      
   }

   void EndRun(TARunInfo* runinfo)
   {
      // Write output
      runinfo->fRoot->fOutputFile->Write();

      // Print stats
      std::cout<<"tdc module stats:"<<std::endl;
      std::cout<<"Total number of adc hits = "<<c_adc<<std::endl;
      std::cout<<"Total number of tdc hits = "<<c_tdc<<std::endl;
      std::cout<<"Total number of adc+tdc combined hits = "<<c_adctdc<<std::endl;
      std::cout<<"Total number of top+bot hits = "<<c_topbot<<std::endl;

      // Delete histograms
      delete hTdcAdcBar;
      delete hTdcAdcTime;
      delete hMatchedTime;
      delete hNTdcHits;
      delete hNMatchedHits;
      delete hMatchedDelta;
      delete hDiffAdc;
      delete hDiffTdc;
      delete hTOFADC;
      delete hTOFTDC;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   // Main function
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      std::cout<<"tdc module analysing event"<<std::endl;
   

      // Unpack Event flow
      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif
      AgEvent* age = ef->fEvent;

      // Unpack tdc data from event
      TdcEvent* tdc = age->tdc;
      TrigEvent* trig = age->trig;

      if( tdc )
         {
            if( tdc->complete )
               {
                  AgBarEventFlow *bef = flow->Find<AgBarEventFlow>();
                  if (!bef) return flow;
                  TBarEvent *barEvt = bef->BarEvent;
                  if (!barEvt) return flow;

                  // MAIN FUNCTIONS
                  AddTDCdata(barEvt,tdc,trig);
                  CombineEnds(barEvt);
                  CalculateZ(barEvt);
                  CalculateTOF(barEvt);
               }
            else
               std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event incomplete"<<std::endl;
         }
      else
         std::cout<<"tdcmodule::AnalyzeFlowEvent  No TDC event"<<std::endl;
//      #ifdef _TIME_ANALYSIS_
//         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bsc_tdc_module",timer_start);
//      #endif

      return flow;
      
   }

   //________________________________
   // MAIN FUNCTIONS

   // Adds data from the tdc to the end hits
   void AddTDCdata(TBarEvent* barEvt, TdcEvent* tdc, TrigEvent* trig)
   {
      // Get endhits from adc module
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      c_adc+=endhits.size();

      // Get tdc data
      std::vector<TdcHit*> tdchits = tdc->hits;
      c_tdc+=tdchits.size();

      // Find a match for each ADC hit from among the TDC hits.
      // Its basically tinder for SiPM data.
      for (EndHit* endhit: endhits)
         {
            int adc_bar = int(endhit->GetBar());
            double trig_time = 0;
            double n_hits = 0;
            double n_good = 0;

            TdcHit* best_match = NULL;
            double tdc_time = 0;
            double smallest_delta = 1;

            // Convert adc time
            double linM = 0.000000001; // from ns to s
            double linB = -2782862e-12;
            double converted_time = linM * endhit->GetADCTime() + linB;

            for (TdcHit* tdchit: tdchits)
               {
                  // Use only rising edge
                  if (tdchit->rising_edge==0) continue;

                  // Skip negative channels
                  if (int(tdchit->chan)<=0) continue;

                  // Gets bar number
                  int tdc_bar = fpga2barID(int(tdchit->fpga),int(tdchit->chan));

                  // Finds trigger time
                  double trig_time=FindTriggerTime(tdchits,tdc_bar%64);

                  // Counts hits
                  n_hits++;
                  if (tdc_bar==adc_bar) n_good++;

                  // Gets hit time
                  double hit_time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time); 
                  double final_time = hit_time-trig_time;

                  // Find tdc hit with closest time to converted adc time on correct channel
                  double delta = converted_time - final_time;
                  if (TMath::Abs(delta)<TMath::Abs(smallest_delta) and tdc_bar==adc_bar)
                     {
                        smallest_delta = delta;
                        best_match = tdchit;
                        tdc_time = final_time;
                     }  

                  // Fills histograms
                  hTdcAdcBar->Fill(adc_bar,tdc_bar);
                  hTdcAdcTime->Fill(endhit->GetADCTime(),final_time);
               }

            // Fills histograms
            hMatchedTime->Fill(endhit->GetADCTime(),tdc_time);
            hNTdcHits->Fill(n_hits);
            hNMatchedHits->Fill(n_good);
            hMatchedDelta->Fill(endhit->GetBar(),smallest_delta);

            // Writes tdc data to hit
            endhit->SetTDCHit(tdc_time);
            c_adctdc+=1;
         }

   }

   void CombineEnds(TBarEvent* barEvt)
   {
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      for (EndHit* bothit: endhits)
         {
            if (!(bothit->IsTDCMatched())) continue; // REQUIRE TDC MATCHING

            // Gets first hit bar info from map
            int bot_bar = bothit->GetBar();

            // Only continue for hits on bottom end
            if (bot_bar>=64 or bot_bar<0) continue;

            // Find top hit
            EndHit* tophit = NULL;
            for (EndHit* hit: endhits)
               {
                  if (hit->GetBar()==bot_bar+64) tophit = hit;
               }

            // Exit if none found
            if (!tophit) continue;
            
            // Adds a BarHit containing the top hit and bottom hit
            barEvt->AddBarHit(bothit,tophit,bot_bar);
            c_topbot+=1;

         }
   }

   void CalculateZ(TBarEvent* barEvt) {

      std::vector<BarHit*> barhits = barEvt->GetBars();
      for (BarHit* hit: barhits)
         {
            int bar = hit->GetBar();
            double diff_tdc = hit->GetTopHit()->GetTDCTime() - hit->GetBotHit()->GetTDCTime();
            double diff_adc = (hit->GetTopHit()->GetADCTime() - hit->GetBotHit()->GetADCTime())*1e-9;
            hDiffAdc->Fill(bar,diff_adc);
            hDiffTdc->Fill(bar,diff_tdc);
         }

   }

   void CalculateTOF(TBarEvent* barEvt) {
         
      std::vector<BarHit*> barhits = barEvt->GetBars();
      for (int i=0;i<barhits.size();i++)
         {
            BarHit* hit1 = barhits.at(i);
            int bar1 = hit1->GetBar();
            for (int j=i+1;j<barhits.size();j++)
               {
                  BarHit* hit2 = barhits.at(j);
                  int bar2 = hit2->GetBar();
                  if (bar1==bar2) continue;
                  double t_ADC_1 = (hit1->GetTopHit()->GetADCTime() + hit1->GetBotHit()->GetADCTime())/2.;
                  double t_ADC_2 = (hit2->GetTopHit()->GetADCTime() + hit2->GetBotHit()->GetADCTime())/2.;
                  double t_TDC_1 = (hit1->GetTopHit()->GetTDCTime() + hit1->GetBotHit()->GetTDCTime())/2.;
                  double t_TDC_2 = (hit2->GetTopHit()->GetTDCTime() + hit2->GetBotHit()->GetTDCTime())/2.;
                  double TOF_ADC = TMath::Abs(t_ADC_1 - t_ADC_2);
                  double TOF_TDC = TMath::Abs(t_TDC_1 - t_TDC_2);
                  hTOFADC->Fill(TOF_ADC);
                  hTOFTDC->Fill(TOF_TDC);
               }
         }
   }

   //________________________________
   // HELPER FUNCTIONS

   double GetFinalTime( uint32_t epoch, uint16_t coarse, uint16_t fine ) // Calculates time from tdc data (in seconds)
   {
      double B = double(fine) - trb3LinearLowEnd;
      double C = trb3LinearHighEnd - trb3LinearLowEnd;
      return double(epoch)/epoch_freq +  double(coarse)/coarse_freq - (B/C)/coarse_freq;
   }

   double FindTriggerTime(std::vector<TdcHit*> hits, int bar)
   {
      double trig_time=0;
      int tdc_fpga=bscTdcMap[bar][1]-1;
      for (auto hit: hits)
         {
            if ( hit->chan != 0 ) continue; // Trigger events are on fpga channel 0
            if ( ! hit->rising_edge ) continue; // Only the rising edge trigger event is good
            if ( hit->fpga > tdc_fpga ) break;
            if ( hit->fpga==tdc_fpga ) 
               {
                  trig_time = GetFinalTime(hit->epoch,hit->coarse_time,hit->fine_time);
               }
         }
      return trig_time;
   }

   int fpga2barID(int fpga, int chan) // Looks up fpga number and channel number in map and returns bar number (0-127)
   {
      int bar = -1;
      if(chan==0)
         return -1;
      else
         for(bar=0; bar<64; bar ++)
            {
               if(fpga==bscTdcMap[bar][1]-1)
                  {
                     if(chan== (bscTdcMap[bar][2]-1)*16+bscTdcMap[bar][3]+1)
                        return bar+64; //top side
                     else if(chan== (bscTdcMap[bar][2]-1)*16+bscTdcMap[bar][4]+1)
                        return bar; //bot side
                  }
            }
      std::cout<<"bsc_tdc_module failed to get bar number for FPGA: "<<fpga<<"\tCHAN: "<<chan<<std::endl;
      return -1;
   }

};


class tdcModuleFactory: public TAFactory
{
public:
   void Help()
   {   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("tdcModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) { }
   }

   void Finish()
   {
      printf("tdcModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("tdcModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new tdcmodule(runinfo);
   }
};

static TARegister tar(new tdcModuleFactory);


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
