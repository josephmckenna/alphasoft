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
   bool fDiag = false;
   bool fProtoTOF = false;
   bool fPulser = false;
   std::string fOffsetFile = ""; 
   bool fNoDelayCorrection = false;
   double ftwA = -999;
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
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
   const double twA = 1.75e-9;

   // Container declaration
   double BVTdcOffsets[128] = {0};
   double ProtoTOFTdcOffsets[16] = {0};
   int BVTdcMap[64][7];
   int protoTOFTdcMap[16][4];
   

   //Histogramm declaration
   TH1D* hTDCbar = NULL;

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
      if (fFlags->fDiag) {
         runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
         gDirectory->mkdir("bsc_tdc_module")->cd();

         // Histogramm declaration
         if (fFlags->fProtoTOF) {
            hTDCbar = new TH1D("hTdcBar","TDC channel occupancy;Channel number",16,-0.5,15.5);
         }
         if (!(fFlags->fProtoTOF)) {
            hTDCbar = new TH1D("hTdcBar","TDC channel occupancy;Channel number",128,-0.5,127.5);
         }
      }

      // Loads BV and protoTOF maps
      TString protoTOFmapfile=getenv("AGRELEASE");
      protoTOFmapfile+="/ana/bscint/protoTOF.map";
      TString BVmapfile=getenv("AGRELEASE");
      BVmapfile+="/ana/bscint/BV.map";
      std::ifstream fBVMap(BVmapfile.Data());
      std::ifstream fProtoTOFMap(protoTOFmapfile.Data());
      if(fBVMap) {
         std::string comment; getline(fBVMap, comment);
         for(int i=0; i<64; i++) {
            fBVMap >> BVTdcMap[i][0] >> BVTdcMap[i][1] >> BVTdcMap[i][2] >> BVTdcMap[i][3] >> BVTdcMap[i][4] >> BVTdcMap[i][5] >> BVTdcMap[i][6];
         }
         fBVMap.close();
      }
      if(fProtoTOFMap) {
         std::string comment; getline(fProtoTOFMap, comment);
         for(int i=0; i<16; i++) {
            fProtoTOFMap >> protoTOFTdcMap[i][0] >> protoTOFTdcMap[i][1] >> protoTOFTdcMap[i][2] >> protoTOFTdcMap[i][3];
         }
         fProtoTOFMap.close();
      }

      // Load TDC offsets from calibration file
      TString BVoffsetfile=getenv("AGRELEASE");
      BVoffsetfile+="/ana/bscint/BVoffsets.calib";
      TString protoTOFoffsetfile=getenv("AGRELEASE");
      protoTOFoffsetfile+="/ana/bscint/protoTOFoffsets.calib";
      std::ifstream fBVoffsets(BVoffsetfile.Data());
      if(fBVoffsets) {
         std::string comment; getline(fBVoffsets, comment);
         int num;
         for(int i=0; i<128; i++) fBVoffsets >> num >> BVTdcOffsets[i];
         if (fFlags->fPrint) {
            printf("BV TDC offset file loaded from %s\n",BVoffsetfile.Data());
            printf("Bar | TDC time offset (ns)\n");
            for (int i=0;i<128;i++) printf("%d | %f\n",i,BVTdcOffsets[i]);
         }
         fBVoffsets.close();
      }
      std::ifstream fprotoTOFoffsets(protoTOFoffsetfile.Data());
      if(fprotoTOFoffsets) {
         std::string comment; getline(fprotoTOFoffsets, comment);
         int num;
         for(int i=0; i<16; i++) fprotoTOFoffsets >> num >> ProtoTOFTdcOffsets[i];
         if (fFlags->fPrint) {
            printf("protoTOF TDC offset file loaded from %s\n",protoTOFoffsetfile.Data());
            printf("Channel | TDC time offset (ns)\n");
            for (int i=0;i<128;i++) printf("%d | %f\n",i,ProtoTOFTdcOffsets[i]);
         }
         fprotoTOFoffsets.close();
      }

   }

   void EndRun(TARunInfo* runinfo)
   {
      // Write output
      runinfo->fRoot->fOutputFile->Write();

      // Print stats
      if( fFlags->fPrint )
         {
            printf("tdc module stats:\n");
            printf("Total number of adc hits = %d\n",c_adc);
            printf("Total number of tdc hits = %d\n",c_tdc);
            if (!fFlags->fPulser) {
               printf("Total number of adc+tdc combined hits = %d\n",c_adctdc);
               printf("Total number of top+bot combined hits = %d\n",c_topbot);
            }
         }

      // Delete histograms
      if (fFlags->fDiag)
         delete hTDCbar;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if( fFlags->fPrint ) printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if( fFlags->fPrint ) printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   // Main function
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
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
      if (fFlags->fTimeCut) {
         if (age->time<fFlags->start_time) {
#ifdef HAVE_MANALYZER_PROFILER
            *flags|=TAFlag_SKIP_PROFILE;
#endif
            return flow;
         }
         if (age->time>fFlags->stop_time) {
#ifdef HAVE_MANALYZER_PROFILER
            *flags|=TAFlag_SKIP_PROFILE;
#endif
            return flow;
         }
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

                  GetTDCTimes(barEvt,tdc);
                  MatchTDCtoADC(barEvt);
                  CombineEnds(barEvt);
                  CalculateZ(barEvt);

                  if( fFlags->fPrint ) printf("tdcmodule::AnalyzeFlowEvent comlpete\n");
               }
            else
               if( fFlags->fPrint )
                  std::cout<<"tdcmodule::AnalyzeFlowEvent  TDC event incomplete\n"<<std::endl;
         }
      else
         if( fFlags->fPrint )         
            std::cout<<"tdcmodule::AnalyzeFlowEvent  No TDC event\n"<<std::endl;

      return flow;
   }

   //________________________________
   // MAIN FUNCTIONS


   // Adds data from the tdc to the end hits
   void GetTDCTimes(TBarEvent* barEvt, TdcEvent* tdc)
   {
      // Get tdc data
      std::vector<TdcHit*> tdchits = tdc->hits;
      c_tdc+=tdchits.size();

      // Computes time for tdc hits
      for (TdcHit* tdchit: tdchits)
         {
            // Use only rising edge
            if (tdchit->rising_edge==0) continue;

            // Skip negative channels
            if (int(tdchit->chan)<=0) continue;

            // Gets channel number
            int barID = -1;
            if (!(fFlags->fProtoTOF)) barID = BVFindBarID(int(tdchit->fpga),int(tdchit->chan));
            if (fFlags->fProtoTOF) barID = protoTOFFindBarID(tdchit->chan);

            // Skips channels 0-7, 48-55, 64-71, 112-119 FIXME remember to change this backa once they are time calibrated
            if ((0<=barID and barID<=7) or (64<=barID and barID<=71) or (48<=barID and barID<=55) or (112<=barID and barID<=119)) continue;

            // Calculates hit time
            double tdc_time = GetFinalTime(tdchit->epoch,tdchit->coarse_time,tdchit->fine_time); 
            
            // Corrects for tdc time offset
            double calib_time = tdc_time;
            if (!(fFlags->fNoDelayCorrection)) {
               if (fFlags->fProtoTOF) {
                  if (barID<0 || barID>=16) { if (fFlags->fPrint) printf("bsc_tdc_module: bar ID = %d out of bounds, time offset calibration not applied\n",barID); }
                  else calib_time = tdc_time - ProtoTOFTdcOffsets[barID]*1e-9;
               }
               if (!(fFlags->fProtoTOF)) {
                  if (barID<0 || barID>=128) { if (fFlags->fPrint) printf("bsc_tdc_module: bar ID = %d out of bounds, time offset calibration not applied\n",barID); }
                  else calib_time = tdc_time - BVTdcOffsets[barID]*1e-9;
               }
            }

            // Fills occupancy histogram
            if (fFlags->fDiag)
               hTDCbar->Fill(barID);

            // Writes tdc data to SimpleTdcHit to add back into the flow
            barEvt->AddTdcHit(barID,calib_time);
         }
   }

   // Matches a tdc hit to each waveform
   void MatchTDCtoADC(TBarEvent* barEvt)
   {
      // Get endhits from adc module
      std::vector<EndHit*> endhits = barEvt->GetEndHits();
      c_adc+=endhits.size();

      // Gets tdc hits added to BarEvent by last function
      std::vector<SimpleTdcHit*> tdchits = barEvt->GetTdcHits();

      for (EndHit* endhit: endhits)
         {
            double tdc_time = 0;
            int bar = int(endhit->GetBar());
    
            // Finds tdc hit
            for (const SimpleTdcHit* tdchit: tdchits)
               {
                  // Checks channel number
                  if (bar!=tdchit->GetBar()) continue;

                  // Saves hit time of first hit in matching channel
                  if (tdc_time==0) tdc_time = tdchit->GetTime();
               }

            // Continue if no match found
            if (tdc_time==0) continue;

            // Corrects for time walk
            double amp = endhit->GetAmp();
            double tw_correction = twA/TMath::Sqrt(amp);
            if (fFlags->ftwA!=-999) tw_correction = fFlags->ftwA/TMath::Sqrt(amp);
            double correct_time = tdc_time - tw_correction;

            // Writes tdc data to endhit
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
                  if (!(hit->GetBar()==bot_chan)) continue;
                  if (!(hit->IsTDCMatched())) continue;
                  bothit = hit;
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
      for (BarHit* hit: barhits)
         {
            int bar = hit->GetBar();
            double diff_tdc = hit->GetBotHit()->GetTDCTime() - hit->GetTopHit()->GetTDCTime();
            double c = 2.99792e8;
            double refrac = 1.93; // From protoTOF tests with time walk correction applied
            double factor = c/refrac * 0.5;
            double zed = diff_tdc*factor;
            hit->SetZed(zed);
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

   int BVFindBarID(int fpga, int chan)
   {
      for (int bar=0; bar<64; bar++) {
         if (fpga==BVTdcMap[bar][1]-1 and chan==BVTdcMap[bar][5]) return bar+64; // top
         if (fpga==BVTdcMap[bar][1]-1 and chan==BVTdcMap[bar][6]) return bar; // bottom
      }
      if (fFlags->fPrint) printf("bsc_tdc_module failed to get bar number for fpga %d chan %d \n",fpga,chan);
      return -1;
   }
   int protoTOFFindBarID(int tdc_chan)
   {
      for (int i=0;i<16;i++) {
         if (protoTOFTdcMap[i][1]==tdc_chan) return protoTOFTdcMap[i][0];
      }
      return -1;
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
      printf("\t--bscpulser\t\t\tanalyze run with calibration pulser data instead of cosmics/hbar data\n");
      printf("\t--bscProtoTOF\t\t\tanalyze run with with TRIUMF prototype instead of full BV\n");
      printf("\t--bscprint\t\t\tverbose mode\n");
      printf("\t--twA float\t\t\tsets the parameter used in the time-walk correction. 0=no correction\n");
      printf("\t--nodelaycorr\t\t\tdisables the channel-by-channel tdc time delay correction\n");
      printf("\t--usetimerange 123.4 567.8\t\tLimit analysis to a time range\n");
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
         if (args[i] == "--bscdiag")
            fFlags.fDiag = true; 
         if( args[i] == "--bscpulser" )
            fFlags.fPulser = true;
         if( args[i] == "--bscProtoTOF" )
            fFlags.fProtoTOF = true;
         if( args[i] == "--twA" )
            fFlags.ftwA = atof(args[i+1].c_str());
         if( args[i] == "--nodelaycorr" )
            fFlags.fNoDelayCorrection = true;
         if( args[i] == "--anasettings" ) 
            json=args[i+1];
         if( args[i] == "--usetimerange" )
            {
               fFlags.fTimeCut=true;
               i++;
               fFlags.start_time=atof(args[i].c_str());
               i++;
               fFlags.stop_time=atof(args[i].c_str());
               printf("Using time range for reconstruction: ");
               printf("%f - %fs\n",fFlags.start_time,fFlags.stop_time);
            }
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
