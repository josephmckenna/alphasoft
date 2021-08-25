#include <iostream>
#include <vector>
#include <string>

#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"

int plothisto(TFile* fin, Int_t col, TCanvas* c1, TCanvas* c2, bool first=true)
{
  vector<string> histolist {"hcosRes2min","hDCAeq2","hDCAgr2","hvr","hNhel","huhD","huhpp","huhpt"};
  int n=0;
  for(auto& hname : histolist)
    {
      fin->cd();
      TH1D* h = (TH1D*) gDirectory->Get(hname.c_str());
      if( hname=="hcosRes2min" || hname=="hDCAeq2" || hname=="hDCAgr2" || hname=="huhpp" || hname=="huhpt" )
	h->Rebin();
      h->Sumw2();
      h->SetLineColor(col); h->SetMarkerColor(col); h->SetMarkerStyle(7); 
      h->Scale(1./h->Integral());
      h->SetStats(0);
      if( n<4 ) 
	{
	  c1->cd(n+1);
	  if(first) h->Draw();
	  else h->Draw("same");
	}
      else
	{
	  c2->cd(n-4+1);
	  //h->Draw();
	  if(first) h->Draw();
	  else h->Draw("same");
	}
      ++n;
    }
  return n;
}

void plotcomp()
{

  TCanvas* ccos=new TCanvas("ccoscomp","ccoscomp",2400,1900);
  ccos->Divide(2,2);
  TCanvas* chel=new TCanvas("chelcomp","chelcomp",2400,1900);
  chel->Divide(2,2);
 
  TLegend* leg = new TLegend(0.8,0.8,0.95,0.95);
  TH1D* h1=new TH1D();
  h1->SetLineColor(kRed);
  leg->AddEntry(h1,"mixing","l");
  TH1D* h2=new TH1D();
  h2->SetLineColor(kBlue);
  leg->AddEntry(h2,"cosmics","l");

  TFile* fpbar = TFile::Open("/daq/alpha_data0/acapra/alphag/CERN2018/plots_mixing2018.root");
  plothisto(fpbar,kRed,ccos,chel);

  TFile* fcosm = TFile::Open("/daq/alpha_data0/acapra/alphag/CERN2018/plots_R3865.root");
  plothisto(fcosm,kBlue,ccos,chel,false);


  ccos->cd(1);
  leg->Draw("same");
  chel->cd(1);
  leg->Draw("same");
}

