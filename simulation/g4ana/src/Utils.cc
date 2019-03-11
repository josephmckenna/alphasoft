#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <set>

#include "SignalsType.h"

#include "TClonesArray.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TEllipse.h"

#include "TMChit.hh"
#include "LookUpTable.hh"
#include "TSpacePoint.hh"

void PlotMCpoints(TCanvas* c, const TClonesArray* points)
{
  int Npoints = points->GetEntries();
  std::cout<<"[main]#  GarfHits --> "<<Npoints<<std::endl; 
  TGraph* gxy = new TGraph(Npoints);
  gxy->SetMarkerStyle(6);
  gxy->SetMarkerColor(kGreen+2);
  gxy->SetTitle("Garfield++ Hits X-Y;x [mm];y [mm]");
  TGraph* grz = new TGraph(Npoints);
  grz->SetMarkerStyle(6);
  grz->SetMarkerColor(kGreen+2);
  grz->SetTitle("Garfield++ Hits R-Z;r [mm];z [mm]");
  TGraph* grphi = new TGraph(Npoints);
  grphi->SetMarkerStyle(6);
  grphi->SetMarkerColor(kGreen+2);
  grphi->SetTitle("Garfield++ Hits R-#phi;r [mm];#phi [deg]");
  TGraph* gzphi = new TGraph(Npoints);
  gzphi->SetMarkerStyle(6);
  gzphi->SetMarkerColor(kGreen+2);
  gzphi->SetTitle("Garfield++ Hits Z-#phi;z [mm];#phi [deg]");
  for( int i=0; i<Npoints; ++i )
    {
      //TMChit* h = (TMChit*) points->ConstructedAt(i);
      TMChit* h = (TMChit*) points->At(i);
      gxy->SetPoint(i,h->GetX(),h->GetY());
      grz->SetPoint(i,h->GetRadius(),h->GetZ());
      grphi->SetPoint(i,h->GetRadius(),h->GetPhi()*TMath::RadToDeg());
      gzphi->SetPoint(i,h->GetZ(),h->GetPhi()*TMath::RadToDeg());
    }      
  c->cd(1);
  gxy->Draw("AP");
  gxy->GetXaxis()->SetRangeUser(109.,190.);
  gxy->GetYaxis()->SetRangeUser(0.,190.);
  c->cd(2);
  grz->Draw("AP");
  grz->GetXaxis()->SetRangeUser(109.,190.);
  grz->GetYaxis()->SetRangeUser(-10.,10.);
  c->cd(3);
  grphi->Draw("AP");
  grphi->GetXaxis()->SetRangeUser(109.,190.);
  grphi->GetYaxis()->SetRangeUser(0.,40.);
  c->cd(4);
  gzphi->Draw("AP");
  gzphi->GetXaxis()->SetRangeUser(-10.,10.);
  gzphi->GetYaxis()->SetRangeUser(0.,40.);
}

void PlotAWhits(TCanvas* c, const TClonesArray* points)
{
  LookUpTable fSTR(0.3, 1.); // uniform field version (simulation)
  int Npoints = points->GetEntries();
  TGraph* gxy = new TGraph(Npoints);
  gxy->SetMarkerStyle(6);
  gxy->SetMarkerColor(kBlack);
  gxy->SetTitle("Garfield++ Hits X-Y;x [mm];y [mm]");
  TGraph* grz = new TGraph(Npoints);
  grz->SetMarkerStyle(6);
  grz->SetMarkerColor(kBlack);
  grz->SetTitle("Garfield++ Hits R-Z;r [mm];z [mm]");
  TGraph* grphi = new TGraph(Npoints);
  grphi->SetMarkerStyle(6);
  grphi->SetMarkerColor(kBlack);
  grphi->SetTitle("Garfield++ Hits R-#phi;r [mm];#phi [deg]");
  TGraph* gzphi = new TGraph(Npoints);
  gzphi->SetMarkerStyle(6);
  gzphi->SetMarkerColor(kBlack);
  gzphi->SetTitle("Garfield++ Hits Z-#phi;z [mm];#phi [deg]");
  for( int j=0; j<Npoints; ++j )
    {
      TMChit* h = (TMChit*) points->At(j);
      double time = h->GetTime(),
	zed = h->GetZ();
      double rad = fSTR.GetRadius( time , zed );
      double phi = h->GetPhi() - fSTR.GetAzimuth( time , zed );
      if( phi < 0. ) phi += TMath::TwoPi();
      if( phi >= TMath::TwoPi() )
	phi = fmod(phi,TMath::TwoPi());
      double y = rad*TMath::Sin( phi ),
      x = rad*TMath::Cos( phi );
      gxy->SetPoint(j,x,y);
      grz->SetPoint(j,rad,zed);
      grphi->SetPoint(j,rad,phi*TMath::RadToDeg());
      gzphi->SetPoint(j,zed,phi*TMath::RadToDeg());
    }
  c->cd(1);
  gxy->Draw("Psame");
  c->cd(2);
  grz->Draw("Psame");
  c->cd(3);
  grphi->Draw("Psame");
  c->cd(4);
  gzphi->Draw("Psame");
}

void PlotRecoPoints(TCanvas* c, const TClonesArray* points)
{
  int Npoints = points->GetEntries();
  std::cout<<"[main]#  Reco --> "<<Npoints<<std::endl; 
  TGraph* gxy = new TGraph(Npoints);
  gxy->SetMarkerStyle(6);
  gxy->SetMarkerColor(kRed);
  gxy->SetTitle("Reco Hits X-Y;x [mm];y [mm]");
  TGraph* grz = new TGraph(Npoints);
  grz->SetMarkerStyle(6);
  grz->SetMarkerColor(kRed);
  grz->SetTitle("Reco Hits R-Z;r [mm];z [mm]");
  TGraph* grphi = new TGraph(Npoints);
  grphi->SetMarkerStyle(6);
  grphi->SetMarkerColor(kRed);
  grphi->SetTitle("Reco Hits R-#phi;r [mm];#phi [deg]");
  TGraph* gzphi = new TGraph(Npoints);
  gzphi->SetMarkerStyle(6);
  gzphi->SetMarkerColor(kRed);
  gzphi->SetTitle("Reco Hits Z-#phi;z [mm];#phi [deg]");
  for( int i=0; i<Npoints; ++i )
    {
      //TSpacePoint* p = (TSpacePoint*) points->ConstructedAt(i);
      TSpacePoint* p = (TSpacePoint*) points->At(i);
      gxy->SetPoint(i,p->GetX(),p->GetY());
      grz->SetPoint(i,p->GetR(),p->GetZ());
      grphi->SetPoint(i,p->GetR(),p->GetPhi()*TMath::RadToDeg());
      gzphi->SetPoint(i,p->GetZ(),p->GetPhi()*TMath::RadToDeg());
    }      
  c->cd(1);
  gxy->Draw("Psame");
  c->cd(2);
  grz->Draw("Psame");
  c->cd(3);
  grphi->Draw("Psame");
  c->cd(4);
  gzphi->Draw("Psame");
}

void DrawTPCxy(TCanvas* c)
{
  TEllipse* TPCcath = new TEllipse(0.,0.,109.,109.);
  TPCcath->SetFillStyle(0);
  c->cd(1);
  TPCcath->Draw("same");
  TEllipse* TPCpads = new TEllipse(0.,0.,190.,190.);
  TPCpads->SetFillStyle(0);
  c->cd(1);
  TPCpads->Draw("same");
}

void PrintSignals(std::vector<signal>* sig)
{
  for(auto s: *sig)
    s.print();
}

TH1D* PlotSignals(std::vector<signal>* sig, std::string name)
{
  std::ostringstream hname;
  hname<<"hsig"<<name;
  std::string htitle(";t [ns];H [a.u.]");
  TH1D* h = new TH1D(hname.str().c_str(),htitle.c_str(),411,0.,16.*411.);
  h->SetStats(kFALSE);
  for(auto s: *sig)
    {
      if( s.t < 16. ) continue;
      h->Fill(s.t,s.height);
    }
  return h;
}

TH1D* PlotOccupancy(std::vector<signal>* sig, std::string name)
{
  std::ostringstream hname;
  hname<<"hocc"<<name;
  std::string htitle("Occupancy Azimuth;AW index / PAD sector");
  TH1D* h = new TH1D(hname.str().c_str(),htitle.c_str(),256,0.,256.);
  h->SetStats(kFALSE);
  for(auto s: *sig)
    {
      if( s.t < 16. ) continue;
      if( name == "anodes" )
	h->Fill(s.idx);
      else if( name == "pads" )
	{
	  // int col = s.sec*8+4;
	  for(int i=0; i<8; ++i)
	    {
	      int col = s.sec*8+i;
	      h->Fill(col);
	    }
	}
    }
  return h;
}

TH2D* PlotSignals(std::vector<signal>* awsignals, 
		  std::vector<signal>* padsignals, std::string type="none")
{
  std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(), 
						     awsignals->end());
  std::multiset<signal, signal::timeorder> pad_bytime(padsignals->begin(), 
						      padsignals->end());
  int Nmatch=0;
  std::ostringstream hname;
  hname<<"hmatch"<<type;
  std::ostringstream htitle;
  htitle<<"AW vs PAD Time with Matching "<<type<<";AW [ns];PAD [ns]";
  TH2D* hh = new TH2D(hname.str().c_str(),htitle.str().c_str(),375,0.,6000.,375,0.,6000.);
  hh->SetStats(kFALSE);
  for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
    {
      for( auto ipd=pad_bytime.begin(); ipd!=pad_bytime.end(); ++ipd )
	{
	  bool match=false;
	  if( type == "time" )
	    {
	      double delta = fabs( iaw->t - ipd->t );
	      if( delta < 16. ) 
		match=true;
	    }
	  else if( type == "sector" )
	    {
	      short sector = short(iaw->idx/8);
	      if( sector == ipd->sec ) 
		match=true;	    
	    }
	  else if( type == "both" )
	    {
	      double delta = fabs( iaw->t - ipd->t );
	      short sector = short(iaw->idx/8);
	      if( delta < 16. && sector == ipd->sec )
		match=true;
	    }
	  else
	    match=true;

	  if( match )
	    {
	      hh->Fill( iaw->t , ipd->t );
	      ++Nmatch;
	    }
	}
    }
  return hh;
}

double Average(std::vector<double>* v)
{
  if( v->size() == 0 ) return -1.;
  double avg=0.;
  for( auto& x: *v ) avg+=x;
  return avg/double(v->size());
}
