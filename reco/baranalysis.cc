#include <iostream>
#include <vector>
using std::cout;
using std::endl;
using std::vector;

#include <stdlib.h>

#include "TH1D.h"
#include "TH2D.h"

#include "TMath.h"
#include "TString.h"
#include "TClonesArray.h"
#include "TTree.h"
#include "TFile.h"

#include "TRandom3.h"
TRandom3 rndm;

#include "TLorentzVector.h"
#include "TVector3.h"

#include "TScintDigi.hh"

//double gBarRadius = 230.;
//int gNbars = 64;
//#include "agg4globals.hh"

void rec(TString filename, 
	 int pdgc,
	 double resolution,
	 TH1D* hTOF, TH1D* hBar,
	 TH1D* hDtScint, TH2D* hDtScintVsAngle)
{
  // open the simulation output
  TFile* fData = new TFile(filename.Data(),"READ");
  cout<<fData->GetName()<<endl;

  TTree* tScint = (TTree*) fData->Get("ScintBarsMCdata");
  // acquire the Scintillators hits
  TClonesArray* scintdigi= new TClonesArray("TScintDigi");
  tScint->SetBranchAddress("ScintBarHits",&scintdigi);

  int NofEvents=tScint->GetEntries();

  resolution *= 1.e-3; // ps -> ns

  for(int e=0; e<NofEvents; ++e)
    {
      tScint->GetEntry(e);

      vector<double> dt,dphi;

      for(int i=0; i<scintdigi->GetEntries(); ++i)
	{
	  if( TMath::Abs( ((TScintDigi*) scintdigi->At(i))->GetTrackPDG() ) != pdgc )
	    continue;
	  
	  double t_i = ((TScintDigi*) scintdigi->At(i))->GetTime(),
	    pos_i = ((TScintDigi*) scintdigi->At(i))->GetPos();

	  double smearTi = rndm.Gaus(t_i,resolution);

	  //	  hTOF->Fill( t_i );
	  hTOF->Fill( smearTi );
	  hBar->Fill( pos_i*TMath::RadToDeg() );

	  for(int j=i+1; j<scintdigi->GetEntries(); ++j)
	    {
	      if( TMath::Abs( ((TScintDigi*) scintdigi->At(j))->GetTrackPDG() ) != pdgc )
	      	continue;

	      if( ((TScintDigi*) scintdigi->At(i))->GetBar() != 
		  ((TScintDigi*) scintdigi->At(j))->GetBar() )
		{
		  double t_j = ((TScintDigi*) scintdigi->At(j))->GetTime(),
		    pos_j = ((TScintDigi*) scintdigi->At(j))->GetPos();

		  double smearTj = rndm.Gaus(t_j,resolution);

		  double deltaT = TMath::Abs( smearTi - smearTj ),
		    deltaPhi = TMath::Abs( pos_i - pos_j );
		  if( deltaPhi > TMath::Pi() )
		    deltaPhi = TMath::TwoPi() - deltaPhi;
		  
		  dt.push_back( deltaT );
		  dphi.push_back( deltaPhi );
		  
		}// hits on different bars
	    }// j-loop
	}// i-loop

      //      cout<<"I found "<<dt.size()<<" combinations in event "<<e<<endl;
      double// maxdt = 0., 
	mindt = 99999., 
	deltangle=-1.;
      for( size_t i = 0; i<dt.size(); ++i )
	{
	  double time = dt.at(i);
	  //	  if( time > maxdt )
	  if( time < mindt )
	    {
	      //	      maxdt = time;
	      mindt = time;
	      deltangle = dphi.at(i);
	    }
	  //	  cout<<time<<"\t"<<maxdt<<"\t\t"<<deltangle*TMath::RadToDeg()<<endl;
	}
      
      // if( maxdt == 0. ) continue;
      // hDtScint->Fill(maxdt);
      // hDtScintVsAngle->Fill(maxdt,deltangle*TMath::RadToDeg());
      if( mindt == 99999. ) continue;
      hDtScint->Fill(mindt);
      hDtScintVsAngle->Fill(mindt,deltangle*TMath::RadToDeg());
      //      cout<<"Event "<<e<<"\t\t"<<maxdt<<"\t\t"<<deltangle*TMath::RadToDeg()<<endl;
    }// event loop

}

void rec(TString filename, 
	 double energy_cut,
	 double resolution,
	 TH1D* hTOF, TH1D* hBar,
	 TH1D* hDtScint, TH2D* hDtScintVsAngle)
{
  // open the simulation output
  TFile* fData = new TFile(filename.Data(),"READ");
  cout<<fData->GetName()<<endl;

  TTree* tScint = (TTree*) fData->Get("ScintBarsMCdata");
  // acquire the Scintillators hits
  TClonesArray* scintdigi= new TClonesArray("TScintDigi");
  tScint->SetBranchAddress("ScintBarHits",&scintdigi);

  int NofEvents=tScint->GetEntries();

  energy_cut *= 1.e-3; // keV -> MeV
  resolution *= 1.e-3; // ps -> ns

  for(int e=0; e<NofEvents; ++e)
    {
      tScint->GetEntry(e);

      vector<double> dt,dphi;

      for(int i=0; i<scintdigi->GetEntries(); ++i)
	{
	  if( ((TScintDigi*) scintdigi->At(i))->GetEnergy() < energy_cut )
	    continue;
	  
	  double t_i = ((TScintDigi*) scintdigi->At(i))->GetTime(),
	    pos_i = ((TScintDigi*) scintdigi->At(i))->GetPos();

	  double smearTi = rndm.Gaus(t_i,resolution);

	  //	  hTOF->Fill( t_i );
	  hTOF->Fill( smearTi );
	  hBar->Fill( pos_i*TMath::RadToDeg() );

	  for(int j=i+1; j<scintdigi->GetEntries(); ++j)
	    {
	      if( ((TScintDigi*) scintdigi->At(j))->GetEnergy() < energy_cut )
		continue;

	      if( ((TScintDigi*) scintdigi->At(i))->GetBar() != 
		  ((TScintDigi*) scintdigi->At(j))->GetBar() )
		{
		  double t_j = ((TScintDigi*) scintdigi->At(j))->GetTime(),
		    pos_j = ((TScintDigi*) scintdigi->At(j))->GetPos();

		  double smearTj = rndm.Gaus(t_j,resolution);

		  double deltaT = TMath::Abs( smearTi - smearTj ),
		    deltaPhi = TMath::Abs( pos_i - pos_j );
		  if( deltaPhi > TMath::Pi() )
		    deltaPhi = TMath::TwoPi() - deltaPhi;
		  
		  dt.push_back( deltaT );
		  dphi.push_back( deltaPhi );
		  
		}// hits on different bars
	    }// j-loop
	}// i-loop

      //      cout<<"I found "<<dt.size()<<" combinations in event "<<e<<endl;
      double mindt = 99999., 
	deltangle=-1.;
      for( size_t i = 0; i<dt.size(); ++i )
	{
	  double time = dt.at(i);
	  if( time < mindt )
	    {
	      mindt = time;
	      deltangle = dphi.at(i);
	    }
	  //	  cout<<time<<"\t"<<mindt<<"\t\t"<<deltangle*TMath::RadToDeg()<<endl;
	}
      
      if( mindt == 99999. ) continue;
      hDtScint->Fill(mindt);
      hDtScintVsAngle->Fill(mindt,deltangle*TMath::RadToDeg());
      //      cout<<"Event "<<e<<"\t\t"<<mindt<<"\t\t"<<deltangle*TMath::RadToDeg()<<endl;
    }// event loop

}

void rec(TString filename, 
	 double energy_cut,
	 double resolution,
	 double angle_cut,
	 TH1D* hTOF, TH1D* hBar,
	 TH1D* hDtScint, TH2D* hDtScintVsAngle)
{
  // open the simulation output
  TFile* fData = new TFile(filename.Data(),"READ");
  cout<<fData->GetName()<<endl;

  TTree* tScint = (TTree*) fData->Get("ScintBarsMCdata");
  // acquire the Scintillators hits
  TClonesArray* scintdigi= new TClonesArray("TScintDigi");
  tScint->SetBranchAddress("ScintBarHits",&scintdigi);

  int NofEvents=tScint->GetEntries();

  energy_cut *= 1.e-3; // keV -> MeV
  resolution *= 1.e-3; // ps -> ns

  for(int e=0; e<NofEvents; ++e)
    {
      tScint->GetEntry(e);

      vector<double> dt,dphi;

      for(int i=0; i<scintdigi->GetEntries(); ++i)
	{
	  if( ((TScintDigi*) scintdigi->At(i))->GetEnergy() < energy_cut )
	    continue;
	  
	  double t_i = ((TScintDigi*) scintdigi->At(i))->GetTime(),
	    pos_i = ((TScintDigi*) scintdigi->At(i))->GetPos();

	  double smearTi = rndm.Gaus(t_i,resolution);

	  //	  hTOF->Fill( t_i );
	  hTOF->Fill( smearTi );
	  hBar->Fill( pos_i*TMath::RadToDeg() );

	  for(int j=i+1; j<scintdigi->GetEntries(); ++j)
	    {
	      if( ((TScintDigi*) scintdigi->At(j))->GetEnergy() < energy_cut )
		continue;

	      if( ((TScintDigi*) scintdigi->At(i))->GetBar() != 
		  ((TScintDigi*) scintdigi->At(j))->GetBar() )
		{
		  double t_j = ((TScintDigi*) scintdigi->At(j))->GetTime(),
		    pos_j = ((TScintDigi*) scintdigi->At(j))->GetPos();

		  double smearTj = rndm.Gaus(t_j,resolution);

		  double deltaT = TMath::Abs( smearTi - smearTj ),
		    deltaPhi = TMath::Abs( pos_i - pos_j );
		  if( deltaPhi > TMath::Pi() )
		    deltaPhi = TMath::TwoPi() - deltaPhi;

		  if( deltaPhi < angle_cut )
		    continue;
		  
		  dt.push_back( deltaT );
		  dphi.push_back( deltaPhi );
		  
		}// hits on different bars
	    }// j-loop
	}// i-loop

      //      cout<<"I found "<<dt.size()<<" combinations in event "<<e<<endl;
      double mindt = 99999., 
	deltangle=-1.;
      for( size_t i = 0; i<dt.size(); ++i )
	{
	  double time = dt.at(i);
	  if( time < mindt )
	    {
	      mindt = time;
	      deltangle = dphi.at(i);
	    }
	  //	  cout<<time<<"\t"<<mindt<<"\t\t"<<deltangle*TMath::RadToDeg()<<endl;
	}
      
      if( mindt == 99999. ) continue;
      hDtScint->Fill(mindt);
      hDtScintVsAngle->Fill(mindt,deltangle*TMath::RadToDeg());
      //      cout<<"Event "<<e<<"\t\t"<<mindt<<"\t\t"<<deltangle*TMath::RadToDeg()<<endl;
    }// event loop

}


void CosmEnergy( TString filename, TH1D* hE, TH1D* ha, TH2D* hEangle)
{
  // open the simulation output
  TFile* fData = new TFile(filename.Data(),"READ");
  cout<<fData->GetName()<<endl;

  // grab TPC MC data tree
  TTree* tMC = (TTree*) fData->Get("MCinfo"); 

  TClonesArray* mcpions = new TClonesArray("TLorentzVector");
  tMC->SetBranchAddress("MCpions",&mcpions);

  TVector3 zaxis(0.,0.,1.);

  for(int e=0; e<tMC->GetEntries(); ++e)
    {
      tMC->GetEntry(e);
      for(int pi=0; pi<mcpions->GetEntries(); ++pi)
	{
	  TLorentzVector* P = (TLorentzVector*) mcpions->At(pi);
	  //	  cout<<P->M()<<endl;
	  hE->Fill( P->E() );
	  TVector3 dir = -1.*((P->Vect()).Unit());
	  //	  TVector3 dir = (P->Vect()).Unit();
	  double cosangle = dir.Dot(zaxis);
	  //	  double cosangle = 1. - dir.Dot(zaxis);
	  ha->Fill( cosangle );
	  //	  ha->Fill( TMath::ACos( cosangle ) );
	  hEangle->Fill( cosangle, P->E() );
	}
    }
  
}

int main(int argc, char** argv)
{
  char type = 'c';
  double time_resolution = 0.,
    Ecut = -1.,
    Acut = -1.;
  if( argc == 2 )
    type              = *argv[1];
  else if( argc == 3 )
    {
      type            = *argv[1];
      time_resolution = atof( argv[2] );
    } 
   else if( argc == 4 )
    {
      type            = *argv[1];
      time_resolution = atof( argv[2] );
      Ecut            = atof( argv[3] );
    }   
   else if( argc == 5 )
    {
      type            = *argv[1];
      time_resolution = atof( argv[2] );
      Ecut            = atof( argv[3] );
      Acut            = atof( argv[4] );
    } 

  cout<<"Time Resolution: "<<time_resolution<<" ps\tEnergy Cut: "<<Ecut<<" keV\tAngle Cut: "<<Acut<<" deg\t";
  
  int pdg_select = 11;
  TString fname("fail.root"),
    foutname("noout.root");
  if( type == 'p' )
    {
      cout<<"pbar"<<endl;
      pdg_select = 211; // pi
      //      fname = TString::Format("%s/outAgTPC_IR109mm_OR190mm_B0.65T_bared_pbar.root",
      fname = TString::Format("%s/outAgTPC_IR109mm_OR190mm_B0.65T_bared_pbar_2.root",
			      getenv("DATADIR"));

      if( Ecut < 0. )
	foutname = TString::Format("%s/BarsMC/histoAgBars_pbar_tres%1.0fps.root",
				   getenv("ANALYSIS_TPC"),time_resolution);
      else
	{
	  if( Acut < 0. )
	    foutname = TString::Format("%s/BarsMC/histoAgBars_pbar_Ecut%1.0fkeV_tres%1.0fps.root",
				       getenv("ANALYSIS_TPC"),Ecut,time_resolution);
	  else
	    foutname = TString::Format("%s/BarsMC/histoAgBars_pbar_Acut%1.2fdeg_Ecut%1.0fkeV_tres%1.0fps.root",
				       getenv("ANALYSIS_TPC"),Acut,Ecut,time_resolution);
	}
    }
  else if( type == 'c' )
    {
      cout<<"cosm"<<endl;
      pdg_select = 13;  // mu
      //      fname = TString::Format("%s/outAgTPC_IR109mm_OR190mm_B0.65T_bared_cosm.root",
      fname = TString::Format("%s/outAgTPC_IR109mm_OR190mm_B0.65T_bared_cosm_2.root",
			      getenv("DATADIR"));
      if( Ecut < 0. )
	foutname = TString::Format("%s/BarsMC/histoAgBars_cosm_tres%1.0fps.root",
				   getenv("ANALYSIS_TPC"),time_resolution);
      else
	{
	  if( Acut < 0. )
	    foutname = TString::Format("%s/BarsMC/histoAgBars_cosm_Ecut%1.0fkeV_tres%1.0fps.root",
				       getenv("ANALYSIS_TPC"),Ecut,time_resolution);
	  else
	    foutname = TString::Format("%s/BarsMC/histoAgBars_cosm_Acut%1.2fdeg_Ecut%1.0fkeV_tres%1.0fps.root",
				       getenv("ANALYSIS_TPC"),Acut,Ecut,time_resolution);
	}
    }

  TFile* fout = TFile::Open(foutname.Data(),"RECREATE");
  cout<<fout->GetName()<<endl;

  TH1D* hDt = new TH1D("hDtScint",
		       "Time Difference Between Pair of Scintillator Hits;#Delta t [ns];Events",
		       5000,0.,5.);
  TH2D* hDtVsAngle = new TH2D("hDtScintVsAngle",
			      "Time Difference Between Pair of Scintillator Hits;#Delta t [ns];#Delta #phi [deg];Events",
			      1000,0.,5.,2000,0.,180.);  
  TH1D* hTOF = new TH1D("hTOFScint","Scintillar Bars TOF;t [ns];hits;",5000,0.,50.);
  TH1D* hPos = new TH1D("hPosScint","Scintillar Bars;#phi [deg];hits;",1000,0.,360.);


  TH1D* hE = new TH1D("hEmuon","Energy Spectrum;E [MeV];#mu^{#pm}",20000,0.,1.e5);
  TH1D* hangle = new TH1D("hangle","Cosine of Direction;cos#alpha;#mu^{#pm}",1000,0.,1.);
  //  TH1D* hangle = new TH1D("hangle","Cosine of Direction;#alpha [rad];#mu^{#pm}",2000,0.,TMath::PiOver2());
  TH2D* hEangle = new TH2D("hEangle",";cos#alpha;E [MeV];#mu^{#pm}",1000,0.,1.,1000,0.,1.e5);

  rndm.SetSeed(2016021616);

  if( Ecut < 0. )
    {
      rec( fname,
	   pdg_select,
	   time_resolution,
	   hTOF, hPos,
	   hDt, hDtVsAngle );
    }
  else
    {
      if( Acut > 0. )	  
	rec( fname,
	     Ecut,
	     time_resolution,
	     hTOF, hPos,
	     hDt, hDtVsAngle );
      else
	{
	  rec( fname,
	       Ecut,
	       time_resolution,
	       Acut,
	       hTOF, hPos,
	       hDt, hDtVsAngle );
	  TString htitle = TString::Format("%s - Angle Cut %1.2fdeg",hDt->GetTitle(),Acut);
	  hDt->SetTitle(htitle.Data());
	}
    }
  
  if( type == 'c' )
    CosmEnergy( fname, hE, hangle, hEangle);
  
  fout->Write();
  fout->Close();
  return 0;
}


//  double max_angle = 5.625, min_angle = max_angle; // one bar
//  double max_angle = 11.25, min_angle = max_angle; // two bars
//  double max_angle = 22.5, min_angle = max_angle;  // four bars
//  double max_angle = 28.125, min_angle = max_angle;// five bars
//  double max_angle = 45., min_angle = max_angle;   // eight bars
//  double max_angle = 56.25, min_angle = max_angle; // ten bars
