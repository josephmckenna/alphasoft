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
static const Float_t ZTOP = 1300; ///< Z position of the top SiPM in the BV bars
static const Float_t ZBOT = -1300; ///< Z position of the bottom SiPM in the BV bars
static const Float_t BV_R = 223.; ///< Radius of the BV in mm
static const Float_t BV_L = 2600.; ///< Length of the BV in mm
static const Float_t LAMBDA = 3800.; //< Attenuation length in cm of the EJ-200 (https://eljentechnology.com/products/plastic-scintillators/ej-200-ej-204-ej-208-ej-212)
// static const V_EJ200  = 3.e10/1.58; //< Speed of light in cm/s in the EJ-200 (https://eljentechnology.com/products/plastic-scintillators/ej-200-ej-204-ej-208-ej-212)
static const Float_t V_EJ200  = 299.792458/1.93; //< Accordint to Gareth (private communication)

///< ############################################################
///< HISTOS AND NUMBERS FOR THE BV BARS
///< ############################################################
void MC_BV_TREE::AnalyzeBVBars() {
   createOutTree();
//_________________________________________________________________________________________

   if (tBVBars == 0) return;

   Long64_t nentries = tBVBars->GetEntriesFast();
   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) //loop on Events
   {
      resetMCBV();

      Long64_t ientry = LoadBVBarsTree(jentry);
      if (ientry < 0) break;
      nb = tBVBars->GetEntry(jentry);   nbytes += nb;
      Int_t ndigi = ScintBarDigiMCTruth->GetEntriesFast();
      if(ndigi==0) continue;

      Int_t barFlag[NBARS]={0}; //quante volte ho un hit sulla barra
      Int_t NbarsHit =0;
      Float_t TotEnergy=0;
      Bool_t IN, OUT;
      ///< First loop on the digi -> to fill variables for "bars" (here 1 digi = 1 bar hit per 1 track)
      for (Int_t i=0;i<ndigi;i++)  //loop on digi //NO CUTS HERE!!!
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

         
         if(barFlag[barN]) //if barN was already hit
         {
            for (Int_t j = BarNumber.size()-1; j >=0; j--) { ///< Find which one to update
               if(BarNumber.at(j)!=barN) continue;
               ///< Updating the variables in case of multiple tracks in the same bar
               BarNTracks.at(j)++;
               Energy.at(j) += Scdigi->GetEnergy();
               if(IN && OUT) {
                  Path.at(j) += sqrt( pow((p_in[0]-p_out[0]),2.) + pow((p_in[1]-p_out[1]),2.) + pow((p_in[2]-p_out[2]),2.)) ;
                  Zeta.at(j) += (p_in[2]+p_out[2])/2.;
                  Time.at(j) += (Scdigi->GetTimeIn()+Scdigi->GetTimeOut())/2.;
               }
               if(IN && !OUT) {
                  Path.at(j) += 10.; ///< Cannot calculate path assuming half bar thickness
                  Zeta.at(j) += p_in[2];
                  Time.at(j) += Scdigi->GetTimeIn();
               }
               if(!IN && OUT) {
                  Path.at(j) += 10.; ///< Cannot calculate path assuming half bar thickness
                  Zeta.at(j) += p_out[2];
                  Time.at(j) += Scdigi->GetTimeOut();
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
               Time.push_back((Scdigi->GetTimeIn()+Scdigi->GetTimeOut())/2.);
            }
            if(IN && !OUT) {
               Path.push_back(10.); ///< Cannot calculate path assuming half bar thickness
               Zeta.push_back(p_in[2]);
               Time.push_back(Scdigi->GetTimeIn());
               // if(p_in[2]==-1300.) {
               //    std::cout << "_____________ Event ________________ " << event << std::endl;
               //    std::cout << "\tBarNumber " << barN << std::endl;
               //    std::cout << "\tEnergy " << Scdigi->GetEnergy() << std::endl;
               //    std::cout << "sqrt(p_in[0] * p_in[1])^2 " << sqrt(p_in[0]*p_in[0]+p_in[1]*p_in[1]) << std::endl;
               //    std::cout << "PDGCode " <<  Scdigi->GetTrackPDG() << std::endl;
               // }
            }
            if(!IN && OUT) {
               Path.push_back(10.); ///< Cannot calculate path assuming half bar thickness
               Zeta.push_back(p_out[2]);
               Time.push_back(Scdigi->GetTimeOut());
            }
         }
         barFlag[barN]+=1;
         if(barFlag[barN]==1) NbarsHit++; //se la barraN è appena stata colpita
      }//end loop on digi
      

      Float_t TOF=nan("");
      Float_t deltaz=nan("");
      Float_t deltaphi=nan("");
      Float_t distance=nan("");
      

      for(Int_t i=0;i<(Int_t)BarNumber.size();i++) ///< Second loop on the BARS ON (here 1 entry = 1 bar)
      {
         if(barFlag[BarNumber.at(i)]==0) {
            std::cout << "ERROR " << std::endl;
            continue; //if bar not hit
         }
         Zeta.at(i)/=barFlag[BarNumber.at(i)]; ///< Dividing for all the tracks
         Time.at(i)/=barFlag[BarNumber.at(i)]; ///< Dividing for all the tracks

      }//end loop on bars

      ///< Variables for the OUT Tree
      event = jentry;
      nDigi = ndigi;
      nBars = NbarsHit;
      fillVariables();

   }//end loop on events
//________________________________________________________________________

   ///< Writing the output TTree
   fileOUT->cd();
   treeMCBV->Write();
   fileOUT->Close();
}

void MC_BV_TREE::createOutTree() {
   treeMCBV = new TTree("tMCBV","Alpha-g MC Gen tree");

   ///< Event general info
   treeMCBV->Branch("event",   &event,  "event/I");
   treeMCBV->Branch("pbar",   &pbar,  "pbar/O"); ///< Bool_t
   treeMCBV->Branch("mc",   &mc,  "mc/O"); ///< Bool_t   
   treeMCBV->Branch("nDigi",   &nDigi,  "nDigi/I");
   treeMCBV->Branch("nBars",   &nBars,  "nBars/I");
   ///< bars info (vectors)
   treeMCBV->Branch("bars_id"   , "std::vector<Int_t>"   , &BarNumber );// bars ID ON
   treeMCBV->Branch("bars_ntrks", "std::vector<Int_t>"   , &BarNTracks );// bars number of tracks
   treeMCBV->Branch("bars_edep" , "std::vector<Float_t>" , &Energy );   // bars Edep
   treeMCBV->Branch("bars_path" , "std::vector<Float_t>" , &Path );     // bars Edep
   treeMCBV->Branch("bars_z"    , "std::vector<Float_t>" , &Zeta );     // bars Z
   treeMCBV->Branch("bars_t"    , "std::vector<Float_t>" , &Time );     // bars Time
   treeMCBV->Branch("bars_phi"  , "std::vector<Float_t>" , &Phi );      // bars Phi
   treeMCBV->Branch("bars_atop" , "std::vector<Float_t>" , &ATop );     // bars ATop
   treeMCBV->Branch("bars_abot" , "std::vector<Float_t>" , &ABot );     // bars ABot
   treeMCBV->Branch("bars_ttop" , "std::vector<Float_t>" , &tTop );     // bars tTop
   treeMCBV->Branch("bars_tbot" , "std::vector<Float_t>" , &tBot );     // bars tBot
   ///< "Pair of bars" (vectors)
   treeMCBV->Branch("pairs_tof"  , "std::vector<Float_t>" , &TOFs );     // TOF 
   treeMCBV->Branch("pairs_dphi" , "std::vector<Float_t>" , &DPHIs );    // Delta Phi
   treeMCBV->Branch("pairs_dzeta", "std::vector<Float_t>" , &DZETAs );   // Delta Zeta
   treeMCBV->Branch("pairs_dist" , "std::vector<Float_t>" , &DISTs );    // Distance
   ///< Event specific info 
   treeMCBV->Branch("tof_min" , &TOF_MIN, "TOF_MIN/F");
   treeMCBV->Branch("tof_max" , &TOF_MAX, "TOF_MAX/F");
   treeMCBV->Branch("tof_mean" , &TOF_MEAN, "TOF_MEAN/F");
   treeMCBV->Branch("tof_std" , &TOF_STD, "TOF_STD/F");
   treeMCBV->Branch("dphi_min" , &DPHI_MIN, "DPHI_MIN/F");
   treeMCBV->Branch("dphi_max" , &DPHI_MAX, "DPHI_MAX/F");
   treeMCBV->Branch("dphi_mean" , &DPHI_MEAN, "DPHI_MEAN/F");
   treeMCBV->Branch("dphi_std" , &DPHI_STD, "DPHI_STD/F");
   treeMCBV->Branch("dzeta_min" , &DZETA_MIN, "DZETA_MIN/F");
   treeMCBV->Branch("dzeta_max" , &DZETA_MAX, "DZETA_MAX/F");
   treeMCBV->Branch("dzeta_mean" , &DZETA_MEAN, "DZETA_MEAN/F");
   treeMCBV->Branch("dzeta_std" , &DZETA_STD, "DZETA_STD/F");
   treeMCBV->Branch("dist_min" , &DIST_MIN, "DIST_MIN/F");
   treeMCBV->Branch("dist_max" , &DIST_MAX, "DIST_MAX/F");
   treeMCBV->Branch("dist_mean" , &DIST_MEAN, "DIST_MEAN/F");
   treeMCBV->Branch("dist_std" , &DIST_STD, "DIST_STD/F");
}

void MC_BV_TREE::fillVariables()
{
   ///< Check data consistency
   Bool_t problem = false;
   if(BarNumber.size()!=BarNTracks.size()) {problem = true;}
   if(BarNumber.size()!=Energy.size()) {problem = true;}
   if(BarNumber.size()!=Path.size()) {problem = true;}
   if(BarNumber.size()!=Time.size()) {problem = true;}
   if(BarNumber.size()!=Zeta.size()) {problem = true;}
   if(BarNumber.size()!=Phi.size()) {problem = true;}

   if(problem) {
      std::cout << "ERROR: variables in fillVariables have differente sizes " << std::endl;
   }
   ///< Calculating ATop, ABot, tTop, tBot
   problem = false;
   Float_t d_top, d_bot, t_top, t_bot;
   for(unsigned int i=0; i<BarNumber.size(); i++) {
      d_top = ZTOP-Zeta.at(i);
      d_bot = Zeta.at(i) - ZBOT;
      if(d_top<0.||d_top>(ZTOP-ZBOT)) {problem = true;}
      if(d_bot<0.||d_bot>(ZTOP-ZBOT)) {problem = true;}
      ///< Adding the energy arriving at the SiPM (Top and Bot)
      ATop.push_back(Energy.at(i) * TMath::Exp(-(d_top)/(LAMBDA)));
      ABot.push_back(Energy.at(i) * TMath::Exp(-(d_bot)/(LAMBDA)));
      ///< Adding the time of arrival at the SiPM (Top and Bot)
      tTop.push_back(Time.at(i)+d_top/V_EJ200);
      tBot.push_back(Time.at(i)+d_bot/V_EJ200);
   }
   if(problem) {
      std::cout << "ERROR: Z and/or t have fishy values) " << std::endl;
   }
   ///< Calcolating "couple of bars" variables
   problem = false;
   for(unsigned int i=0; i<BarNumber.size(); i++) {
      for(unsigned int j=i+1; j<BarNumber.size(); j++) {
         Float_t tof=nan(""), dphi=nan(""), dzeta = nan(""), dist=nan("");
         tof = fabs(Time.at(i)-Time.at(j));
         dphi = fabs(Phi.at(i)-Phi.at(j));
         if(dphi>TMath::Pi()) dphi = 2*TMath::Pi() - dphi; ///< -PI < dphi < PI
         dzeta = Zeta.at(i)-Zeta.at(j);
         dist = sqrt(pow(dzeta, 2.) + pow(2.*BV_R*TMath::Sin(dphi/2.),2.));
         TOFs.push_back(tof);
         DPHIs.push_back(dphi);
         DZETAs.push_back(dzeta);
         DISTs.push_back(dist);
      }
   }
   MeanSigma(TOFs, TOF_MIN, TOF_MAX, TOF_MEAN, TOF_STD);
   MeanSigma(DPHIs, DPHI_MIN, DPHI_MAX, DPHI_MEAN, DPHI_STD);
   MeanSigma(DZETAs, DZETA_MIN, DZETA_MAX, DZETA_MEAN, DZETA_STD);
   MeanSigma(DISTs, DIST_MIN, DIST_MAX, DIST_MEAN, DIST_STD);


   treeMCBV->Fill();
}

void MC_BV_TREE::resetMCBV()
{
      nDigi = 0.;
      nBars = 0.;
      BarNumber.clear();
      BarNTracks.clear();
      Energy.clear();
      Path.clear();
      Time.clear();
      Zeta.clear();
      Phi.clear();
      ATop.clear();
      ABot.clear();
      tTop.clear();
      tBot.clear();
      TOF_MIN = nan(""); TOF_MAX = nan(""); TOF_MEAN = nan(""); TOF_STD = nan("");
      DPHI_MIN = nan(""); DPHI_MAX = nan(""); DPHI_MEAN = nan(""); DPHI_STD = nan("");
      DIST_MIN = nan(""); DIST_MAX = nan(""); DIST_MEAN = nan(""); DIST_STD = nan("");
      DZETA_MIN = nan(""); DZETA_MAX = nan(""); DZETA_MEAN = nan(""); DZETA_STD = nan("");
      TOFs.clear();
      DPHIs.clear();
      DZETAs.clear();
      DISTs.clear();
}

void MC_BV_TREE::MeanSigma(std::vector<Float_t> v, Float_t& min, Float_t& max, Float_t& mean, Float_t& sigma) {

   if(v.size()==0) return; ///< No vector size
   min = *min_element(v.begin(), v.end());
   max = *max_element(v.begin(), v.end());
   Float_t sum = (Float_t) std::accumulate(v.begin(), v.end(), 0.0);
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
