#define BUILD_AG
#include "TSpacePoint.hh"
#include "TStoreEvent.hh"
#include "TStoreHelix.hh"
//#include "TFitHelix.hh"

#include "IntGetters.h"
#include "TStringGetters.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TF1.h"

#include "TLegend.h"
#include "TPaveText.h"
#include "TStyle.h"
#include "TROOT.h"

#include "TObjArray.h"
#include "TClonesArray.h"
#include "TString.h"
#include "TObjString.h"
#include "TVector3.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

using namespace std;


TString tag;
int RunNumber=3863;
TString savFolder;

map<string,TH1D*> hNhel;
map<string,TH1D*> hhpattreceff;
TCanvas* chel;

// reco helices
map<string,TH1D*> hhD;
map<string,TH1D*> hhc;
map<string,TH1D*> hhchi2R;
map<string,TH1D*> hhchi2Z;
TCanvas* chelprop;

map<string,TH1D*> hpt;
map<string,TH1D*> hpz;
map<string,TH1D*> hpp;
TCanvas* chelmom;
map<string,TH2D*> hptz;

// reco helices spacepoints
map<string,TH2D*> hhspxy;
map<string,TH2D*> hhspzr;
map<string,TH2D*> hhspzp;
map<string,TH2D*> hhsprp;

// reco vertex
map<string,TH1D*> hvr;
map<string,TH1D*> hvphi;
map<string,TH1D*> hvz;
map<string,TH2D*> hvxy;
TCanvas* cvtx;

map<string,TH1D*> hNusedhel;
map<string,TH1D*> huhpattreceff;
TCanvas* cusehel;

// used helices
map<string,TH1D*> huhD;
map<string,TH1D*> huhc;
map<string,TH1D*> huhchi2R;
map<string,TH1D*> huhchi2Z;
TCanvas* cusehelprop;

map<string,TH1D*> huhpt;
map<string,TH1D*> huhpz;
map<string,TH1D*> huhpp;
TCanvas* cusehelmom;
map<string,TH2D*> huhptz;

// used helices spacepoints
map<string,TH2D*> huhspxy;
map<string,TH2D*> huhspzr;
map<string,TH2D*> huhspzp;
map<string,TH2D*> huhsprp;

// cosmic time distribution
map<string,TH1D*> hpois;
map<string,TF1*> fcosrate;
TCanvas* cpois;

bool first=true;
map<string,int> cols;

void MakeHistos(string f)
{
  // reco helices
  hNhel[f] = new TH1D("hNhel","Reconstructed Helices",10,0.,10.);
  hNhel[f]->SetLineColor(cols[f]);

  hhD[f] = new TH1D("hhD","hel D;[mm]",200,0.,200.);
  hhD[f]->SetLineColor(cols[f]);
  hhc[f] = new TH1D("hhc","hel c;[mm^{-1}]",200,-1.e-1,1.e-1);
  hhc[f]->SetLineColor(cols[f]);
  hhchi2R[f] = new TH1D("hhchi2R","hel #chi^{2}_{R}",100,0.,50.);
  hhchi2R[f]->SetLineColor(cols[f]);
  hhchi2Z[f] = new TH1D("hhchi2Z","hel #chi^{2}_{Z}",100,0.,50.);
  hhchi2Z[f]->SetLineColor(cols[f]);

  hpt[f] = new TH1D("hpt","helix Transverse Momentum;p_{T} [MeV/c]",1000,0.,2000.);
  hpt[f]->SetLineColor(cols[f]);
  hpz[f] = new TH1D("hpz","helix Longitudinal Momentum;p_{Z} [MeV/c]",2000,-1000.,1000.);
  hpz[f]->SetLineColor(cols[f]);
  hpp[f] = new TH1D("hpp","helix Total Momentum;p_{tot} [MeV/c]",1000,0.,2000.);
  hpp[f]->SetLineColor(cols[f]);
  hptz[f] = new TH2D("hptz","helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
		     100,0.,2000.,200,-1000.,1000.);
  hptz[f]->SetMarkerColor(cols[f]);

  // reco helices spacepoints
  hhpattreceff[f] = new TH1D("hhpattreceff","Track Finding Efficiency",201,-1.,200.);
  hhpattreceff[f]->SetLineWidth(2);
  hhpattreceff[f]->SetLineColor(cols[f]);
  hhspxy[f] = new TH2D("hhspxy","Spacepoints in Helices;x [mm];y [mm]",
		       100,-190.,190.,100,-190.,190.);
  hhspxy[f]->SetStats(kFALSE);
  hhspzr[f] = new TH2D("hhspzr","Spacepoints in Helices;z [mm];r [mm]",
		       600,-1200.,1200.,61,109.,174.);
  hhspzr[f]->SetStats(kFALSE);
  hhspzp[f] = new TH2D("hhspzp","Spacepoints in Helices;z [mm];#phi [deg]",
		       600,-1200.,1200.,100,0.,360.);
  hhspzp[f]->SetStats(kFALSE);
  hhsprp[f] = new TH2D("hhsprp","Spacepoints in Helices;#phi [deg];r [mm]",
		       100,0.,TMath::TwoPi(),61,109.,174.);
  hhsprp[f]->SetStats(kFALSE);


  // used helices
  hNusedhel[f] = new TH1D("hNusedhel","Used Helices",10,0.,10.);
  hNusedhel[f]->SetLineColor(cols[f]);

  huhD[f] = new TH1D("huhD","Used Hel D;[mm]",200,0.,200.);
  huhD[f]->SetLineColor(cols[f]);
  huhc[f] = new TH1D("huhc","Used Hel c;[mm^{-1}]",200,-1.e-1,1.e-1);
  huhc[f]->SetLineColor(cols[f]);
  huhchi2R[f] = new TH1D("huhchi2R","Used Hel #chi^{2}_{R}",100,0.,50.);
  huhchi2R[f]->SetLineColor(cols[f]);
  huhchi2Z[f] = new TH1D("huhchi2Z","Used Hel #chi^{2}_{Z}",100,0.,50.);
  huhchi2Z[f]->SetLineColor(cols[f]);

  huhpt[f] = new TH1D("huhpt","Used Helix Transverse Momentum;p_{T} [MeV/c]",1000,0.,2000.);
  huhpt[f]->SetLineColor(cols[f]);
  huhpz[f] = new TH1D("huhpz","Used Helix Longitudinal Momentum;p_{Z} [MeV/c]",2000,-1000.,1000.);
  huhpz[f]->SetLineColor(cols[f]);
  huhpp[f] = new TH1D("huhpp","Used Helix Total Momentum;p_{tot} [MeV/c]",1000,0.,2000.);
  huhpp[f]->SetLineColor(cols[f]);
  huhptz[f] = new TH2D("huhptz","Used Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
		       100,0.,2000.,200,-1000.,1000.);
  huhptz[f]->SetMarkerColor(cols[f]);


  // used helices spacepoints
  huhspxy[f] = new TH2D("huhspxy","Spacepoints in Used Helices;x [mm];y [mm]",
			100,-190.,190.,100,-190.,190.);
  huhspxy[f]->SetStats(kFALSE);
  huhspzr[f] = new TH2D("huhspzr","Spacepoints in Used Helices;z [mm];r [mm]",
			600,-1200.,1200.,61,109.,174.);
  huhspzr[f]->SetStats(kFALSE);
  huhspzp[f] = new TH2D("huhspzp","Spacepoints in Used Helices;z [mm];#phi [deg]",
			600,-1200.,1200.,100,0.,360.);
  huhspzp[f]->SetStats(kFALSE);

  huhsprp[f] = new TH2D("huhsprp","Spacepoints in Used Helices;#phi [deg];r [mm]",
			100,0.,TMath::TwoPi(),90,109.,174.);
  huhsprp[f]->SetStats(kFALSE);

  // reco vertex
  hvr[f] = new TH1D("hvr","Vertex Radius;r [mm]",190,0.,190.);
  hvr[f]->SetLineColor(cols[f]);
  hvphi[f] = new TH1D("hvphi","Vertex #phi; [deg]",360,-180.,180.);
  hvphi[f]->SetMinimum(0.);
  hvphi[f]->SetLineColor(cols[f]);
  hvz[f] = new TH1D("hvz","Vertex Z;z [mm]",1000,-1152.,1152.);
  hvz[f]->SetLineColor(cols[f]);
  hvxy[f] = new TH2D("hvxy","Vertex X-Y;x [mm];y [mm]",200,-190.,190.,200,-190.,190.);

  // cosmic time distribution
  hpois[f] = new TH1D("hpois","Delta t between cosmics;#Delta t [ms]",200,0.,111.);
  hpois[f]->SetMarkerColor(cols[f]);
  hpois[f]->SetMarkerStyle(8);
  hpois[f]->SetLineColor(cols[f]);
}


void ProcessHelix(TStoreHelix* hel, string f)
{
  hhD[f]->Fill(hel->GetD());
  hhc[f]->Fill(hel->GetC());
  hhchi2R[f]->Fill(hel->GetRchi2());
  hhchi2Z[f]->Fill(hel->GetZchi2());

  //  hel->GetMomentumV().Print();

  hpt[f]->Fill(hel->GetMomentumV().Perp());
  hpz[f]->Fill(hel->GetMomentumV().Z());
  hpp[f]->Fill(hel->GetMomentumV().Mag());
  hptz[f]->Fill(hel->GetMomentumV().Perp(),hel->GetMomentumV().Z());

  const TObjArray* sp = hel->GetSpacePoints();
  for( int ip = 0; ip<sp->GetEntries(); ++ip )
    {
      TSpacePoint* ap = (TSpacePoint*) sp->At(ip);
      hhspxy[f]->Fill( ap->GetX(), ap->GetY() );
      hhspzp[f]->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
      hhspzr[f]->Fill( ap->GetZ(), ap->GetR() );
      hhsprp[f]->Fill( ap->GetPhi(), ap->GetR() );
    }
}

void ProcessUsed(TFitHelix* hel, string f)
{
  huhD[f]->Fill(hel->GetD());
  huhc[f]->Fill(hel->GetC());
  huhchi2R[f]->Fill(hel->GetRchi2());
  huhchi2Z[f]->Fill(hel->GetZchi2());

  //  hel->GetMomentumV().Print();

  huhpt[f]->Fill(hel->GetMomentumV().Perp());
  huhpz[f]->Fill(hel->GetMomentumV().Z());
  huhpp[f]->Fill(hel->GetMomentumV().Mag());
  huhptz[f]->Fill(hel->GetMomentumV().Perp(),hel->GetMomentumV().Z());

  const vector<TSpacePoint*> *sp = hel->GetPointsArray();
  for( unsigned int ip = 0; ip<sp->size(); ++ip )
    {
      TSpacePoint* ap = sp->at(ip);
      huhspxy[f]->Fill( ap->GetX(), ap->GetY() );
      huhspzp[f]->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
      huhspzr[f]->Fill( ap->GetZ(), ap->GetR() );
      huhsprp[f]->Fill( ap->GetPhi(), ap->GetR() );
    }
}


void ProcessVertex(TVector3* v, string f)
{
  hvr[f]->Fill(v->Perp());
  hvphi[f]->Fill(v->Phi()*TMath::RadToDeg());
  hvz[f]->Fill(v->Z());
  hvxy[f]->Fill(v->X(),v->Y());
}

void ProcessTree( TTree* tin, string f )
{
  TStoreEvent* event = new TStoreEvent();
  tin->SetBranchAddress("StoredEvent", &event);
  double temp=0.;
  double Nvtx=0.;
  for(int e=0; e<tin->GetEntries(); ++e)
    {
      if( e%1000 == 0 ) cout<<"*** "<<e<<endl;
      event->Reset();
      tin->GetEntry(e);

      const TObjArray* helices = event->GetHelixArray();
      int Nhelices = helices->GetEntries();
      //      cout<<"Number of Helices: "<<Nhelices<<endl;
      double Npoints = 0.;
      for(int i=0; i<Nhelices; ++i)
	{
	  TStoreHelix* aHelix = (TStoreHelix*) helices->At(i);
	  ProcessHelix( aHelix, f );
	  Npoints += double(aHelix->GetNumberOfPoints());
	}
      hNhel[f]->Fill( double(Nhelices) );
      if( Nhelices )
	{
	  hhpattreceff[f]->Fill(Npoints/double(Nhelices));
	}

      if( Nhelices >= 2 && Nhelices < 4 )
	{
	  double delta = (event->GetTimeOfEvent() - temp)*1.e3;
	  hpois[f]->Fill( delta );
	  temp = event->GetTimeOfEvent();
	}

      TVector3 vtx = event->GetVertex();
      if(event->GetVertexStatus()>0)
      	{
      	  const TObjArray* used_hel = event->GetUsedHelices();
      	  hNusedhel[f]->Fill( double(used_hel->GetEntries()) );
      	  for(int ih=0; ih<used_hel->GetEntries(); ++ih) 
      	    ProcessUsed((TFitHelix*) used_hel->At(ih),f);
      	  ProcessVertex(&vtx, f);
      	  ++Nvtx;
      	}
    }
  cout<<"Number of Events Processed: "<<tin->GetEntries()<<endl;
  cout<<"Number of Reconstructed Vertexes: "<<Nvtx<<endl;
  cout<<"Total Runtime: "<<temp<<" s"<<endl;
  cout<<"Cosmic Rate: "<<Nvtx/temp<<" s^-1"<<endl;
}

void DisplayHisto(string f)
{
  // cosmic time distribution
  if( hpois[f]->GetEntries() > 0 )
    {
      cpois->cd();
      // cosmic time distribution
      if(first) hpois[f]->Draw("P");
      else hpois[f]->Draw("Psame");
      hpois[f]->Fit("expo","Q0EMW");
      fcosrate[f] = hpois[f]->GetFunction("expo");
      TString funcname=f;
      funcname+=fcosrate[f]->GetName();
      fcosrate[f]->SetName(funcname);
      if( fcosrate[f] )
	{
	  double rate = fabs( fcosrate[f]->GetParameter(1) )*1.e3,
	    rate_err = fabs( fcosrate[f]->GetParError(1) )*1.e3;
	  TString srate = TString::Format("Cosmic Rate %s R%d: (%1.1f#pm%1.1f) Hz",
					  f.c_str(),RunNumber,rate,rate_err);
	  cout<<srate<<endl;
	  fcosrate[f]->Draw("same");
	  TPaveText* trate = new TPaveText(0.5,0.53,0.87,0.6,"NDC");
	  trate->AddText(srate.Data());
	  trate->SetFillColor(0);
	  trate->Draw();
	}
    }

  // reco helices
  if(hhD[f]->GetEntries())
    {
      chel->cd(1);
      if(first) hNhel[f]->Draw();
      else  hNhel[f]->Draw("same");
      chel->cd(2);
      if(first) hhpattreceff[f]->Draw();
      else hhpattreceff[f]->Draw("same");


      chelprop->cd(1);
      if(first) hhD[f]->Draw();
      else hhD[f]->Draw("same");
      chelprop->cd(2);
      if(first) hhc[f]->Draw();
      else hhc[f]->Draw("same");
      chelprop->cd(3);
      if(first) hhchi2R[f]->Draw();
      else hhchi2R[f]->Draw("same");
      chelprop->cd(4);
      if(first) hhchi2Z[f]->Draw();
      else  hhchi2Z[f]->Draw("same");
  

     
      chelmom->cd(1);
      if(first) hpt[f]->Draw();
      else hpt[f]->Draw("same");
      chelmom->cd(2);
      if(first) hpz[f]->Draw();
      else  hpz[f]->Draw("same");
      chelmom->cd(3);
      if(first) hpp[f]->Draw();
      else hpp[f]->Draw("same");

      chelmom->cd(4);
      if(first) hptz[f]->Draw();
      else  hptz[f]->Draw("same");

   
      TString cname = "spacepoints_helices";
      cname+=tag;
      TCanvas* chsp = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      chsp->Divide(2,2);
      chsp->cd(1);
      hhspxy[f]->Draw("colz");
      chsp->cd(2);
      hhspzr[f]->Draw("colz");
      chsp->cd(3);
      hhspzp[f]->Draw("colz");
      chsp->cd(4);
      hhsprp[f]->Draw("colz");
      chsp->SaveAs(savFolder+cname+TString(".pdf"));
      chsp->SaveAs(savFolder+cname+TString(".pdf"));
    }

  // vertex
  if( hvr[f]->GetEntries() )
    {
      cvtx->cd(1);
      if(first) hvr[f]->Draw();
      else  hvr[f]->Draw("same");
      cvtx->cd(2);
      if(first) hvphi[f]->Draw();
      else hvphi[f]->Draw("same");
      cvtx->cd(3);
      if(first) hvz[f]->Draw();
      else  hvz[f]->Draw("same");
      cvtx->cd(4);
      if(first) hvxy[f]->Draw();
      else hvxy[f]->Draw("same");
  
    }

  // used helices
  if(huhD[f]->GetEntries())
    {
      if(first) hNusedhel[f]->Draw();
      else  hNusedhel[f]->Draw("same");

 
      cusehelprop->cd(1);
      if(first) huhD[f]->Draw();
      else  huhD[f]->Draw("same");
      cusehelprop->cd(2);
      if(first) huhc[f]->Draw();
      else huhc[f]->Draw("same");
      cusehelprop->cd(3);
      if(first) huhchi2R[f]->Draw();
      else huhchi2R[f]->Draw("same");
      cusehelprop->cd(4);
      if(first) huhchi2Z[f]->Draw();
      else huhchi2Z[f]->Draw("same");
      
      cusehelmom->cd(1);
      if(first) huhpt[f]->Draw();
      else huhpt[f]->Draw("same");
      cusehelmom->cd(2);
      if(first) huhpz[f]->Draw();
      else huhpz[f]->Draw("same");
      cusehelmom->cd(3);
      if(first) huhpp[f]->Draw();
      else huhpp[f]->Draw("same");
      cusehelmom->cd(4);
      if(first) huhptz[f]->Draw();
      else huhptz[f]->Draw("same");
  

      TString cname = "spacepoints_usedhelices";
      cname+=tag;
      TCanvas* chsp = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      chsp->Divide(2,2);

      chsp->cd(1);
      huhspxy[f]->Draw("colz");
      chsp->cd(2);
      huhspzr[f]->Draw("colz");
      chsp->cd(3);
      huhspzp[f]->Draw("colz");
      chsp->cd(4);
      huhsprp[f]->Draw("colz");
      chsp->SaveAs(savFolder+cname+TString(".pdf"));
      chsp->SaveAs(savFolder+cname+TString(".pdf"));
    }

}

void MakeCanvases()
{
  TString cname = "time_distribution_between_cosmics";
  cname+=tag;
  cpois = new TCanvas(cname.Data(),cname.Data(),1300,1000);

  cname = "chel";
  cname+=tag;
  chel = new TCanvas(cname.Data(),cname.Data(),1700,800);
  chel->Divide(2,1);

  cname ="chelprop";
  cname+=tag;
  chelprop = new TCanvas(cname.Data(),cname.Data(),1400,1400);
  chelprop->Divide(2,2);

  cname ="chelmom";
  cname+=tag;
  chelmom = new TCanvas(cname.Data(),cname.Data(),1400,1400);
  chelmom->Divide(2,2);


  cname = "cusehel";
  cname+=tag;
  cusehel = new TCanvas(cname.Data(),cname.Data(),1000,800);
  cusehel->Divide(2,1);

  cname ="cusehelprop";
  cname+=tag;
  cusehelprop = new TCanvas(cname.Data(),cname.Data(),1400,1400);
  cusehelprop->Divide(2,2);

  cname ="cusehelmom";
  cname+=tag;
  cusehelmom = new TCanvas(cname.Data(),cname.Data(),1400,1400);
  cusehelmom->Divide(2,2);

  cname="cvtx";
  cname+=tag;
  cvtx = new TCanvas(cname.Data(),cname.Data(),1400,1400);
  cvtx->Divide(2,2);
}

void SaveHistos()
{
  savFolder=MakeAutoPlotsFolder("time");
  cpois->SaveAs(savFolder+cpois->GetName()+TString(".pdf"));
  cpois->SaveAs(savFolder+cpois->GetName()+TString(".pdf"));  

  chel->SaveAs(savFolder+chel->GetName()+TString(".pdf"));
  chel->SaveAs(savFolder+chel->GetName()+TString(".pdf"));

  chelprop->SaveAs(savFolder+chelprop->GetName()+TString(".pdf"));
  chelprop->SaveAs(savFolder+chelprop->GetName()+TString(".pdf"));

  chelmom->SaveAs(savFolder+chelmom->GetName()+TString(".pdf"));
  chelmom->SaveAs(savFolder+chelmom->GetName()+TString(".pdf"));


  cvtx->SaveAs(savFolder+cvtx->GetName()+TString(".pdf"));
  cvtx->SaveAs(savFolder+cvtx->GetName()+TString(".pdf"));

  cusehel->SaveAs(savFolder+cusehel->GetName()+TString(".pdf"));
  cusehel->SaveAs(savFolder+cusehel->GetName()+TString(".pdf"));


  cusehelprop->SaveAs(savFolder+cusehelprop->GetName()+TString(".pdf"));
  cusehelprop->SaveAs(savFolder+cusehelprop->GetName()+TString(".pdf"));

  cusehelmom->SaveAs(savFolder+cusehelmom->GetName()+TString(".pdf"));
  cusehelmom->SaveAs(savFolder+cusehelmom->GetName()+TString(".pdf"));
}


void fitterComp()
{
  vector<string> fitter_type{"M1","M2"};
  cols["M1"]=kBlue;
  cols["M2"]=kRed;
  gStyle->SetPalette(kRainBow);
  MakeCanvases();
  tag="_R";
  tag+=RunNumber;
  for(auto s: fitter_type)
    {
      TString fname=TString::Format("%s/test/cosmics%dsub000%s.root",getenv("DATADIR"),RunNumber,s.c_str());
      TFile* fin=TFile::Open(fname,"READ");
      if( !fin && !fin->IsOpen() ) continue;
      cout<<fin->GetName()<<" is open"<<endl;
      TObjString* sett = (TObjString*) gROOT->FindObject("ana_settings");
      cout<<sett->GetString()<<endl;
      tag+=s;

      MakeHistos( s );

      TTree* tin = (TTree*) fin->Get("StoreEventTree");
      cout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<endl;
      ProcessTree( tin , s );

      DisplayHisto( s );
      first=false;
    }

  SaveHistos();

}
