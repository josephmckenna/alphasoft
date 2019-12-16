//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "A2Flow.h"
#include "TSISChannels.h"
#include "TSISEvent.h"

#include "AnalysisTimer.h"
#include <iostream>
class ColdDumpTemperatureFitModuleFlags
{
public:
   bool fPrint = false;

};
class ColdDumpTemperatureFitModule: public TARunObject
{
private:
   std::deque<A2Spill*> ColdDumps;
   
   int possible_channels[5];

  // this is an array of, rather, transfer times
  std::vector<Double_t> AllSISTimes; //?? What is this used for?

  std::vector<double> DumpTimes;
  std::vector<int>    DumpCounts;
  
  std::vector<double> BackgroundTimes;
  std::vector<int>    BackgroundCounts;

  std::vector<double> Psi_Final;
  std::vector<double> F_esc;
  double ElectrodeMap[27][1001];
public:
   ColdDumpTemperatureFitModuleFlags* fFlags;
   bool fTrace = false;

   void loadPsiTable() 
   {
      // loads Psi Table to memory 
      std::ifstream file ( "tempFitterPsiTable.csv"); // declare file stream: http://www.cplusplus.com/reference/iostream/ifstream/
      std::string value;
      getline(file, value, '\n');   // Skip the first line

      while (file.good())
      {
         for (int i = 0; i < 3; i++)
         {
            if (i<2)
               getline(file, value,','); // read a string until next comma: http://www.cplusplus.com/reference/string/getline/
            else
               getline(file, value, '\n'); // read a string until next line
            // PsiTable[i].push_back(atof(value.c_str()));
            if (i == 1)
               Psi_Final.push_back(atof(value.c_str()));
            else if (i == 2)
               F_esc.push_back(exp10(atof(value.c_str())));

            // cout<<PsiTable[i].back()<<"\t";
            // if (i==2)
            // std::cout<<std::endl;
         }
         //cout << string( value, 1, value.length()-2 ); // display value removing the first and the last character from it
      }
   }

   void loadElectrodeMaps()
   {
      // loads Psi Table to memory 
      for (int i = 0; i < 27; i++)
      {
         char filename[200];
         sprintf("%s/a2lib/electrodeMaps/electrodeMap_%d.csv",getenv("AGRELEASE"),i);
         std::ifstream file(filename);
         std::string value;
         for (int j = 0; j < 1001; j++)
         {
            if (j < 1000)
            {
               getline(file, value,','); // read a string until next comma: http://www.cplusplus.com/reference/string/getline/
            }
            else
            {
               getline(file, value, '\n'); // read a string until next line
               printf("%d, %d, ", i, j);
               std::cout<<atof(value.c_str())<<"\n";
            }
            // IGNORE 
            if ((atof(value.c_str()) > 10.0) || (atof(value.c_str()) < 1.E-6))
            {
               printf("BAD! ");
               std::cout<<atof(value.c_str())<<"\n";
               continue;
            }
            ElectrodeMap[i][j] = atof(value.c_str());

            /*cout<<atof(value.c_str())<<"\t";
            if (i==2)
               std::cout<<std::endl;*/
         }
         //cout << string( value, 1, value.length()-2 ); // display value removing the first and the last character from it
      }
   } 
   ColdDumpTemperatureFitModule(TARunInfo* runinfo, ColdDumpTemperatureFitModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::ctor!\n");
      // for fit!
      loadElectrodeMaps();
      loadPsiTable();
   }

   ~ColdDumpTemperatureFitModule()
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      possible_channels[0]=42; 
      possible_channels[1]=SISChannels->GetChannel("SIS_PMT_ATOM_OR");
      possible_channels[2]=SISChannels->GetChannel("SIS_PMT_ATOM_AND");
      possible_channels[3]=SISChannels->GetChannel("SIS_PMT_CATCH_OR");
      possible_channels[4]=SISChannels->GetChannel("SIS_PMT_CATCH_AND");
      
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif
      A2SpillFlow* SpillFlow= flow->Find<A2SpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            A2Spill* s=SpillFlow->spill_events.at(i);
            //s->Print();
            if (!s->SeqData) continue;
            int thisSeq=s->SeqData->SequenceNum;
            if (thisSeq==0) //Catching trap
            //if (strcmp(s->SeqName.c_str(),"cat")==0)
            if (strcmp(s->Name.c_str(),"\"Cold Dump\"")==0)
            {
               A2Spill* ColdDump=new A2Spill(s);
               A2ScalerData* sc=ColdDump->ScalerData;
               //Find which channels had the most counts in... 
               
               int highest_channel = -1;
               int highest_counts = 0;
  
               for (int i = 0; i < 5; i++)
               {
                  int ch = possible_channels[i];
                  int ch_counts = sc->DetectorCounts[ch];
                  printf("channel %d has %d counts\n", ch, ch_counts);
                  if (ch_counts > highest_counts)
                  {
                     highest_counts = ch_counts;
                     highest_channel = ch;
                  }
               }
               if (highest_counts == 0)
                  printf("none of the channels seem to work!");
                  
               //Fill array of doubles#
               
               double tmin=sc->StartTime;
               double tmax=sc->StopTime;
               int size=SpillFlow->SIS_Events[highest_channel].size();
               for (int i=0; i<size; i++)
               {
                  double run_time=SpillFlow->SIS_Events[highest_channel][i].t;
                  if (run_time <= tmin) continue; //Data too early... skip
                  if (run_time <= tmax) { //Is inside the dump
                     // printf("%f,%d\n", det_event->GetRunTime(),det_event->GetCountsInChannel());         
                     DumpTimes.push_back(run_time);
                     DumpCounts.push_back(SpillFlow->SIS_Events[highest_channel][i].counts);
                  }
                  else //I am after the dump... so I am background data (run_time>tmax)
                  {
                     BackgroundTimes.push_back(run_time);
                     BackgroundCounts.push_back(SpillFlow->SIS_Events[highest_channel][i].counts);
                  }
               }

               //Get All times (use clock channel)
               size=SpillFlow->SIS_Events[0].size();
               for (int i=0; i<size; i++)
               {
                  double run_time=SpillFlow->SIS_Events[highest_channel][i].t;
                  if(run_time <= tmin)  continue; //Data too early ... SKIP!                                                                                        
                  if(run_time > tmax) break; 
                  AllSISTimes.push_back(run_time);
               }
               std::cout<<"I have "<<DumpTimes.size()<<" sis bins for temperature fit, and "<<BackgroundTimes.size()<<" bins for background fitting"<<std::endl;
               //ColdDumps.push_back(ColdDump);
               
               
               
    
  //Add some processing here!
  // This loads counts and edge times into semi-pre-processed arrays
  // un-needed, if zero counts were included, ... which will be in 2020!
  // BinEdgetimes: n+1 elements. start with inferred starttime of bin 0
  // at each bin, we add:
  //   binCount to binCounts array
  //   inferred end of bin time 
  std::vector<double> BinEdgetimes;
  std::vector<int> BinCounts;
  
  Double_t prev_bin_endtime;
  Double_t bin_starttime;
  Double_t bin_endtime; 
  Int_t bin_count;
  
  
  // first bin's edgetime
  prev_bin_endtime = (AllSISTimes.at(0) + AllSISTimes.at(1)) / 2;
  BinEdgetimes.push_back(prev_bin_endtime); 
  
  int j = 0; // counter for DumpTimes
  
  for (int i = 1; i < AllSISTimes.size() - 1; i++) { 
    if (AllSISTimes.at(i) < DumpTimes.at(j)) // it's a 0 bin event -- continue!
      continue; 
    else if (AllSISTimes.at(i) == DumpTimes.at(j)) {
      // printf("Match found! %f, %f\n", AllSISTimes.at(i) , DumpTimes.at(j));
      // Match found!
      
      bin_starttime = (AllSISTimes.at(i-1) + AllSISTimes.at(i)) / 2; 
      bin_endtime = (AllSISTimes.at(i) + AllSISTimes.at(i+1)) / 2; 
      bin_count = DumpCounts.at(j);
      
      if (bin_starttime == prev_bin_endtime) {
        // no zero bins in between 
        BinEdgetimes.push_back(bin_endtime);
        BinCounts.push_back(bin_count); 
        } else {
        // there WAS a zero bin!!! 
        // end of this zero bin is the start of current bin
        BinEdgetimes.push_back(bin_starttime);
        BinCounts.push_back(0);
        // and again, push non-zero bin into vectors
        BinEdgetimes.push_back(bin_endtime);
        BinCounts.push_back(bin_count); 
      }
            
      prev_bin_endtime = bin_endtime;
      
      // increment the next non-zero bin to Match for
      j++;
      
      if (j == DumpTimes.size()) { // end of days!!
        // printf("end of days!\n");
        if (!(bin_endtime == tmax)) {
          // add one more zero bin
          BinEdgetimes.push_back(tmax);
          BinCounts.push_back(0);
        }
        break;
      }
    } else // AllSISTimes.at(i) > DumpTimes.at(j) // PROCESS IS BROKEN ...
      throw std::invalid_argument("ERROR! ERROR! ERROR!!! ABORT!\n");
  }
  
  
  // get lambda_noise (units: counts per second)
  int total_background_counts = 0;
  for (int i = 0; i < BackgroundCounts.size(); i++) {
    total_background_counts = total_background_counts + BackgroundCounts[i];
  }

  // for weird statistical purposes ... 
  total_background_counts--;
  double lambda_noise = ((Double_t) total_background_counts) / (BackgroundTimes.back() - tmax);
  printf("%d counts in %f seconds to get lambda_noise: %f counts per second\n", 
    total_background_counts, (BackgroundTimes.back() - tmax), lambda_noise);




  // LOOPING THE RAMP HAPPENS here!!

  // Spline the ramp file
  // string dumpFiles[] = {"dumpfiles/ColdDumpE4E5.dump", "dumpfiles/ColdDump_E5E6_500ms_withOffsets_20141105.dat", "dumpfiles/ColdDump_E11_500ms_20141105.dat", "dumpfiles/cold_dump_E09E10_clear_positrons_1.2_mixE11_E13-E14.dumpfile", "dumpfiles/ColdDumpHalf_C3C4.dat", "dumpfiles/ColdDumpE4E5.dump", "dumpfiles/pbar_dump_E13E14.dat", "dumpfiles/ARTrappingv1.1_ColdDump_pre-mixing_20160713.dat", "dumpfiles/LifetimeFinalColdDumpRightFromE14", "dumpfiles/AT_pbar_cold_dump_E14E15", "dumpfiles/Pre-mix_SlowDump_2s_20180505.dat"};
  
  
  // TODO here: an attempt to reverse engineer the ramp!!
  // loads electrode maps!
  loadElectrodeMaps(); 



TSeq_State* seqState=GetStateAfterDump(runNumber, description, repetition, offset);
  
  //How to clean up array:
  std::vector<double> init=seqState->GetAnalogueOut()->AOi;
  std::vector<double> fin=seqState->GetAnalogueOut()->AOf;
  for ( int i=0; i< init.size(); i++)
     std::cout<<init.at(i) <<"\t"<<fin.at(i)<<std::endl;
     
  std::vector<double> electrodeVoltagesInit;
  std::vector<double> electrodeVoltagesFinal;
  if (seqState->GetSeqNum()==ATOM)
  for (int i=0; i<7; i++)
  {
     electrodeVoltagesInit.push_back(init[0]);
     electrodeVoltagesFinal.push_back(fin[0]);
  }
  for (int i=0; i<init.size(); i++)
  {
     electrodeVoltagesInit.push_back(init[i]);
     electrodeVoltagesFinal.push_back(fin[i]);
  }
  if (seqState->GetSeqNum()==RECATCH)
  for (int i=0; i<20; i++)
  {
     electrodeVoltagesInit.push_back(init[init.size()-1]);
     electrodeVoltagesFinal.push_back(fin[fin.size()-1]);
  }
/*
  double electrodeVoltagesInit[] = {0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, // E1 to E7
  0.0021, -60.0012, -21.0007, -20.0009, -50.0006, 0.0011, 0.0011, 0.0011, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021}; // E8 to E27, hard coded 
  // (last four entries for this particular XML are pbar-mix ekick (soft ekick) (channel 0), and nullx3)
  
  double electrodeVoltagesFinal[] = {0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, // E1 to E7
  0.0021, -60.0012, -21.0007, -20.0009, -18.9989, 0.0011, 0.0011, 0.0011, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021, 0.0021}; // E8 to E27, hard coded
  // as you can see, for this HARD CODED RUN, it is only E12 that changes 
  
  */
  double dz = 0.51463E-3; //
  double zs[1001];
  for (j = 0; j < 1001; j++) {
    zs[j] = j*dz;
  }
  
  
  int N_z = 1001; // 1001 splice points to consider
  
  double t_ramp = 0.5; // the ramp takes half a second exactly
  int N_time_points = 1000; // 1000 different time points for spline (.5 ms)
  double ts[N_time_points+1]; 
  
  for (int n = 0; n < N_time_points+1; n++) {
    ts[n] = n*(t_ramp / N_time_points);
  }

  double voltages_ramp[1001][N_time_points+1];
  
  for (int j = 0; j < 1001; j++) {
    for (int n = 0; n < N_time_points+1; n++) {
      voltages_ramp[j][n] = 0;
    }
  }

  for (int n = 0; n < N_time_points+1; n++) {
    // a linear time interpolation of electrode_i and electrode_f is electrode_curr
    double electrode_curr[27];
    double lambda_ramp = ((double) n) / N_time_points;

    // a linear combination of laplace solutions is computed from electrode_curr
    double V_curr[1001];

    for (int i = 0; i < 27; i++) {
      electrode_curr[i] = electrodeVoltagesInit[i]*(1. - lambda_ramp)
        + electrodeVoltagesFinal[i]*lambda_ramp;

      for (int j = 0; j < 1001; j++) {
        // j demarcates space (z)
        // n demarcates time (t)
        // i is eletrode index
        V_curr[j] = electrode_curr[i]*ElectrodeMap[i][j];
        
        voltages_ramp[j][n] = voltages_ramp[j][n] + V_curr[j];
        // electrode_curr[i]*ElectrodeMap[i][j];
      }
    /*
    TGraph *gr1 = new TGraph (1001, zs, V_curr);
    gr1->Draw();
    gPad->WaitPrimitive();
    // gPad->SetLogy(1);*/
    }
  }


 

  // double barrier_ramp[1001]; 
  
  
  for (int n = 0; n < N_time_points+1; n++) {
    double V_curr[1001];
    for (int j = 0; j < 1001; j++) {
      V_curr[j] = voltages_ramp[j][n];
    }
    /*
    if ((n == 0) || (n == N_time_points - 1)) {
      TGraph *gr1 = new TGraph (1001, zs, V_curr);
      gr1->Draw();
      // this here computes the barrier voltage ramp of a single time point
      // barrier_ramp[n] = ComputeBarrierVoltage(n, V_curr, j_min, j_max);
      gPad->WaitPrimitive();
    }*/
  }
  
  
 
  // implemented here, is Joe's idea as to how to find where-to-consider
  // (as to ignore where plasma is NOT trapped)
  double d_voltage_ramp[1001]; // change in V(z) from t = 0 to t = t_f
  for (int j = 0; j < 1001; j++) {
    d_voltage_ramp[j] = voltages_ramp[j][N_time_points-1] -
                        voltages_ramp[j][0];
          
    printf("%d, %f, %f, %f\n", j, d_voltage_ramp[j], voltages_ramp[j][N_time_points-1], voltages_ramp[j][0]);
  }
  


  double d_voltage_threshold = 1.0; // in volts
  // we won't consider regions of the trap where |V(z, tfin) - V(z, 0)| < d_V_threshold
  int j_min = 0;
  int j_max = 1001;
  for (int j = 0; j < 1000; j++) {
    if ((d_voltage_ramp[j] > d_voltage_threshold) ||
        (d_voltage_ramp[j] < -d_voltage_threshold)) {
      // over threshold!
      j_min = j;
      break;
    }
  }

  for (int j = 999; j >= 0; j--) {
    if ((d_voltage_ramp[j] > d_voltage_threshold) ||
        (d_voltage_ramp[j] < -d_voltage_threshold)) {
      // over threshold!
      j_max = j;
      break;
    }
  }
  
  printf("z minimum: %f\n\n", j_min*dz);
  printf("z maximum: %f\n\n", j_max*dz);
  
  /*TGraph *gr1 = new TGraph (1001, zs, d_voltage_ramp);
  gr1->Draw();
  gPad->WaitPrimitive();*/
  
  double x_min_guess = 0;
  // this computes the ramp!

  
  
  double barrier_ramp[N_time_points+1];
  for (int i = 0; i < N_time_points+1; i++) {
    barrier_ramp[i] = 0;
  }
  
  
  for (int n = 0; n < N_time_points+1; n++) {
    double V_curr[1001];
        
    for (int j = 0; j < 1001; j++) {
      // j demarcates space (z)
      // n demarcates time (t)
      V_curr[j] = voltages_ramp[j][n]; 
    }
    
    /*TSpline5* Vz = new TSpline5("interpolated function", zs, V_curr, 1001); 
    double dVdz_ = [](Double_t z) {return Vz->Derivative(z);};
    // zsVs->Derivative()
    
    TF1 *dVdz = new TF1("fa1", dVdz_, 0, 1000*dz); 
    // dz*(j_min - 0.5)*dz, dz*(j_max + 0.5));*/
    
    
    // if (n == 0) {
    // initialize the guess!!
    // V_curr
    // find zeros manually ... good god!
    std::vector<int> zeros;
    std::vector<int> maxima;
    std::vector<int> minima;
    // HARD CODED
    for (int j = j_min; j < j_max; j++) {
      if ((V_curr[j] > V_curr[j-1]) && (V_curr[j] > V_curr[j+1])) {
        zeros.push_back(j);
        maxima.push_back(j);
        printf("Maximum! at %f, value %f, %f, %f\n", j*dz, V_curr[j-1], V_curr[j], V_curr[j+1]);
      } else if ((V_curr[j] < V_curr[j-1]) && (V_curr[j] < V_curr[j+1])) {
        zeros.push_back(j);
        minima.push_back(j);
        printf("Minimum! at %f, value %f\n", j*dz, V_curr[j]);
        // printf("Minimum! at %f, value %f, %f, %f\n", j*dz, V_curr[j-1], V_curr[j], V_curr[j+1]);
      }
    }
      
      // we want candidate minima!
      
    //}
    
    // hacked ... really, there should be TWO
    if ((maxima.size() == 1) && (minima.size() == 1)) {
      
      double barrier_l = V_curr[maxima[0]] - V_curr[minima[0]];
      // double barrier_r = V_curr[maxima[0]] - V_curr[minima[1]]; 
      barrier_ramp[n] = barrier_l; // (((barrier_l) < (barrier_r)) ? (barrier_l) : (barrier_r));
      printf("at time %d, ramp %f\n", n, barrier_ramp[n]);
    }
    else {
      break;
    }
  }
  
  TGraph *gr1 = new TGraph (1000, ts, barrier_ramp);
  gr1->Draw();
  // this here computes the barrier voltage ramp of a single time point
  // barrier_ramp[n] = ComputeBarrierVoltage(n, V_curr, j_min, j_max);
  // gPad->WaitPrimitive();
  
  // return;

  
  
  
  // we, for now, only have a single dump file
  /*string dumpFiles[] = {"dumpfiles/cold_dump_E09E10_clear_positrons_1.2_mixE11_E13-E14.dumpfile"};
  
  
  std::vector<double> best_fit_score;
  std::vector<int> num_bins;
  std::vector<double> inferred_temperature;
  
  for (int i = 0; i < sizeof(dumpFiles)/sizeof(dumpFiles[0]); i++) {
    // printf(dumpFiles[i]);
    cout << "value of text: " << dumpFiles[i] << endl;
    
    
    char dumpFile[dumpFiles[i].size() + 1]; 
    strcpy(dumpFile, dumpFiles[i].c_str());
      
    //dumpFile[dumpFiles[i].length() + 1]; 
    // dumpFiles[i]
    // char* dumpFile = dumpFiles[i][];
    
    // const char* dumpFile="dumpfiles/cold_dump_E09E10_clear_positrons_1.2_mixE11_E13-E14.dumpfile";

    
    TSpline5* dumpRamp = InterpolateVoltageRamp(dumpFile);*/
    
  TSpline5* dumpRamp = new TSpline5("our spline!", ts, barrier_ramp, N_time_points);
  
    if(!dumpRamp){Error("PlotEnergyDump","NO voltage ramp function"); return 0;}
    
    // energy (temperature) histogram
    Double_t RampTmin=dumpRamp->GetXmin();
    Double_t RampTmax=dumpRamp->GetXmax();
    
    printf("ramp min and max are: %f, %f\n", RampTmin, RampTmax);
    printf("tmin and tmax are: %f, %f\n", tmin, tmax);

    if ( RampTmax<0.) {
      Error("PlotEnergyDump","Ramp invalid? Can't work out how long it was"); 
      return 0; 
    }
    
    Double_t Emin = dumpRamp->Eval(RampTmax);
    Double_t Emax = dumpRamp->Eval(RampTmin);
    printf("Emin and Emax are: %f, %f\n", Emin, Emax);
      

    // NEXT: more preprocessing
    // we are going to care only about bins whose:
    // binEdgeEnergy_high > Eplot_min
    // binEdgeEnergy_low < Eplot_max * Eplot_max_ratio
    // Eplot_max = last "interesting bin", with counts >= Eplot_max_counts

    Double_t Eplot_min = 0.001; // fixed
    Double_t Eplot_max; 
    int Eplot_max_counts = 5;
    Double_t Eplot_max_ratio = 2.;

    // computes Eplot_max  
    for (Int_t i = 0; i < BinCounts.size(); i++) {
      Double_t BinCentertime = (BinEdgetimes[i]+BinEdgetimes[i+1])/2;
      Double_t BinEnergy = dumpRamp->Eval(BinCentertime - tfromramp);
      if ((BinEnergy > Eplot_max) && (BinCounts[i] > Eplot_max_counts))
        Eplot_max = BinEnergy; 
    }
      
      
    /* 
     * finishes the preprocessing
     * 
     * loads stuff into global variables, 
     * only if in Eplot_min/Eplot_max range as defined above: 
     *   BinEdgeEnergies -- the energies at bins' edges (n+1 values)
     *   BinCenterEnergies -- the energies at bins' centers (n values)
     *   BinCenterCounts -- the counts at bins' centers (n values)
     *   BinDtimes -- the sample time for bin (used to compute expected noise counts)
     *   BinCenterLnGamma -- = log(factorial(N_i)), term in log likelihood calculation
     * 
     */
    BinEdgeEnergies.clear();
    BinCenterEnergies.clear();
    BinCenterCounts.clear();
    BinDtimes.clear();
    BinCenterLnGamma.clear();


    for (Int_t i = BinCounts.size() - 1; i >= 0; i--) {
      if (dumpRamp->Eval(BinEdgetimes[i+1] - tfromramp) < Eplot_min)
        continue; // skip bins with binEdgeEnergy_low < Eplot_min
      
      
      Double_t BinCentertime = (BinEdgetimes[i]+BinEdgetimes[i+1])/2;
      Double_t BinEnergy = dumpRamp->Eval(BinCentertime - tfromramp);
      Double_t BinDtime = TMath::Abs(BinEdgetimes[i+1] - BinEdgetimes[i]);

      BinEdgeEnergies.push_back(dumpRamp->Eval(BinEdgetimes[i+1] - tfromramp));
      BinCenterEnergies.push_back(BinEnergy);
      BinCenterCounts.push_back(BinCounts[i]);
      BinDtimes.push_back(BinDtime);
      BinCenterLnGamma.push_back(TMath::LnGamma(1 + BinCounts[i]));
      
      printf("Energy Edge: %f, Energy Center: %f, Counts: %d\n", 
          dumpRamp->Eval(BinEdgetimes[i+1] - tfromramp),
          BinEnergy, BinCounts[i]);
          
      if (BinEnergy > Eplot_max_ratio*Eplot_max) // last one!
        break; 
    }
    
    BinEdgeEnergies.push_back(Eplot_max_ratio*Eplot_max);
    printf("maximum plotted energy: %f\n", Eplot_max_ratio*Eplot_max);
    
    /* this has to do with competing ramp files
     * // CONTINUE WITHOUT DOING FIT
    if (BinCenterCounts.size() < 10) {
      best_fit_score.push_back(0);
      num_bins.push_back(BinCenterCounts.size());
      inferred_temperature.push_back(0);
      continue;
    }*/
    
    auto minuitFunction = [](int& nDim, double* gout, double& result, double par[], int flg) {
      result = negLogLikelihood(par);
    };
    
    // TVirtualFitter::SetDefaultFitter("Minuit2"); 
    
    // TODO have TFitter Minuit2 working!
    // TVirtualFitter * minimizer = TVirtualFitter::Fitter(0,2); 
    //TVirtualFitter* = new TVirtualFitter(2);
    //TVirtualFitter->SetDefaultFitter
    // TFitterMinuit* minimizer = new TFitterMinuit(0, 2);
    TFitter* minimizer = new TFitter(2);
    
    minimizer->SetFCN(minuitFunction);

    minimizer->SetParameter(0,"T", 95, 10, 1.0, 20000);
    minimizer->SetParameter(1,"l_p", 0.18E-2,1.0E-3,1.0E-6, 2.E-2); // 
    minimizer->SetParameter(2,"N_D", 1.311, 0.2,0.1, 15);
    minimizer->SetParameter(3,"RwRp", 170.4, 2, 2.2, 2200);

    minimizer->ExecuteCommand("SIMPLEX",0,0);
    

    minimizer->ExecuteCommand("MIGRAD",0,0);

    printf("\n\n\n\n");
    

    double par[4];
    
    for (int i = 0; i < 4; i++) {
      par[i] = minimizer->GetParameter(i);
      printf("best param %d: %f\n", i, minimizer->GetParameter(i));
    }
    
    /*
     * to do with competing ramp files
    best_fit_score.push_back(negLogLikelihood(par));
    num_bins.push_back(BinCenterCounts.size());
    inferred_temperature.push_back(par[0]);*/
    
    
    
    
   
    // FINALLY: PLOT EVERYTHING OUT!
    
    // temporary array as to fit into hh bin category 
    Double_t BinEdgeEnergies_[BinEdgeEnergies.size()];
    std::copy(BinEdgeEnergies.begin(), BinEdgeEnergies.end(), BinEdgeEnergies_);
    // TH1D* global variable definied above

    hh = new TH1D("pbar_temperature",
      "Hist with variable bin width",
      BinCenterCounts.size(),
      BinEdgeEnergies_
    );
    
    // fill in the bins!
    for (Int_t i = 0; i < BinCenterCounts.size(); i++) {
      printf("Energy: %f, Counts: %d\n", hh->GetBinCenter(i+1), BinCenterCounts[i]);
      // hh->Fill(hh->GetBinCenter(i), BinCounts[i]);
      hh->SetBinContent(i+1, BinCenterCounts[i]);
      hh->SetBinError(i+1, sqrt(hh->GetBinContent(i+1)));
    }
    
    // hh->GetXaxis()->SetRangeUser(-Eplot_max_ratio*Eplot_max, 0.);  
    hh->SetMarkerColor(kRed);
    hh->SetMarkerStyle(7);
    hh->SetLineColor(kBlack);
    
    // this plot FIT  
    
    std::vector<double> exp_BinCenterCounts = expectedCounts(par);

    Double_t x[BinCenterEnergies.size()];
    std::copy(BinCenterEnergies.begin(), BinCenterEnergies.end(), x);
    
    Double_t y[BinCenterEnergies.size()];
    std::copy(exp_BinCenterCounts.begin(), exp_BinCenterCounts.end(), y);

    for (int i = 0; i < BinCenterEnergies.size() - 4; i++) {
      printf("%f, %f \n", x[i], y[i]);
    }

    //gr = new TGraph(BinCenterEnergies.size() - 4,BinCenterEnergies.data(),y);
    gr = new TGraph(BinCenterEnergies.size() - 4,x,y);
    gr->SetLineColor(4);
    gr->Draw("AC SAME");

    // hh->Scale(1, "width");
    
    hh->Draw("E1 HIST SAME"); // hh->Draw("HIST L SAME");  
    gPad->SetLogy(1);
    
    //gr->SetTitle(dumpFile);

    
    // gPad->WaitPrimitive();
  // }
  
  /*
  printf("\n\n\n\n");
  for (int i = 0; i < best_fit_score.size(); i++) {
    cout << "at: " << dumpFiles[i] << endl;
    printf("T: %f, %f, %d\n", inferred_temperature[i], best_fit_score[i], num_bins[i]);
  }
  */
  return flow;
            }
            
         }
      }

      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"catch_efficiency_module",timer_start);
      #endif
      return flow; 
  }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class ColdDumpTemperatureFitModuleFactory: public TAFactory
{
public:
   ColdDumpTemperatureFitModuleFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("ColdDumpTemperatureFitModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("ColdDumpTemperatureFitModuleFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("ColdDumpTemperatureFitModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new ColdDumpTemperatureFitModule(runinfo, &fFlags);
   }
};

static TARegister tar(new ColdDumpTemperatureFitModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
