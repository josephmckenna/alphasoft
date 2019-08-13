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
   int pedestal_length = 100;
   int threshold = 1400;

   // https://daq.triumf.ca/elog-alphag/alphag/1961
   const double epoch_freq = 97656.25; // 200MHz/(2<<11);
   const double coarse_freq = 200.0e6; // 200MHz

   const double coarse_range = 10240026.0; // ~10.24 us
   const double fine_range = 5000.0; // 5 ns

   // linear calibration:
   // $ROOTANASYS/libAnalyzer/TRB3Decoder.hxx
   const double trb3LinearLowEnd = 17.0;
   const double trb3LinearHighEnd = 473.0;

   // Container declaration
   std::map<int,std::vector<BarHit>> ADCHits;
   std::map<int,std::vector<double>> TDCHits;
   std::map<int,std::vector<BarHit>> ADCTDCHits;
   std::map<int,std::vector<BarHit>> FullHits;
   int bscTdcMap[64][5];
   
   double first_trig_time; // Trigger time of first event

   // Disable broken tdc bars
   std::vector<int> badbars = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 25, 27, 35, 52, 55, 59, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 89, 91, 99, 116, 119, 123}; // Broken tdc bars
   bool disable_badbars = false;

   //Histogramm declaration
   TH2D* hTdc = NULL;
   TH2D* hAmp = NULL;
   TH2D* hInt = NULL;
   TH2D* hAmpVInt = NULL;
   TH2D* hTdcVAmp = NULL;
   TH2D* hTdcVInt = NULL;
   TH2D* hHitsAdcVTdc = NULL;
   TH2D* hHitsBotVTop = NULL;
   TH2D* hAdctimeVTdc = NULL;
   TH2D* hAdctimepsVTdc = NULL;
   TH2D* hAdctimepsMinusTdc = NULL;
   TH2D* hBotMinusTop = NULL;
   TH2D* hZed = NULL;
   TH2D* hBotVTop = NULL;
   TH2D* hBotMinusTopVsAmpTop = NULL;
   TH2D* hBotMinusTopVsAmpBot = NULL;
   TH2D* hBotMinusTopVsIntTop = NULL;
   TH2D* hBotMinusTopVsIntBot = NULL;
   TH1D* hHitsAdcTdcMatched = NULL;
   TH1D* hMissedAdcHits = NULL;
   TH1D* hMissedTdcHits = NULL;
   TH1D* hHitsBotTopMatched = NULL;
   TH1D* hHitsBotTopUnmatched = NULL;
   TH2D* hTrigEventVTDCTrig = NULL;
   TH2D* hTrigEventMinusTDCTrig = NULL;
   TH1D* hTdcFirst = NULL;


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
      hTdc = new TH2D("hTdc","TDC time since trig;Bar;Time [ps]",128,-0.5,127.5,2000,-1600000,-1100000);
      hTdcFirst = new TH1D("hTdcFirst","TDC time since trig, only first hit in each event;Time [ps]",2000,-1600000,-1100000.);
      hAmp = new TH2D("hAmp","Peak amplitude;Bar;Amplitude",128,-0.5,127.5,2000,0.,35000.);
      hInt = new TH2D("hInt","Peak integral;Bar;Integral",128,-0.5,127.5,2000,0.,700000.);
      hAmpVInt = new TH2D("hAmpVInt","Peak amplitude vs integral;Amplitude;Integral",2000,0.,35000.,2000,0.,700000.);
      hTdcVAmp = new TH2D("hTdcVAmp","Peak amplitude vs TDC time;Time [ps];Amplitude",2000,-1600000,-1100000,2000,0,35000);
      hTdcVInt = new TH2D("hTdcVInt","Peak integral vs TDC time;Time [ps];Integral",2000,-1600000,-1100000,2000,0,700000);
      hHitsAdcVTdc = new TH2D("hHitsAdcVTdc","Number of hits on ADC vs TDC;ADC hits;TDC hits",8,-0.5,7.5,8,-0.5,7.5);
      hHitsBotVTop = new TH2D("hHitsBotVTop","Number of hits on Bottom vs Top;Bottom hits;Top hits",8,-0.5,7.5,8,-0.5,7.5);
      hAdctimeVTdc = new TH2D("hAdctimeVTdc","Raw ADC 'time' vs TDC time;ADC 'time' [ADC bins];TDC time [ps]",200,0,200,2000,-1600000,-1100000);
      hAdctimepsVTdc = new TH2D("hAdctimepsVTdc","Corrected ADC time vs TDC time;ADC time [ps];TDC time [ps]",2000,-1600000,-1100000,2000,-1600000,-1100000);
      hAdctimepsMinusTdc = new TH2D("hAdctimepsMinusTdc","Calculated ADC time minus TDC time;Bar;Time Difference [ps]",128,-0.5,127.5,2000,-100000,100000);
      hBotMinusTop = new TH2D("hBotMinusTop","Bottom TDC time minus top TDC time;Bar;Time Difference [ps]",64,-0.5,63.5,2000,-60000,60000);
      hZed = new TH2D("hZed","Zed calculated from TDC Bottom-Top time difference;Bar;Zed [m]",64,-0.5,63.5,2000,-3.,3.);
      hBotVTop = new TH2D("hBotVTop","Bottom and top TDC time; Bottom time [ps];Top time [ps]",2000,-1600000,-1100000,2000,-1600000,-1100000);
      hBotMinusTopVsAmpBot = new TH2D("hBotMinusTopVsAmpBot","Bottom-top time difference vs bottom peak amplitude;Time Difference [ps];Amplitude",2000,-60000,60000,2000,0,35000);
      hBotMinusTopVsAmpTop = new TH2D("hBotMinusTopVsAmpTop","Bottom-top time difference vs top peak amplitude;Time Difference [ps];Amplitude",2000,-60000,60000,2000,0,35000);
      hBotMinusTopVsIntBot = new TH2D("hBotMinusTopVsIntBot","Bottom-top time difference vs bottom peak integral;Time Difference [ps];Amplitude",2000,-60000,60000,2000,0,700000);
      hBotMinusTopVsIntTop = new TH2D("hBotMinusTopVsIntTop","Bottom-top time difference vs top peak integral;Time Difference [ps];Amplitude",2000,-60000,60000,2000,0,700000);
      hHitsAdcTdcMatched = new TH1D("hHitsAdcTdcMatched","Count of ADC and TDC hits sucessfully matched;Bar",128,-0.5,127.5);
      hMissedAdcHits = new TH1D("hMissedAdcHits","Count of missed ADC hits (TDC hits without a partner);Bar",128,-0.5,127.5);
      hMissedTdcHits = new TH1D("hMissedTdcHits","Count of missed TDC hits (ADC hits without a partner);Bar",128,-0.5,127.5);
      hHitsBotTopMatched = new TH1D("hHitsBotTopMatched","Count of bottom and top hits sucessfully matched;Bar",64,-0.5,63.5);
      hHitsBotTopUnmatched = new TH1D("hHitsBotTopUnmatched","Count of bottom and top hits which were not matched;Bar",64,-0.5,63.5);
      hTrigEventVTDCTrig = new TH2D("hTrigEventVTDCTrig","Trig Event time vs TDC trigger time;Trig Event Time [s];TDC trigger time [s]",2000,0.,25.,2000,0.,25.);
      hTrigEventMinusTDCTrig = new TH2D("hTrigEventMinusTDCTrig","Trig Event time minus TDC trigger time;Trig event time [s];Trig event time minus TDC trigger time [ps]",2000,0.,25.,2000,-100000000.,100000000.);


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
      delete hTdcFirst;
      delete hAmp;
      delete hInt;
      delete hAmpVInt;
      delete hTdcVAmp;
      delete hTdcVInt;
      delete hHitsAdcVTdc;
      delete hHitsBotVTop;
      delete hAdctimeVTdc;
      delete hAdctimepsVTdc;
      delete hAdctimepsMinusTdc;
      delete hBotMinusTop;
      delete hZed;
      delete hBotVTop;
      delete hBotMinusTopVsAmpBot;
      delete hBotMinusTopVsAmpTop;
      delete hBotMinusTopVsIntBot;
      delete hBotMinusTopVsIntTop;
      delete hHitsAdcTdcMatched;
      delete hMissedAdcHits;
      delete hMissedTdcHits;
      delete hHitsBotTopMatched;
      delete hHitsBotTopUnmatched;
      delete hTrigEventVTDCTrig;
      delete hTrigEventMinusTDCTrig;
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

      // First trigger time of the whole run
      if (!first_trig_time)
         {
            auto hits = tdc->hits;
            for (auto hit: hits)
               {
                  int barID = fpga2barID(int(hit->fpga),int(hit->chan));
                  double trig_time=FindTriggerTime(hits,barID%64);
                  if (!first_trig_time || trig_time<first_trig_time) first_trig_time = trig_time;
               }
         }

      if( tdc )
         {
            if( tdc->complete )
               {
                  // Add function here !!!
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

   static bool compareAmplitude(BarHit hit1, BarHit hit2) // Helper function for sorting hits by ADC amplitude
   {
      if (hit1.GetAmpBot()!=-999. && hit2.GetAmpBot()!=-999.) return (hit1.GetAmpBot()>hit2.GetAmpBot());
      if (hit1.GetAmpTop()!=-999. && hit2.GetAmpTop()!=-999.) return (hit1.GetAmpTop()>hit2.GetAmpTop());
      return false;
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
            for (auto adchit: ADCHits[bar])
               {
                  if (TDCHits[bar].size()==0) break; // Go to next bar if there are no more TDC hits on this bar
                  // Converts adc time to picoseconds (very approximate, very ghetto)
                  double linM = 8906.0771;
                  double linB = -2631014.4;
                  double adctime;
                  if (bar<64) adctime = adchit.GetADCTimeBot();
                  else adctime = adchit.GetADCTimeTop();
                  double adctime_ps = linM * adctime + linB;
                  // Finds tdc hit with closest time
                  std::vector<double> time_diff; // Distance between current adc hit time and each tdc hit on bar
                  for (double tdc: TDCHits[bar]) time_diff.push_back(abs(tdc-adctime_ps));
                  int min_diff_index = std::min_element(time_diff.begin(), time_diff.end())-time_diff.begin(); // Minimize distance
                  double tdctime = TDCHits[bar].at(min_diff_index);
                  TDCHits[bar].erase(TDCHits[bar].begin()+min_diff_index); // Don't match another adc hit to the chosen tdc hit
                  // Creates a new BarHit with ADC and TDC hit info
                  BarHit hit;
                  if (bar<64) // bot
                     {
                        hit.SetADCHit( bar, -999., adchit.GetAmpBot(), -999., adchit.GetADCTimeBot(), -999., adchit.GetIntegralBot());
                        hit.SetTDCHit( bar, -999., tdctime);
                        hTdcVAmp->Fill(tdctime,adchit.GetAmpBot());
                        hTdcVInt->Fill(tdctime,adchit.GetIntegralBot());
                     }
                  else // top
                     {
                        hit.SetADCHit( bar, adchit.GetAmpTop(), -999., adchit.GetADCTimeTop(), -999., adchit.GetIntegralTop(), -999.);
                        hit.SetTDCHit( bar, tdctime, -999.);
                        hTdcVAmp->Fill(tdctime,adchit.GetAmpTop());
                        hTdcVInt->Fill(tdctime,adchit.GetIntegralTop());
                     }
                  ADCTDCHits[bar].push_back(hit);
                  // Fills histos
                  hAdctimeVTdc->Fill(adctime,tdctime);
                  hAdctimepsVTdc->Fill(adctime_ps,tdctime);
                  hAdctimepsMinusTdc->Fill(bar,adctime_ps-tdctime);
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
                  for (auto tophit: ADCTDCHits[bar+64]) time_diff.push_back(abs(tophit.GetTDCTop()-bottime));
                  int min_diff_index = std::min_element(time_diff.begin(), time_diff.end())-time_diff.begin(); // Find the top hit with the closest time
                  BarHit tophit = ADCTDCHits[bar+64].at(min_diff_index);
                  double toptime = tophit.GetTDCTop();
                  ADCTDCHits[bar+64].erase(ADCTDCHits[bar+64].begin()+min_diff_index); // Don't match another bot hit to the same top hit
                  // Create new BarHit with TDC+ADC and top+bottom data
                  BarHit hit;
                  hit.SetADCHit( bar, tophit.GetAmpTop(), bothit.GetAmpBot(), tophit.GetADCTimeTop(), bothit.GetADCTimeBot(), tophit.GetIntegralTop(), bothit.GetIntegralBot() );
                  hit.SetTDCHit( bar, toptime, bottime );
                  FullHits[bar].push_back(hit);
                  // Fills histos
                  hBotMinusTop->Fill(bar,bottime-toptime);
                  hZed->Fill(bar,hit.GetTDCZed());
                  hBotVTop->Fill(bottime,toptime);
                  hBotMinusTopVsAmpTop->Fill(bottime-toptime,hit.GetAmpTop());
                  hBotMinusTopVsAmpBot->Fill(bottime-toptime,hit.GetAmpBot());
                  hBotMinusTopVsIntTop->Fill(bottime-toptime,hit.GetIntegralTop());
                  hBotMinusTopVsIntBot->Fill(bottime-toptime,hit.GetIntegralBot());
                  nMatch++;
               }
            for (int ii=0;ii<nMatch;ii++) hHitsBotTopMatched->Fill(bar);
            for (int ii=0;ii<nBot-nMatch;ii++) hHitsBotTopUnmatched->Fill(bar);
            for (int ii=0;ii<nTop-nMatch;ii++) hHitsBotTopUnmatched->Fill(bar);
         }
   }

    double FindTriggerTime(std::vector<TdcHit*> hits, int bar)
   {
      double trig_time=0;
      int tdc_fpga=bscTdcMap[bar][1]-1;
      for (auto hit: hits)
         {
            if ( hit->chan != 0 ) continue;
            if ( ! hit->rising_edge ) continue;
            if ( hit->fpga > tdc_fpga ) break;
            if ( hit->fpga==tdc_fpga ) trig_time = GetFinalTime(hit->epoch,hit->coarse_time,hit->fine_time);
         }
      return trig_time;
   }


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
            if (disable_badbars && std::find(badbars.begin(), badbars.end(), barID) != badbars.end()) continue;
            ADCHits[barID].push_back(hit);
            // Fills histos
            if (barID<64) //bot
               {
                  hAmp->Fill(barID,hit.GetAmpBot());
                  hInt->Fill(barID,hit.GetIntegralBot());
                  hAmpVInt->Fill(hit.GetAmpBot(),hit.GetIntegralBot());
               }
            else //top
               {
                  hAmp->Fill(barID,hit.GetAmpTop());
                  hInt->Fill(barID,hit.GetIntegralTop());
                  hAmpVInt->Fill(hit.GetAmpTop(),hit.GetIntegralTop());
               }
         }
   }

   void getTDCHits(TdcEvent* tdc, TrigEvent* trig) // Fills TDCHits
   {
      std::vector<TdcHit*> hits = tdc->hits; // Gets tdc hits from flow
      ResetTDCHits();
      double earliest_hit = 0; // Earliest hit in event across all bars
      for (auto hit: hits)
         {
            if (hit->rising_edge==0) continue;
            if (hit->chan<=0) continue;
            int barID = fpga2barID(int(hit->fpga),int(hit->chan));
            if (disable_badbars && std::find(badbars.begin(), badbars.end(), barID) != badbars.end()) continue;
            double final_time = GetFinalTime(hit->epoch,hit->coarse_time,hit->fine_time); // Calculates time from tdc data
            double trig_time=FindTriggerTime(hits,barID%64);
            TDCHits[barID].push_back(final_time-trig_time); // Sorts tdc hits by bar
            // Fills histos
            if (!earliest_hit || final_time-trig_time<earliest_hit) earliest_hit=final_time-trig_time;
            hTdc->Fill(barID,final_time-trig_time);
            hTrigEventVTDCTrig->Fill(trig->time,(trig_time-first_trig_time)*0.000000000001);
            hTrigEventMinusTDCTrig->Fill(trig->time,trig->time*1000000000000-(trig_time-first_trig_time));
         }
      if (earliest_hit) hTdcFirst->Fill(earliest_hit);
      std::cout<<"       FIRST HIT = "<<earliest_hit<<std::endl;
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

   double GetFinalTime( uint32_t epoch, uint16_t coarse, uint16_t fine ) // Calculates time from tdc data
   {
      double B = double(fine) - trb3LinearLowEnd;
      double C = trb3LinearHighEnd - trb3LinearLowEnd;
      return double(epoch)*coarse_range +  double(coarse)*fine_range - (B/C) * fine_range;
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
