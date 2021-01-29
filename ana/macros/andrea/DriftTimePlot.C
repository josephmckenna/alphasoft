#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TCanvas.h"

#include <iostream>
#include <iomanip>
using namespace std;

#include "IntGetters.h"
void DriftTimePlot()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  int run = GetRunNumber(TString(fin->GetName()));
  cout<<fin->GetName()<<"\t"<<run<<endl;

  fin->cd("/awdeconv");
  TH1D* htt = (TH1D*)gROOT->FindObject("hTimeTop");
  htt->SetStats(kFALSE);
  htt->SetLineColor(kOrange);
  htt->SetLineWidth(2);
  htt->SetTitle("Drift Time Spectrum after Deconvolution");
  htt->Rebin(32);
  
  fin->cd("/paddeconv");
  TH1D* htpad = (TH1D*)gROOT->FindObject("hTimePad");
  htpad->SetStats(kFALSE);
  htpad->SetLineColor(kBlack);
  htpad->SetLineWidth(2);
  htpad->Rebin(32);

  TString cname = "deconvDriftTimeR";
  cname+=run;
  TCanvas* cdec = new TCanvas(cname,cname,1000,600);
  htt->Draw();
  htpad->Draw("same");
  //htt->GetXaxis()->SetRangeUser(1000.,2500.);
  //htpad->Draw();
  //htpad->GetXaxis()->SetRangeUser(1000.,2500.);

  TLegend* legdec = new TLegend(0.65,0.62,0.84,0.73);
  legdec->AddEntry(htt,"AW","l");
  legdec->AddEntry(htpad,"pad","l");
  legdec->Draw("same");

  cdec->SaveAs(".pdf"); cdec->SaveAs(".pdf");
}
