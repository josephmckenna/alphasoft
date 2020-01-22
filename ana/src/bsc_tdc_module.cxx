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
   const double max_adc_tdc_diff_t = 100e-9; // s, maximum allowed time between ADC time and matched TDC time
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
   TH2D* hTdc = NULL;
   TH2D* hHitsPerBarPerEvent = NULL;
   TH2D* hHitsPerEvent = NULL;
   TH2D* hAdctimeVTdc = NULL;
   TH2D* hAdctimeVTdcAll = NULL;
   TH2D* hAdctimeMinusTdc = NULL;
   TH2D* hAdcTdcMatch = NULL;
   TH2D* hBotMinusTop = NULL;
   TH2D* hBotVTop = NULL;
   TH2D* hBotTopMatch = NULL;
   TH2D* hZed = NULL;


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
      hTdc = new TH2D("hTdc","TDC time since trig;Bar;Time [ns]",128,-0.5,127.5,2000,0.,700.);
      hHitsPerBarPerEvent = new TH2D("hHitsPerBarPerEvent","Number of TDC hits per ADC hit;Bar Number;# TDC hits",128,-0.5,127.5,10,-0.5,9.5);
      hHitsPerEvent = new TH2D("hHitsPerEvent","Number of hits per event (all bars);# ADC hits;# TDC hits",20,0,20,140,0,140);
      hAdctimeVTdc = new TH2D("hAdctimeVTdc","Corrected ADC time vs TDC time;ADC time [ns];TDC time [ns]",2000,0.,700.,2000,0.,700.);
      hAdctimeVTdcAll = new TH2D("hAdctimeVTdcAll","Corrected ADC time vs TDC time including TDC hits not matched to each ADC hit;ADC time [ns];TDC time [ns]",2000,0.,700.,2000,0.,700.);
      hAdctimeMinusTdc = new TH2D("hAdctimeMinusTdc","Calculated ADC time minus TDC time;Bar;Time Difference [ns]",128,-0.5,127.5,2000,-100,100);
      hAdcTdcMatch = new TH2D("hAdcTdcMatch","Number of ADC hits which found a matching TDC hit;Bar;0 = Unmatched, 1 = Matched",128,-0.5,127.5,2,-0.5,1.5);
      hBotMinusTop = new TH2D("hBotMinusTop","Bottom TDC time minus top TDC time;Bar;Time Difference [ns]",64,-0.5,63.5,2000,-60,60);
      hBotVTop = new TH2D("hBotVTop","Bottom and top TDC time; Bottom time [ns];Top time [ns]",2000,0.,500.,2000,0.,700.);
      hBotTopMatch = new TH2D("hBotTopMatch","Number of bottom hits which found a matching top hit;Bar;0 = Unmatched, 1 = Matched",64,-0.5,63.5,2,-0.5,1.5);
      hZed = new TH2D("hZed","Zed calculated from TDC time;Bar;Zed [m]",64,-0.5,63.5,1000,-3.,3.);


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
      runinfo->fRoot->fOutputFile->Write();

      delete hTdc;
      delete hHitsPerBarPerEvent;
      delete hHitsPerEvent;
      delete hAdctimeVTdc;
      delete hAdctimeVTdcAll;
      delete hAdctimeMinusTdc;
      delete hAdcTdcMatch;
      delete hBotMinusTop;
      delete hBotVTop;
      delete hBotTopMatch;
      delete hZed;
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
                  TBarEvent *barEvt = bef->BarEvent;
                  if (!barEvt) return flow;

                  // MAIN FUNCTIONS
                  std::vector<std::vector<double>> TDCHits = getTDCHits(tdc,trig);
                  mergeADCTDC(barEvt, TDCHits);
                  combineTopBot(barEvt);
                  FillHistos(barEvt);
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

   // GET TDC HIT TIMES SORTED BY BAR
   std::vector<std::vector<double>> getTDCHits(TdcEvent* tdc, TrigEvent* trig) 
   {
      std::vector<std::vector<double>> TDCHits(128);
      std::vector<TdcHit*> hits = tdc->hits; 
      for (int i=0;i<int(hits.size());i++)
         {
            TdcHit* hit = hits.at(i);
            if (hit->chan<=0) continue;
            if (hit->rising_edge==0) continue;
            int barID = fpga2barID(int(hit->fpga),int(hit->chan));
            if (barID==-1) continue;
            double hit_time = GetFinalTime(hit->epoch,hit->coarse_time,hit->fine_time); 
            double trig_time=FindTriggerTime(hits,barID%64);
            double final_time = hit_time-trig_time;
            TDCHits[barID].push_back(final_time); 

            // FILLS HISTOS
            hTdc->Fill(barID,TimeConversion(final_time));
         }
      return TDCHits;
   }

   // ADD TDC DATA TO THE ADC HITS
   // Note:: In the majority of cases, there are two (rising edge) hits in each event.
   //        (There is always only 1 ADC hit per event).
   //        The second is likely some reflection or other electronic interference.
   //        My current solution is to simply match to the closest time hit on the bar,
   //        which is almost always the first hit.
   void mergeADCTDC( TBarEvent* barEvt, std::vector<std::vector<double>> TDCHits) 
   {
      std::vector<EndHit*> endhits = barEvt->GetEndHits();

      // COUNTING STATS
      int nTDC = 0;
      for (int i=0;i<128;i++) nTDC += TDCHits.at(i).size();
      hHitsPerEvent->Fill(endhits.size(),nTDC);

      for (EndHit* endhit: endhits)
         {
            int bar = endhit->GetBar();

            // KEEP UNMATCHED HITS
            hHitsPerBarPerEvent->Fill(bar,TDCHits[bar].size());
            if (TDCHits[bar].size()==0) continue;

            // CONVERT ADC TIMESTAMP TO SECONDS
            double linM = 0.00000001; // 100 Msamples/s
            double linB = -2784291.121733e-12;
            double adctime = linM * endhit->GetADCTime() + linB;
            
            // MATCH ADC HITS TO CLOSEST TIME TDC HIT
            std::vector<double> time_diff; 
            for (double tdc: TDCHits[bar])
               {
                  hAdctimeVTdcAll->Fill(TimeConversion(adctime),TimeConversion(tdc));
                  time_diff.push_back(TMath::Abs(tdc-adctime));
               }
            int min_diff_index = std::min_element(time_diff.begin(), time_diff.end())-time_diff.begin();
            if (time_diff.at(min_diff_index) > max_adc_tdc_diff_t) continue; // MAXIMUM ALLOWED TIME DIFFERENCE
            double tdctime = TDCHits[bar].at(min_diff_index);
            TDCHits[bar].erase(TDCHits[bar].begin()+min_diff_index); // WITHOUT REPLACEMENT

            // WRITES BAR EVENT
            endhit->SetTDCHit( bar, tdctime);
            
            hAdctimeVTdc->Fill(TimeConversion(adctime),TimeConversion(tdctime));
            hAdctimeMinusTdc->Fill(bar,TimeConversion(adctime)-TimeConversion(tdctime));
         }
   }

   // MATCHES TOP AND BOTTOM HITS
   void combineTopBot(TBarEvent* barEvt) 
   {
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      for (EndHit* bothit: endhits)
         {
            if (!(bothit->IsTDCMatched())) continue; // REQUIRE TDC MATCHING
            int bar = bothit->GetBar();
            if (bar>=64) continue;
            double bottime = bothit->GetTDCTime();
            double min_diff = max_top_bot_diff_t;
            EndHit* tophit;
            bool matched = false;
            for (EndHit* hit: endhits)
               {
                  if (hit->GetBar()!=bar+64) continue;
                  double diff = TMath::Abs(hit->GetTDCTime()-bottime);
                  if (diff<min_diff)
                     {
                        min_diff = diff;
                        tophit = hit;
                        matched = true;
                     }
               }
            hBotTopMatch->Fill(bar,matched);
            if (!matched) continue;
            barEvt->AddBarHit(bothit,tophit);
         }
   }

   // FILLS HISTOS
   void FillHistos(TBarEvent* barEvt)
   {
      std::vector<EndHit*> EndHits = barEvt->GetEndHits();
      std::vector<BarHit*> BarHits = barEvt->GetBars();

      for (EndHit* hit: EndHits)
         {
            int bar = hit->GetBar();
            hAdcTdcMatch->Fill(bar,hit->IsTDCMatched());
         }
      for (BarHit* hit: BarHits)
         {
            int bar = hit->GetBar();
            hBotMinusTop->Fill(bar,(hit->GetBotHit()->GetTDCTime()-hit->GetTopHit()->GetTDCTime())*1e9);
            hBotVTop->Fill(TimeConversion(hit->GetBotHit()->GetTDCTime()),TimeConversion(hit->GetTopHit()->GetTDCTime()));
            hZed->Fill(bar,hit->GetTDCZed());
         }
   }

   //________________________________
   // HELPER FUNCTIONS

    double FindTriggerTime(std::vector<TdcHit*> hits, int bar)
   {
      double trig_time=0;
      int tdc_fpga=bscTdcMap[bar][1]-1;
      for (auto hit: hits)
         {
            if ( hit->chan != 0 ) continue; // Trigger events are on fpga channel 0
            if ( ! hit->rising_edge ) continue; // Only the rising edge trigger event is good
            if ( hit->fpga > tdc_fpga ) break;
            if ( hit->fpga==tdc_fpga ) trig_time = GetFinalTime(hit->epoch,hit->coarse_time,hit->fine_time);
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

   double TimeConversion (double time ) // Removes offset which we don't understand, converts to ns
   {
      return (time+1784291.121733e-12)*1e9;
   }

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
