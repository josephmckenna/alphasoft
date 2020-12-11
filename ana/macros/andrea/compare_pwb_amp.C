#include <TString.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TStyle.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TPaletteAxis.h>
#include <TPaveStats.h>

#include "padplot.C"

void compare_pwb_amp()
{
  //int col[]={kBlack,kRed,kGreen,kBlue};
  //int run[]={38739,904026,904133,904139};
  //TString lab[]={"CERN data","AW3.2kV,low-CO2","AW3.1kV","AW3.2kV"};

  //int col[]={kRed,kBlue};
  //int run[]={38739,904353};
  //TString lab[]={"2018 data (CERN)","2020 data (TRIUMF)"};

  // int col[]={kBlack,kRed,kBlue};
  // int run[]={38739,904353,904275};
  // TString lab[]={"CERN data","AW3.2kV","AW3.1kV"};

  // int col[]={kRed,kBlue,kBlack};
  // int run[]={903916,904014,904353};
  // TString lab[]={"horiz. rotation", "horiz. all PWBs", "vert."};

  int col[]={kRed,kBlack};
  int run[]={38739,904503};
  TString lab[]={"CERN AW @3.1kV","TRIUMF AW @3.2kV"};


  int Nfiles=sizeof(run)/sizeof(int);
  cout<<"Number of files: "<<Nfiles<<endl;

  TProfile* pamp[Nfiles];
  TH1D* pocc[Nfiles];
  TH1D* pspp[Nfiles];
  TH1D* pspt[Nfiles];

  TString cname=TString::Format("ccompareamp");
  TCanvas* c1 = new TCanvas(cname.Data(),cname.Data(),1900,1000);
  cname=TString::Format("ccompareocc");
  TCanvas* c2 = new TCanvas(cname.Data(),cname.Data(),1900,1000);
  cname=TString::Format("ccomparepad");
  TCanvas* c3 = new TCanvas(cname.Data(),cname.Data(),1900,1000);
  cname=TString::Format("ccomparespt");
  TCanvas* c4 = new TCanvas(cname.Data(),cname.Data(),1900,1000);
  TLegend* leg = new TLegend(0.7,0.7,0.9,0.9);
  TLegend* leg2= new TLegend(0.41,0.17,0.61,0.39);
  double max4,max3,max2; max4=max3=max2=0.;
  for( int i=0; i<Nfiles; ++i )
    {
      TString fname=TString::Format("%s/agmini/cosmics%d.root",
				    getenv("DATADIR"),run[i]);
      cout<<fname<<endl;
      TFile* fin=TFile::Open(fname,"READ");

      plot_spacepoints(fin);

      pspp[i]=hsppad->ProjectionX();
      pspp[i]->Scale(1./pspp[i]->Integral());
      pspp[i]->SetStats(kFALSE);
      pspp[i]->SetMinimum(0.);
      pspp[i]->SetLineColor(col[i]);
      pspp[i]->SetMarkerColor(col[i]);
      c3->cd();
      if(i) pspp[i]->Draw("same");
      else  pspp[i]->Draw();
      max3=max3>pspp[i]->GetBinContent(pspp[i]->GetMaximumBin())?max3:pspp[i]->GetBinContent(pspp[i]->GetMaximumBin());

      pspt[i]=hspzp->ProjectionX();
      pspt[i]->Scale(1.e2/pspt[i]->Integral());
      pspt[i]->SetStats(kFALSE);
      pspt[i]->SetMinimum(0.);
      pspt[i]->SetLineColor(col[i]);
      pspt[i]->SetMarkerColor(col[i]);
      c4->cd();
      if(i) pspt[i]->Draw("same");
      else  pspt[i]->Draw();
      max4=max4>pspt[i]->GetBinContent(pspt[i]->GetMaximumBin())?max4:pspt[i]->GetBinContent(pspt[i]->GetMaximumBin());


      gDirectory->cd("paddeconv");
      TH2D* hocc = (TH2D*)gROOT->FindObject("hOccPad");
      
      //pocc[i] = hocc->ProfileX();
      pocc[i] = hocc->ProjectionX();
      pocc[i]->Scale(1.e2/pocc[i]->Integral());
      pocc[i]->SetStats(kFALSE);
      pocc[i]->SetMinimum(0.);
      pocc[i]->SetLineColor(col[i]);
      pocc[i]->SetMarkerColor(col[i]);
      c2->cd();
      if(i) pocc[i]->Draw("same");
      else  pocc[i]->Draw();
      max2=max2>pocc[i]->GetBinContent(pocc[i]->GetMaximumBin())?max2:pocc[i]->GetBinContent(pocc[i]->GetMaximumBin());

      gDirectory->cd("pwbwf");
      pamp[i] = (TProfile*) gROOT->FindObject("hPwbAmp_prox");
      pamp[i]->SetStats(kFALSE);
      pamp[i]->SetMinimum(0.);
      pamp[i]->SetMaximum(4096.);
      pamp[i]->SetLineColor(col[i]);
      pamp[i]->SetMarkerColor(col[i]);
      c1->cd();
      if(i) pamp[i]->Draw("same");
      else  pamp[i]->Draw();

      //TString lname=TString::Format("Run %d ",run[i]);
      //lname+=lab[i];
      TString lname(lab[i]);
      leg->AddEntry(pamp[i],lname,"pl");
      leg2->AddEntry(pamp[i],lname,"pl");
    }
  c1->cd();
  leg->Draw("same");

  c2->cd();
  TString htitle=pocc[0]->GetTitle();
  htitle+=" Averaged over sec";
  pocc[0]->SetTitle(htitle);
  pocc[0]->GetYaxis()->SetTitle("Fraction [%] over dataset");
  pocc[0]->GetYaxis()->SetRangeUser(0.,max2*1.1);
  leg2->Draw("same");

  c3->cd();
  htitle=pspp[0]->GetTitle();
  htitle+=" Averaged over sec";
  pspp[0]->SetTitle(htitle);
  pspp[0]->GetYaxis()->SetTitle("Fraction [%] over dataset");
  pspp[0]->GetYaxis()->SetRangeUser(0.,max3*1.1);
  leg2->Draw("same");

  c4->cd();
  htitle=pspt[0]->GetTitle();
  htitle+=" Averaged over #phi";
  pspt[0]->SetTitle(htitle);
  pspt[0]->GetYaxis()->SetTitle("Fraction [%] over dataset");
  pspt[0]->GetYaxis()->SetRangeUser(0.,max4*1.1);
  leg2->Draw("same");

  c1->SaveAs(".pdf"); c1->SaveAs(".pdf");
  c2->SaveAs(".pdf"); c2->SaveAs(".pdf");
  c3->SaveAs(".pdf"); c3->SaveAs(".pdf");
  c4->SaveAs(".pdf"); c4->SaveAs(".pdf");
}
