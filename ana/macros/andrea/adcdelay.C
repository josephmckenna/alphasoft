#include <iostream>
#include <cassert>

#include <TFile.h>
#include <TROOT.h>
#include <TMath.h>
#include <TH2D.h>
#include <TRandom2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TF2.h>
#include <TH1.h>
#include <Math/Functor.h>
#include <TPolyLine3D.h>
#include <Math/Vector3D.h>
#include <Fit/Fitter.h>


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
  TH2D *h2;

  SumDistance2(TH2D *h) : h2(h) {}

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
  double operator() (const double *par) 
  {
    assert(h2 != 0);
    int nx = h2->GetNbinsX(),
      ny = h2->GetNbinsY();
    double sum = 0.0;
    for(int bx=1;bx<=nx;++bx)
      {
	double xi=h2->GetXaxis()->GetBinCenter(bx);
	for(int by=1;by<=ny;++by)
	  {
	    double yi=h2->GetYaxis()->GetBinCenter(by);
	    int b = h2->GetBin(bx,by);
	    double bc = h2->GetBinContent(b);
	    double d = distance2(xi,yi,bc,par);
	    sum += d;
	  }
      }
    if (first) 
      {
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
  gDirectory->cd("match_el");

  double tmax=3000.;
   
  TH2D* hawpadsector = (TH2D*)gROOT->FindObject("hawcol_sector_time");
  hawpadsector->GetXaxis()->SetRangeUser(0.,tmax);
  hawpadsector->GetYaxis()->SetRangeUser(0.,tmax);
  hawpadsector->SetStats(kFALSE);


  // ROOT::Fit::Fitter  fitter;


  // // make the functor objet
  // SumDistance2 sdist(hawpadsector);
  // ROOT::Math::Functor fcn(sdist,4);
  // // set the function and the initial parameter values
  // double pStart[4] = {1,1,1,1};
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



  TCanvas* c1 = new TCanvas("adcdelay","adcdelay",1600,1400); 
  //hawpadsector->Draw("surf2");
  hawpadsector->Draw("col");

  // // get fit parameters
  // const double * parFit = result.GetParams();

  // // draw the fitted line
  // int n = 1000;
  // double t0 = 0;
  // double dt = 10;
  // TPolyLine3D *l = new TPolyLine3D(n);
  // for (int i = 0; i <n;++i) 
  //   {
  //     double t = t0+ dt*i/n;
  //     double x,y,z;
  //     line(t,parFit,x,y,z);
  //     z*=10000.;
  //     l->SetPoint(i,x,y,z);
  //     // cout<<i<<"\t"<<x<<"\t"<<y<<"\t"<<z<<endl;
  //   }
  // l->SetLineColor(kRed);
  // l->Draw("same");
}
