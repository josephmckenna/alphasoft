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

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"

#include "TBarEvent.hh"

class TdcFlags
{
public:
   bool fPrint = false;
   bool fProtoTOF = false;
   bool fPulser = false;
   bool fWriteOffsets = false;
   std::string fOffsetFile = ""; 
   double ftwA = 0;
   AnaSettings* ana_settings=0;
};


class tdcmodule: public TARunObject
{
public:
   TdcFlags* fFlags;

private:

   // Constant value declaration
   const double max_adc_tdc_diff_t; // s, maximum allowed time between ADC time and matched TDC time
   const double max_top_bot_diff_t; // s, maximum allowed time between top TDC time and matched bot TDC time
      // https://daq.triumf.ca/elog-alphag/alphag/1961
   const double epoch_freq = 97656.25; // 200MHz/(2<<11); KO+Thomas approved right frequency
   const double coarse_freq = 200.0e6; // 200MHz
      // linear calibration:
      // $ROOTANASYS/libAnalyzer/TRB3Decoder.hxx
   const double trb3LinearLowEnd = 17.0;
   const double trb3LinearHighEnd = 450.0;
   // Time walk correction, dt = A/sqrt(amp)
   //const double twA = 2.48753687e-09;
   const double twA = 1.75e-9;

   // Container declaration
   int protoTOFTdcMap[16][4];
   double TdcOffsets[128] = {0};
   double ProtoTOFTdcOffsets[16] = {0};
   int bscTdcMap[64][5];
   

   //Histogramm declaration
   TH2D* hTdcAdcTime = NULL;
   TH1D* hTdcChan = NULL;
   TH1D* hNTdcHits = NULL;
   TH1D* hNMatchedHits = NULL;
   TH2D* hBarDiffAdc = NULL;
   TH2D* hBarDiffTdc = NULL;
   TH1D* hBarDiffDiffTdc = NULL;
   TH2D* hDiffAdc = NULL;
   TH2D* hDiffTdc = NULL;
   TH1D* hTOFTDC = NULL;
   TH2D* hTdcDiffByChan = NULL;
   TH1D* hTdcDiffByChanMedian = NULL;
   TH1D* hTDCbar=0;

   // Counter initialization
   int c_adc = 0;
   int c_tdc = 0;
   int c_adctdc = 0;
   int c_topbot = 0;

public:

   tdcmodule(TARunInfo* runinfo, TdcFlags* flags): 
      TARunObject(runinfo), fFlags(flags),
      max_adc_tdc_diff_t(flags->ana_settings->GetDouble("BscModule","max_adc_tdc_diff")),
      max_top_bot_diff_t(flags->ana_settings->GetDouble("BscModule","max_top_bot_diff"))
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="bsc tdc module";
#endif
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
      if (fFlags->fProtoTOF and fFlags->fPulser) {
         hTdcChan = new TH1D("hTdcChan","Number of hits on tdc channel;tdc channel",34,-1.5,32.5);
         hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",50,-0.5,49.5);
         hTdcDiffByChan = new TH2D("hTdcDiffByChan","Time difference relative to channel 0;tdc channel;time [s]",16,0.5,16.5,200,-5e-9,5e-9);
      }
      if (!(fFlags->fProtoTOF) and fFlags->fPulser) {
         hTDCbar = new TH1D("hTdcBar","Hits on each bar;tdc bar",128,-0.5,127.5);
         hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",250,-0.5,249.5);
         hTdcDiffByChan = new TH2D("hTdcDiffByChan","Time difference relative to channel 0;tdc channel;time [s]",128,-0.5,127.5,200,-5e-9,5e-9);

      }
      if (fFlags->fProtoTOF and !(fFlags->fPulser)) {
         hTdcChan = new TH1D("hTdcChan","Number of hits on tdc channel;tdc channel",34,-1.5,32.5);
         hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",10,-0.5,9.5);
         hBarDiffAdc = new TH2D("hBarDiffAdc","ADC time difference between ends of bars;Time difference bar A [s];Time difference bar B [s]",200,-15e-9,15e-9,200,-15e-9,15e-9);
         hBarDiffTdc = new TH2D("hBarDiffTdc","TDC time difference between ends of bars (channel-by-channel and time-walk correction);Time difference bar A [s];Time difference bar B [s]",200,-10e-9,10e-9,200,-10e-9,10e-9);
         hBarDiffDiffTdc = new TH1D("hBarDiffDiffTdc","(BarA top - BarA bottom) - (BarB top - BarB bottom) for TDC (channel-by-channel and time-walk correction);TDC Time difference [s]",200,-4e-9,4e-9);
         hTOFTDC = new TH1D("hTOFTDC","Time of flight calculated using TDC (channel-by-channel and time-walk correction);Time of flight [s]",200,-5e-9,5e-9);

      }
      if (!(fFlags->fProtoTOF) and !(fFlags->fPulser)) {
         hTDCbar = new TH1D("hTdcBar","Hits on each bar;tdc bar",128,-0.5,127.5);
         hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",50,-0.5,49.5);
         hTdcAdcTime = new TH2D("hTdcAdcTime","adc vs tdc time;adc time;tdc time",250,1000,1500,200,-2.0e-6,0);
         hNMatchedHits = new TH1D("hNMatchedHits","Number of TDC hits in channel corresponding to ADC hit;Number of tdc hits",50,-0.5,49.5);
         hDiffAdc = new TH2D("hDiffAdc","ADC time difference between ends;Bar number;Time [s]",64,-0.5,63.5,200,-50e-9,50e-9);
         hDiffTdc = new TH2D("hDiffTdc","TDC time difference between ends;Bar number;Time [s]",64,-0.5,63.5,200,-50e-9,50e-9);
         hTOFTDC = new TH1D("hTOFTDC","Time of flight calculated using TDC (channel-by-channel and time-walk correction);Time of flight [s]",200,-20e-9,20e-9);

      }

      // Load Bscint tdc map
      if (fFlags->fProtoTOF) {
         TString mapfile=getenv("AGRELEASE");
         mapfile+="/ana/bscint/";
         mapfile+="protoTOF.map";
         std::ifstream fbscMap(mapfile.Data());
         if(fbscMap)
         {
            std::string comment;
            getline(fbscMap, comment);
            for(int i=0; i<16; i++)
               {
                  fbscMap >> protoTOFTdcMap[i][0] >> protoTOFTdcMap[i][1] >> protoTOFTdcMap[i][2] >> protoTOFTdcMap[i][3];
               }
            fbscMap.close();
         }

         // Load TDC offsets from calibration file
         TString offsetfile=getenv("AGRELEASE");
         offsetfile+="/ana/bscint/";
         offsetfile+="bscoffsets.calib";
         std::ifstream ftdcoff(offsetfile.Data());
         if(ftdcoff)
         {
            std::string comment;
            int num;
            getline(ftdcoff, comment);
            for(int i=0; i<16; i++)
               {
                  ftdcoff >> num >> ProtoTOFTdcOffsets[i];
               }
            ftdcoff.close();
         }
      }
      if ( !(fFlags->fProtoTOF) ) {
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

   }

   void EndRun(TARunInfo* runinfo)
   {
         if( fFlags->fPulser )  // Pulser run
         {
            // Calculates median time for each pulser bin
            runinfo->fRoot->fOutputFile->cd();
            gDirectory->cd("bsc_tdc_module");
            hTdcDiffByChanMedian = hTdcDiffByChan->QuantilesX(0.5,"hTdcDiffByChanMedian");
            hTdcDiffByChanMedian->SetTitle("Median time offset by TDC channel;TDC channel;TDC time [s]");

            // Writes offsets to file
            if ( fFlags->fWriteOffsets )
               {
                  std::ofstream Ofile;
                  Ofile.open(fFlags->fOffsetFile);
                  Ofile<<"TDC channel | Offset(s)\n";
                  for (int i=1;i<=16;i++) Ofile<<i<<"\t"<<hTdcDiffByChanMedian->GetBinContent(i)<<"\n";
                  //for (int i=1;i<=16;i++) Ofile<<i<<"\t"<<hTdcSDiffByChan->ProjectionY("Projection",i,i)->GetMean()<<"\n";
                  Ofile.close();
               }
         }

      // Write output
      runinfo->fRoot->fOutputFile->Write();

      // Print stats
      if( fFlags->fPrint )
         {
            printf("tdc module stats:\n");
            printf("Total number of adc hits = %d\n",c_adc);
            printf("Total number of tdc hits = %d\n",c_tdc);
            if (!fFlags->fPulser)
               {
                  printf("Total number of adc+tdc combined hits = %d\n",c_adctdc);
                  printf("Total number of top+bot hits = %d\n",c_topbot);
               }
         }

      // Delete histograms
      delete hTdcChan;
      delete hTdcAdcTime;
      delete hNTdcHits;
      delete hNMatchedHits;
      delete hDiffAdc;
      delete hDiffTdc;
      delete hBarDiffAdc;
      delete hBarDiffTdc;
      delete hBarDiffDiffTdc;
      delete hTOFTDC;
      delete hTdcDiffByChan;
      delete hTdcDiffByChanMedian;
      delete hTDCbar;
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
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      AgEvent* age = ef->fEvent;
      if(!age)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      // Unpack tdc data from event
      TdcEvent* tdc = age->tdc;

      if( tdc )
         {
            if( tdc->complete )
               {
                  AgBarEventFlow *bef = flow->Find<AgBarEventFlow>();
                  if (!bef) return flow;
                  TBarEvent *barEvt = bef->BarEvent;
                  if (!barEvt) return flow;
                  if( fFlags->fPrint ) printf("tdcmodule::AnalyzeFlowEvent analysing event\n");

                  if( !(fFlags->fPulser) )
                     {
                        AddTDCdata(barEvt,tdc);
                        CombineEnds(barEvt);
                        CalculateZ(barEvt);
                        CalculateTOF(barEvt);
                     }
                  if( fFlags->fPulser )
                     {
                        PulserAnalysis(barEvt,tdc);
                     }
                  if( fFlags->fPrint ) printf("tdcmodule::AnalyzeFlowEvent comlpete\n");
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

   void PulserAnalysis(TBarEvent* barEvt, TdcEvent* tdc)
   {
      // Get endhits from adc module
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      c_adc+=endhits.size();

      // Get tdc data
      std::vector<TdcHit*> tdchits = tdc->hits;
      c_tdc+=tdchits.size();

      double n_hits = 0;

      if (fFlags->fProtoTOF) {
         std::vector<double> hit_times(17);

         for (TdcHit* tdchit: tdchits) {
            // Use only rising edge
            if (tdchit->rising_edge==0) continue;
   
            // Use only fpga 1
            if (int(tdchit->fpga)!=1) continue;
   
            // Only channels 1-16
            int tdc_chan = int(tdchit->chan);
            if  (tdc_chan<1 or tdc_chan>16) continue;
            // Counts hits
            n_hits++;
            // Calculates hit time
            double time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time);
            double corrected_time = time - ProtoTOFTdcOffsets[tdc2adc(tdc_chan)]*1e-9;
            hit_times[tdc_chan] = corrected_time;

            // Fills histograms
            hTdcChan->Fill(int(tdchit->chan));
         }
         for (int i=1;i<=16;i++) {
            // Time difference from channel 0
            double time_diff = hit_times[i]-hit_times[0];
            hTdcDiffByChan->Fill(i,time_diff);
         }
      }
      if (!(fFlags->fProtoTOF)) {
         std::vector<double> hit_times(128);

         for (TdcHit* tdchit: tdchits) {
            // Use only rising edge
            if (tdchit->rising_edge==0) continue;
            // Gets bar ID
            int tdc_bar = fpga2barID(int(tdchit->fpga),int(tdchit->chan));
   
            // Counts hits
            n_hits++;
            // Calculates hit time
            double time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time);
            double corrected_time = time - TdcOffsets[tdc_bar]*1e-9;
            hit_times[tdc_bar] = corrected_time;
            // Fills histograms
            hTdcChan->Fill(int(tdchit->chan));
         }
         for (int i=0;i<128;i++) {
            // Time difference from channel 0
            double time_diff = hit_times[i]-hit_times[0];
            hTdcDiffByChan->Fill(i,time_diff);
         }
      }
      
      // Fills histograms
      hNTdcHits->Fill(n_hits);

   }



   // Adds data from the tdc to the end hits
   void AddTDCdata(TBarEvent* barEvt, TdcEvent* tdc)
   {
      // Get endhits from adc module
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      c_adc+=endhits.size();

      // Get tdc data
      std::vector<TdcHit*> tdchits = tdc->hits;
      c_tdc+=tdchits.size();

      for (EndHit* endhit: endhits)
         {
            TdcHit* matched_tdc_hit = NULL;
            double tdc_time = 0;
            double n_hits = 0;
            double n_good = 0;

            int bar = int(endhit->GetBar());
    
            // Finds tdc hits
            for (TdcHit* tdchit: tdchits)
               {
                  // Use only rising edge
                  if (tdchit->rising_edge==0) continue;

                  // Skip negative channels
                  if (int(tdchit->chan)<=0) continue;

                  // Gets bar number
                  int tdc_bar = fpga2barID(int(tdchit->fpga),int(tdchit->chan));

                  // Checks channel number
                  bool correct_channel = false;
                  if ( !(fFlags->fProtoTOF)) {
                     correct_channel = (tdc_bar==bar);
                  }
                  if (fFlags->fProtoTOF) {
                     int tdc_chan = -1;
                     for (int i = 0;i<16;i++)
                        {
                           if (protoTOFTdcMap[i][0]==endhit->GetBar()) tdc_chan = protoTOFTdcMap[i][1];
                        }
                     correct_channel = (int(tdchit->chan)==tdc_chan);
                  }

                  // Counts hits
                  n_hits++;
                  if (correct_channel) n_good++;

                  // Calculates hit time
                  double a_hit_time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time); 

                  // Yoinks the very first tdc hit on the right channel and calls it a day
                  if (tdc_time == 0 and correct_channel) {
                     tdc_time = a_hit_time;
                     matched_tdc_hit = tdchit;
                  }

                  // Fills histograms
                  if (fFlags->fProtoTOF) {
                     hTdcChan->Fill(int(tdchit->chan));
                  }
                  if (!(fFlags->fProtoTOF)) {
                     hTDCbar->Fill(tdc_bar);
                     hTdcAdcTime->Fill(endhit->GetADCTime(),tdc_time);
                  }

               }

            // Fills histograms
            hNTdcHits->Fill(n_hits);
            if (!(fFlags->fProtoTOF)) {
               hNMatchedHits->Fill(n_good);
            }

            // Corrects for tdc time offset
            double calib_time = tdc_time;
            if (fFlags->fProtoTOF) {
               if ( !(matched_tdc_hit)) continue;
               int tdc_chan = int(matched_tdc_hit->chan);
               if (tdc_chan>0 and tdc_chan<=16) calib_time = tdc_time - ProtoTOFTdcOffsets[tdc2adc(tdc_chan)] * 1e-9;
            }

            // Corrects for time walk
            double amp = endhit->GetAmp();
            double tw_correction = twA/TMath::Sqrt(amp);
            if (fFlags->ftwA!=0) tw_correction = fFlags->ftwA/TMath::Sqrt(amp);
            double correct_time = calib_time - tw_correction;

            // Writes tdc data to hit
            //endhit->SetTDCHit(correct_time);
            endhit->SetTDCHit(correct_time,calib_time,tdc_time);
            c_adctdc+=1;
         }
   }


   void CombineEnds(TBarEvent* barEvt)
   {
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      printf("Found %d endhits\n",endhits.size());
      for (EndHit* tophit: endhits)
         {
            if (!(tophit->IsTDCMatched())) continue; // REQUIRE TDC MATCHING

            // Gets first hit bar info from map
            int top_chan = tophit->GetBar();
            //            printf("top chan = %d",top_chan);
            int bot_chan = -1;
            int bar_num = -1;
            int end_num = -1;
            if (fFlags->fProtoTOF) {
               for (int i = 0;i<16;i++) {
                  if (protoTOFTdcMap[i][0]==tophit->GetBar()) {
                        bar_num = protoTOFTdcMap[i][2];
                        end_num = protoTOFTdcMap[i][3];
                     }
               }
            }

            // Exit if not top hit
            if (fFlags->fProtoTOF) {
               if (end_num!=0) continue;
            }
            if ( !(fFlags->fProtoTOF)) {
               if (top_chan<64) continue;
            }

            // Find corresponding bottom hit info from map
            if (fFlags->fProtoTOF) {
               for (int i=0;i<16;i++) {
                  if (protoTOFTdcMap[i][2]==bar_num and protoTOFTdcMap[i][3]==1) {
                     bot_chan = protoTOFTdcMap[i][0];
                  }
               }
               // Exit if none found
               if (bot_chan==-1) continue;
            }
            if ( !(fFlags->fProtoTOF)) {
               bot_chan = top_chan-64;
            }

            // Find bottom hit
            EndHit* bothit = NULL;
            for (EndHit* hit: endhits)
               {
                  if (hit->GetBar()==bot_chan) bothit = hit;
               }

            // Exit if none found
            if (!bothit) continue;


            // Adds a BarHit containing the top hit and bottom hit
            if (fFlags->fProtoTOF) {
               barEvt->AddBarHit(bothit,tophit,bar_num);
            }
            if ( !(fFlags->fProtoTOF) ) {
               barEvt->AddBarHit(bothit,tophit,bot_chan);
            }
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
            if (fFlags->fProtoTOF) {
               if (bar==0) {
                  diff_adc_0 = diff_adc;
                  diff_tdc_0 = diff_tdc;
               }
               if (bar==1) {
                  diff_adc_1 = diff_adc;
                  diff_tdc_1 = diff_tdc;
               }
            }
            if ( !(fFlags->fProtoTOF) ) {
               hDiffAdc->Fill(bar,diff_adc);
               hDiffTdc->Fill(bar,diff_tdc);
            }
         }
      if (fFlags->fProtoTOF and diff_adc_0!=0 and diff_adc_1!=0)
         {
            double c = 2.99792e8;
            double refrac = 1.58;
            double factor = c/refrac * 0.5;
            hBarDiffTdc->Fill(diff_tdc_0,diff_tdc_1);
            hBarDiffAdc->Fill(diff_adc_0,diff_adc_1);
            hBarDiffDiffTdc->Fill(diff_tdc_1-diff_tdc_0);
         }

   }

   void CalculateTOF(TBarEvent* barEvt) {

      if (!(fFlags->fProtoTOF)) return;

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
                  double TOF_ADC = t_ADC_1 - t_ADC_2;
                  double TOF_TDC = t_TDC_1 - t_TDC_2;
                  if (bar1==1) {
                     TOF_ADC = t_ADC_2 - t_ADC_1;
                     TOF_TDC = t_TDC_2 - t_TDC_1;
                  }
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

   int tdc2adc( int tdc_chan )
   {
      int adc_chan=-1;
      for (int i = 0;i<16;i++)
      {
         if (protoTOFTdcMap[i][1]==tdc_chan) adc_chan = protoTOFTdcMap[i][0];
      }
      return adc_chan;
   }
   int adc2tdc( int adc_chan )
   {
      int tdc_chan=-1;
      for (int i = 0;i<16;i++)
      {
         if (protoTOFTdcMap[i][0]==adc_chan) tdc_chan = protoTOFTdcMap[i][1];
      }
      return tdc_chan;
   }

};


class tdcModuleFactory: public TAFactory
{
public:
   TdcFlags fFlags;
public:
   void Help()
   {  
      printf("TdcModuleFactory::Help\n");
      printf("\t--anasettings /path/to/settings.json\t\t load the specified analysis settings\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("tdcModuleFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++) { 
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
         if (args[i] == "--bscprint")
            fFlags.fPrint = true; 
         if( args[i] == "--bscpulser" )
            fFlags.fPulser = true;
         if( args[i] == "--bscProtoTOF" )
            fFlags.fProtoTOF = true;
         if( args[i] == "--bscoffsetfile" )
            {
               fFlags.fWriteOffsets = true;
               fFlags.fOffsetFile = args[i+1].c_str();
            }
         if( args[i] == "--twA" )
            fFlags.ftwA = atof(args[i+1].c_str());
         if( args[i] == "--anasettings" ) 
            json=args[i+1];
      }
      fFlags.ana_settings=new AnaSettings(json.Data());
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
