#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLine.h"

#include <vector>
#include <string>
#include <iostream>
using namespace std;

double garftime=3962.7;//ns

#include "GetMaxDriftTime.icc"

TGraphErrors* gta;
TGraphErrors* gtd;
TH1D* htemp;
TLine* driftline;
TGraph* gtac;
TGraph* gtdc;

vector<double> LoopOverRuns(vector<int> runv, string folder, TGraphErrors* g1, TGraphErrors* g2)
{
  vector<double> timeav(runv.size());
//  vector<double> timedv(runv.size());
  int window=10;
  int n=0;
  for(auto& run: runv)
    {
      TString fname=TString::Format("%s/%s/cosmics%d.root",getenv("DATADIR"),folder.c_str(),run);
      TFile* fin=TFile::Open(fname,"READ");
      if( fin->IsOpen() )
	cout<<fin->GetName()<<endl;
      else
	break;

      gDirectory->cd("/awdeconv");
      
      TString pname=TString::Format("hTimeAmpTop_pxR%d",run);
      TH2D* htamp = (TH2D*) gROOT->FindObject("hTimeAmpTop");
      if( !htamp ) continue;
      TProfile* px = htamp->ProfileX(pname);
      double td=GetMaxDriftTime(px,window);
      timeav.push_back(td);
      g1->SetPoint(n,double(n+0.5),td);
      g1->SetPointError(n,0.,px->GetXaxis()->GetBinWidth(1));


      TH1D* htdrift = (TH1D*) gROOT->FindObject("hTimeTop");
      if( !htdrift ) continue;
      htdrift->Rebin(10);
      td=GetMaxDriftTime(htdrift,window,0.8);
      //      timedv.push_back(td);
      g2->SetPoint(n,double(n+0.5),td);
      g2->SetPointError(n,0.,htdrift->GetXaxis()->GetBinWidth(1));

      ++n;
    }
 
  return timeav;
}

void plot_drift_time_histo()
{

  TString cname="ctestdrifttime";
  TCanvas* ct = new TCanvas(cname,cname,1800,900);
  ct->Divide(1,2);

  ct->cd(1);
  htemp->Draw();
  driftline->Draw("same");
  gta->Draw("Psame");
  //  gtac->Draw("Psame");
  gPad->SetGrid();

  TLegend* leg = new TLegend(0.7,0.13,0.89,0.25);
  leg->AddEntry(driftline,"Garf++ Simulation","l");
  leg->AddEntry(gta,"Data: B=0T w/ sleeve","p");
  leg->Draw("same");


  ct->cd(2);
  htemp->Draw();
  gtd->Draw("Psame");
  //  gtdc->Draw("Psame");
  driftline->Draw("same");
  gPad->SetGrid();

}


void testtime()
{
  //int runlist[]={904690, 904549, 904472, 904554, 904648, 904474, 904685, 904501, 904547, 904555, 904503, 4513,4533,4541};
  int runlist[]={904648,4513,4533,4541,4553,4574,4576};
  vector<int> runv(runlist,runlist+sizeof(runlist)/sizeof(int));
  std::sort(runv.begin(),runv.end());
  int Nruns=runv.size();

  gta = new TGraphErrors(runv.size());
  gta->SetMarkerStyle(20);
  gta->SetMarkerColor(kBlue);

  gtd = new TGraphErrors(runv.size());
  gtd->SetMarkerStyle(20);
  gtd->SetMarkerColor(kRed);


  vector<double> timeav = LoopOverRuns( runv , "test", gta, gtd);
 

  htemp = new TH1D("htemp","Drift Time Vs Run Number;Run Number;Drift Time [ns]",Nruns,0.,double(Nruns));
  htemp->SetStats(kFALSE);
  for(int b=1; b<=htemp->GetNbinsX(); ++b)
    htemp->GetXaxis()->SetBinLabel(b,std::to_string(runv.at(b-1)).c_str());

  driftline = new TLine(0.,garftime,double(Nruns),garftime);
  driftline->SetLineColor(kBlack);
  driftline->SetLineWidth(2);
  
  double maxtd=*std::max_element(timeav.begin(),timeav.end());
  cout<<maxtd<<endl;
  // htemp->GetYaxis()->SetRangeUser(maxtd*0.7,maxtd*1.03);
  //htemp->GetYaxis()->SetRangeUser(0.,maxtd*1.1);
  htemp->GetYaxis()->SetRangeUser(garftime-600.,garftime+600.);


  // int cernlist[]={4513,4533,4541};

  // vector<int> cernruns(cernlist,cernlist+sizeof(cernlist)/sizeof(int));
  // std::sort(cernruns.begin(),cernruns.end());

  // gtac = new TGraph(cernruns.size());
  // gtac->SetMarkerStyle(8);
  // gtac->SetMarkerColor(kGreen);

  // gtdc = new TGraph(cernruns.size());
  // gtdc->SetMarkerStyle(8);
  // gtdc->SetMarkerColor(kMagenta);

  // vector<double> tdcern = LoopOverRuns( cernruns , "CERN2021", gtac, gtdc);

  plot_drift_time_histo();
}
