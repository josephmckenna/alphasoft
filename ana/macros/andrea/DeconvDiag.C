#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPaveStats.h"

#include <iostream>
#include <iomanip>
using namespace std;

#include "IntGetters.h"

void DeconvDiag()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  int run = GetRunNumber(TString(fin->GetName()));
  cout<<fin->GetName()<<"\t"<<run<<endl;
  gDirectory->cd("/match_el");
  TH2D* haw = (TH2D*) gROOT->FindObject("hAmpAwDeconv");
  TH2D* hpad = (TH2D*) gROOT->FindObject("hAmpPadDeconv");

  TProfile* haw_prof=haw->ProfileX();
  TProfile* hpad_prof=hpad->ProfileX();

  TString cname="cdeconvdR";
  cname+=run;
  TCanvas* c1=new TCanvas(cname,cname,1900,1700);
  c1->Divide(2,2);
  c1->cd(1);
  haw->Draw("colz");
  c1->cd(2);
  hpad->Draw("colz");
  // c1->cd(3);
  // haw->ProjectionX()->Draw();
  // c1->cd(4);
  // hpad->ProjectionX()->Draw();

  gStyle->SetOptStat(11);
  gStyle->SetOptFit(111);

  c1->cd(3);
  haw_prof->Draw();
  haw_prof->Fit("pol1","CF","",1500.,11500.);
  gPad->Update();
  TPaveStats *staw = (TPaveStats*)haw_prof->FindObject("stats");
  staw->SetX1NDC(0.72);
  staw->SetY1NDC(0.15);
  staw->SetX2NDC(0.97);
  staw->SetY2NDC(0.44);
 
  c1->cd(4);
  hpad_prof->Draw();
  hpad_prof->Fit("pol1","CF","",100.,3800.);
  gPad->Update();
  TPaveStats *stpa = (TPaveStats*)hpad_prof->FindObject("stats");
  stpa->SetX1NDC(0.2);
  stpa->SetX2NDC(0.45);
  stpa->SetY1NDC(0.59);
  stpa->SetY2NDC(0.88);

  c1->SaveAs(".pdf");  c1->SaveAs(".pdf");
}
