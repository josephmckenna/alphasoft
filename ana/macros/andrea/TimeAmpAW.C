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
  TString cname("ctaR");
  cname+=run;
  TCanvas* c1 = new TCanvas(cname,cname,2000,2000);
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
      gPad->SetGridy();
      //hta[i]->Draw();
      hta_px[i]->GetXaxis()->SetRangeUser(1599.,2000.);
      maxamp=maxamp>hta_px[i]->GetBinContent(hta_px[i]->GetMaximumBin())?
	maxamp:hta_px[i]->GetBinContent(hta_px[i]->GetMaximumBin());
      hta_px[i]->Draw("HIST");
    }

  c1->SaveAs(".pdf");   c1->SaveAs(".pdf");

  for(int i=0; i<Nawb; ++i)
    {
      hta_px[i]->GetYaxis()->SetRangeUser(0.,maxamp*1.1);
    }

  gDirectory->cd("/match_el");
  TH2D* hpc = (TH2D*) gROOT->FindObject("hawamp_match_amp_pc");
  TH1D* hpc_px=hpc->ProjectionX();
  cname="cpc"; cname+=run;
  TCanvas* c2 = new TCanvas(cname,cname,1700,1200);
  c2->Divide(1,3);
  c2->cd(1);
  hpc_px->Draw();
  double maxpc=hpc_px->GetBinContent(hpc_px->GetMaximumBin());
  cout<<hpc_px->GetName()<<" max: "<<maxpc<<endl;

  int TotBins=hpc_px->GetNbinsX();
  int Nbins=int(TotBins*0.5);
  cout<<hpc_px->GetName()<<" half bins: "<<Nbins<<endl;
  double xlim=double(TotBins*0.5);
  TH1D* hpcbot=new TH1D("hpcbot","AW Amplitude in PC with Matching Pad",Nbins,0.,xlim);
  hpcbot->SetStats(kFALSE);
  hpcbot->SetLineColor(kBlue); hpcbot->SetMarkerColor(kBlue);
  TH1D* hpctop=new TH1D("hpctop","AW Amplitude in PC with Matching Pad",Nbins,0.,xlim);
  hpctop->SetStats(kFALSE);
  hpctop->SetLineColor(kRed); hpctop->SetMarkerColor(kRed);
  for( int b=1; b<=Nbins; ++b)
    {
      double bcbot=hpc_px->GetBinContent(b);
      hpcbot->SetBinContent(b,bcbot);
      int tb=TotBins-b+1;
      double bctop=hpc_px->GetBinContent(tb);
      hpctop->SetBinContent(b,bctop);
      //cout<<b<<"\t"<<bcbot<<"\t"<<tb<<"\t"<<bctop<<endl;
    }
  c2->cd(2);
  hpcbot->Draw("hist");
  hpctop->Draw("histsame");
  TLegend* leg = new TLegend(0.7,0.2,0.85,0.35);
  leg->AddEntry(hpcbot,"Bottom Match","pl");
  leg->AddEntry(hpctop,"Top Match x+288","pl");
  leg->Draw("same");

  c2->cd(3);
  TH1D* hpcdiff=new TH1D("hpcdiff","AW Amplitude in PC Difference BotVsTop",Nbins,0.,xlim);
  hpcdiff->SetStats(kFALSE);
  hpcdiff->SetLineColor(kBlack); hpcdiff->SetMarkerColor(kBlack);
  hpcdiff->SetMarkerStyle(8);
  hpcdiff->Add(hpcbot,hpctop,-1.);
  hpcdiff->Scale(1./maxpc);
  hpcdiff->Draw("P");
  gPad->SetGridy();

  c2->SaveAs(".pdf");   c2->SaveAs(".pdf");
  

  gDirectory->cd("/sigpoints");
  TH2D* hsptawamp = (TH2D*) gROOT->FindObject("hAWspTimeAmp");
  TH1D* hsptawamp_px=hsptawamp->ProjectionX();
  cname="csptawampR";
  cname+=run;
  TCanvas* c3 = new TCanvas(cname,cname,1700,1200);
  hsptawamp_px->Draw();
  //hsptawamp->Draw();
  c3->SaveAs(".pdf");   c3->SaveAs(".pdf");
}
