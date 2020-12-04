#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

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

class TdcFlags
{
public:
   bool fPrint = false;
};


class tdcmodule: public TARunObject
{
public:
   TdcFlags* fFlags;

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
   const double trb3LinearHighEnd = 450.0;

   // Container declaration
   int bscTdcMap[4][4];
   

   //Histogramm declaration
   TH1D* hTdcChan = NULL;
   TH2D* hTdcAdcTime = NULL;
   TH2D* hGoodTime = NULL;
   TH2D* hMatchedTime = NULL;
   TH1D* hNTdcHits = NULL;
   TH1D* hNMatchedHits = NULL;
   TH2D* hBarDiffAdc = NULL;
   TH2D* hBarDiffAdcM = NULL;
   TH1D* hBarDiffDiffAdc = NULL;
   TH2D* hBarDiffTdc = NULL;
   TH2D* hBarDiffTdcM = NULL;
   TH1D* hBarDiffDiffTdc = NULL;
   TH1D* hTOFADC = NULL;
   TH1D* hTOFTDC = NULL;
   TH2D* hNMatchedByChan = NULL;

   // Counter initialization
   int c_adc = 0;
   int c_tdc = 0;
   int c_adctdc = 0;
   int c_topbot = 0;
   double previous_time = 0;

public:

   tdcmodule(TARunInfo* runinfo, TdcFlags* flags): 
      TARunObject(runinfo), fFlags(flags)
   {
      ModuleName="bsc tdc module";
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
      hTdcChan = new TH1D("hTdcChan","Number of hits on tdc channel;tdc channel",16,0.5,16.5);
      hTdcAdcTime = new TH2D("hTdcAdcTime","adc vs tdc time;adc time;tdc time",250,1000,1500,200,-2.0e-6,0);
      hGoodTime = new TH2D("hGoodTime","adc vs tdc time for possible matches;adc time;tdc time",250,1000,1500,200,-2.0e-6,-0.8e-6);
      hMatchedTime = new TH2D("hMatchedTime","adc vs tdc time for matched hit on correct channel;adc time;tdc time",250,1000,1500,200,-2.0e-6,-1.2e-6);
      hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",11,-0.5,10.5);
      hNMatchedHits = new TH1D("hNMatchedHits","Number of TDC hits in correct channel;Number of tdc hits",11,-0.5,10.5);
      hBarDiffAdc = new TH2D("hBarDiffAdc","ADC time difference between ends of bars;Time difference bar A [s];Time difference bar B [s]",200,-15e-9,15e-9,200,-15e-9,15e-9);
      hBarDiffAdcM = new TH2D("hBarDiffAdcM","ADC time difference between ends of bars converted to meters;Time difference bar A [m];Time difference bar B [m]",200,-1.5,1.5,200,-1.5,1.5);
      hBarDiffTdc = new TH2D("hBarDiffTdc","TDC time difference between ends of bars;Time difference bar A [s];Time difference bar B [s]",200,-10e-9,10e-9,200,-10e-9,10e-9);
      hBarDiffTdcM = new TH2D("hBarDiffTdcM","TDC time difference between ends of bars converted to meters;Time difference bar A [m];Time difference bar B [m]",200,-1,1,200,-1,1);
      hBarDiffDiffAdc = new TH1D("hBarDiffDiffAdc","(BarA top - BarA bottom) - (BarB top - BarB bottom) for ADC;ADC Time difference [s]",200,-10e-9,10e-9);
      hBarDiffDiffTdc = new TH1D("hBarDiffDiffTdc","(BarA top - BarA bottom) - (BarB top - BarB bottom) for TDC;TDC Time difference [s]",200,-4e-9,4e-9);
      hTOFADC = new TH1D("hTOFADC","Time of flight calculated using ADC;Time of flight [s]",200,-100e-9,100e-9);
      hTOFTDC = new TH1D("hTOFTDC","Time of flight calculated using TDC;Time of flight [s]",200,-5e-9,5e-9);
      hNMatchedByChan = new TH2D("hNMatchedByChan","Number of TDC hits in correct channel;adc channel;Number of tdc hits",16,-0.5,15.5,30,-0.5,29.5);


      // Load Bscint tdc map
      TString mapfile=getenv("AGRELEASE");
      mapfile+="/ana/bscint/";
      mapfile+="protoTOF.map";
      std::ifstream fbscMap(mapfile.Data());
      if(fbscMap)
         {
            std::string comment;
            getline(fbscMap, comment);
            for(int i=0; i<4; i++)
               {
                  fbscMap >> bscTdcMap[i][0] >> bscTdcMap[i][1] >> bscTdcMap[i][2] >> bscTdcMap[i][3];
               }
            fbscMap.close();
         }

   }

   void EndRun(TARunInfo* runinfo)
   {
      // Fits time diff diff histogram
      TF1 *sgfit = new TF1("sgfit","gaus",-3e-9,3e-9);
      hBarDiffDiffTdc->Fit("sgfit","RQ");

      // Write output
      runinfo->fRoot->fOutputFile->Write();

      // Print stats
//      if( fFlags->fPrint )
         {
            printf("tdc module stats:\n");
            printf("Total number of adc hits = %d\n",c_adc);
            printf("Total number of tdc hits = %d\n",c_tdc);
            printf("Total number of adc+tdc combined hits = %d\n",c_adctdc);
            printf("Total number of top+bot hits = %d\n",c_topbot);
            printf("TDC time diff diff sigma = %.3e\n",sgfit->GetParameter(2));
         }

      // Delete histograms
      delete hTdcChan;
      delete hTdcAdcTime;
      delete hMatchedTime;
      delete hGoodTime;
      delete hNTdcHits;
      delete hNMatchedHits;
      delete hBarDiffAdc;
      delete hBarDiffTdc;
      delete hBarDiffAdcM;
      delete hBarDiffTdcM;
      delete hBarDiffDiffAdc;
      delete hBarDiffDiffTdc;
      delete hTOFADC;
      delete hTOFTDC;
      delete hNMatchedByChan;
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
      if( fFlags->fPrint ) printf("tdcmodule::AnalyzeFlowEvent run %d\n",runinfo->fRunNo);

      // Unpack Event flow
      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }

      AgEvent* age = ef->fEvent;
      if(!age)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      
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
               if( fFlags->fPrint )
                  std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event incomplete"<<std::endl;
         }
      else
         if( fFlags->fPrint )         
            std::cout<<"tdcmodule::AnalyzeFlowEvent  No TDC event"<<std::endl;

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
            double trig_time = 0;
            TdcHit* matched_tdc_hit = NULL;
            double hit_time = 0;
            double tdc_time = 0;
            double n_hits = 0;
            double n_good = 0;

            // Gets corresponding tdc channel
            int tdc_chan = -1;
            for (int i = 0;i<4;i++)
               {
                  if (bscTdcMap[i][0]==endhit->GetBar()) tdc_chan = bscTdcMap[i][1];
               }
    
            // Finds tdc hits
            for (TdcHit* tdchit: tdchits)
               {
                  // Use only rising edge
                  if (tdchit->rising_edge==0) continue;

                  // Use only fpga 1
                  if (int(tdchit->fpga)!=1) continue;

                  // Finds trigger time (on channel 0)
                  if (int(tdchit->chan)==0 and trig_time==0) trig_time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time);

                  // Skip channel 0
                  if (int(tdchit->chan)==0) continue;

                  // Counts hits
                  n_hits++;
                  if (int(tdchit->chan)==tdc_chan) n_good++;

                  // Calculates hit time
                  double a_hit_time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time); 
                  double a_final_time = a_hit_time-trig_time;

                  // Yoinks the very first tdc hit on the right channel and calls it a day
                  if (tdc_time == 0 and int(tdchit->chan)==tdc_chan) {
                     hit_time = a_hit_time;
                     tdc_time = a_final_time;
                     matched_tdc_hit = tdchit;
                  }

                  // Fills histograms
                  hTdcChan->Fill(int(tdchit->chan));
                  hTdcAdcTime->Fill(endhit->GetADCTime(),a_final_time);
                  if (int(tdchit->chan)==tdc_chan)
                     {
                        hGoodTime->Fill(endhit->GetADCTime(),a_final_time);
                     }  

               }

            // Fills histograms
            hMatchedTime->Fill(endhit->GetADCTime(),tdc_time);
            hNTdcHits->Fill(n_hits);
            hNMatchedHits->Fill(n_good);
            hNMatchedByChan->Fill(int(endhit->GetBar()),n_good);

            // Writes tdc data to hit
            endhit->SetTDCHit(tdc_time);
            c_adctdc+=1;
         }
   }

   void CombineEnds(TBarEvent* barEvt)
   {
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      for (EndHit* tophit: endhits)
         {
            if (!(tophit->IsTDCMatched())) continue; // REQUIRE TDC MATCHING

            // Gets first hit bar info from map
            int top_chan = tophit->GetBar();
            int bot_chan = -1;
            int bar_num = -1;
            int end_num = -1;
            for (int i = 0;i<4;i++) {
                  if (bscTdcMap[i][0]==tophit->GetBar()) {
                        bar_num = bscTdcMap[i][2];
                        end_num = bscTdcMap[i][3];
                     }
               }

            // Only continue for hits on top end
            if (end_num!=0) continue;

            // Find corresponding bottom hit info from map
            for (int i=0;i<4;i++) {
               if (bscTdcMap[i][2]==bar_num and bscTdcMap[i][3]==1) {
                  bot_chan = bscTdcMap[i][0];
               }
            }

            // Exit if none found
            if (bot_chan==-1) continue;

            // Find bottom hit
            EndHit* bothit = NULL;
            for (EndHit* hit: endhits)
               {
                  if (hit->GetBar()==bot_chan) bothit = hit;
               }

            // Exit if none found
            if (!bothit) continue;
            
            // Adds a BarHit containing the top hit and bottom hit
            barEvt->AddBarHit(bothit,tophit,bar_num);
            c_topbot+=1;

         }
   }

   void CalculateZ(TBarEvent* barEvt) {

      std::vector<BarHit*> barhits = barEvt->GetBars();
      double diff_adc_0 = 0;
      double diff_tdc_0 = 0;
      double diff_adc_1 = 0;
      double diff_tdc_1 = 0;
      for (BarHit* hit: barhits)
         {
            int bar = hit->GetBar();
            double diff_tdc = hit->GetTopHit()->GetTDCTime() - hit->GetBotHit()->GetTDCTime();
            double diff_adc = (hit->GetTopHit()->GetADCTime() - hit->GetBotHit()->GetADCTime())*1e-9;
            if (bar==0)
               {
                  diff_adc_0 = diff_adc;
                  diff_tdc_0 = diff_tdc;
               }
            if (bar==1)
               {
                  diff_adc_1 = diff_adc;
                  diff_tdc_1 = diff_tdc;
               }
         }
      if (diff_adc_0!=0 and diff_adc_1!=0)
         {
            double c = 2.99792e8;
            double refrac = 1.58;
            double factor = c/refrac * 0.5;
            hBarDiffTdc->Fill(diff_tdc_0,diff_tdc_1);
            hBarDiffAdc->Fill(diff_adc_0,diff_adc_1);
            hBarDiffTdcM->Fill(diff_tdc_0*factor,diff_tdc_1*factor);
            hBarDiffAdcM->Fill(diff_adc_0*factor,diff_adc_1*factor);
            hBarDiffDiffTdc->Fill(diff_tdc_1-diff_tdc_0);
            hBarDiffDiffAdc->Fill(diff_adc_1-diff_adc_0);
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



};


class tdcModuleFactory: public TAFactory
{
public:
   TdcFlags fFlags;
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

      for (unsigned i=0; i<args.size(); i++) { 
         if (args[i] == "--bscprint")
            fFlags.fPrint = true; 
      }
   }

   void Finish()
   {
      printf("tdcModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("tdcModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new tdcmodule(runinfo,&fFlags);
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
