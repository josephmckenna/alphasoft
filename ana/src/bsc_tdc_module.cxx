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
   // Time walk correction, dt = A/sqrt(amp)
   const double twA = 2.48753687e-09;

   // Container declaration
   int protoTOFTdcMap[4][4];
   double TdcOffsets[16] = {0};
   int bscTdcMap[64][5];
   

   //Histogramm declaration
   TH2D* hTdcAdcTime = NULL;
   TH1D* hTdcChan = NULL;
   TH1D* hNTdcHits = NULL;
   TH1D* hNMatchedHits = NULL;
   TH2D* hBarDiffAdc = NULL;
   TH1D* hBarDiffDiffAdc = NULL;
   TH2D* hBarDiffTdc = NULL;
   TH1D* hBarDiffDiffTdc = NULL;
   TH2D* hDiffAdc = NULL;
   TH2D* hDiffTdc = NULL;
   TH1D* hTOFADC = NULL;
   TH1D* hTOFTDC = NULL;
   TH2D* hNMatchedByChan = NULL;
   TH2D* hTdcDiffByChan = NULL;
   TH1D* hTdcDiffByChanMedian = NULL;
   TH1D* hTdcCalibrationOffsets = NULL;
   TH2D* hAmpSameEnds = NULL;
   TH2D* hAmpOppositeEnds = NULL;
   TH2D* hTWSameEnds = NULL;
   TH2D* hTWOppositeEnds = NULL;
   TH1D* hTime_1_0 = NULL;
   TH1D* hTime_2_0 = NULL;
   TH1D* hTime_2_1 = NULL;



   // Counter initialization
   int c_adc = 0;
   int c_tdc = 0;
   int c_adctdc = 0;
   int c_topbot = 0;

public:

   tdcmodule(TARunInfo* runinfo, TdcFlags* flags): 
      TARunObject(runinfo), fFlags(flags)
   {
#ifdef MANALYZER_PROFILER
      ModuleName="bsc tdc module";
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
      if (fFlags->fProtoTOF) {
         hTdcChan = new TH1D("hTdcChan","Number of hits on tdc channel;tdc channel",16,0.5,16.5);
         hTdcCalibrationOffsets = new TH1D("hTdcCalibrationOffsets","Calibration offset from file;TDC channel;Time offset (s)",16,0.5,16.5);
         hTdcAdcTime = new TH2D("hTdcAdcTime","adc vs tdc time;adc time;tdc time",250,1000,1500,200,-2.0e-6,0);
         if( !(fFlags->fPulser) ) { // Normal run
            hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",11,-0.5,10.5);
            hNMatchedHits = new TH1D("hNMatchedHits","Number of TDC hits in correct channel;Number of tdc hits",11,-0.5,10.5);
            hBarDiffAdc = new TH2D("hBarDiffAdc","ADC time difference between ends of bars;Time difference bar A [s];Time difference bar B [s]",200,-15e-9,15e-9,200,-15e-9,15e-9);
            hBarDiffTdc = new TH2D("hBarDiffTdc","TDC time difference between ends of bars;Time difference bar A [s];Time difference bar B [s]",200,-10e-9,10e-9,200,-10e-9,10e-9);
            hBarDiffDiffAdc = new TH1D("hBarDiffDiffAdc","(BarA top - BarA bottom) - (BarB top - BarB bottom) for ADC;ADC Time difference [s]",200,-10e-9,10e-9);
            hBarDiffDiffTdc = new TH1D("hBarDiffDiffTdc","(BarA top - BarA bottom) - (BarB top - BarB bottom) for TDC;TDC Time difference [s]",200,-4e-9,4e-9);
            hTOFADC = new TH1D("hTOFADC","Time of flight calculated using ADC;Time of flight [s]",200,-100e-9,100e-9);
            hTOFTDC = new TH1D("hTOFTDC","Time of flight calculated using TDC;Time of flight [s]",200,-5e-9,5e-9);
            hNMatchedByChan = new TH2D("hNMatchedByChan","Number of TDC hits in correct channel;adc channel;Number of tdc hits",16,-0.5,15.5,30,-0.5,29.5);
            hAmpSameEnds = new TH2D("hAmpSameEnds","Amplitude for hits on same end of different bars;Amplitude Bar A [V];Amplitude Bar B [V]",1000,0,4.,1000,0,4.);
            hAmpOppositeEnds = new TH2D("hAmpOppositeEnds","Amplitude for hits on opposite ends of same bar;Amplitude Top [V];Amplitude Bottom [V]",1000,0,4.,1000,0,4.);
            hTWSameEnds = new TH2D("hTWSameEnds","Time walk correction for hits on same end of different bars;Time walk correction Bar A [s];Time walk correction Bar B [s]",1000,0,4e-9,1000,0,4e-9);
            hTWOppositeEnds = new TH2D("hTWOppositeEnds","Time walk correction for hits on opposite ends of same bar;Time walk correction Top [s];Time walk correction Bottom [s]",1000,0,4e-9,1000,0,4e-9);
         }
         if( fFlags->fPulser ) { // Pulser run
            hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",65,-0.5,64.5);
            hNMatchedHits = new TH1D("hNMatchedHits","Number of TDC hits in correct channel;Number of tdc hits",65,-0.5,64.5);
            hTdcDiffByChan = new TH2D("hTdcDiffByChan","Pulser TDC time with reference to channel 1 hit;tdc channel;TDC time [s]",16,0.5,16.5,2000,-3e-9,5e-9);
            hTime_1_0 = new TH1D("hTime_1_0","Time on channel 1 minus 0;Time [s]",1000,170e-9,190e-9);
            hTime_2_0 = new TH1D("hTime_2_0","Time on channel 2 minus 0;Time [s]",1000,170e-9,190e-9);
            hTime_2_1 = new TH1D("hTime_2_1","Time on channel 2 minus 1;Time [s]",1000,0e-9,1e-9);
         }
      }
      if ( !(fFlags->fProtoTOF) ) {
         hTdcAdcTime = new TH2D("hTdcAdcTime","adc vs tdc time;adc time;tdc time",250,1000,1500,200,-2.0e-6,0);
         hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",100,-0.5,99.5);
         hNMatchedHits = new TH1D("hNMatchedHits","Number of TDC hits in channel corresponding to ADC hit;Number of tdc hits",30,-0.5,29.5);
         hDiffAdc = new TH2D("hDiffAdc","ADC time difference between ends;Bar number;Time [s]",64,-0.5,63.5,200,-50e-9,50e-9);
         hDiffTdc = new TH2D("hDiffTdc","TDC time difference between ends;Bar number;Time [s]",64,-0.5,63.5,200,-50e-9,50e-9);
         hTOFADC = new TH1D("hTOFADC","Time of flight calculated using ADC;Time of flight [s]",200,-1000e-9,1000e-9);
         hTOFTDC = new TH1D("hTOFTDC","Time of flight calculated using TDC;Time of flight [s]",200,-20e-9,20e-9);
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
            for(int i=0; i<4; i++)
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
                  ftdcoff >> num >> TdcOffsets[i];
                  hTdcCalibrationOffsets->Fill(i+1,TdcOffsets[i]);
                  hTdcCalibrationOffsets->SetBinError(i+1,0);
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
                  if (fFlags->ftwA!=0) printf("twA = %.6e\n",fFlags->ftwA);
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
      delete hBarDiffDiffAdc;
      delete hBarDiffDiffTdc;
      delete hTOFADC;
      delete hTOFTDC;
      delete hNMatchedByChan;
      delete hTdcDiffByChan;
      delete hTdcDiffByChanMedian;
      delete hTdcCalibrationOffsets;
      delete hAmpSameEnds;
      delete hAmpOppositeEnds;
      delete hTWSameEnds;
      delete hTWOppositeEnds;
      delete hTime_1_0;
      delete hTime_2_0;
      delete hTime_2_1;
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
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      AgEvent* age = ef->fEvent;
      if(!age)
      {
#ifdef MANALYZER_PROFILER
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
                        //CalculateTOF(barEvt);
                     }
                  if( fFlags->fPulser )
                     {
                        PulserAnalysis(barEvt,tdc);
                     }
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

      if (fFlags->fProtoTOF) {
         double n_hits = 0;
         std::vector<double> tdc_time(17,0);

         for (TdcHit* tdchit: tdchits) {
            // Use only rising edge
            if (tdchit->rising_edge==0) continue;
      
            // Use only fpga 1
            if (int(tdchit->fpga)!=1) continue;
      
            // Only channels 1-16
            int tdc_chan = int(tdchit->chan);
            if (tdc_chan==0) {
               tdc_time[0] = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time);
            }
            if  (tdc_chan<1 or tdc_chan>16) continue;

            // Counts hits
            n_hits++;

            // Calculates hit time
            double time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time);

            // Calibrates for time offset
            double calib_time = time;
            //if (tdc_chan>0 and tdc_chan<=16) calib_time = time - TdcOffsets[tdc_chan-1];

            // Checks if channel was already hit (uses first hit only)
            if (tdc_time[int(tdchit->chan)]!=0) continue;
            tdc_time[int(tdchit->chan)] = calib_time;

            // Fills histograms
            hTdcChan->Fill(int(tdchit->chan));

         }
      
         // Fills histograms
         hNTdcHits->Fill(n_hits);
         for (int i=1;i<17;i++) hTdcDiffByChan->Fill(i,tdc_time[i]-tdc_time[1]);
         hTime_1_0->Fill(tdc_time[1]-tdc_time[0]);
         hTime_2_0->Fill(tdc_time[2]-tdc_time[0]);
         hTime_2_1->Fill(tdc_time[2]-tdc_time[1]);
      }

      if (!(fFlags->fProtoTOF) ) {

      }

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

                  // Checks channel number
                  bool correct_channel = false;
                  if ( !(fFlags->fProtoTOF)) {
                     // Gets bar number
                     int tdc_bar = fpga2barID(int(tdchit->fpga),int(tdchit->chan));
                     correct_channel = (tdc_bar==bar);
                  }
                  if (fFlags->fProtoTOF) {
                     int tdc_chan = -1;
                     for (int i = 0;i<4;i++)
                        {
                           if (bscTdcMap[i][0]==endhit->GetBar()) tdc_chan = bscTdcMap[i][1];
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
                  hTdcAdcTime->Fill(endhit->GetADCTime(),tdc_time);

               }

            // Fills histograms
            hNTdcHits->Fill(n_hits);
            hNMatchedHits->Fill(n_good);

            // Corrects for tdc time offset
            double calib_time = tdc_time;
            if (fFlags->fProtoTOF) {
               if ( !(matched_tdc_hit)) continue;
               int tdc_chan = int(matched_tdc_hit->chan);
               if (tdc_chan>0 and tdc_chan<=16) calib_time = tdc_time - TdcOffsets[tdc_chan-1];
            }

            // Corrects for time walk
            double amp = endhit->GetAmp();
            double tw_correction = twA/TMath::Sqrt(amp);
            if (fFlags->ftwA!=0) tw_correction = fFlags->ftwA/TMath::Sqrt(amp);
            double correct_time = calib_time - tw_correction;

            // Writes tdc data to hit
            endhit->SetTDCHit(correct_time);
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
            if (fFlags->fProtoTOF) {
               for (int i = 0;i<4;i++) {
                  if (bscTdcMap[i][0]==tophit->GetBar()) {
                        bar_num = bscTdcMap[i][2];
                        end_num = bscTdcMap[i][3];
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
               for (int i=0;i<4;i++) {
                  if (bscTdcMap[i][2]==bar_num and bscTdcMap[i][3]==1) {
                     bot_chan = bscTdcMap[i][0];
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
                  hAmpSameEnds->Fill(hit1->GetTopHit()->GetAmp(),hit2->GetTopHit()->GetAmp());
                  hAmpSameEnds->Fill(hit1->GetBotHit()->GetAmp(),hit2->GetBotHit()->GetAmp());
                  hAmpOppositeEnds->Fill(hit1->GetTopHit()->GetAmp(),hit1->GetBotHit()->GetAmp());
                  hAmpOppositeEnds->Fill(hit2->GetTopHit()->GetAmp(),hit2->GetBotHit()->GetAmp());
                  hTWSameEnds->Fill(twA/TMath::Sqrt(hit1->GetTopHit()->GetAmp()),twA/TMath::Sqrt(hit2->GetTopHit()->GetAmp()));
                  hTWSameEnds->Fill(twA/TMath::Sqrt(hit1->GetBotHit()->GetAmp()),twA/TMath::Sqrt(hit2->GetBotHit()->GetAmp()));
                  hTWOppositeEnds->Fill(twA/TMath::Sqrt(hit1->GetTopHit()->GetAmp()),twA/TMath::Sqrt(hit1->GetBotHit()->GetAmp()));
                  hTWOppositeEnds->Fill(twA/TMath::Sqrt(hit2->GetTopHit()->GetAmp()),twA/TMath::Sqrt(hit2->GetBotHit()->GetAmp()));
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
