#define DATA_BV_TREE_cxx
#include "DATA_BV_TREE.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

static const Double_t speedC=2.99792458e8; //[m/s]
static const Double_t Veff=speedC/1.93;
static const Double_t Radius=0.223; //[m]

void DATA_BV_TREE::Loop()
{  
  CreateOutputTree();
 
  Int_t nNoBarHits=0; 
  Int_t AnomalousAmp=0;

  if (fChain == 0) return;
  Long64_t nentries = fChain->GetEntriesFast();
  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=0; jentry<nentries;jentry++) { ///< Loop on events - INIT
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

      ClearTreeVariables();    
      //cout<< jentry<<endl;
      //cout<<"EventNumber "<<EventNumber<<endl;

      std::vector<TBarHit*> BarHit  = BarEvent->GetBars();

      if(BarHit.size()==0) {
        nNoBarHits++;
        continue;
      }

      t_event = (Double_t)BarEvent->GetRunTime();
      event = jentry;
      pbar = t_event>t_pbars ? true : false;
      mc = false;
      nDigi = 0;
      nBars = BarHit.size();
        
      for(Int_t i=0; i<BarHit.size(); i++) { ///< Loop on the "bar hits" - INIT
        if(BarHit.at(i)->GetBar()>=64) {
	        cout<<"Warning barnumber>64"<<endl;
	        return;
        }	
	      BarNumber.push_back(BarHit.at(i)->GetBar());
	      BarNTracks.push_back(0.);
	      Energy.push_back(0.);
	      Path.push_back(0.);
	      Zeta.push_back((Double_t)BarHit.at(i)->GetTDCZed());
	      Time.push_back((Double_t)(BarHit.at(i)->GetTDCTop()+(Double_t)BarHit.at(i)->GetTDCBot())/2.);
	      Phi.push_back((Double_t)BarHit.at(i)->GetPhi());

	      ATop.push_back((Double_t)BarHit.at(i)->GetAmpTop());
	      ABot.push_back((Double_t)BarHit.at(i)->GetAmpBot());
	      tTop.push_back((Double_t)BarHit.at(i)->GetTDCTop());
	      tBot.push_back((Double_t)BarHit.at(i)->GetTDCBot());
  
	      if(tTop.size() != tBot.size()) {
          std::cout << "ERROR: tTop.size != tBot.size()"<< std::endl;
        }
	    } ///< Loop on the "bar hits" - END

    FillVariables();
  } ///< Loop on events - END

  cout<<"NEvents "<<nentries<<endl;
  cout<<"nNoBarHits "<<nNoBarHits<<endl;
   
  cout<<endl;
  cout<<"AnomalousAmp "<<AnomalousAmp<<endl;
    
  fOut->cd();
  treeDataBV->Write();
  fOut->Close();

   
}


void DATA_BV_TREE::CreateOutputTree()
{
  fOut->cd();
  treeDataBV = new TTree("tDataBV","Alpha-g MC Gen tree");
  ///< Event general info
  treeDataBV->Branch("t_event",  &t_event, "t_event/D");
  treeDataBV->Branch("event",   &event,  "event/I");
  treeDataBV->Branch("pbar",    &pbar,  "pbar/O"); ///< Bool_t
  treeDataBV->Branch("mc",      &mc,  "mc/O"); ///< Bool_t   
  treeDataBV->Branch("nDigi",   &nDigi,  "nDigi/I");
  treeDataBV->Branch("nBars",   &nBars,  "nBars/I");
  ///< bars info (vectors)
  treeDataBV->Branch("bars_id"   , "std::vector<Int_t>"   , &BarNumber );// bars ID ON
  treeDataBV->Branch("bars_ntrks", "std::vector<Int_t>"   , &BarNTracks );// bars number of tracks
  treeDataBV->Branch("bars_edep" , "std::vector<Double_t>" , &Energy     );   // bars Edep
  treeDataBV->Branch("bars_path" , "std::vector<Double_t>" , &Path       );     // bars Edep
  treeDataBV->Branch("bars_z"    , "std::vector<Double_t>" , &Zeta       );     // bars Z
  treeDataBV->Branch("bars_t"    , "std::vector<Double_t>" , &Time       );     // bars Time
  treeDataBV->Branch("bars_phi"  , "std::vector<Double_t>" , &Phi        );      // bars Phi
  treeDataBV->Branch("bars_atop" , "std::vector<Double_t>" , &ATop       );     // bars ATop
  treeDataBV->Branch("bars_abot" , "std::vector<Double_t>" , &ABot       );     // bars ABot
  treeDataBV->Branch("bars_ttop" , "std::vector<Double_t>" , &tTop       );     // bars tTop
  treeDataBV->Branch("bars_tbot" , "std::vector<Double_t>" , &tBot       );     // bars tBot
  ///< "Pair of bars" (vectors)
  treeDataBV->Branch("pairs_tof"  , "std::vector<Double_t>" , &TOFs );     // TOF 
  treeDataBV->Branch("pairs_dphi" , "std::vector<Double_t>" , &DPHIs );    // Delta Phi
  treeDataBV->Branch("pairs_dzeta", "std::vector<Double_t>" , &DZETAs );   // Delta Zeta
  treeDataBV->Branch("pairs_dist" , "std::vector<Double_t>" , &DISTs );    // Distance
  ///< Event specific info 
  treeDataBV->Branch("tof_min" ,    &TOF_MIN   , "TOF_MIN/D");
  treeDataBV->Branch("tof_max" ,    &TOF_MAX   , "TOF_MAX/D");
  treeDataBV->Branch("tof_mean" ,   &TOF_MEAN  , "TOF_MEAN/D");
  treeDataBV->Branch("tof_std" ,    &TOF_STD   , "TOF_STD/D");
  treeDataBV->Branch("dphi_min" ,   &DPHI_MIN  , "DPHI_MIN/D");
  treeDataBV->Branch("dphi_max" ,   &DPHI_MAX  , "DPHI_MAX/D");
  treeDataBV->Branch("dphi_mean" ,  &DPHI_MEAN , "DPHI_MEAN/D");
  treeDataBV->Branch("dphi_std" ,   &DPHI_STD  , "DPHI_STD/D");
  treeDataBV->Branch("dzeta_min" ,  &DZETA_MIN , "DZETA_MIN/D");
  treeDataBV->Branch("dzeta_max" ,  &DZETA_MAX , "DZETA_MAX/D");
  treeDataBV->Branch("dzeta_mean" , &DZETA_MEAN, "DZETA_MEAN/D");
  treeDataBV->Branch("dzeta_std" ,  &DZETA_STD , "DZETA_STD/D");
  treeDataBV->Branch("dist_min" ,   &DIST_MIN  , "DIST_MIN/D");
  treeDataBV->Branch("dist_max" ,   &DIST_MAX  , "DIST_MAX/D");
  treeDataBV->Branch("dist_mean" ,  &DIST_MEAN , "DIST_MEAN/D");
  treeDataBV->Branch("dist_std" ,   &DIST_STD  , "DIST_STD/D");
}



void DATA_BV_TREE::ClearTreeVariables()
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

void DATA_BV_TREE::FillVariables()
{
  ///< Loop on the "bar hits" - INIT
   ///< Check data consistency
   Bool_t problem = false;
   if(BarNumber.size()!=Time.size()) {problem = true;}
   if(BarNumber.size()!=Zeta.size()) {problem = true;}
   if(BarNumber.size()!=Phi.size()) {problem = true;}

   if(problem) {
      std::cout << "ERROR: variables in fillVariables have differente sizes " << std::endl;
   }

   ///< Calcolating "couple of bars" variables
   problem = false;
   for(unsigned int i=0; i<BarNumber.size(); i++) {
      for(unsigned int j=i+1; j<BarNumber.size(); j++) {
        Double_t tof=nan(""), dphi=nan(""), dzeta = nan(""), dist=nan("");
        tof = fabs(Time.at(i)-Time.at(j));
        //  std::cout << "Time.at(i) " << Time.at(i) << " Time.at(j) " << Time.at(j) << " TOF " << tof << std::endl; 
        dphi = fabs(Phi.at(i)-Phi.at(j));
        if(dphi>TMath::Pi()) dphi = 2*TMath::Pi() - dphi; ///< -PI < dphi < PI
        ///< Setting dZeta according to which "hit" comes first.
        ///< This is very important for cosmics data
        if(Time.at(i)<=Time.at(j)) { 
          dzeta = Zeta.at(i)-Zeta.at(j);
        } else {
          dzeta = Zeta.at(j)-Zeta.at(i);
        }
        dist = sqrt(pow(dzeta, 2.) + pow(2.*Radius*TMath::Sin(dphi/2.),2.));
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

   treeDataBV->Fill();
}

void DATA_BV_TREE::MeanSigma(std::vector<Double_t> v, Double_t& min, Double_t& max, Double_t& mean, Double_t& sigma)
{
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
