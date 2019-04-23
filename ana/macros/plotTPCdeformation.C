#include "SignalsType.h"

int GetRunNumber( TString fname )
{
  TRegexp re("[0-9][0-9][0-9][0-9][0-9]");
  int pos = fname.Index(re);
  int run = TString(fname(pos,5)).Atoi();
  return run;
}

void phspectrum( TFile* fin )
{
  //  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  fin->cd();
  int RunNumber = GetRunNumber( fin->GetName() );

  int rowhot = 150, sechot = 7,
    rowcold = 300, seccold = 24;

  padmap pads;
  int hotpad = pads.index(sechot,rowhot),
    coldpad =  pads.index(seccold,rowcold);

  gDirectory->cd("paddeconv/pwbwf");
  TH2D* hPWbAmp = (TH2D*) gROOT->FindObject("hPwbAmp");

  TString hname = TString::Format("hamphotch%d_%d_%d",hotpad,sechot,rowhot);
  TString htitle = TString::Format("Hot Pad;p.h. [ADC]");
  TH1D* hchhotamp = new TH1D(hname,htitle,1000,0.,5100.);

  hname = TString::Format("hampcoldch%d_%d_%d",coldpad,seccold,rowcold);
  htitle = TString::Format("Cold Pad;p.h. [ADC]");
  TH1D* hchcoldamp = new TH1D(hname,htitle,1000,0.,5100.);

  for(int b=1; b<=hPWbAmp->GetNbinsY(); ++b)
    {
      int bin = hPWbAmp->GetBin(hotpad+1,b);
      double bc = hPWbAmp->GetBinContent(bin);
      hchhotamp->SetBinContent(b,bc);

      bin = hPWbAmp->GetBin(coldpad+1,b);
      bc = hPWbAmp->GetBinContent(bin);
      hchcoldamp->SetBinContent(b,bc);
    }

  int rb = 10;
  hchhotamp->Rebin(rb);
  hchcoldamp->Rebin(rb);

  double maxy = hchhotamp->GetBinContent(hchhotamp->GetMaximumBin())>hchcoldamp->GetBinContent(hchcoldamp->GetMaximumBin())?hchhotamp->GetBinContent(hchhotamp->GetMaximumBin()):hchcoldamp->GetBinContent(hchcoldamp->GetMaximumBin());
  maxy*=1.1;
  hchhotamp->GetYaxis()->SetRangeUser(0.,maxy);
  hchcoldamp->GetYaxis()->SetRangeUser(0.,maxy);

  hchhotamp->GetXaxis()->SetRangeUser(0.,4200.);
  hchcoldamp->GetXaxis()->SetRangeUser(0.,4200.);

  TString cname="PadPulseSpectrumR";
  cname += RunNumber;
  TCanvas* c1 = new TCanvas(cname,cname,1800,1600);
  c1->Divide(1,2);
  c1->cd(1);
  hchhotamp->Draw();
  c1->cd(2);
  hchcoldamp->Draw();
  c1->SaveAs(".pdf");
}

void plotTPCdeformation()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  int RunNumber = GetRunNumber( fin->GetName() );

  gDirectory->cd("match_el");
  TH1D* hnm = (TH1D*) gROOT->FindObject("hNmatch");
  double Nevents = (double) hnm->GetEntries();

  gDirectory->cd("/");
  gDirectory->cd("paddeconv");
  
  TH2D* hpadocc = (TH2D*) gROOT->FindObject("hOccPad");
  hpadocc->SetStats(kFALSE);

  if( RunNumber == 4318 || RunNumber == 4335 )
    {
      int bin = hpadocc->GetBin(504,22);
      hpadocc->SetBinContent(bin,0.);
    }

  TH2D* hOF = (TH2D*) gROOT->FindObject("hPadOverflow");
  hOF->SetStats(kFALSE);

  TH2D* hofl = new TH2D("hofl","Number of Overflow/Occupancy Pads;row;sec;N",576,0.,576.,32,0.,32.);
  hofl->SetStats(kFALSE);
  hofl->Divide(hOF,hpadocc);

  TH2D* hscaoverflow = new TH2D("hscaoverflow","Overflow Frequency by AFTER;Along the axis;Along the Circle",16,0.,16.,16,0.,16.);
  hscaoverflow->SetStats(kFALSE);

  TH2D* hscaofl = new TH2D("hscaofl","Number of Overflow/Occupancy AFTER;row;sec;N",16,0.,16.,16,0.,16.);
  hscaofl->SetStats(kFALSE);

  for(int r = 0; r<576; ++r)
    {
      int sca_row = r/36;
      for(int s = 0; s<32; ++s)
	{
	  int sca_col = (s-1)/2;
	  if( s == 0 ) sca_col = 15;
	  int bin = hOF->GetBin(r+1,s+1);
	  double amp = hOF->GetBinContent( bin );
	  hscaoverflow->Fill(sca_row,sca_col,amp);

	  double occ = hpadocc->GetBinContent( bin );
	  if( occ > 0. )
	    {
	      double ratio = amp / occ;
	      hscaofl->Fill(sca_row,sca_col,ratio);
	    }
	}
    }
  
  //gDirectory->cd("paddeconv/pwbwf");
  gDirectory->cd("pwbwf");
  TProfile* p = (TProfile*) gROOT->FindObject("hPwbAmp_prox");
  p->SetStats(kFALSE);
  p->SetMinimum(0.);
  p->SetMaximum(4096.);

  TH2D* hscamp = new TH2D("hscaamp","Average Maximum WF Amplitude by AFTER;Along the axis;Along the Circle",16,0.,16.,16,0.,16.);
  hscamp->SetStats(kFALSE);

  TH2D* hpadamp = new TH2D("hpadamp","Average Maximum WF Amplitude",576,0.,576.,32,0.,32.);
  hpadamp->SetStats(kFALSE);

  padmap pads;
  for(int b=1; b<=p->GetNbinsX(); ++b)
    {
      double amp = p->GetBinContent( b );
      int r,s;
      pads.get(b-1,s,r);
      int sca_row = r/36, sca_col = (s-1)/2;
      if( s == 0 ) sca_col = 15;

      hscamp->Fill(sca_row,sca_col,amp);
      hpadamp->SetBinContent(r+1,s+1,amp);
    }
  hscamp->Scale(1./72.);

  TString cname = "PadOccupancyR";
  cname += RunNumber;
  hpadocc->Scale(1./Nevents);
  TCanvas* c1 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hpadocc->Draw("colz");
  c1->Update();
  TPaletteAxis *pal1 = (TPaletteAxis*) hpadocc->GetListOfFunctions()->FindObject("palette");
  pal1->SetX1NDC(0.91);
  pal1->SetX2NDC(0.92);
  c1->SaveAs(".pdf");
  
  cname = "PadAverageAmpR";
  cname += RunNumber;
  TCanvas* c2 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hpadamp->Draw("colz");
  c2->Update();
  TPaletteAxis *pal2 = (TPaletteAxis*) hpadamp->GetListOfFunctions()->FindObject("palette");
  pal2->SetX1NDC(0.91);
  pal2->SetX2NDC(0.92);
  c2->SaveAs(".pdf");


  cname = "PadOverflowR";
  cname += RunNumber;
  hOF->Scale(1./Nevents);
  TCanvas* c3 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hOF->Draw("colz");
  c3->Update();
  TPaletteAxis *pal3 = (TPaletteAxis*) hOF->GetListOfFunctions()->FindObject("palette");
  pal3->SetX1NDC(0.91);
  pal3->SetX2NDC(0.92);
  c3->SaveAs(".pdf");

  cname = "AFTERampR";
  cname += RunNumber;
  TCanvas* c4 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hscamp->Draw("colz");
  c4->Update();
  TPaletteAxis *pal4 = (TPaletteAxis*) hscamp->GetListOfFunctions()->FindObject("palette");
  pal4->SetX1NDC(0.91);
  pal4->SetX2NDC(0.92);
  c4->SaveAs(".pdf");

  cname = "AFTERoverflowR";
  cname += RunNumber;
  TCanvas* c5 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hscaoverflow->Draw("colz");
  c5->Update();
  TPaletteAxis *pal5 = (TPaletteAxis*) hscaoverflow->GetListOfFunctions()->FindObject("palette");
  pal5->SetX1NDC(0.91);
  pal5->SetX2NDC(0.92);
  c5->SaveAs(".pdf");
  

  cname = "PadAMPR";
  cname += RunNumber;
  TCanvas* c6 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  p->Draw();
  c6->SaveAs(".pdf");

  cname = "PadOverflowOverOccupancyR";
  cname += RunNumber;
  TCanvas* ca = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hofl->Draw("colz");
  ca->Update();
  TPaletteAxis *pala = (TPaletteAxis*) hofl->GetListOfFunctions()->FindObject("palette");
  pala->SetX1NDC(0.91);
  pala->SetX2NDC(0.92);
  ca->SaveAs(".pdf");

 
  
  cname = "AFTEROverflowOverOccupancyR";
  cname += RunNumber;
  TCanvas* cb = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hscaofl->Draw("colz");
  cb->Update();
  TPaletteAxis *palb = (TPaletteAxis*) hscaofl->GetListOfFunctions()->FindObject("palette");
  palb->SetX1NDC(0.91);
  palb->SetX2NDC(0.92);
  cb->SaveAs(".pdf");

  phspectrum(fin);
}
