#include <iostream>
#include "TFile.h"
#include "TROOT.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TString.h"


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

  // TProfile* hpx = hawpadsector->ProfileX();
  // TProfile* hpy = hawpadsector->ProfileY();

  // TGraph* gawpadsector = new TGraph();
  // int n=0;
  // for(int bx=1; bx<=hawpadsector->GetNbinsX(); ++bx)
  //   {
  //     for(int by=1; by<=hawpadsector->GetNbinsY(); ++by)
  // 	{
  // 	  int bb = hawpadsector->GetBin(bx,by);
  // 	  double x = hawpadsector->GetXaxis()->GetBinCenter(bb);
  // 	  double y = hawpadsector->GetYaxis()->GetBinCenter(bb);
  // 	  double bc = hawpadsector->GetBinContent(bb);
  // 	  cout<<bb<<"\t"<<bx<<"\t"<<x<<"\t"<<by<<"\t"<<y<<"\t"<<bc<<endl;
  // 	  for(int i=0; i<int(bc); ++i)
  // 	    gawpadsector->SetPoint(n++,x,y);
  // 	}
  //   }
 
  // // TGraph* gawpadsector = (TGraph*) gROOT->FindObject("gawcol_sector_time");
  // gawpadsector->SetMarkerStyle(8);
  // gawpadsector->SetMarkerColor(kBlack);
  
  // TH2D* hawpadmatch = (TH2D*)gROOT->FindObject("hawcol_match_time");
  // hawpadmatch->GetXaxis()->SetRangeUser(0.,tmax);
  // hawpadmatch->GetYaxis()->SetRangeUser(0.,tmax); 
  // TH2D* htime = (TH2D*)gROOT->FindObject("hawcol_time");
  // htime->GetXaxis()->SetRangeUser(0.,tmax);
  // htime->GetYaxis()->SetRangeUser(0.,tmax);


  TCanvas* c1 = new TCanvas("adcdelay","adcdelay",1600,1400);
  //  c1->Divide(2,2);
  //  c1->cd(1);
  hawpadsector->Draw();
  // c1->cd(2);
  // hpy->Draw();
  // c1->cd(3);
  // hpx->Draw();
  // c1->cd(4);
  // gawpadsector->Draw("AP");

  // c1->cd(3);
  // hawpadmatch->Draw();
  // c1->cd(4);
  // htime->Draw();

}
