#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"

#include <iostream>
#include <iomanip>
using namespace std;

#include "IntGetters.h"

const int Nawb=16;
const int Ncha=16;

void TimeAmpAW()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  int run = GetRunNumber(TString(fin->GetName()));
  cout<<fin->GetName()<<"\t"<<run<<endl;
  gDirectory->cd("/awdeconv/adcwf/adc32");
  TString hname("hadcampch000");
  TH2D* h0 = (TH2D*) gROOT->FindObject(hname);
  int NbinsX=h0->GetNbinsX(), NbinsY=h0->GetNbinsY();
  double xmin=h0->GetXaxis()->GetXmin(), ymin=h0->GetYaxis()->GetXmin(),
    xmax=h0->GetXaxis()->GetXmax(), ymax=h0->GetYaxis()->GetXmax();
  TH2D* hta[Nawb];
  TH1D* hta_px[Nawb];
  TCanvas* c1 = new TCanvas("cta","cta",2000,2000);
  c1->Divide(4,4);
  double maxamp=0.;
  for(int i=0; i<Nawb; ++i)
    {
      hname=TString::Format("htampawb%02d",i);
      TString htitle=TString::Format("Pulse Height Vs. Time AWB T%02d;Time [ns];P.H. [a.u.]",i);
      hta[i]=new TH2D(hname,htitle,NbinsX,xmin,xmax,NbinsY,ymin,ymax);
      for(int j=0; j<Ncha; ++j)
	{
	  int aw = i*Nawb+j;
	  hname = TString::Format("hadcampch%03d",aw);
	  //cout<<"T"<<setfill('0')<<setw(2)<<i<<"\t"<<aw<<"\t"<<hname<<endl;
	  TH2D* htemp = (TH2D*) gROOT->FindObject(hname);
	  hta[i]->Add(htemp);
	}
      //hta[i]->RebinX();
      hta_px[i]=hta[i]->ProjectionX();
      c1->cd(i+1);
      //hta[i]->Draw();
      hta_px[i]->GetXaxis()->SetRangeUser(1599.,2000.);
      maxamp=maxamp>hta_px[i]->GetBinContent(hta_px[i]->GetMaximumBin())?
	maxamp:hta_px[i]->GetBinContent(hta_px[i]->GetMaximumBin());
      hta_px[i]->Draw("HIST");
    }

  for(int i=0; i<Nawb; ++i)
    {
      hta_px[i]->GetYaxis()->SetRangeUser(0.,maxamp*1.1);
    }

  gDirectory->cd("/match_el");
  TH2D* hpc = (TH2D*) gROOT->FindObject("hawamp_match_amp_pc");
  TH1D* hpc_px=hpc->ProjectionX();
  TCanvas* c2 = new TCanvas("cpc","cpc",1700,1200);
  hpc_px->Draw();

  gDirectory->cd("/sigpoints");
  TH2D* hsptawamp = (TH2D*) gROOT->FindObject("hAWspTimeAmp");
  TH1D* hsptawamp_px=hsptawamp->ProjectionX();
  TCanvas* c3 = new TCanvas("csptawamp","csptawamp",1700,1200);
  hsptawamp_px->Draw();
  //hsptawamp->Draw();
}
