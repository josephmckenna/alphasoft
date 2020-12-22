#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TPaveStats.h"

#include <iostream>
#include <iomanip>
using namespace std;

#include "IntGetters.h"


void AmpProf()
{
  // int col[]={kBlue,kRed};
  int col[]={kBlue,kRed,kGreen,kBlack};

  int run[][4]={{904547,904555,904562,904565}};

  TString lab[][4]={{"VT-sleeve-AW3.2kV","HZ-sleeve-AW3.2kV",
		     "HZ-AW3.2kV","HZ-AW3.2kV"}};
  TString tag[]={"AW3.2kV"};

  // TString tag[]={"HZ","VT","AW3.2kV","AW3.1kV"};
  // int run[][2]={{904555,904554},{904547,904549},{904547,904555},{904549,904554}};
  // TString lab[][2]={{"horiz. sleeve AW@3.2kV","horiz. sleeve AW@3.1kV"},
  // 		    {"vert. sleeve AW@3.2kV","vert. sleeve AW@3.1kV"},
  // 		    {"vert. sleeve AW@3.2kV","horiz. sleeve AW@3.2kV"},
  // 		    {"vert. sleeve AW@3.1kV","horiz. sleeve AW@3.1kV"}};

  int Ntags=sizeof(tag)/sizeof(TString);
  cout<<"Number of comparisons: "<<Ntags<<endl;

  for( int n=0; n<Ntags; ++n )
    {
      int Nfiles=sizeof(run[n])/sizeof(int);
      //     int Nfiles=2;
      cout<<"Number of files: "<<Nfiles<<endl;

      vector<TProfile*> padc(Nfiles);
      vector<TProfile*> ppwb(Nfiles);

      TString cname="cadcprof";
      cname+=tag[n];
      TCanvas* c1 = new TCanvas(cname.Data(),cname.Data(),1900,1000);
      //  c1->Divide(1,2);
      TLegend* leg = new TLegend(0.8,0.8,0.99,0.95); 
      double maxy=0.;
      for( int i=0; i<Nfiles; ++i )
	{
	  TString fname=TString::Format("%s/test/cosmics%d.root",
					getenv("DATADIR"),run[n][i]);
	  cout<<fname<<endl;
	  TFile* fin=TFile::Open(fname,"READ");
  
	  gDirectory->cd("/awdeconv/adcwf");
	  padc[i] = (TProfile*) gROOT->FindObject("hAdcAmp_prox");
	  padc[i]->SetStats(kFALSE);
	  padc[i]->GetXaxis()->SetNdivisions(-16);
	  padc[i]->SetLineColor(col[i]); padc[i]->SetMarkerColor(col[i]);
	  padc[i]->SetMarkerStyle(20);
	  
	  gDirectory->cd("/paddeconv/pwbwf");
	  ppwb[i] = (TProfile*) gROOT->FindObject("hPwbAmp_prox");
	  ppwb[i]->SetStats(kFALSE);      
	  ppwb[i]->SetLineColor(col[i]); ppwb[i]->SetMarkerColor(col[i]);
	  ppwb[i]->SetMarkerStyle(20);
	  
	  maxy=maxy>padc[i]->GetBinContent(padc[i]->GetMaximumBin())?
	    maxy:padc[i]->GetBinContent(padc[i]->GetMaximumBin());
	  
	  TString lname=TString::Format("R%d %s",run[n][i],lab[n][i].Data());
	  leg->AddEntry(padc[i],lname,"pl");
	  
	  if(i)
	    {
	      //c1->cd(1);
	      padc[i]->Draw("same");
	      // c1->cd(2);
	      // ppwb[i]->Draw("same");
	    }
	  else
	    {
	      //c1->cd(1);
	      padc[0]->Draw();
	      // c1->cd(2);
	      // ppwb[0]->Draw();
	    }
	}
      c1->cd(1);
      padc[0]->GetYaxis()->SetRangeUser(0.,maxy*1.1);
      gPad->SetGrid();
      leg->Draw("same");

      c1->SaveAs(".pdf"); c1->SaveAs(".pdf");
    }
}
