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
   const double twA = 1.75e-9;

   // Container declaration
   int protoTOFTdcMap[16][4];
   double TdcOffsets[16] = {0};
   int bscTdcMap[64][5];
   

   //Histogramm declaration
   TH1D* hTdcChan = NULL;
   TH1D* hNTdcHits = NULL;
   TH2D* hBarDiffAdc = NULL;
   TH2D* hBarDiffTdc = NULL;
   TH2D* hBarDiffTdcPart = NULL;
   TH2D* hBarDiffTdcRaw = NULL;
   TH1D* hBarDiffDiffTdc = NULL;
   TH1D* hBarDiffDiffTdcPart = NULL;
   TH1D* hBarDiffDiffTdcRaw = NULL;
   TH2D* hDiffAdc = NULL;
   TH2D* hDiffTdc = NULL;
   TH1D* hTOFTDC = NULL;
   TH1D* hTOFTDCPart = NULL;
   TH1D* hTOFTDCRaw = NULL;
   TH2D* hTdc1DiffByChan = NULL;
   TH1D* hTdc1DiffByChanMedian = NULL;
   TH2D* hTdc1DiffByChanCorrected = NULL;
   TH2D* hTdc2DiffByChan = NULL;
   TH1D* hTdc2DiffByChanMedian = NULL;
   TH2D* hTdc2DiffByChanCorrected = NULL;
   TH2D* hTdc1FallDiffByChan = NULL;
   TH2D* hTdc1FallDiffByChanCorrected = NULL;
   TH2D* hTdc2FallDiffByChanCorrected = NULL;
   TH2D* hTdc2FallDiffByChan = NULL;
   TH2D* hTdc12DiffByChan = NULL;
   TH2D* hTdc1Duration = NULL;
   TH2D* hTdc2Duration = NULL;
   TH2D* hTdcGap = NULL;
   TH2D* hTdcRise2Rise = NULL;
   TH2D* hTWOppositeEnds = NULL;
   TH2D* hOffset2 = NULL;
   TH2D* hOffset3 = NULL;
   TH2D* hOffset4 = NULL;
   TH2D* hOffset5 = NULL;
   TH2D* hOffset6 = NULL;
   TH2D* hOffset7 = NULL;
   TH2D* hOffset8 = NULL;
   TH2D* hOffset9 = NULL;



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
         hTdcChan = new TH1D("hTdcChan","Number of hits on tdc channel;tdc channel",34,-1.5,32.5);
         if( !(fFlags->fPulser) ) { // Normal run
            hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",11,-0.5,10.5);
            hBarDiffAdc = new TH2D("hBarDiffAdc","ADC time difference between ends of bars;Time difference bar A [s];Time difference bar B [s]",200,-15e-9,15e-9,200,-15e-9,15e-9);
            hBarDiffTdc = new TH2D("hBarDiffTdc","TDC time difference between ends of bars (channel-by-channel and time-walk correction);Time difference bar A [s];Time difference bar B [s]",200,-10e-9,10e-9,200,-10e-9,10e-9);
            hBarDiffTdcPart = new TH2D("hBarDiffTdcPart","TDC time difference between ends of bars (channel-by-channel correction only);Time difference bar A [s];Time difference bar B [s]",200,-10e-9,10e-9,200,-10e-9,10e-9);
            hBarDiffTdcRaw = new TH2D("hBarDiffTdcRaw","TDC time difference between ends of bars (no corrections);Time difference bar A [s];Time difference bar B [s]",200,-10e-9,10e-9,200,-10e-9,10e-9);
            hBarDiffDiffTdc = new TH1D("hBarDiffDiffTdc","(BarA top - BarA bottom) - (BarB top - BarB bottom) for TDC (channel-by-channel and time-walk correction);TDC Time difference [s]",200,-4e-9,4e-9);
            hBarDiffDiffTdcPart = new TH1D("hBarDiffDiffTdcPart","(BarA top - BarA bottom) - (BarB top - BarB bottom) for TDC (channel-by-channel corrections only);TDC Time difference [s]",200,-4e-9,4e-9);
            hBarDiffDiffTdcRaw = new TH1D("hBarDiffDiffTdcRaw","(BarA top - BarA bottom) - (BarB top - BarB bottom) for TDC (no corrections);TDC Time difference [s]",200,-4e-9,4e-9);
            hTOFTDC = new TH1D("hTOFTDC","Time of flight calculated using TDC (channel-by-channel and time-walk correction);Time of flight [s]",200,-5e-9,5e-9);
            hTOFTDCPart = new TH1D("hTOFTDCPart","Time of flight calculated using TDC (channel-by-channel correction only);Time of flight [s]",200,-5e-9,5e-9);
            hTOFTDCRaw = new TH1D("hTOFTDCRaw","Time of flight calculated using TDC (no corrections);Time of flight [s]",200,-5e-9,5e-9);
            hTWOppositeEnds = new TH2D("hTWOppositeEnds","Time walk correction for hits on opposite ends of same bar;Time walk correction Top [s];Time walk correction Bottom [s]",1000,0,4e-9,1000,0,4e-9);
         }
         if( fFlags->fPulser ) { // Pulser run
            hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",65,-0.5,64.5);
            hTdc1DiffByChan = new TH2D("hTdc1DiffByChan","Pulser TDC time offset for first hit;adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hTdc1DiffByChanCorrected = new TH2D("hTdc1DiffByChanCorrected","Pulser TDC time offset for first hit (after corrections);adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hTdc2DiffByChan = new TH2D("hTdc2DiffByChan","Pulser TDC time offset for second hit;adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hTdc2DiffByChanCorrected = new TH2D("hTdc2DiffByChanCorrected","Pulser TDC time offset for second hit (after corrections);adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hTdc12DiffByChan = new TH2D("hTdc12DiffByChan","Pulser TDC time offset second hit minus first hit;adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hTdc1FallDiffByChan = new TH2D("hTdc1FallDiffByChan","Pulser TDC time offset for first hit falling edge;adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hTdc1FallDiffByChanCorrected = new TH2D("hTdc1FallDiffByChanCorrected","Pulser TDC time offset for first hit falling edge (after corrections);adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hTdc2FallDiffByChan = new TH2D("hTdc2FallDiffByChan","Pulser TDC time offset for second hit falling edge;adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hTdc1Duration = new TH2D("hTdc1Duration","First pulse time-over-threshold;adc channel;TDC time [ns]",16,-0.5,15.5,1000,0,100);
            hTdc2Duration = new TH2D("hTdc2Duration","Second pulse time-over-threshold;adc channel;TDC time [ns]",16,-0.5,15.5,1000,0,100);
            hTdcGap = new TH2D("hTdcGap","Gap between first and second pulses;adc channel;TDC time [ns]",16,-0.5,15.5,1000,0,150);
            hTdcRise2Rise = new TH2D("hhTdcRise2Rise","Difference between first and second pulse starts;adc channel;TDC time [ns]",16,-0.5,15.5,1000,0,300);
            hTdc2FallDiffByChanCorrected = new TH2D("hTdc2FallDiffByChanCorrected","Pulser TDC time offset for second hit falling edge (after corrections);adc channel;TDC time [ns]",16,-0.5,15.5,2000,-4,8);
            hOffset2 = new TH2D("hOffset2","Offset on channel 2;Hit number;Time [s]",1000,0,5e6,1000,-5e-9,7e-9);
            hOffset3 = new TH2D("hOffset3","Offset on channel 3;Hit number;Time [s]",1000,0,5e6,1000,-5e-9,7e-9);
            hOffset4 = new TH2D("hOffset4","Offset on channel 4;Hit number;Time [s]",1000,0,5e6,1000,-5e-9,7e-9);
            hOffset5 = new TH2D("hOffset5","Offset on channel 5;Hit number;Time [s]",1000,0,5e6,1000,-5e-9,7e-9);
            hOffset6 = new TH2D("hOffset6","Offset on channel 6;Hit number;Time [s]",1000,0,5e6,1000,-5e-9,7e-9);
            hOffset7 = new TH2D("hOffset7","Offset on channel 7;Hit number;Time [s]",1000,0,5e6,1000,-5e-9,7e-9);
            hOffset8 = new TH2D("hOffset8","Offset on channel 8;Hit number;Time [s]",1000,0,5e6,1000,-5e-9,7e-9);
            hOffset9 = new TH2D("hOffset9","Offset on channel 9;Hit number;Time [s]",1000,0,5e6,1000,-5e-9,7e-9);
         }
      }
      if ( !(fFlags->fProtoTOF) ) {
         hNTdcHits = new TH1D("hNTdcHits","Number of TDC hits in event;Number of tdc hits",100,-0.5,99.5);
         hDiffAdc = new TH2D("hDiffAdc","ADC time difference between ends;Bar number;Time [s]",64,-0.5,63.5,200,-50e-9,50e-9);
         hDiffTdc = new TH2D("hDiffTdc","TDC time difference between ends;Bar number;Time [s]",64,-0.5,63.5,200,-50e-9,50e-9);
         hTOFTDC = new TH1D("hTOFTDC","Time of flight calculated using TDC (channel-by-channel and time-walk correction);Time of flight [s]",200,-20e-9,20e-9);
         hTOFTDCPart = new TH1D("hTOFTDCPart","Time of flight calculated using TDC (channel-by-channel correction only);Time of flight [s]",200,-20e-9,20e-9);
         hTOFTDCRaw = new TH1D("hTOFTDCRaw","Time of flight calculated using TDC (no corrections);Time of flight [s]",200,-20e-9,20e-9);
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
                  ftdcoff >> num >> TdcOffsets[i];
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
            hTdc1DiffByChanMedian = hTdc1DiffByChan->QuantilesX(0.5,"hTdc1DiffByChanMedian");
            hTdc1DiffByChanMedian->SetTitle("Median time offset by TDC channel for second hit;TDC channel;TDC time [s]");
            hTdc2DiffByChanMedian = hTdc2DiffByChan->QuantilesX(0.5,"hTdc2DiffByChanMedian");
            hTdc2DiffByChanMedian->SetTitle("Median time offset by TDC channel for second hit;TDC channel;TDC time [s]");

            // Writes offsets to file
            if ( fFlags->fWriteOffsets )
               {
                  std::ofstream Ofile;
                  Ofile.open(fFlags->fOffsetFile);
                  Ofile<<"ADC channel | Offset(s)\n";
                  for (int i=1;i<=16;i++) Ofile<<i<<"\t"<<hTdc1DiffByChanMedian->GetBinContent(i)<<"\n";
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
                  if (fFlags->ftwA!=0) printf("twA = %.6e sigma_TOF = %.9e\n",fFlags->ftwA,hTOFTDC->GetStdDev());
               }
         }

      // Delete histograms
      delete hTdcChan;
      delete hNTdcHits;
      delete hDiffAdc;
      delete hDiffTdc;
      delete hBarDiffAdc;
      delete hBarDiffTdc;
      delete hBarDiffTdcPart;
      delete hBarDiffTdcRaw;
      delete hBarDiffDiffTdc;
      delete hBarDiffDiffTdcPart;
      delete hBarDiffDiffTdcRaw;
      delete hTOFTDC;
      delete hTOFTDCPart;
      delete hTOFTDCRaw;
      delete hTdc1DiffByChan;
      delete hTdc1DiffByChanMedian;
      delete hTdc1DiffByChanCorrected;
      delete hTdc2DiffByChan;
      delete hTdc2DiffByChanMedian;
      delete hTdc2DiffByChanCorrected;
      delete hTdc12DiffByChan;
      delete hTdc1FallDiffByChan;
      delete hTdc1FallDiffByChanCorrected;
      delete hTdc2FallDiffByChan;
      delete hTdc2FallDiffByChanCorrected;
      delete hTdc1Duration;
      delete hTdc2Duration;
      delete hTdcGap;
      delete hTdcRise2Rise;
      delete hTWOppositeEnds;
      delete hOffset2;
      delete hOffset3;
      delete hOffset4;
      delete hOffset5;
      delete hOffset6;
      delete hOffset7;
      delete hOffset8;
      delete hOffset9;
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
                        CalculateTOF(barEvt);
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
         std::vector<double> first_hit(17,0);
         std::vector<double> second_hit(17,0);
         std::vector<double> first_hit_corrected(17,0);
         std::vector<double> second_hit_corrected(17,0);
         std::vector<double> first_hit_fall(17,0);
         std::vector<double> second_hit_fall(17,0);
         std::vector<double> first_hit_fall_corrected(17,0);
         std::vector<double> second_hit_fall_corrected(17,0);

         for (TdcHit* tdchit: tdchits) {
            // Use only rising edge
            //if (tdchit->rising_edge==0) continue;
            printf("fpga %d channel %d rising %d\n",int(tdchit->fpga),int(tdchit->chan),int(tdchit->rising_edge));
      
            // Use only fpga 1
            if (int(tdchit->fpga)!=1) continue;
      
            // Only channels 1-16
            int tdc_chan = int(tdchit->chan);
            //if  (tdc_chan<1 or tdc_chan>16) continue;

            // Counts hits
            n_hits++;

            // Calculates hit time
            double time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time);
            double corrected_time = time - TdcOffsets[tdc2adc(tdc_chan)]*1e-9;
            if (fFlags->fPrint) printf("GS: channel %d rising %d time %.12e correction %.12e final %.12e\n",tdc_chan,tdchit->rising_edge,time,TdcOffsets[tdc2adc(tdc_chan)]*1e-9,corrected_time);

            if (tdchit->rising_edge==1) {
               if (first_hit[tdc_chan]!=0 and second_hit[tdc_chan]==0) {
                  second_hit[tdc_chan]=time;
                  second_hit_corrected[tdc_chan]=corrected_time;
               }
               if (first_hit[tdc_chan]==0) {
                  first_hit[tdc_chan]=time;
                  first_hit_corrected[tdc_chan]=corrected_time;
               }
            }
            if (tdchit->rising_edge==0) {
               if (first_hit_fall[tdc_chan]!=0 and second_hit_fall[tdc_chan]==0) {
                  second_hit_fall[tdc_chan]=time;
                  second_hit_fall_corrected[tdc_chan]=corrected_time;
               }
               if (first_hit_fall[tdc_chan]==0) {
                  first_hit_fall[tdc_chan]=time;
                  first_hit_fall_corrected[tdc_chan]=corrected_time;
               }
            }

            // Fills histograms
            hTdcChan->Fill(int(tdchit->chan));

         }
      
         // Fills histograms
         hNTdcHits->Fill(n_hits);
         for (int i=1;i<17;i++) hTdc1DiffByChan->Fill(tdc2adc(i),1e9*(first_hit[i]-first_hit[1]));
         for (int i=1;i<17;i++) hTdc1DiffByChanCorrected->Fill(tdc2adc(i),1e9*(first_hit_corrected[i]-first_hit_corrected[1]));
         for (int i=1;i<17;i++) hTdc2DiffByChan->Fill(tdc2adc(i),1e9*(second_hit[i]-second_hit[1]));
         for (int i=1;i<17;i++) hTdc2DiffByChanCorrected->Fill(tdc2adc(i),1e9*(second_hit_corrected[i]-second_hit_corrected[1]));
         for (int i=1;i<17;i++) hTdc1FallDiffByChan->Fill(tdc2adc(i),1e9*(first_hit_fall[i]-first_hit_fall[1]));
         for (int i=1;i<17;i++) hTdc1FallDiffByChanCorrected->Fill(tdc2adc(i),1e9*(first_hit_fall_corrected[i]-first_hit_fall_corrected[1]));
         for (int i=1;i<17;i++) hTdc2FallDiffByChan->Fill(tdc2adc(i),1e9*(second_hit_fall[i]-second_hit_fall[1]));
         for (int i=1;i<17;i++) hTdc2FallDiffByChanCorrected->Fill(tdc2adc(i),1e9*(second_hit_fall_corrected[i]-second_hit_fall_corrected[1]));
         for (int i=1;i<17;i++) hTdc12DiffByChan->Fill(tdc2adc(i),1e9*((second_hit[i]-second_hit[1])-(first_hit[i]-first_hit[1])));
         for (int i=1;i<17;i++) hTdc1Duration->Fill(tdc2adc(i),1e9*(first_hit_fall[i]-first_hit[i]));
         for (int i=1;i<17;i++) hTdc2Duration->Fill(tdc2adc(i),1e9*(second_hit_fall[i]-second_hit[i]));
         for (int i=1;i<17;i++) hTdcGap->Fill(tdc2adc(i),1e9*(second_hit[i]-first_hit_fall[i]));
         for (int i=1;i<17;i++) hTdcRise2Rise->Fill(tdc2adc(i),1e9*(second_hit[i]-first_hit[i]));
         hOffset2->Fill(c_tdc,second_hit[2]-second_hit[1]);
         hOffset3->Fill(c_tdc,second_hit[3]-second_hit[1]);
         hOffset4->Fill(c_tdc,second_hit[4]-second_hit[1]);
         hOffset5->Fill(c_tdc,second_hit[5]-second_hit[1]);
         hOffset6->Fill(c_tdc,second_hit[6]-second_hit[1]);
         hOffset7->Fill(c_tdc,second_hit[7]-second_hit[1]);
         hOffset8->Fill(c_tdc,second_hit[8]-second_hit[1]);
         hOffset9->Fill(c_tdc,second_hit[9]-second_hit[1]);
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


               }

            // Fills histograms
            hNTdcHits->Fill(n_hits);

            // Corrects for tdc time offset
            double calib_time = tdc_time;
            if (fFlags->fProtoTOF) {
               if ( !(matched_tdc_hit)) continue;
               int tdc_chan = int(matched_tdc_hit->chan);
               if (tdc_chan>0 and tdc_chan<=16) calib_time = tdc_time - TdcOffsets[tdc2adc(tdc_chan)] * 1e-9;
            }

            // Corrects for time walk
            double amp = endhit->GetAmp();
            double tw_correction = twA/TMath::Sqrt(amp);
            if (fFlags->ftwA!=0) tw_correction = fFlags->ftwA/TMath::Sqrt(amp);
            double correct_time = calib_time - tw_correction;

            // Writes tdc data to hit
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
            printf("top chan = %d",top_chan);
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
      double diff_tdc_raw_0 = 0;
      double diff_tdc_part_0 = 0;
      double diff_adc_1 = 0;
      double diff_tdc_1 = 0;
      double diff_tdc_raw_1 = 0;
      double diff_tdc_part_1 = 0;
      for (BarHit* hit: barhits)
         {
            int bar = hit->GetBar();
            double diff_tdc = hit->GetTopHit()->GetTDCTime() - hit->GetBotHit()->GetTDCTime();
            double diff_tdc_part = hit->GetTopHit()->GetTDCTimePartCalib() - hit->GetBotHit()->GetTDCTimePartCalib();
            double diff_tdc_raw = hit->GetTopHit()->GetTDCTimeRaw() - hit->GetBotHit()->GetTDCTimeRaw();
            double diff_adc = (hit->GetTopHit()->GetADCTime() - hit->GetBotHit()->GetADCTime())*1e-9;
            if (fFlags->fProtoTOF) {
               if (bar==0) {
                  diff_adc_0 = diff_adc;
                  diff_tdc_0 = diff_tdc;
                  diff_tdc_raw_0 = diff_tdc_raw;
                  diff_tdc_part_0 = diff_tdc_part;
               }
               if (bar==1) {
                  diff_adc_1 = diff_adc;
                  diff_tdc_1 = diff_tdc;
                  diff_tdc_raw_1 = diff_tdc_raw;
                  diff_tdc_part_1 = diff_tdc_part;
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
            hBarDiffTdcRaw->Fill(diff_tdc_raw_0,diff_tdc_raw_1);
            hBarDiffTdcPart->Fill(diff_tdc_part_0,diff_tdc_part_1);
            hBarDiffAdc->Fill(diff_adc_0,diff_adc_1);
            hBarDiffDiffTdc->Fill(diff_tdc_1-diff_tdc_0);
            hBarDiffDiffTdcRaw->Fill(diff_tdc_raw_1-diff_tdc_raw_0);
            hBarDiffDiffTdcPart->Fill(diff_tdc_part_1-diff_tdc_part_0);
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
                  double t_TDC_raw_1 = (hit1->GetTopHit()->GetTDCTimeRaw() + hit1->GetBotHit()->GetTDCTimeRaw())/2.;
                  double t_TDC_raw_2 = (hit2->GetTopHit()->GetTDCTimeRaw() + hit2->GetBotHit()->GetTDCTimeRaw())/2.;
                  double t_TDC_part_1 = (hit1->GetTopHit()->GetTDCTimePartCalib() + hit1->GetBotHit()->GetTDCTimePartCalib())/2.;
                  double t_TDC_part_2 = (hit2->GetTopHit()->GetTDCTimePartCalib() + hit2->GetBotHit()->GetTDCTimePartCalib())/2.;
                  double TOF_ADC = t_ADC_1 - t_ADC_2;
                  double TOF_TDC = t_TDC_1 - t_TDC_2;
                  double TOF_TDC_Raw = t_TDC_raw_1 - t_TDC_raw_2;
                  double TOF_TDC_Part = t_TDC_part_1 - t_TDC_part_2;
                  if (bar1==1) {
                     TOF_ADC = t_ADC_2 - t_ADC_1;
                     TOF_TDC = t_TDC_2 - t_TDC_1;
                     TOF_TDC_Raw = t_TDC_raw_2 - t_TDC_raw_1;
                     TOF_TDC_Part = t_TDC_part_2 - t_TDC_part_1;
                  }
                  hTOFTDC->Fill(TOF_TDC);
                  hTOFTDCRaw->Fill(TOF_TDC_Raw);
                  hTOFTDCPart->Fill(TOF_TDC_Part);
                  if (fFlags->fProtoTOF) {
                     hTWOppositeEnds->Fill(twA/TMath::Sqrt(hit1->GetTopHit()->GetAmp()),twA/TMath::Sqrt(hit1->GetBotHit()->GetAmp()));
                     hTWOppositeEnds->Fill(twA/TMath::Sqrt(hit2->GetTopHit()->GetAmp()),twA/TMath::Sqrt(hit2->GetBotHit()->GetAmp()));
                  }
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
