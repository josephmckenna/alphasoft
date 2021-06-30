#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLine.h"

#include <list>
#include <vector>
#include <iostream>
using namespace std;

#include "GetMaxDriftTime.icc"



void adcph()
{
  int runT=3873, 
     runC=4620;
    // runT = 904547, 
    //runT=904648, 
    //runC=4576;
  TString fname=TString::Format("%s/test/cosmics%d.root",getenv("DATADIR"),runT);
  list<TString> file_list {TString::Format("%s/test/cosmics%d.root",getenv("DATADIR"),runT),
      TString::Format("%s/CERN2021/cosmics%d.root",getenv("DATADIR"),runC)};
  //"/daq/alpha_data0/acapra/alphag/CERN2021/cosmics4541.root"     };
  int col[]={kBlue,kRed};
  //TString lname[]={TString::Format("TRIUMF R%d",runT),TString::Format("CERN R%d",runC)};
TString lname[]={TString::Format("CERN R%d",runT),TString::Format("CERN R%d",runC)};

  const int Nfiles=file_list.size();
  int i=0;
  
  vector<TProfile*> awat(Nfiles);
  vector<TLine*> ltime(Nfiles);
  vector<TH1D*> adcamp(Nfiles);
  vector<TProfile*> awamp(Nfiles);

  TString cname=TString::Format("cadcphR%dR%d",runT,runC);
  TCanvas* c1 = new TCanvas(cname.Data(),cname.Data(),1900,2600);
  c1->Divide(1,3);
  TLegend* leg = new TLegend(0.8,0.65,0.99,0.95); 
  double maxy1=0.,maxy2=0.,maxy3=0.;
  double maxt=3800.;

  for(auto &f: file_list)
    {
      TFile* fin=TFile::Open(f,"READ"); 
      if( fin->IsOpen() )
	cout<<fin->GetName()<<endl;
      else
	break;

      gDirectory->cd("/awdeconv");
      
      TString pname=TString::Format("hTimeAmpTop_px%d",i);
      awat[i] = ((TH2D*) gROOT->FindObject("hTimeAmpTop"))->ProfileX(pname);
      //awat[i]->Scale(i+1);
      cout<<awat[i]->GetName()<<"\t"<<awat[i]->GetEntries()<<"\t"<<awat[i]->GetMean()<<endl;
      awat[i]->SetStats(kFALSE);
      // awat[i]->GetXaxis()->SetNdivisions(-16);
      awat[i]->SetLineColor(col[i]); awat[i]->SetMarkerColor(col[i]);
      awat[i]->SetMarkerStyle(8);

      maxy1=maxy1>awat[i]->GetBinContent(awat[i]->GetMaximumBin())?
	maxy1:awat[i]->GetBinContent(awat[i]->GetMaximumBin());
      cout<<"maxy: "<<maxy1<<endl;


      double td=GetMaxDriftTime(awat[i]);
      cout<<i<<" "<<td<<endl;
      TLine* lll = new TLine(td,0.,td,maxy1);
      ltime.push_back(lll);
      ltime.back()->SetLineColor(col[i]);
      ltime.back()->SetLineStyle(2);
      

      gDirectory->cd("/awdeconv/adcwf");

      adcamp[i] = (TH1D*) gROOT->FindObject("hAdcWfAmp");
      adcamp[i]->SetStats(kFALSE);
      //adcamp[i]->GetXaxis()->SetNdivisions(-16);
      adcamp[i]->SetLineColor(col[i]); adcamp[i]->SetMarkerColor(col[i]);
      adcamp[i]->SetMarkerStyle(8);

      maxy2=maxy2>adcamp[i]->GetBinContent(adcamp[i]->GetMaximumBin())?
	maxy2:adcamp[i]->GetBinContent(adcamp[i]->GetMaximumBin());
      cout<<"maxy: "<<maxy2<<endl;


      awamp[i] = (TProfile*) gROOT->FindObject("hAdcAmp_prox");
      awamp[i]->SetStats(kFALSE);
      awamp[i]->GetXaxis()->SetNdivisions(-16);
      awamp[i]->SetLineColor(col[i]); awamp[i]->SetMarkerColor(col[i]);
      awamp[i]->SetMarkerStyle(8);

      maxy3=maxy3>awamp[i]->GetBinContent(awamp[i]->GetMaximumBin())?
	maxy3:awamp[i]->GetBinContent(awamp[i]->GetMaximumBin());
      cout<<"maxy: "<<maxy3<<endl;

	  
      leg->AddEntry(adcamp[i],lname[i],"pl");
	  
      if(i)
	{
	  c1->cd(1);
	  awat[i]->Draw("same");
	  c1->cd(2);
	  adcamp[i]->Draw("same");
	  c1->cd(3);
	  awamp[i]->Draw("same");
	}
      else
	{
	  c1->cd(1);
	  awat[0]->Draw();
	  c1->cd(2);
	  adcamp[0]->Draw();
	  c1->cd(3);
	  awamp[0]->Draw();
	}



      c1->cd(1);
      ltime.back()->Draw("same");
      TString strtd=TString::Format("Max Drift: %1.3fns",td);
      leg->AddEntry(ltime.back(),strtd,"l");

      ++i;
    }

  c1->cd(1);
  // awat[0]->GetXaxis()->SetRangeUser(-160.,maxt);
  awat[0]->GetYaxis()->SetRangeUser(0.,maxy1*1.1);
  awat[0]->GetXaxis()->SetTitle("Drift Time [ns]");
  awat[0]->GetYaxis()->SetTitle("Deconv Amplitude [a.u.]");
  gPad->SetGrid();
  leg->Draw("same");
  
  c1->cd(2);
  //adcamp[0]->GetYaxis()->SetRangeUser(0.,maxy1*1.1);
  adcamp[0]->GetXaxis()->SetTitle("ADC Pulse Height");
  adcamp[0]->GetYaxis()->SetTitle("Number of Hits");
  gPad->SetGrid();
  gPad->SetLogy();
  //leg->Draw("same");

  c1->cd(3);
  awamp[0]->GetYaxis()->SetRangeUser(0.,maxy3*1.1);
  gPad->SetGrid();
  //  leg->Draw("same");

  c1->SaveAs(".pdf"); c1->SaveAs(".pdf");
}
