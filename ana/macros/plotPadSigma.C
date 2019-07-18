#include "SignalsType.h"
padmap pads;

void plotPadSigma()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  gDirectory->cd("padmatch");
  TH2D* hps = (TH2D*) gROOT->FindObject("hcogpadssigma");
  hps->SetStats(kFALSE);
  TString cname = "PadSigmaR"; cname += GetRunNumber(TString(fin->GetName()));
  TCanvas* c1 = new TCanvas(cname,cname,1400,2500);
  c1->Divide(1,3);
  c1->cd(1);
  hps->Draw("colz");

  int binymin = hps->GetYaxis()->FindBin(1.5);
  int binymax = hps->GetYaxis()->FindBin(10.);
  //  int binymax = -1;
  cout<<"Cut on sigma: "<<binymin<<" "<<binymax<<endl;
  TProfile* hpsx = hps->ProfileX("hprofpadsigma",binymin,binymax);
  hpsx->SetMarkerStyle(7);
  hpsx->SetLineColor(kBlack);
  c1->cd(2);
  hpsx->Draw();
  hpsx->Fit("pol0","Q0");
  TF1* favgsig = hpsx->GetFunction("pol0");

  TH1D* hpsmax = new TH1D("hPadMaxSigma","Max Pad Sigma;pad index;#sigma [mm]",32*576,0.,32.*576.);
  TH2D* hPadSigma = new TH2D("hPadSigma","Induced Charge Sigma;row;sec;[mm]",
			     576,-0.5,575.5,32,-0.5,31.5);
  hPadSigma->SetStats(kFALSE);
  for(int bx=1; bx<=hps->GetNbinsX(); ++bx)
    {
      TString hname=TString::Format("%s_%d",hps->GetName(),bx);
      TH1D* h=hps->ProjectionY(hname, bx,bx);
      double max_sigma = h->GetBinCenter(h->GetMaximumBin());
      hpsmax->SetBinContent(bx,max_sigma);
      int row,sec;
      pads.get(bx-1,sec,row);
      if( row == 0 || row == 575 || 
	  (sec==23 && row==384) || 
	  (sec==21 && row==506) ||
	  (sec==12 && row==574) ||
	  (sec==24 && row==384) )// continue;
	max_sigma = favgsig->GetParameter(0);
      int bin = hPadSigma->GetBin(row+1,sec+1);
      hPadSigma->SetBinContent(bin,max_sigma);
    }
  c1->cd(3);
  hpsmax->Draw();
  c1->SaveAs(".pdf");

  cname = "RowColPadSigmaR"; cname += GetRunNumber(TString(fin->GetName()));
  TCanvas* c2 = new TCanvas(cname,cname,1600,1400);
  hPadSigma->Draw("colz");
  c2->Update();
  TPaletteAxis *pal2 = (TPaletteAxis*) hPadSigma->GetListOfFunctions()->FindObject("palette");
  pal2->SetX1NDC(0.91);
  pal2->SetX2NDC(0.92);
  c2->SaveAs(".pdf");


  TH2D* hpa = (TH2D*) gROOT->FindObject("hcogpadsamp");
  hpa->SetStats(kFALSE);
  cname = "PadAmpR"; cname += GetRunNumber(TString(fin->GetName()));
  TCanvas* c3 = new TCanvas(cname,cname,1400,2500);
  c3->Divide(1,2);
  c3->cd(1);
  hpa->Draw("colz");

  binymin = hpa->GetYaxis()->FindBin(50);
  binymax = hpa->GetYaxis()->FindBin(5000.);
  cout<<"Cut on sigma: "<<binymin<<" "<<binymax<<endl;
  TProfile* hpax = hpa->ProfileX("hprofpadamp",binymin,binymax);
  hpax->SetMarkerStyle(7);
  hpax->SetLineColor(kBlack);
  c3->cd(2);
  hpax->Draw();
  c3->SaveAs(".pdf");

  TH2D* hPadQAmp = new TH2D("hPadQAmp","Induced Charge Amplitude;row;sec;[a.u.]",
  			     576,-0.5,575.5,32,-0.5,31.5);
  hPadQAmp->SetStats(kFALSE);
  for(int b=1; b<=hpax->GetNbinsX(); ++b)
    {
      int row,sec;
      pads.get(b-1,sec,row);
      int bin = hPadQAmp->GetBin(row+1,sec+1);
      hPadQAmp->SetBinContent(bin,hpax->GetBinContent(b));
    }
  cname = "RowColPadQAmpR"; cname += GetRunNumber(TString(fin->GetName()));
  TCanvas* c4 = new TCanvas(cname,cname,1600,1400);
  hPadQAmp->Draw("colz");
  c4->Update();
  TPaletteAxis *pal4 = (TPaletteAxis*) hPadQAmp->GetListOfFunctions()->FindObject("palette");
  pal4->SetX1NDC(0.91);
  pal4->SetX2NDC(0.92);
  c4->SaveAs(".pdf");



  TH2D* hpi = (TH2D*) gROOT->FindObject("hcogpadsint");
  hpi->SetStats(kFALSE);
  cname = "PadIntR"; cname += GetRunNumber(TString(fin->GetName()));
  TCanvas* c5 = new TCanvas(cname,cname,1400,2500);
  c5->Divide(1,2);
  c5->cd(1);
  hpa->Draw("colz");

  binymin = hpi->GetYaxis()->FindBin(18);
  //  binymax = hpi->GetYaxis()->FindBin(5000.);
  binymax = -1;
  cout<<"Cut on sigma: "<<binymin<<" "<<binymax<<endl;
  TProfile* hpix = hpi->ProfileX("hprofpadint",binymin,binymax);
  hpix->SetMarkerStyle(7);
  hpix->SetLineColor(kBlack);
  c5->cd(2);
  hpix->Draw();
  c5->SaveAs(".pdf");

  TH2D* hPadQInt = new TH2D("hPadQInt","Induced Charge Integral;row;sec;[a.u.]",
			    576,-0.5,575.5,32,-0.5,31.5);
  hPadQInt->SetStats(kFALSE);
  for(int b=1; b<=hpix->GetNbinsX(); ++b)
    {
      int row,sec;
      pads.get(b-1,sec,row);
      int bin = hPadQInt->GetBin(row+1,sec+1);
      hPadQInt->SetBinContent(bin,hpix->GetBinContent(b));
    }
  cname = "RowColPadQIntR"; cname += GetRunNumber(TString(fin->GetName()));
  TCanvas* c6 = new TCanvas(cname,cname,1600,1400);
  hPadQInt->Draw("colz");
  c6->Update();
  TPaletteAxis *pal6 = (TPaletteAxis*) hPadQInt->GetListOfFunctions()->FindObject("palette");
  pal6->SetX1NDC(0.91);
  pal6->SetX2NDC(0.92);
  c6->SaveAs(".pdf");

  double xmax = 40.; // mm
  cout<<"Z err < "<<xmax<<endl;
  gDirectory->cd("/paddeconv");
  TH1D* hErrPad = (TH1D*) gROOT->FindObject("hErrPad");
  cname = "cErrPad"; cname += GetRunNumber(TString(fin->GetName()));
  TCanvas* c7 = new TCanvas(cname,cname,1200,1000);
  hErrPad->Draw();
  hErrPad->GetXaxis()->SetTitle("Z err [mm]");
  hErrPad->GetXaxis()->SetRangeUser(0.,xmax);
  c7->SaveAs(".pdf");
}
