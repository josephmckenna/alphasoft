#define MC_BV_TREE_cxx
#include "MC_BV_TREE.h"
#include <TH2.h>
#include <TRandom.h>
#include <TMath.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TStyle.h>
#include <TCanvas.h>

static const Int_t   NBARS=64;
static const Double_t ZTOP = 1.300; ///< Z position of the top SiPM in the BV bars
static const Double_t ZBOT = -1.300; ///< Z position of the bottom SiPM in the BV bars
static const Double_t BV_R = 0.223; ///< Radius of the BV in mm
static const Double_t BV_L = 2.600; ///< Length of the BV in mm
// static const Double_t LAMBDA = 3.800; //< Attenuation length in cm of the EJ-200 (https://eljentechnology.com/products/plastic-scintillators/ej-200-ej-204-ej-208-ej-212)
static const Double_t LAMBDA = 1.300; //< Attenuation length in cm from Germano presentation 16/05/2022 [m]
// static const C_EJ200  = 3.e10/1.58; //< Speed of light in cm/s in the EJ-200 (https://eljentechnology.com/products/plastic-scintillators/ej-200-ej-204-ej-208-ej-212)
// static const Double_t C_EJ200  = 299.792458/1.93; //< According to Gareth (private communication) [mm/ns]
// static const Double_t C_EJ200  = 2.99792458e8/1.93; //< According to Gareth (private communication) [m/s]
static const Double_t C_EJ200  = 2.99792458e8/1.71; //< New value from Germano presentation 16/05/2022 [m/s]
static const Double_t mm_to_m = 1.e-3;
static const Double_t ns_to_s = 1.e-9;
///< Cuts 
static const Double_t ATOP_CUT = 0.81;
static const Double_t ABOT_CUT = 0.81; ///< Allowing asymmetric cut
///< Resolutions
static const Double_t t_res = 0.5*1.e-9; ///< 500 ps = 0.5 ns (time is in seconds)
static const Double_t a_top_res = 1.0;
static const Double_t a_bot_res = 1.0;
///< ############################################################
///< HISTOS AND NUMBERS FOR THE BV BARS
///< ############################################################
void MC_BV_TREE::AnalyzeBVBars() {
   createOutTree();
//_________________________________________________________________________________________

   if (tBVBars == 0) return;

   Long64_t nentries = tBVBars->GetEntriesFast();
   Long64_t nbytes = 0, nb = 0;
   for (Long64_t iev=0; iev<nentries;iev++) //loop on Events
   {
      resetEventVariables();
      Long64_t ientry = LoadBVBarsTree(iev);
      if (ientry < 0) break;
      nb = tBVBars->GetEntry(iev);   nbytes += nb;
      Int_t ndigi = ScintBarDigiMCTruth->GetEntriesFast();
      if(ndigi==0) continue;

      Int_t barFlag[NBARS]={0}; //quante volte ho un hit sulla barra
      Int_t NbarsHit =0;
      Double_t TotEnergy=0;
      Bool_t IN, OUT;
      ///< First loop on the digi -> to fill variables for "bars" (here 1 digi = 1 bar hit per 1 track)
      for (Int_t i=0;i<ndigi;i++)  ///< loop on digis - INIT
      {
         TBSCHit *Scdigi = (TBSCHit*) ScintBarDigiMCTruth->At(i);
         Int_t barN = Scdigi->GetBar();

         ///< Check the entrance and exit of the particles
         double p_in[3];
         double p_out[3];
         Scdigi->GetPos_in(p_in);
         Scdigi->GetPos_out(p_out);
         IN = true; OUT = true;
         if(isnan(Scdigi->GetTimeIn()))    IN    = false;
         if(isnan(Scdigi->GetTimeOut()))   OUT   = false;
         if(!IN && !OUT) continue; ///< Do not consider particles that do not have IN && OUT info
         // if(fabs(Scdigi->GetTrackPDG())==11) continue; ///< Show only electrons
         // if(fabs(Scdigi->GetTrackPDG())==13) continue; ///< Show only muons
         // if(fabs(Scdigi->GetTrackPDG())==211) continue; ///< Show only pions

         for(unsigned int k=0; k<3; k++) {
            p_in[k] *= mm_to_m;    ///< mm -> m
            p_out[k] *= mm_to_m;   ///< mm -> m
         }
         if(barFlag[barN]) { //if barN was already hit
            for (Int_t j=0; j<BarNumber.size(); j++) { ///< Find which one to update
               if(BarNumber.at(j)!=barN) continue;
               ///< Updating the variables in case of multiple tracks in the same bar
               BarNTracks.at(j)++;
               Energy.at(j) += Scdigi->GetEnergy();
               if(IN && OUT) {
                  Path.at(j) += sqrt( pow((p_in[0]-p_out[0]),2.) + pow((p_in[1]-p_out[1]),2.) + pow((p_in[2]-p_out[2]),2.)) ;
                  Zeta.at(j) += (p_in[2]+p_out[2])/2.;
                  Time.at(j) += (Scdigi->GetTimeIn()*ns_to_s+Scdigi->GetTimeOut()*ns_to_s)/2.;
               }
               if(IN && !OUT) {
                  Path.at(j) += 10.*mm_to_m; ///< Cannot calculate path assuming half bar thickness
                  Zeta.at(j) += p_in[2];
                  Time.at(j) += Scdigi->GetTimeIn()*ns_to_s;
               }
               if(!IN && OUT) {
                  Path.at(j) += 10.*mm_to_m; ///< Cannot calculate path assuming half bar thickness
                  Zeta.at(j) += p_out[2];
                  Time.at(j) += Scdigi->GetTimeOut()*ns_to_s;
               }
            }
         } else { ///< this bar has not been registered (NEW BAR HIT)
            BarNumber.push_back(barN);
            BarNTracks.push_back(1);
            Energy.push_back(Scdigi->GetEnergy());
            Phi.push_back(barN*TMath::Pi()/32);
            if(IN && OUT) {
               Path.push_back(sqrt( pow((p_in[0]-p_out[0]),2.) + pow((p_in[1]-p_out[1]),2.) + pow((p_in[2]-p_out[2]),2.)) );
               Zeta.push_back((p_in[2]+p_out[2])/2.);
               Time.push_back((Scdigi->GetTimeIn()*ns_to_s+Scdigi->GetTimeOut()*ns_to_s)/2.);
            }
            if(IN && !OUT) {
               Path.push_back(10.*mm_to_m); ///< Cannot calculate path assuming half bar thickness
               Zeta.push_back(p_in[2]);
               Time.push_back(Scdigi->GetTimeIn()*ns_to_s);
            }
            if(!IN && OUT) {
               Path.push_back(10.*mm_to_m); ///< Cannot calculate path assuming half bar thickness
               Zeta.push_back(p_out[2]);
               Time.push_back(Scdigi->GetTimeOut()*ns_to_s);
            }
         }
         barFlag[barN]+=1;
         if(barFlag[barN]==1) NbarsHit++; //se la barraN è appena stata colpita
      } ///< loop on digis - END
      
      ///< Second loop on the BARS ON (here 1 entry = 1 bar)
      ///< This second loop is needed to correct the Zeta and the Time in case of more than one "DIGI" in the same BAR
      for(Int_t i=0;i<(Int_t)BarNumber.size();i++) 
      {
         if(barFlag[BarNumber.at(i)]==0) {
            std::cout << "ERROR " << std::endl;
            continue; //if bar not hit
         }
         Zeta.at(i)/=barFlag[BarNumber.at(i)]; ///< Dividing for all the tracks
         Time.at(i)/=barFlag[BarNumber.at(i)]; ///< Dividing for all the tracks
      } //end loop on bars

      ///< Variables for the OUT Tree
      event = iev;
      nDigi = ndigi;
      if(NbarsHit>0) fillOutputEvent();

   }//end loop on events
//________________________________________________________________________

   ///< Writing the output TTree
   fileOUT->cd();
   treeMCBV->Write();
   fileOUT->Close();
}


void MC_BV_TREE::fillOutputEvent()
{
   ///< Check data consistency
   Bool_t problem = false;
   if(BarNumber.size()!=BarNTracks.size())   {problem = true;}
   if(BarNumber.size()!=Energy.size())       {problem = true;}
   if(BarNumber.size()!=Path.size())         {problem = true;}
   if(BarNumber.size()!=Time.size())         {problem = true;}
   if(BarNumber.size()!=Zeta.size())         {problem = true;}
   if(BarNumber.size()!=Phi.size())          {problem = true;}

   if(problem) {
      std::cout << "ERROR: variables in fillOutputEvent have differente sizes " << std::endl;
      return;
   }

   Bool_t trigger = applyTrigger(); 

   if(trigger) treeMCBV->Fill();
}

Bool_t MC_BV_TREE::applyTrigger() {
   Bool_t problem = false;
   Double_t d_top, d_bot, t_top, t_bot;
   Double_t a_top, a_bot;
   nBarEnds = BarNumber.size()*2.; ///< At the beginning for each bars, the two ends are ON
   for(unsigned int k=0; k<BarNumber.size(); k++) {
      d_top = ZTOP         - Zeta.at(k);
      d_bot = Zeta.at(k)   - ZBOT;
      if(fabs(d_bot)<1.e-6) d_bot = 0.; ///< Correcting for precision limits (when Zeta.at(k) = -1.3)
      problem = false;
      if(d_top<0.||d_top>(ZTOP-ZBOT)) {problem = true;}
      if(d_bot<0.||d_bot>(ZTOP-ZBOT)) {problem = true;}
      if(problem) {
         std::cout << "ERROR: Z have fishy values) " << std::endl;
      }
      ///< Calculating the energy reaching bar's ends
      a_top = Energy.at(k) * TMath::Exp(-(d_top)/(LAMBDA));
      a_bot = Energy.at(k) * TMath::Exp(-(d_bot)/(LAMBDA));
      a_top  += gRandom->Gaus(0,a_top_res*sqrt(a_top));
      a_bot  += gRandom->Gaus(0,a_bot_res*sqrt(a_bot));
      ///< Calculating the time to reach bar's ends
      t_top = Time.at(k)+d_top/C_EJ200+gRandom->Gaus(0.,t_res);    
      t_bot = Time.at(k)+d_bot/C_EJ200+gRandom->Gaus(0.,t_res);
      ///< Removing bars below threshold (either ATOP or ABOT)
      if(a_top<ATOP_CUT) nBarEnds--;
      if(a_bot<ATOP_CUT) nBarEnds--;
      if(a_top<ATOP_CUT||a_bot<ABOT_CUT) continue;
      ///< Now, the cut has been passed, load all the variables
      bars_id   .push_back(BarNumber.at(k)); 
      bars_ntrks.push_back(BarNTracks.at(k)); 
      bars_edep .push_back(Energy.at(k)); 
      bars_path .push_back(Path.at(k)); 
      bars_z    .push_back(Zeta.at(k)); 
      bars_t    .push_back(Time.at(k)); 
      bars_phi  .push_back(Phi.at(k)); 
      bars_atop.push_back(a_top);
      bars_abot.push_back(a_bot);
      bars_ttop.push_back(t_top);    
      bars_tbot.push_back(t_bot);
   } ///< End of loop over the "hits"

   t_event = (Double_t)event*1.e-3; ///< Time is in seconds => one event each ms [totally arbitrary]
   nBars = (Int_t)bars_id.size();
   ///< Calcolating "couple of bars" variables
   for(unsigned int i=0; i<bars_id.size(); i++) {
      for(unsigned int j=i+1; j<bars_id.size(); j++) {
         Double_t tof=nan(""), dphi=nan(""), dzeta = nan(""), dist=nan("");
         tof = fabs(bars_t.at(i)-bars_t.at(j));
         dphi = fabs(bars_phi.at(i)-bars_phi.at(j));
         if(dphi>TMath::Pi()) dphi = 2*TMath::Pi() - dphi; ///< -PI < dphi < PI
         dzeta = bars_z.at(i)-bars_z.at(j);
         dist = sqrt(pow(dzeta, 2.) + pow(2.*BV_R*TMath::Sin(dphi/2.),2.));
         pairs_tof.push_back(tof);
         pairs_dphi.push_back(dphi);
         pairs_dzeta.push_back(dzeta);
         pairs_dist.push_back(dist);
      }
   }
   MeanSigma(pairs_tof, TOF_MIN, TOF_MAX, TOF_MEAN, TOF_STD);
   MeanSigma(pairs_dphi, DPHI_MIN, DPHI_MAX, DPHI_MEAN, DPHI_STD);
   MeanSigma(pairs_dzeta, DZETA_MIN, DZETA_MAX, DZETA_MEAN, DZETA_STD);
   MeanSigma(pairs_dist, DIST_MIN, DIST_MAX, DIST_MEAN, DIST_STD);
   return true;
}

void MC_BV_TREE::resetEventVariables()
{
      nDigi = 0.;
      nBars = 0.;
      nBarEnds = 0.;
      BarNumber.clear();
      BarNTracks.clear();
      Energy.clear();
      Path.clear();
      Time.clear();
      Zeta.clear();
      Phi.clear();
      bars_id   .clear();
      bars_ntrks.clear();
      bars_edep .clear();
      bars_path .clear();
      bars_z    .clear();
      bars_t    .clear();
      bars_phi  .clear();
      bars_atop .clear();
      bars_abot .clear();
      bars_ttop .clear();
      bars_tbot .clear();
      ///< "Pair of bars" (vectors)
      pairs_tof  .clear();
      pairs_dphi .clear();
      pairs_dzeta.clear();
      pairs_dist .clear();
      TOF_MIN = nan(""); TOF_MAX = nan(""); TOF_MEAN = nan(""); TOF_STD = nan("");
      DPHI_MIN = nan(""); DPHI_MAX = nan(""); DPHI_MEAN = nan(""); DPHI_STD = nan("");
      DIST_MIN = nan(""); DIST_MAX = nan(""); DIST_MEAN = nan(""); DIST_STD = nan("");
      DZETA_MIN = nan(""); DZETA_MAX = nan(""); DZETA_MEAN = nan(""); DZETA_STD = nan("");
}

void MC_BV_TREE::MeanSigma(std::vector<Double_t> v, Double_t& min, Double_t& max, Double_t& mean, Double_t& sigma) {

   if(v.size()==0) return; ///< No vector size
   min = *min_element(v.begin(), v.end());
   max = *max_element(v.begin(), v.end());
   Double_t sum = (Double_t) std::accumulate(v.begin(), v.end(), 0.0);
   mean = sum/(v.size());   
   sigma = 0.; ///< It can be changed (max-min)/2 <?>
   ///< In case of a single element -> return without calculating the sigma
   if(v.size()==1) return; ///< Single vector size ==> sigma = 0.
   ///< In case of more than one element -> calculate sigma
   sum = 0.;
   for(unsigned int i=0; i<v.size(); i++) {
      sum += pow(v.at(i)-mean,2.);
   }
   sigma = sqrt(sum/(v.size()-1));
   return;

}

void MC_BV_TREE::createOutTree() {
   treeMCBV = new TTree("tMCBV","Alpha-g MC Gen tree");

   ///< Event general info
   treeMCBV->Branch("event",   &event,  "event/I");
   treeMCBV->Branch("t_event",   &t_event,  "t_event/D");
   treeMCBV->Branch("pbar",   &pbar,  "pbar/O"); ///< Bool_t
   treeMCBV->Branch("mc",   &mc,  "mc/O"); ///< Bool_t   
   treeMCBV->Branch("nDigi",   &nDigi,  "nDigi/I");
   treeMCBV->Branch("nBars",   &nBars,  "nBars/I");
   treeMCBV->Branch("nBarEnds",   &nBarEnds,  "nBarEnds/I");
   ///< bars info (vectors)
   treeMCBV->Branch("bars_id"   , "std::vector<Int_t>"    , &bars_id );       // bars ID ON
   treeMCBV->Branch("bars_ntrks", "std::vector<Int_t>"    , &bars_ntrks );    // bars number of tracks
   treeMCBV->Branch("bars_edep" , "std::vector<Double_t>" , &bars_edep );     // bars Edep
   treeMCBV->Branch("bars_path" , "std::vector<Double_t>" , &bars_path );     // bars Edep
   treeMCBV->Branch("bars_z"    , "std::vector<Double_t>" , &bars_z );        // bars Z
   treeMCBV->Branch("bars_t"    , "std::vector<Double_t>" , &bars_t );        // bars Time
   treeMCBV->Branch("bars_phi"  , "std::vector<Double_t>" , &bars_phi );      // bars Phi
   treeMCBV->Branch("bars_atop" , "std::vector<Double_t>" , &bars_atop );     // bars ATop
   treeMCBV->Branch("bars_abot" , "std::vector<Double_t>" , &bars_abot );     // bars ABot
   treeMCBV->Branch("bars_ttop" , "std::vector<Double_t>" , &bars_ttop );     // bars tTop
   treeMCBV->Branch("bars_tbot" , "std::vector<Double_t>" , &bars_tbot );     // bars tBot
   ///< "Pair of bars" (vectors)
   treeMCBV->Branch("pairs_tof"  , "std::vector<Double_t>" , &pairs_tof );     // TOF 
   treeMCBV->Branch("pairs_dphi" , "std::vector<Double_t>" , &pairs_dphi );    // Delta Phi
   treeMCBV->Branch("pairs_dzeta", "std::vector<Double_t>" , &pairs_dzeta );   // Delta Zeta
   treeMCBV->Branch("pairs_dist" , "std::vector<Double_t>" , &pairs_dist );    // Distance
   ///< Event specific info 
   treeMCBV->Branch("tof_min" , &TOF_MIN, "TOF_MIN/D");
   treeMCBV->Branch("tof_max" , &TOF_MAX, "TOF_MAX/D");
   treeMCBV->Branch("tof_mean" , &TOF_MEAN, "TOF_MEAN/D");
   treeMCBV->Branch("tof_std" , &TOF_STD, "TOF_STD/D");
   treeMCBV->Branch("dphi_min" , &DPHI_MIN, "DPHI_MIN/D");
   treeMCBV->Branch("dphi_max" , &DPHI_MAX, "DPHI_MAX/D");
   treeMCBV->Branch("dphi_mean" , &DPHI_MEAN, "DPHI_MEAN/D");
   treeMCBV->Branch("dphi_std" , &DPHI_STD, "DPHI_STD/D");
   treeMCBV->Branch("dzeta_min" , &DZETA_MIN, "DZETA_MIN/D");
   treeMCBV->Branch("dzeta_max" , &DZETA_MAX, "DZETA_MAX/D");
   treeMCBV->Branch("dzeta_mean" , &DZETA_MEAN, "DZETA_MEAN/D");
   treeMCBV->Branch("dzeta_std" , &DZETA_STD, "DZETA_STD/D");
   treeMCBV->Branch("dist_min" , &DIST_MIN, "DIST_MIN/D");
   treeMCBV->Branch("dist_max" , &DIST_MAX, "DIST_MAX/D");
   treeMCBV->Branch("dist_mean" , &DIST_MEAN, "DIST_MEAN/D");
   treeMCBV->Branch("dist_std" , &DIST_STD, "DIST_STD/D");
}

