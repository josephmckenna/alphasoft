#include "IntGetters.h"
#include "TFile.h"
#include "TROOT.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"

void plotBSCocc(double adc_scale=1.0, bool logy=false)
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname = fin->GetName();
  int RunNumber = GetRunNumber( fname );
  cout<<"Run Number: "<<RunNumber<<endl;
 
  gDirectory->cd("/bsc");
  TH1D* hADCocc = (TH1D*) gROOT->FindObject("hBars");
  hADCocc->SetLineColor(kRed);
  cout<<hADCocc->GetName()<<"\t"<<hADCocc->GetEntries()<<endl;
  hADCocc->SetStats(0);
  hADCocc->Scale(adc_scale);
 
  gDirectory->cd("/bsc_tdc_module");
  TH1D* hTDCocc = (TH1D*) gROOT->FindObject("hTdcBar");
  hTDCocc->SetLineColor(kBlue);
  cout<<hTDCocc->GetName()<<"\t"<<hTDCocc->GetEntries()<<endl;
  hTDCocc->SetStats(0);

  TString cname = TString::Format("BSCoccR%d",RunNumber);
  if(logy) cname+="_logy";
  TCanvas* c1 = new TCanvas(cname,cname,1900,1000);
  hADCocc->Draw("hist");
  hTDCocc->Draw("histsame");

  hADCocc->GetXaxis()->SetNdivisions(532);
  
  double maxy = hADCocc->GetBinContent(hADCocc->GetMaximumBin())>hTDCocc->GetBinContent(hTDCocc->GetMaximumBin())?hADCocc->GetBinContent(hADCocc->GetMaximumBin()):hTDCocc->GetBinContent(hTDCocc->GetMaximumBin());
  if(!logy)
    hADCocc->GetYaxis()->SetRangeUser(0.,maxy*1.1);
  else
    hADCocc->GetYaxis()->SetRangeUser(1.,maxy*1.1);
  
  TLegend* leg;
  if( !logy )
    leg = new TLegend(0.83,0.76,0.95,0.89);
  else
    leg = new TLegend(0.13,0.26,0.25,0.39);
  
  TString legname=TString::Format("ADCx%1.0f",adc_scale);
  leg->AddEntry(hADCocc,legname,"l");
  leg->AddEntry(hTDCocc,"TDC","l");
  leg->Draw("same");

  if(logy) c1->SetLogy();

  c1->SetGrid();
  c1->SaveAs(".pdf");
}
