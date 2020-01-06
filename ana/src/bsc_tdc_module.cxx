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
   double tdcTimeDiff[64]={0};
   double time_top[64]={0};
   double time_bot[64]={0};
   

   //Histogramm declaration
   TH2D *hTdcTime = NULL;
   TH2D *hTimeDiff = NULL;
   TH2D *hTdcZed = NULL;
   TH1D *hTdcMissedEvent = NULL;

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
      hTdcTime=new TH2D("hTdcTime","Time measured on TDC;Channel;Time [ps]",
                        128,-0.5,127.5,1000,0.,10000000);
      hTimeDiff=new TH2D("hTimeDiff","Time difference per bar;Bar;Time [ps]",
                         64,-0.5,63.5,6000,-60000,60000);
      hTdcZed=new TH2D("hTdcZed","Zed of the events;Bar;Zed [m]",
                         64,-0.5,63.5,6000,-3,3);
      hTdcMissedEvent=new TH1D("hTdcMissedEvent", "Event missed by TDC;Bar;",
                               64,-0.5,63.5);


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
      delete hTimeDiff;
      delete hTdcTime;
      delete hTdcZed;
      delete hTdcMissedEvent;
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
      START_TIMER
      #endif
      AgEvent* age = ef->fEvent;

      // Unpack tdc data from event
      TdcEvent* tdc = age->tdc;
      
      for(int ii=0; ii<128; ii++)
         {
            for(int jj=0; jj<5; jj++)
               firstHit[ii][jj]=0;
         }

      for(int ii=0; ii<64; ii++)
         {
            adcHits[ii]=0;
            tdcTimeDiff[ii]=0;
            time_top[ii]=0;
            time_bot[ii]=0;
         }

      ResetADCHits();
      ResetTDCHits();
      ResetADCTDCHits();
      ResetFullHits();

      if( tdc )
         {
            if( tdc->complete )
               {
                  //std::cout<<"tdcmodule::AnalyzeFlowEvent  good TDC event"<<std::endl;

                  // Add function here !!!
                  cleanHits(tdc); //feed firstHit tab
                  getAdcHits(flow); //feed adcHits tab
                  getTdcTime(tdc);

                  flow=feedFlow(flow);

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

   double getZedTdc(double timeDiff)
   {

      double speed=TMath::C();
      double cFactor=1.58;
      double ZedTdc=((speed/cFactor) * double(timeDiff)*1.e-12)*0.5; //in meter

      return ZedTdc;
   }

    void getTdcTime(TdcEvent* tdc)
   {
      std::vector<TdcHit*> hits = tdc->hits;

      for(int bar=0; bar<64; bar ++)
         {
            tdcTimeDiff[bar]=0.;

            if(adcHits[bar]==1)
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
                        double final_time_top=firstHit[bar][3];
                        double final_time_bot=firstHit[bar+64][3];

                        double trig_time=FindTriggerTime(hits,bar);

                        time_top[bar]=final_time_top-trig_time;
                        time_bot[bar]=final_time_bot-trig_time;

                        double diff_time=time_top[bar]-time_bot[bar];
                        tdcTimeDiff[bar]=diff_time;

                        //std::cout<<"-------------------> Event on bar "<<bar<<" time top is "<<time_top<<" and time bot is "<<time_bot<<" and trigger is "<<trig_time<<" diff time is "<<diff_time<<"Final time top = "<<final_time_top<<" et final time bot = "<<final_time_bot<<std::endl;
                        hTdcTime->Fill(bar, final_time_top);
                        hTdcTime->Fill(bar+64, final_time_bot);
                        hTimeDiff->Fill(bar, diff_time);
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

      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>();
      if( !bef ) return;
      TBarEvent *barEvt=bef->BarEvent;
      std::vector<BarHit>* flowAdcHits=barEvt->GetBars();

      //Reset adcHits tab
      for(int ii=0; ii<64; ii++)
         adcHits[ii]=0;

      for(int ii=0; ii<int(flowAdcHits->size()); ii++)
         {
            int barID=flowAdcHits->at(ii).GetBar();
            adcHits[barID]=1;
            //std::cout<<"---------------------------->ADC hit on bar "<<barID<<std::endl;
         }
   }

    double FindTriggerTime(std::vector<TdcHit*> hits, int bar)
   {
      double trig_time=0;
      int tdc_fpga=bscTdcMap[bar][1]-1;
      for (auto hit: hits)
         {
            for(int jj=0; jj<5; jj++)
               firstHit[ii][jj]=-1;
         }

      for(auto it=hits.begin(); it!=hits.end(); ++it)
         {
            if(int((*it)->fpga)==fpga && int((*it)->chan)==chan)
            { /* Not the first hit */ }
            else if(int((*it)->chan)==0 ||((*it)->rising_edge)==0 )
               {/*  */}
            else
               {
                  //Load new fpga and chan value
                  fpga=int((*it)->fpga);
                  chan=int((*it)->chan);

                  //Find bar ID
                  bar=fpga2barID(fpga,chan);

                  //Get Time Value
                  double coarse_time = GetCoarseTime((*it)->epoch,(*it)->coarse_time);
                  double fine_time = double((*it)->fine_time);
                  double final_time = GetFinalTime((*it)->coarse_time,fine_time);

                  //Feed "firstHit" tab
                  firstHit[bar][0]=double(bar);
                  firstHit[bar][1]=coarse_time;
                  firstHit[bar][2]=fine_time;
                  firstHit[bar][3]=final_time;
                  //std::cout<< "------------------------> first hit on bar ID="<<*firstHit[bar][0]<< " and coarse-time ="<<*firstHit[bar][1]<<"ns  Final time = "<<final_time<<" ps"<<" fine time = "<<fine_time<<std::endl;

               }

         }
      return trig_time;
   }


   int fpga2barID(int fpga, int chan) // Looks up fpga number and channel number in map and returns bar number (0-127)
   {
      int bar=-1;
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
      return bar;
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
