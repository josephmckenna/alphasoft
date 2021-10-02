#include "PlotDumps.C+"

// important constants 
Double_t eps0 = 8.9E-12; // # in Farads/meter
Double_t joule_to_eV = 6.242E18; // conversion
Double_t kB = 1.38064852E-23; // kB units ... (Joules / Kelvin)
Double_t e = 1.60217646E-19; // coulombs

Double_t pi = 3.14159265358979323846; 

// so that this computation doesn't need to be performed again 
Double_t eps0e = eps0 / e;
Double_t kBe = kB / e;
enum {PBAR,RECATCH,ATOM,POS}; //Copied from alphaAnalysis.cxx

// global, so debugable!  
TH1D* hh; 
TF1* f1;
TGraph *gr;
 
std::vector<double> Psi_Final;
std::vector<double> F_esc;

Double_t lambda_noise;

// ARRAYS OF IMPORTANCE (vectors, really ...)
std::vector<double> BinEdgeEnergies;
std::vector<double> BinCenterEnergies;
std::vector<int> BinCenterCounts;
std::vector<double> BinDtimes;
std::vector<double> BinCenterLnGamma;


// This returns the expected Bin Counts, as a vector of your parameters
std::vector<double> expectedCounts(Double_t *par) {
  int interpSize = Psi_Final.size() - 1;

  // parameters
  Double_t T    = par[0];
  Double_t l_p  = par[1];
  Double_t N_D  = par[2];
  Double_t RwRp = par[3];

  // and then, return spline fit! of T vs. RwRp
  Double_t Vs[interpSize];
  Double_t Ns[interpSize];
  
  for (int i = 0; i < interpSize; i++) {
    int j = interpSize - 1 - i;
    Vs[i] = (T*kBe)*(Psi_Final[j] + 0.5*(N_D*N_D - F_esc[j])*log(RwRp) + 0.25*N_D*N_D);
    Ns[i] = l_p*(T*kBe)*eps0e*pi*F_esc[j];
  }
  
  // TODO: do spline on log counts? 
  
  // something to do with interpolating and then taking a derivative!
  TSpline5* VsNsSpline = 
    new TSpline5("interpolated function", Vs, Ns, interpSize); 
  
  std::vector<double> expectedCounts;
  
  Double_t prevEnergy = BinEdgeEnergies[0];
  
  for (int i = 0; i < BinCenterCounts.size(); i++) { 
    Double_t currEnergy = BinEdgeEnergies[i+1];
    Double_t Dtime = BinDtimes[i];
    
    Double_t exp_counts = 
      TMath::Abs(VsNsSpline->Eval(currEnergy) - VsNsSpline->Eval(prevEnergy))
      + lambda_noise*Dtime;
    
    expectedCounts.push_back(exp_counts);
    
    prevEnergy = currEnergy; 
  }
  
  return expectedCounts;
}


// this here will return the log Likelihood, given your bins ... 
Double_t negLogLikelihood(Double_t *par)
{
  std::vector<double> exp_BinCenterCounts = expectedCounts(par);
  
  
  // now, go through each bin 
  Double_t logLikelihood = 0;
  
  for (int i = 0; i < BinCenterCounts.size(); i++) { 
    Double_t exp_counts = exp_BinCenterCounts[i];
    Double_t obs_counts = BinCenterCounts[i];
        
    logLikelihood = logLikelihood + 
      (obs_counts*log(exp_counts) - exp_counts - BinCenterLnGamma[i]);
  }
  
  
  // add in priors here?
  
  return -logLikelihood;
}

void loadPsiTable() {
  // loads Psi Table to memory 
  ifstream file ( "tempFitterPsiTable.csv"); // declare file stream: http://www.cplusplus.com/reference/iostream/ifstream/
  string value;
  getline(file, value, '\n');   // Skip the first line

  while (file.good()) {
    for (int i = 0; i < 3; i++) {
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

//double ElectrodeMap[27][1001];
std::vector<std::vector<double>> ElectrodeMap;
void loadElectrodeMaps(int SequencerID) {
  int nElectrodes=-1;
  std::string seqName;
  if (SequencerID==ATOM || SequencerID==RECATCH )
  {
     nElectrodes=27;
     seqName="ATOM_TRAP";
  }
  if (SequencerID==PBAR)
  {
     nElectrodes=19;
     seqName="CATCH_TRAP";
  }
  ElectrodeMap.resize(nElectrodes);
  // loads Psi Table to memory 
  for (int i = 0; i < nElectrodes; i++) {
    char path[300];
    sprintf(path,"%s/aux/electrodeMaps/%s/electrodeMap_%d.csv",getenv("RELEASE"),seqName.c_str(),i);
    ifstream file(path);
    string value;
    ElectrodeMap.at(i).resize(1001);
    for (int j = 0; j < 1001; j++) {
      if (j < 1000)
        getline(file, value,','); // read a string until next comma: http://www.cplusplus.com/reference/string/getline/
      else {
        
        getline(file, value, '\n'); // read a string until next line
        printf("%d, %d, ", i, j);
        cout<<atof(value.c_str())<<"\n";
      }
      
      // IGNORE 
      if ((atof(value.c_str()) > 10.0) || (atof(value.c_str()) < 1.E-6)) {
        printf("BAD! ");
        cout<<atof(value.c_str())<<"\n";
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

Double_t GetSaveOfDumpEvent(Int_t runNumber, const char *eventName, const char *description, Int_t repetition, Int_t offset, bool exact_match)
{
  TSeq_Event *seqEvent = FindSequencerEvent(runNumber, eventName, description, repetition, offset, exact_match);
  if (seqEvent == NULL)
  {
    Error("MatchEventToTime", "\033[33mCould not find sequencer event %s (%s) in run %d\033[00m", eventName, description, runNumber);
    return -1;
  }

  Double_t runTime = Get_RunTime_of_SequencerEvent(runNumber, seqEvent, offset);

  delete seqEvent;
  return runTime;
}


TSeq_State* GetStateAfterDump(Int_t runNumber, char* description, Int_t repetition=1, Int_t offset=0, bool exact_match=false)
{


  TSeq_Event *seqEvent = FindSequencerEvent(runNumber, "startDump", description, repetition, offset, exact_match);
  int state=seqEvent->getonState()+1;
  int seq=seqEvent->GetSeqNum();
  std::cout<<"STATE WE ARE LOOKING FOR:"<<state<<std::endl;
  TSeq_State* seqState=new TSeq_State();
  TTree* t=GetSequencerStateTree(runNumber);
  t->SetBranchAddress("SequenceState",&seqState);
  for ( int i=0; i< t->GetEntries(); i++)
  {
    t->GetEntry(i);
    //Must be from same sequence
    if (seqState->GetSeqNum()!=seq) continue;
  seqState->Print();
    //Not at the right state yet:
    if (seqState->GetState()<state) continue;
    //Match found:
    if (seqState->GetState()==state) break;
  }
  return seqState;
  
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
}


void SavePMTData(Int_t runNumber, char* description, Int_t repetition=1, Int_t offset=0, Int_t channel=-1)
{
  // for fit!
  loadPsiTable();


  Int_t ch=channel;
  printf("Channel %d\n\n", ch);
  //Get time range values
  Double_t tmin = MatchEventToTime(runNumber,"startDump",description,repetition,offset);

  // HARD CODED!
  Double_t startOffset = 0.002; // dump starts two milliseconds after the start dump trigger
  Double_t tfromramp = tmin + startOffset;

  Double_t tmax = MatchEventToTime(runNumber,"stopDump",description,repetition,offset);
  //Prepare reading data from root tree
  TSISChannels* sisch = new TSISChannels(runNumber);
  TString hname = sisch->GetDescription(ch,runNumber);
  
  TTree* det_tree = Get_Sis_Tree(runNumber, ch);
  TSisEvent* det_event = new TSisEvent();
  det_tree->SetBranchAddress("SisEvent", &det_event );
  
  
  // chnel(runNumber, tmin, tmax); 
  int ts_chan=0;
  if (ch>32) ts_chan=32;
  TTree* ts_tree = Get_Sis_Tree(runNumber, ts_chan);
  TSisEvent* ts_event = new TSisEvent();
  ts_tree->SetBranchAddress("SisEvent",&ts_event );

  std::vector<Double_t> DumpTimes;
  std::vector<int> DumpCounts;
  
  std::vector<Double_t> BackgroundTimes;
  std::vector<int> BackgroundCounts;

  
  //Do the thing!!!1
  for(Int_t i=0; i < det_tree->GetEntries(); ++i) {
    det_tree->GetEntry(i);
    Double_t run_time = det_event->GetRunTime();
    if (run_time <= tmin) continue; //Data too early... skip
    if (run_time <= tmax) { //Is inside the dump
       // printf("%f,%d\n", det_event->GetRunTime(),det_event->GetCountsInChannel());         
       DumpTimes.push_back(det_event->GetRunTime());
       DumpCounts.push_back(det_event->GetCountsInChannel());
    }
    else //I am after the dump... so I am background data (run_time>tmax)
    {
        BackgroundTimes.push_back(det_event->GetRunTime());
        BackgroundCounts.push_back(det_event->GetCountsInChannel());       
    }
  }
  
  
  // this is an array of, rather, transfer times
  std::vector<Double_t> AllSISTimes;
  for(Int_t i=0; i<ts_tree->GetEntries(); ++i) {
    ts_tree->GetEntry(i);
    Double_t run_time = ts_event->GetRunTime();

    if(run_time <= tmin) 
      continue; //Data too early ... SKIP!                                                                                        
    if(run_time <= tmax) { //Is inside the dump                                                                                                      
      AllSISTimes.push_back(run_time);
    }
  }
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
  lambda_noise = ((Double_t) total_background_counts) / (BackgroundTimes.back() - tmax);
  printf("%d counts in %f seconds to get lambda_noise: %f counts per second\n", 
    total_background_counts, (BackgroundTimes.back() - tmax), lambda_noise);




  // LOOPING THE RAMP HAPPENS here!!

  // Spline the ramp file
  // string dumpFiles[] = {"dumpfiles/ColdDumpE4E5.dump", "dumpfiles/ColdDump_E5E6_500ms_withOffsets_20141105.dat", "dumpfiles/ColdDump_E11_500ms_20141105.dat", "dumpfiles/cold_dump_E09E10_clear_positrons_1.2_mixE11_E13-E14.dumpfile", "dumpfiles/ColdDumpHalf_C3C4.dat", "dumpfiles/ColdDumpE4E5.dump", "dumpfiles/pbar_dump_E13E14.dat", "dumpfiles/ARTrappingv1.1_ColdDump_pre-mixing_20160713.dat", "dumpfiles/LifetimeFinalColdDumpRightFromE14", "dumpfiles/AT_pbar_cold_dump_E14E15", "dumpfiles/Pre-mix_SlowDump_2s_20180505.dat"};
  


TSeq_State* seqState=GetStateAfterDump(runNumber, description, repetition, offset);
  
  
  // TODO here: an attempt to reverse engineer the ramp!!
  // loads electrode maps!
  std::cout<<"Loading maps...."<<std::endl;
  loadElectrodeMaps(seqState->GetSeqNum()); 

std::cout<<"Map loaded"<<std::endl;

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
    double electrode_curr[electrodeVoltagesInit.size()];
    double lambda_ramp = ((double) n) / N_time_points;

    // a linear combination of laplace solutions is computed from electrode_curr
    double V_curr[1001];

    for (int i = 0; i <electrodeVoltagesInit.size(); i++) {
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
  //    printf("Energy: %f, Counts: %d\n", hh->GetBinCenter(i+1), BinCenterCounts[i]);
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

//    for (int i = 0; i < BinCenterEnergies.size() - 4; i++) {
//      printf("%f, %f \n", x[i], y[i]);
//    }

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
  return;
}



/*
// computes the correct channel!
void getChannel(Int_t runNumber, Double_t tmin, Double_t tmax) {
  for (Int_t ch = 0; ch < 64; ch++) {
    TTree* temp_tree = Get_Sis_Tree(runNumber, ch);
    int counts = temp_tree->GetEntries();
    printf("%d, %d, %d\n", ch, counts, Count_SIS_Triggers(runNumber, ch, tmin, tmax));
  }
  return;
}

*/


void SavePMTDataBestChannel(Int_t runNumber, char* description, Int_t repetition=1, Int_t offset=0) {
  // AUTOMATICALLY FINDS THE GOOD CHANNEL, i.e. channel with most counts 
  
  //Get time range values
  Double_t tmin = MatchEventToTime(runNumber,"startDump",description,repetition,offset);
  Double_t tmax = MatchEventToTime(runNumber,"stopDump",description,repetition,offset);
  //Prepare reading data from root tree
  TSISChannels* sisch = new TSISChannels(runNumber);

  int possible_channels[5] = {
    42, 
    sisch->GetChannel("SIS_PMT_ATOM_OR"),
    sisch->GetChannel("SIS_PMT_ATOM_AND"),
    sisch->GetChannel("SIS_PMT_CATCH_OR"), 
    sisch->GetChannel("SIS_PMT_CATCH_AND")
  } ; 
  
  int highest_channel = -1;
  int highest_counts = 0;
  
  for (int i = 0; i < 5; i++) {
    int ch = possible_channels[i];
    int ch_counts = Count_SIS_Triggers(runNumber, ch, tmin, tmax);
    
    printf("channel %d has %d counts\n", ch, ch_counts);
    
    if (ch_counts > highest_counts) {
      highest_counts = ch_counts;
      highest_channel = ch;
    }
  }
    
  if (highest_counts == 0) {
    printf("none of the channels seem to work!");
    return; 
  }

  return SavePMTData(runNumber, description, repetition, offset, highest_channel);    
}

void SavePMTDataAtomStick(Int_t runNumber, char* description, Int_t repetition=1, Int_t offset=0)
{
  return SavePMTData(runNumber, description, repetition, offset, 42);  
}

void SavePMTDataAtomOR(Int_t runNumber, char* description, Int_t repetition=1, Int_t offset=0)
{
  TSISChannels* sisch = new TSISChannels(runNumber);
  return SavePMTData(runNumber, description, repetition, offset, sisch->GetChannel("SIS_PMT_ATOM_OR"));  
}

void SavePMTDataAtomAND(Int_t runNumber, char* description, Int_t repetition=1, Int_t offset=0)
{
  TSISChannels* sisch = new TSISChannels(runNumber);
  return SavePMTData(runNumber, description, repetition, offset, sisch->GetChannel("SIS_PMT_ATOM_AND"));  
}
void SavePMTDataCatchOR(Int_t runNumber, char* description, Int_t repetition=1, Int_t offset=0)
{
  TSISChannels* sisch = new TSISChannels(runNumber);
  return SavePMTData(runNumber, description, repetition, offset, sisch->GetChannel("SIS_PMT_CATCH_OR"));  
}

void SavePMTDataCatchAND(Int_t runNumber, char* description, Int_t repetition=1, Int_t offset=0)
{
  TSISChannels* sisch = new TSISChannels(runNumber);
  return SavePMTData(runNumber, description, repetition, offset, sisch->GetChannel("SIS_PMT_CATCH_AND"));  
}
