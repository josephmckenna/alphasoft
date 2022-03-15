#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TLine.h"

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


  gStyle->SetOptStat(11);
  gStyle->SetOptFit(111);

  c1->cd(3);
  haw_prof->Draw();
  haw_prof->Fit("pol1","CF","",1500.,11500.);
  haw_prof->GetXaxis()->SetRangeUser(0.,12000.);
  gPad->Update();
  TPaveStats *staw = (TPaveStats*)haw_prof->FindObject("stats");
  staw->SetX1NDC(0.72);
  staw->SetY1NDC(0.15);
  staw->SetX2NDC(0.97);
  staw->SetY2NDC(0.44);

  TF1* faw=haw_prof->GetFunction("pol1");
  vector<double> ADCthr = {1500,2000,2500};
  vector<double> AWthr;
  vector<TLine*> law;
  for(auto th : ADCthr )
    {
      double y=faw->Eval(th);
      AWthr.push_back( y );
      law.push_back( new TLine(th,0.,th,y) );
      law.push_back( new TLine(0.,y,th,y) );
      cout<<"ADC thr: "<<th<<" aval thr: "<<y<<endl; 
    }

  for(size_t il=0; il<law.size(); ++il)
    {
      law.at(il)->SetLineColor(kBlack);
      law.at(il)->SetLineStyle(2);
      c1->cd(3);
      law.at(il)->Draw("same");
    }

 
  c1->cd(4);
  hpad_prof->Draw();
  hpad_prof->Fit("pol1","CF","",100.,3800.);
  hpad_prof->GetXaxis()->SetRangeUser(0.,3850.);
  gPad->Update();
  TPaveStats *stpa = (TPaveStats*)hpad_prof->FindObject("stats");
  stpa->SetX1NDC(0.2);
  stpa->SetX2NDC(0.45);
  stpa->SetY1NDC(0.59);
  stpa->SetY2NDC(0.88);


  TF1* fpad=hpad_prof->GetFunction("pol1");
  vector<double> PWBthr = {150,200,300};
  vector<double> PADthr;
  vector<TLine*> lpad;
  for(auto th : PWBthr )
    {
      double y=fpad->Eval(th);
      PADthr.push_back( y );
      lpad.push_back( new TLine(th,0.,th,y) );
      lpad.push_back( new TLine(0.,y,th,y) );
      cout<<"PWB thr: "<<th<<" aval thr: "<<y<<endl; 
    }

  for(size_t il=0; il<lpad.size(); ++il)
    {
      lpad.at(il)->SetLineColor(kBlack);
      lpad.at(il)->SetLineStyle(2);
      c1->cd(4);
      lpad.at(il)->Draw("same");
    }

  

  c1->SaveAs(".pdf");  c1->SaveAs(".pdf");


  gDirectory->cd("/awdeconv/adcwf");
  TH2D* hawadc = (TH2D*) gROOT->FindObject("hAdcAmp");
  hawadc->GetXaxis()->SetNdivisions(-16);
  hawadc->GetXaxis()->SetLabelSize(0.02);
  TH1D* hwfadc = (TH1D*) gROOT->FindObject("hAdcWfAmp");
  hwfadc->GetXaxis()->SetNdivisions(32);
  hwfadc->GetXaxis()->SetLabelSize(0.02);

  gDirectory->cd("/paddeconv/pwbwf");
  TH2D* hpadpwb = (TH2D*) gROOT->FindObject("hPwbAmp");
  TH1D* hwfpwb = (TH1D*) gROOT->FindObject("hPwbWfAmp");
  hwfpwb->GetXaxis()->SetNdivisions(32);
  hwfpwb->GetXaxis()->SetLabelSize(0.02);

  cname="cwfampR";
  cname+=run;
  TCanvas* c2=new TCanvas(cname,cname,1900,1700);
  c2->Divide(2,2);
  c2->cd(1);
  gPad->SetLogy();
  gPad->SetGrid();
  hawadc->Draw();
  c2->cd(3);
  gPad->SetLogy();
  gPad->SetGrid();
  hwfadc->Draw();
  c2->cd(2);
  gPad->SetLogy();
  gPad->SetGrid();
  hpadpwb->Draw();
  c2->cd(4);
  gPad->SetLogy();
  gPad->SetGrid();
  hwfpwb->Draw();

  c2->SaveAs(".pdf");  c2->SaveAs(".pdf");

}
