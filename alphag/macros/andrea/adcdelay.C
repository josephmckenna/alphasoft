#include <iostream>
#include <cassert>

#include <TFile.h>
#include <TROOT.h>
#include <TMath.h>
#include <TH2D.h>
#include <TGraph2D.h>
#include <TGraph.h>
#include <TRandom2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TF2.h>
#include <TH1.h>
#include <Math/Functor.h>
#include <TPolyLine3D.h>
#include <Math/Vector3D.h>
#include <Fit/Fitter.h>

#include "IntGetters.h"

using namespace ROOT::Math;


// define the parametric line equation
void line(double t, const double *p, double &x, double &y, double &z) 
{
  // a parametric line is define from 6 parameters but 4 are independent
  // x0,y0,z0,z1,y1,z1 which are the coordinates of two points on the line
  // can choose z0 = 0 if line not parallel to x-y plane and z1 = 1;
  x = p[0] + p[1]*t;
  y = p[2] + p[3]*t;
  z = t;
}


bool first = true;
// function Object to be minimized
struct SumDistance2 
{
  // data member of the object
  //TH2D *h2;
  TGraph2D *fGraph;

  //  SumDistance2(TH2D *h) : h2(h) {}
  SumDistance2(TGraph2D *g) : fGraph(g) {}

  // calculate distance line-point
  double distance2(double x,double y,double z, const double *p) 
  {
    // distance line point is D= | (xp-x0) cross  ux |
    // where ux is direction of line and x0 is a point in the line (like t = 0)
    XYZVector xp(x,y,z);
    XYZVector x0(p[0], p[2], 0. );
    XYZVector x1(p[0] + p[1], p[2] + p[3], 1. );
    XYZVector u = (x1-x0).Unit();
    double d2 = ((xp-x0).Cross(u)).Mag2();
    return d2;
  }

  // implementation of the function to be minimized
   double operator() (const double *par) {
      assert(fGraph != 0);
      double * x = fGraph->GetX();
      double * y = fGraph->GetY();
      double * z = fGraph->GetZ();
      int npoints = fGraph->GetN();
      double sum = 0;
      for (int i  = 0; i < npoints; ++i) {
         double d = distance2(x[i],y[i],z[i],par);
         sum += d;
      }
      if (first) {
         std::cout << "Total Initial distance square = " << sum << std::endl;
      }
      first = false;
      return sum;
   }
};


void adcdelay()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  if( !gDirectory->cd("/") ) 
    {
      cout<<"something is wrong, exiting..."<<endl;
      gROOT->ProcessLine(".q");
    }
  int run = GetRunNumber(TString(fin->GetName()));
  cout<<fin->GetName()<<"\t"<<run<<endl;

  gDirectory->cd("match_el");

  double tmax=4500.;
   
  TH2D* hawpadsector = (TH2D*)gROOT->FindObject("hawcol_sector_time");
  hawpadsector->SetStats(kFALSE);
  hawpadsector->RebinX(8);
  hawpadsector->RebinY(8);


  TString cname="adcdelayR";
  cname+=run;
  TCanvas* c1 = new TCanvas(cname,cname,1600,1400); 
  //hawpadsector->Draw("surf2");
  hawpadsector->Draw("col");
  //hawpadsector->Draw();
  hawpadsector->GetXaxis()->SetRangeUser(0.,tmax);
  hawpadsector->GetYaxis()->SetRangeUser(0.,tmax);
  c1->SetGrid();
  c1->Update();

  cname="adcdelay_py_R";
  cname+=run;
  TCanvas* c2 = new TCanvas(cname,cname,1600,1400);
  bool first=true;
  // TGraph2D * grx = new TGraph2D();
  TGraph* ggx = new TGraph();
  ggx->SetName("gprojpad");
  int N=0;  double maxy=0.;
  for(int b=1; b<=hawpadsector->GetNbinsX(); ++b)
    {
      TString hpname=TString::Format("%s_py_%d",hawpadsector->GetName(),b);
      TH1D* hpy = hawpadsector->ProjectionY(hpname,b,b,"e");
      hpy->SetLineColor(kViolet);
      hpy->SetStats(0);
      double taw=hawpadsector->GetXaxis()->GetBinCenter(b);
      int mb = hpy->GetMaximumBin();
      //  double bc = hpy->GetBinContent(mb);
      double tpad = hpy->GetBinCenter(mb);
      //      cout<<b<<"\t"<<hpy->GetEntries()<<"\tt aw: "<<taw<<" t pad: "<<tpad<<"\tmax bin: "<<mb<<" bc: "<<bc<<endl;
      if(hpy->GetEntries()==0) continue;
      //grx->SetPoint(N,taw,tpad,bc);
      ggx->SetPoint(N,taw,tpad);
      ++N;
      c2->cd();
      if( first ) hpy->Draw("hist");
      else hpy->Draw("histsame");
      maxy=maxy>hpy->GetBinContent(hpy->GetMaximumBin())?maxy:hpy->GetBinContent(hpy->GetMaximumBin());
      hpy->GetYaxis()->SetRangeUser(0.,maxy*1.1);
      first=false;
    }

  ggx->SetMarkerStyle(20);
  ggx->SetMarkerColor(kBlack);
  ggx->SetTitle("AW vs. PAD time;AW [ns];PAD [ns]");
  ggx->GetXaxis()->SetRangeUser(0.,tmax);
  ggx->GetYaxis()->SetRangeUser(0.,tmax);
  cname="adcdelay_graph_R";
  cname+=run;
  TCanvas* c3 =  new TCanvas(cname,cname,1600,1400); 
  c3->cd();
  ggx->Draw("AP");
  ggx->Fit("pol1","MCF0","",800.,3800.);
  TF1* fgx=ggx->GetFunction("pol1");
  fgx->SetLineColor(kRed);
  fgx->Draw("same");

  cname="adcdelay_px_R";
  cname+=run;
  TCanvas* c5 = new TCanvas(cname,cname,1600,1400);
  first=true;
  // TGraph2D * gry = new TGraph2D();
  TGraph* ggy = new TGraph();
  ggy->SetName("gprojaw");
  N=0;  maxy=0.;
  for(int b=1; b<=hawpadsector->GetNbinsY(); ++b)
    {
      TString hpname=TString::Format("%s_px_%d",hawpadsector->GetName(),b);
      TH1D* hpx = hawpadsector->ProjectionX(hpname,b,b,"e");
      hpx->SetLineColor(kOrange);
      double tpad=hawpadsector->GetYaxis()->GetBinCenter(b);
      hpx->SetAxisRange(17.,tmax);
      int mb = hpx->GetMaximumBin();
      //double bc = hpx->GetBinContent(mb);
      double taw = hpx->GetBinCenter(mb);
      //      cout<<b<<"\t"<<hpx->GetEntries()<<"\tt aw: "<<taw<<" t pad: "<<tpad<<"\tmax bin: "<<mb<<" bc: "<<bc<<endl;
      if(hpx->GetEntries()==0) continue;
 
      ggy->SetPoint(N,taw,tpad);
      ++N;
      c5->cd();
      if( first ) hpx->Draw("hist");
      else hpx->Draw("histsame");
      maxy=maxy>hpx->GetBinContent(hpx->GetMaximumBin())?maxy:hpx->GetBinContent(hpx->GetMaximumBin());
      hpx->GetYaxis()->SetRangeUser(0.,maxy*1.1);
      first=false;
    }

  ggy->SetMarkerStyle(20);
  ggy->SetMarkerColor(kBlue);
  ggy->SetTitle("AW vs. PAD time;AW [ns];PAD [ns]");
 
  c3->cd();
  ggy->Draw("Psame");
  ggy->Fit("pol1","MCF0","",800.,3800.);
  TF1* fgy = ggy->GetFunction("pol1");
  fgy->SetLineColor(kGreen);
  fgy->Draw("same");
  c3->SetGrid();

  TLegend* leg = new TLegend(0.7,0.29,0.87,0.49);
  leg->AddEntry(ggx,"proj pad","p");
  leg->AddEntry(fgx," fit pad","l");
  leg->AddEntry(ggy,"proj aw","p");
  leg->AddEntry(fgy,"fit aw","l");
  c3->cd();
  leg->Draw("same");
  

  c1->SaveAs(".pdf");   c1->SaveAs(".pdf");
  c2->SaveAs(".pdf");   c2->SaveAs(".pdf");
  c3->SaveAs(".pdf");   c3->SaveAs(".pdf");
  c5->SaveAs(".pdf");   c5->SaveAs(".pdf");
}


//void FitGraph2d(){
  // TString cname="adcdelay_fit_R";
  // cname+=run;
  // TCanvas* c4 =  new TCanvas(cname,cname,1600,1400); 
  // grx->Draw();

  // ROOT::Fit::Fitter  fitter;
  // // make the functor object
  // SumDistance2 sdist(grx);
  // ROOT::Math::Functor fcn(sdist,4);
  // // set the function and the initial parameter values
  // double pStart[4] = {0.,1.,0.,1.};
  // fitter.SetFCN(fcn,pStart);
  // // set step sizes different than default ones (0.3 times parameter values)
  // for (int i = 0; i < 4; ++i) fitter.Config().ParSettings(i).SetStepSize(0.01);

  // bool ok = fitter.FitFCN();
  // if (!ok) {
  //   Error("line3Dfit","Line3D Fit failed");
  //   // return 1;
  // }

  // const ROOT::Fit::FitResult & result = fitter.Result();

  // std::cout << "Total final distance square " << result.MinFcnValue() << std::endl;
  // result.Print(std::cout);
  
  // // get fit parameters
  // const double * parFit = result.GetParams();

  // // draw the fitted line
  // int n = 1000;
  // double t0 = 0.;
  // double dt = 10.;
  // TPolyLine3D *l = new TPolyLine3D(n);
  // for (int i = 0; i <n;++i) 
  //   {
  //     double t = t0 + dt*i/n;
  //     double x,y,z;
  //     line(t,parFit,x,y,z);
  //     l->SetPoint(i,x,y,z);
  //     // cout<<i<<"\t"<<x<<"\t"<<y<<"\t"<<z<<endl;
  //   }
  // l->SetLineColor(kRed);
  // c4->cd();
  // l->Draw("same");
  //  c4->SaveAs(".pdf");   c4->SaveAs(".pdf");
  //}
