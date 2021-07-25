#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"

#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

#include "IntGetters.h"

void persistency_plot()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  int run = GetRunNumber(TString(fin->GetName()));
  cout<<fin->GetName()<<"\t"<<run<<endl;

  int xsize=1300, ysize=700;
  if( gROOT->IsBatch() ){
    xsize=3200; ysize=1700;}

  double maxy=0.;
  double miny=DBL_MAX;
  TString cname;

  bool s = gDirectory->cd("/awdeconv");
  if(s) {
  TH1D* ht= (TH1D*) gROOT->FindObject("hTimeTop");
  ht->SetMinimum(0.);
  
  cname="ccdeconvR";
  cname+=run;
  TCanvas* cc = new TCanvas(cname,cname,1700,900);
  ht->Draw();
  ht->Rebin(32);
 
  maxy=maxy>ht->GetBinContent(ht->GetMaximumBin())?maxy:ht->GetBinContent(ht->GetMaximumBin());
  double mint=1400.,maxt=2000;
  TLine* l1 = new TLine(mint,0.,mint,maxy);
  l1->SetLineColor(kRed);
  l1->SetLineStyle(2);
  TLine* l2 = new TLine(maxt,0.,maxt,maxy);
  l2->SetLineColor(kRed);
  l2->SetLineStyle(2);
  l1->Draw("same");
  l2->Draw("same");
  cc->SaveAs(".png");
  cc->SaveAs(".pdf");}

  gDirectory->cd("/awpersistency");

  vector<TH1D*> h;
  h.reserve(256);

  vector<TCanvas*> c;
  c.reserve(16);
  
  maxy=0.;
  for(int i=0; i<16; ++i)
    {
      cname=TString::Format("cAWB%02dpersistencyR%d",i,run);
      c[i]=new TCanvas(cname,cname,2100,1700);
      c[i]->Divide(4,4);
      int astart=16*i,pix=1;
      for(int aw=astart; aw<astart+16; ++aw)
	{
	  TString hname=TString::Format("hwfaw%03d",aw);
	  h[aw] = (TH1D*) gROOT->FindObject(hname);
	  //h[aw]->Scale(1./h[aw]->Integral());
	  //h[aw]->Scale(-1.);
	  maxy=maxy>h[aw]->GetBinContent(h[aw]->GetMaximumBin())?maxy:h[aw]->GetBinContent(h[aw]->GetMaximumBin());
	  miny=miny<h[aw]->GetBinContent(h[aw]->GetMinimumBin())?miny:h[aw]->GetBinContent(h[aw]->GetMinimumBin());
	  c[i]->cd(pix);
	  h[aw]->Draw("hist");
	  //h[aw]->SetMinimum(0.);
	  ++pix;
	  //h[aw]->GetXaxis()->SetRangeUser(30.,300.);
	} 
    }
  miny*=1.1;
  maxy*=1.1;
  cout<<miny<<"\t"<<maxy<<endl;

  // //  double ADCdelay = 640.;
  // double ADCdelay = -32.;
  // mint+=ADCdelay;
  // maxt+=ADCdelay;
  // mint/=16.;
  // maxt/=16.;
   for(int i=0; i<16; ++i)
     {
       int astart=16*i,pix=1;
       for(int aw=astart; aw<astart+16; ++aw)
   	{
   	  c[i]->cd(pix);
   	  h[aw]->GetYaxis()->SetRangeUser(miny,maxy);
  // 	  TLine* llow = new TLine(mint,miny,mint,maxy);
  // 	  llow->SetLineColor(kRed);
  // 	  llow->SetLineStyle(2);
  // 	  TLine* lup = new TLine(maxt,miny,maxt,maxy);
  // 	  lup->SetLineColor(kRed);
  // 	  lup->SetLineStyle(2);
  // 	  llow->Draw("same");
  // 	  lup->Draw("same");
   	  ++pix;
   	}
      c[i]->SaveAs(".png");
      c[i]->SaveAs(".pdf");
    }

}
