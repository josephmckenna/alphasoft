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

   // Constante value declaration
   double matching_tolerance = 100e-9; // s, maximum allowed time between ADC time and matched TDC time
   double adc_tdc_offset = 0; // s, used to match the centre of the pulse (ADC) to the centre of the pusle (TDC)
   ////double adc_tdc_offset = -60e-9; // s, used to match the centre of the pulse (ADC) to the centre of the pusle (TDC)

   // https://daq.triumf.ca/elog-alphag/alphag/1961
   const double epoch_freq = 97656.25; // 200MHz/(2<<11); KO+Thomas approved right frequency
   const double coarse_freq = 200.0e6; // 200MHz


   // linear calibration:
   // $ROOTANASYS/libAnalyzer/TRB3Decoder.hxx
   const double trb3LinearLowEnd = 17.0;
   const double trb3LinearHighEnd = 473.0;

   // Container declaration
   std::map<int,std::vector<BarHit>> ADCHits; // bar hits directly from bsc_adc_module.cxx
   std::map<int,std::vector<double>> TDCHits; // tdc time values to be calculated
   std::map<int,std::vector<BarHit>> ADCTDCHits; // bar hits combining adc and tdc data
   std::map<int,std::vector<BarHit>> FullHits; // bar hits combining top and bottom (combined adc and tdc) hits
   int bscTdcMap[64][5];
   

   //Histogramm declaration
   TH2D* hTdc = NULL;
   TH2D* hAmp = NULL;
   TH2D* hInt = NULL;
   TH2D* hAmpVInt = NULL;
   TH3D* hAmpVIntVChan = NULL;
   TH2D* hTdcVAmp = NULL;
   TH2D* hTdcVInt = NULL;
   TH2D* hHitsAdcVTdc = NULL;
   TH2D* hHitsBotVTop = NULL;
   TH2D* hAdctimeVTdc = NULL;
   TH2D* hAdctimensVTdc = NULL;
   TH2D* hAdctimensMinusTdc = NULL;
   TH2D* hBotMinusTop = NULL;
   TH2D* hZed = NULL;
   TH2D* hBotVTop = NULL;
   TH2D* hZedVsAmpTop = NULL;
   TH2D* hZedVsAmpBot = NULL;
   TH2D* hZedVsInt = NULL;
   TH1D* hHitsAdcTdcMatched = NULL;
   TH1D* hMissedAdcHits = NULL;
   TH1D* hMissedTdcHits = NULL;
   TH1D* hHitsBotTopMatched = NULL;
   TH1D* hHitsBotTopUnmatched = NULL;
   TH2D* hRiseTime = NULL;
   TH1D* hTdcDuration = NULL;
   TH1D* hTdcDouble = NULL;
   TH2D* hTdcVsMaxAmp = NULL;


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
      hAmp = new TH2D("hAmp","Peak amplitude;Bar;Amplitude",128,-0.5,127.5,2000,0.,35000.);
      hInt = new TH2D("hInt","Peak integral;Bar;Integral",128,-0.5,127.5,2000,0.,700000.);
      hAmpVInt = new TH2D("hAmpVInt","Peak amplitude vs integral;Amplitude;Integral",2000,0.,35000.,2000,0.,700000.);
      hAmpVIntVChan = new TH3D("hAmpVIntVChan","Peak amplitude vs integral vs channel number;Amplitude;Integral",200,0.,35000.,200,0.,700000.,128,-0.5,127.5);
      hTdcVAmp = new TH2D("hTdcVAmp","Peak amplitude vs TDC time;Time [ns];Amplitude",2000,0.,700.,2000,0,35000);
      hTdcVInt = new TH2D("hTdcVInt","Peak integral vs TDC time;Time [ns];Integral",2000,0.,700.,2000,0,700000);
      hHitsAdcVTdc = new TH2D("hHitsAdcVTdc","Number of hits on ADC vs TDC;ADC hits;TDC hits",10,-0.5,9.5,10,-0.5,9.5);
      hHitsBotVTop = new TH2D("hHitsBotVTop","Number of hits on Bottom vs Top;Bottom hits;Top hits",10,-0.5,9.5,10,-0.5,9.5);
      hAdctimeVTdc = new TH2D("hAdctimeVTdc","Raw ADC 'time' vs TDC time;ADC 'time' [ADC bins];TDC time [ns]",200,0,200,2000,0.,700.);
      hAdctimensVTdc = new TH2D("hAdctimensVTdc","Corrected ADC time vs TDC time;ADC time [ns];TDC time [ns]",2000,0.,700.,2000,0.,700.);
      hAdctimensMinusTdc = new TH2D("hAdctimensMinusTdc","Calculated ADC time minus TDC time;Bar;Time Difference [ns]",128,-0.5,127.5,2000,-100,100);
      hBotMinusTop = new TH2D("hBotMinusTop","Bottom TDC time minus top TDC time;Bar;Time Difference [ns]",64,-0.5,63.5,2000,-60,60);
      hZed = new TH2D("hZed","Zed calculated from TDC Bottom-Top time difference;Bar;Zed [m]",64,-0.5,63.5,2000,-3.,3.);
      hBotVTop = new TH2D("hBotVTop","Bottom and top TDC time; Bottom time [ns];Top time [ns]",2000,0.,700.,2000,0.,700.);
      hZedVsAmpBot = new TH2D("ZedVsAmpTop","Zed vs bottom peak amplitude;Zed [m];Amplitude",2000,-3,3,2000,0,35000);
      hZedVsAmpTop = new TH2D("ZedVsAmpBot","Zed vs bottom peak amplitude;Zed [m];Amplitude",2000,-3,3,2000,0,35000);
      hZedVsInt = new TH2D("hZedVsInt","Zed vs bottom peak integral;Zed [m];Integral",2000,-3,3,2000,0,700000);
      hHitsAdcTdcMatched = new TH1D("hHitsAdcTdcMatched","Count of ADC and TDC hits sucessfully matched;Bar",128,-0.5,127.5);
      hMissedAdcHits = new TH1D("hMissedAdcHits","Count of missed ADC hits (TDC hits without a partner);Bar",128,-0.5,127.5);
      hMissedTdcHits = new TH1D("hMissedTdcHits","Count of missed TDC hits (ADC hits without a partner);Bar",128,-0.5,127.5);
      hHitsBotTopMatched = new TH1D("hHitsBotTopMatched","Count of bottom and top hits sucessfully matched;Bar",64,-0.5,63.5);
      hHitsBotTopUnmatched = new TH1D("hHitsBotTopUnmatched","Count of bottom and top hits which were not matched;Bar",64,-0.5,63.5);
      hRiseTime = new TH2D("hRiseTime","Rise time of fully matched hits;Peak ampltitude;Rise time [ns]",2000,0.,35000.,200,0,2000);
      hTdcDuration = new TH1D("hTdcDuration","Time between rising and falling edge of tdc signal;Time [ns]",2000,0.,400);
      hTdcDouble = new TH1D("hTdcDouble","Time between two rising edge tdc signals;Time [ns]",2000,0.,400);
      hTdcVsMaxAmp = new TH2D("hTdcVsMaxAmp","Peak amplitude of largest in event vs TDC time;Time [ns];Amplitude",2000,0.,700.,2000,0,35000);


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
      delete hAmp;
      delete hInt;
      delete hAmpVInt;
      delete hAmpVIntVChan;
      delete hTdcVAmp;
      delete hTdcVInt;
      delete hHitsAdcVTdc;
      delete hHitsBotVTop;
      delete hAdctimeVTdc;
      delete hAdctimensVTdc;
      delete hAdctimensMinusTdc;
      delete hBotMinusTop;
      delete hZed;
      delete hBotVTop;
      delete hZedVsAmpTop;
      delete hZedVsAmpBot;
      delete hZedVsInt;
      delete hHitsAdcTdcMatched;
      delete hMissedAdcHits;
      delete hMissedTdcHits;
      delete hHitsBotTopMatched;
      delete hHitsBotTopUnmatched;
      delete hRiseTime;
      delete hTdcDuration;
      delete hTdcDouble;
      delete hTdcVsMaxAmp;
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

      ResetADCHits();
      ResetTDCHits();
      ResetADCTDCHits();
      ResetFullHits();

      if( tdc )
         {
            if( tdc->complete )
               {
                  // MAIN FUNCTIONS
                  getADCHits(flow);
                  getTDCHits(tdc,trig);
                  combineADCTDC();
                  combineTopBot();
                  flow=feedForward(flow);
               }
            else
               std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event incomplete"<<std::endl;
         }
      else
         std::cout<<"tdcmodule::AnalyzeFlowEvent  No TDC event"<<std::endl;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bsc_tdc_module",timer_start);
      #endif
      return flow;
   }

   //________________________________
   // MAIN FUNCTIONS

   void getADCHits(TAFlowEvent* flow) // Fills ADCHits
   {
      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>(); // Gets list of adc hits from flow
      if( !bef ) return;
      TBarEvent *barEvt=bef->BarEvent;
      std::vector<BarHit>* flowAdcHits=barEvt->GetBars();
      ResetADCHits();
      for (auto hit: *flowAdcHits) 
         {
            // Sorts adc hits by bar
            int barID = hit.GetBar();
            ADCHits[barID].push_back(hit);
            // Fills histos
            if (barID<64) //bot
               {
                  hAmp->Fill(barID,hit.GetAmpBot());
                  hInt->Fill(barID,hit.GetIntegralBot());
                  hAmpVInt->Fill(hit.GetAmpBot(),hit.GetIntegralBot());
                  hAmpVIntVChan->Fill(hit.GetAmpBot(),hit.GetIntegralBot(),barID);
               }
            else //top
               {
                  hAmp->Fill(barID,hit.GetAmpTop());
                  hInt->Fill(barID,hit.GetIntegralTop());
                  hAmpVInt->Fill(hit.GetAmpTop(),hit.GetIntegralTop());
                  hAmpVIntVChan->Fill(hit.GetAmpTop(),hit.GetIntegralTop(),barID);
               }
         }
   }

   void getTDCHits(TdcEvent* tdc, TrigEvent* trig) // Fills TDCHits
   {
      std::vector<TdcHit*> hits = tdc->hits; // Gets tdc hits from flow
      ResetTDCHits();
      for (int i=0;i<int(hits.size())-1;i++)
         {
            TdcHit* rising_hit = hits.at(i);
            TdcHit* falling_hit = hits.at(i+1);
            if (rising_hit->rising_edge==0 || falling_hit->rising_edge==1) continue;
            if (rising_hit->chan<=0 || falling_hit->chan<=0) continue;
            int barID = fpga2barID(int(rising_hit->fpga),int(rising_hit->chan));
            if (fpga2barID(int(falling_hit->fpga),int(falling_hit->chan)) != barID )
               { std::cout<<"getTDCHits: Rising and falling hits have different bar IDs!"<<std::endl; continue; }
            double rising_hit_time = GetFinalTime(rising_hit->epoch,rising_hit->coarse_time,rising_hit->fine_time); // Calculates time from tdc data
            double falling_hit_time = GetFinalTime(falling_hit->epoch,falling_hit->coarse_time,falling_hit->fine_time); // Calculates time from tdc data
            double trig_time=FindTriggerTime(hits,barID%64);
            double final_rising_hit_time = rising_hit_time-trig_time;
            double final_falling_hit_time = falling_hit_time-trig_time;
            double final_time = final_rising_hit_time;
            ////double final_time = (final_rising_hit_time+final_falling_hit_time)/2.0;
            TDCHits[barID].push_back(final_time); // Sorts tdc hits by bar
            // Fills histos
            hTdc->Fill(barID,TimeConversion(final_time));
            hTdcDuration->Fill((final_falling_hit_time-final_rising_hit_time)*1e9);
         }
      for (int i=0;i<int(hits.size())-3;i++)
         {
            TdcHit* first_hit = hits.at(i);
            TdcHit* second_hit = hits.at(i+2);
            if (first_hit->rising_edge && second_hit->rising_edge && first_hit->chan>0 && second_hit->chan>0)
               {
                  double first_time = GetFinalTime(first_hit->epoch,first_hit->coarse_time,first_hit->fine_time);
                  double second_time = GetFinalTime(second_hit->epoch,second_hit->coarse_time,second_hit->fine_time);
                  hTdcDouble->Fill((second_time-first_time)*1e9);
               }
         }
   }

   TAFlowEvent* feedForward(TAFlowEvent* flow) // Adds the fully formed BarHits in FullHits into the event flow
   {
      TBarEvent* BarEvent = new TBarEvent();
      for (int bar=0;bar<64;bar++)
         {
            for (auto hit: FullHits[bar]) BarEvent->AddHit(hit);
         }
      flow = new AgBarEventFlow(flow, BarEvent);
      return flow;
   }


   void combineADCTDC() // Matches each ADC hit with a TDC hit and fills ADCTDCHits
   {
      ResetADCTDCHits();
      for (int bar=0; bar<128; bar++)
         {
            int nAdc=ADCHits[bar].size();
            int nTdc=TDCHits[bar].size();
            int nMatch=0;
            hHitsAdcVTdc->Fill(nAdc,nTdc);
            std::sort(ADCHits[bar].begin(),ADCHits[bar].end(),compareAmplitude); // Sorts ADC hits by amplitude; match the largest peaks first
            if (bar<64)
               {
                  if (ADCHits[bar].size()>0) for (double tdc: TDCHits[bar]) hTdcVsMaxAmp->Fill(TimeConversion(tdc),ADCHits[bar][0].GetAmpBot());
               }
            else
               {
                  if (ADCHits[bar].size()>0) for (double tdc: TDCHits[bar]) hTdcVsMaxAmp->Fill(TimeConversion(tdc),ADCHits[bar][0].GetAmpTop());
               }
            for (auto adchit: ADCHits[bar])
               {
                  if (TDCHits[bar].size()==0) break; // Go to next bar if there are no more TDC hits on this bar
                  // Converts adc time to seconds (offset is from a linear regression)
                  double linM = 0.00000001; // 100 Msamples/s
                  double linB = -2784291.121733e-12;
                  double adctime;
                  if (bar<64) adctime = adchit.GetADCTimeBot();
                  else adctime = adchit.GetADCTimeTop();
                  double adctime_s = linM * adctime + linB + adc_tdc_offset;
                  // Finds tdc hit with closest time
                  std::vector<double> time_diff; // Distance between current adc hit time and each tdc hit on bar
                  for (double tdc: TDCHits[bar]) time_diff.push_back(TMath::Abs(tdc-adctime_s));
                  int min_diff_index = std::min_element(time_diff.begin(), time_diff.end())-time_diff.begin(); // Minimize distance
                  if (time_diff.at(min_diff_index) > matching_tolerance) continue;
                  double tdctime = TDCHits[bar].at(min_diff_index);
                  TDCHits[bar].erase(TDCHits[bar].begin()+min_diff_index); // Don't match another adc hit to the chosen tdc hit
                  // Creates a new BarHit with ADC and TDC hit info
                  BarHit hit;
                  if (bar<64) // bot
                     {
                        hit.SetADCHit( bar, -999., adchit.GetAmpBot(), -999., adchit.GetADCTimeBot(), -999., adchit.GetIntegralBot(), -999., adchit.GetRiseBot());
                        hit.SetTDCHit( bar, -999., tdctime);
                        hTdcVAmp->Fill(TimeConversion(tdctime),adchit.GetAmpBot());
                        hTdcVInt->Fill(TimeConversion(tdctime),adchit.GetIntegralBot());
                     }
                  else // top
                     {
                        hit.SetADCHit( bar, adchit.GetAmpTop(), -999., adchit.GetADCTimeTop(), -999., adchit.GetIntegralTop(), -999., adchit.GetRiseTop(), -999.);
                        hit.SetTDCHit( bar, tdctime, -999.);
                        hTdcVAmp->Fill(TimeConversion(tdctime),adchit.GetAmpTop());
                        hTdcVInt->Fill(TimeConversion(tdctime),adchit.GetIntegralTop());
                     }
                  ADCTDCHits[bar].push_back(hit);
                  // Fills histos
                  hAdctimeVTdc->Fill(adctime,TimeConversion(tdctime));
                  hAdctimensVTdc->Fill(TimeConversion(adctime_s),TimeConversion(tdctime));
                  hAdctimensMinusTdc->Fill(bar,(adctime_s-tdctime)*1e9);
                  nMatch++;
               }
            for (int ii=0;ii<nMatch;ii++) hHitsAdcTdcMatched->Fill(bar);
            for (int ii=0;ii<nTdc-nMatch;ii++) hMissedAdcHits->Fill(bar);
            for (int ii=0;ii<nAdc-nMatch;ii++) hMissedTdcHits->Fill(bar);
         }
   }

   void combineTopBot() // Matches each bottom hit to a top hit and fills FullHits
   {
      ResetFullHits();
      for (int bar=0;bar<64;bar++)
         {
            int nBot = ADCTDCHits[bar].size();
            int nTop = ADCTDCHits[bar+64].size();
            int nMatch = 0;
            hHitsBotVTop->Fill(nBot,nTop);
            for (auto bothit: ADCTDCHits[bar])
               {
                  if (ADCTDCHits[bar+64].size()==0) break; // Go to next bar if there are no more top hits on this bar
                  double bottime = bothit.GetTDCBot();
                  // Finds top hit with closest time
                  std::vector<double> time_diff; // Time difference between current bot hit and each top hit on bar
                  for (auto tophit: ADCTDCHits[bar+64]) time_diff.push_back(TMath::Abs(tophit.GetTDCTop()-bottime));
                  int min_diff_index = std::min_element(time_diff.begin(), time_diff.end())-time_diff.begin(); // Find the top hit with the closest time
                  BarHit tophit = ADCTDCHits[bar+64].at(min_diff_index);
                  double toptime = tophit.GetTDCTop();
                  ADCTDCHits[bar+64].erase(ADCTDCHits[bar+64].begin()+min_diff_index); // Don't match another bot hit to the same top hit
                  // Create new BarHit with TDC+ADC and top+bottom data
                  BarHit hit;
                  hit.SetADCHit( bar, tophit.GetAmpTop(), bothit.GetAmpBot(), tophit.GetADCTimeTop(), bothit.GetADCTimeBot(), tophit.GetIntegralTop(), bothit.GetIntegralBot(), tophit.GetRiseTop(), bothit.GetRiseBot());
                  hit.SetTDCHit( bar, toptime, bottime );
                  FullHits[bar].push_back(hit);
                  // Fills histos
                  hBotMinusTop->Fill(bar,(bottime-toptime)*1e9);
                  hZed->Fill(bar,hit.GetTDCZed());
                  hBotVTop->Fill(TimeConversion(bottime),TimeConversion(toptime));
                  hZedVsAmpBot->Fill(hit.GetTDCZed(),hit.GetAmpBot());
                  hZedVsAmpTop->Fill(hit.GetTDCZed(),hit.GetAmpTop());
                  hZedVsInt->Fill(hit.GetTDCZed(),hit.GetIntegralBot());
                  hRiseTime->Fill(hit.GetAmpTop(),hit.GetRiseTop()*10); // ns
                  hRiseTime->Fill(hit.GetAmpBot(),hit.GetRiseBot()*10); // ns
                  nMatch++;
               }
            for (int ii=0;ii<nMatch;ii++) hHitsBotTopMatched->Fill(bar);
            for (int ii=0;ii<nBot-nMatch;ii++) hHitsBotTopUnmatched->Fill(bar);
            for (int ii=0;ii<nTop-nMatch;ii++) hHitsBotTopUnmatched->Fill(bar);
         }
   }

   //________________________________
   // HELPER FUNCTIONS

   static bool compareAmplitude(BarHit hit1, BarHit hit2) // Helper function for sorting hits by ADC amplitude
   {
      if (hit1.GetAmpBot()!=-999. && hit2.GetAmpBot()!=-999.) return (hit1.GetAmpBot()>hit2.GetAmpBot());
      if (hit1.GetAmpTop()!=-999. && hit2.GetAmpTop()!=-999.) return (hit1.GetAmpTop()>hit2.GetAmpTop());
      return false;
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

   void ResetADCHits()
      {
         //for (int ii=0; ii<64; ii++) ADCHits[ii].clear();
         ADCHits.clear();
      }
   void ResetTDCHits()
      {
         //for (int ii=0; ii<64; ii++) TDCHits[ii].clear();
         TDCHits.clear();
      }
   void ResetADCTDCHits()
      {
         //for (int ii=0; ii<64; ii++) ADCTDCHits[ii].clear();
         ADCTDCHits.clear();
      }
   void ResetFullHits()
      {
         //for (int ii=0; ii<64; ii++) FullHits[ii].clear();
         FullHits.clear();
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
