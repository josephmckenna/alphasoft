#include "SignalsType.h"
padmap pads;
int rowhot = 150, sechot = 7,
  rowcold = 300, seccold = 24;

int RunNumber=0;

bool saveas=false;

std::map<std::pair<int,int>,int> padmap;
char sca[2][2] = {{'A','D'},{'B','C'}};
std::map<int,std::pair<string,string>> robin;

void ReadMap()
{
  TString fname = TString::Format("%s/ana/pad.map",getenv("AGRELEASE"));
  ifstream f(fname.Data());
  int s,r,m;
  while(1)
    {
      f>>s>>r>>m;
      if( !f.good() ) break;
      pair<int,int> p = std::make_pair(s,r);
      padmap[p]=m;
    }
  f.close();
}

string GetPWB(int sec, int row)
{
  pair<int,int> p = std::make_pair(sec,row);
  int pwb = padmap[p];
  string out("PWB");
  if( pwb < 10 ) out+='0';    
  out+=std::to_string(pwb);
  return out;
}

string GetSCA(int sec, int row)
{
  int x = int(row/36)%2;
  int s=sec-1;
  if(s<0) s=31;
  int y=int(s/2)%2;
  string out("SCA");
  out+=sca[x][y];
  return out;
}


void phspectrum( TFile* fin )
{
  fin->cd();

  int hotpad = pads.index(sechot,rowhot),
    coldpad =  pads.index(seccold,rowcold);

  gDirectory->cd("paddeconv/pwbwf");
  TH2D* hPWbAmp = (TH2D*) gROOT->FindObject("hPwbAmp");

  TString hname = TString::Format("hamphotch%d_%d_%d",hotpad,sechot,rowhot);
  TString htitle = TString::Format("Hot Pad;p.h. [ADC]");
  TH1D* hchhotamp = new TH1D(hname,htitle,1000,0.,4200.);

  hname = TString::Format("hampcoldch%d_%d_%d",coldpad,seccold,rowcold);
  htitle = TString::Format("Cold Pad;p.h. [ADC]");
  TH1D* hchcoldamp = new TH1D(hname,htitle,1000,0.,4200.);

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

  // hchhotamp->GetXaxis()->SetRangeUser(0.,4200.);
  // hchcoldamp->GetXaxis()->SetRangeUser(0.,4200.);

  TString cname="PadPulseSpectrumR";
  cname += RunNumber;
  TCanvas* c1 = new TCanvas(cname,cname,1800,1600);
  c1->Divide(1,2);
  c1->cd(1);
  hchhotamp->Draw();
  c1->cd(2);
  hchcoldamp->Draw();
  if(saveas) c1->SaveAs(".pdf");
}

void phspectrum_tracks( TFile* fin )
{
  gStyle->SetOptStat("nem");

  int hotpad = pads.index(sechot,rowhot),
    coldpad =  pads.index(seccold,rowcold);

  fin->cd();
  gDirectory->cd("phspectrum");
  TH2D* hpwbphspect = (TH2D*) gDirectory->Get("hpwbphspect");
  TString hname = TString::Format("hphspecthotpad%d_%d_%d",hotpad,sechot,rowhot);
  TH1D* hhotphspect = hpwbphspect->ProjectionY(hname,hotpad+1,hotpad+1);
  hhotphspect->SetTitle("Hot Pad Spectrum for tracks;p.h. [ADC]");

  hhotphspect->Rebin(10);
  hname = TString::Format("hphspectcoldpad%d_%d_%d",coldpad,seccold,rowcold);
  TH1D* hcoldphspect = hpwbphspect->ProjectionY(hname,coldpad+1,coldpad+1);
  hcoldphspect->SetTitle("Cold Pad Spectrum for tracks;p.h. [ADC]");
  hcoldphspect->Rebin(10);

  double maxy = hhotphspect->GetBinContent(hhotphspect->GetMaximumBin())>hcoldphspect->GetBinContent(hcoldphspect->GetMaximumBin())?hhotphspect->GetBinContent(hhotphspect->GetMaximumBin()):hcoldphspect->GetBinContent(hcoldphspect->GetMaximumBin());
  maxy*=1.1;
  hhotphspect->GetYaxis()->SetRangeUser(0.,maxy);
  hcoldphspect->GetYaxis()->SetRangeUser(0.,maxy);

  TString cname="PadTracksPHSpectrumR";
  cname += RunNumber;
  TCanvas* c1 = new TCanvas(cname,cname,1800,1600);
  c1->Divide(1,2);
  c1->cd(1);
  hhotphspect->Draw();
  c1->cd(2);
  hcoldphspect->Draw();
  if(saveas) c1->SaveAs(".pdf");

  int bin = hhotphspect->FindBin(4096);
  cout<<"Entries hot pad: "<<hhotphspect->Integral(1,bin-1)<<endl;
  cout<<"Entries hot pad overflow: "<<hhotphspect->Integral(bin,bin+1)<<"\t"<<hhotphspect->GetBinContent(bin)<<endl;
  bin = hcoldphspect->FindBin(4096);
  cout<<"Entries cold pad: "<<hcoldphspect->Integral(1,bin-1)<<endl;
  cout<<"Entries cold pad overflow: "<<hcoldphspect->Integral(bin,bin+1)<<"\t"<<hcoldphspect->GetBinContent(bin)<<endl;

  TH2D* hadcphspect = (TH2D*) gDirectory->Get("hadcphspect");
  int hotsecaw = sechot*8+3;
  hname = TString::Format("hphspecthotaw%d_%d",hotsecaw,sechot);
  TH1D* hhotsec = hadcphspect->ProjectionY(hname,hotsecaw+1,hotsecaw+1);
  hhotsec->SetTitle("AW in Hot Pad Sector Spectrum for tracks;p.h. [ADC]");
  hhotsec->Rebin(10);
  int coldsecaw = seccold*8+3;
  hname = TString::Format("hphspectcoldaw%d_%d",coldsecaw,seccold);
  TH1D* hcoldsec = hadcphspect->ProjectionY(hname,coldsecaw+1,coldsecaw+1);
  hcoldsec->SetTitle("AW in Cold Pad Sector Spectrum for tracks;p.h. [ADC]");
  hcoldsec->Rebin(10);

  maxy = hhotsec->GetBinContent(hhotsec->GetMaximumBin())>hcoldsec->GetBinContent(hcoldsec->GetMaximumBin())?hhotsec->GetBinContent(hhotsec->GetMaximumBin()):hcoldsec->GetBinContent(hcoldsec->GetMaximumBin());
  maxy*=1.1;
  hhotsec->GetYaxis()->SetRangeUser(0.,maxy);
  hcoldsec->GetYaxis()->SetRangeUser(0.,maxy);

  cname="AWTracksPHSpectrumR";
  cname += RunNumber;
  TCanvas* c2 = new TCanvas(cname,cname,1800,1600);
  c2->Divide(1,2);
  c2->cd(1);
  hhotsec->Draw();
  c2->cd(2);
  hcoldsec->Draw();
  if(saveas) c2->SaveAs(".pdf");

  bin = hhotsec->FindBin(16384);
  cout<<"Entries hot pad sec aw: "<<hhotsec->Integral(1,bin-1)<<endl;
  cout<<"Entries hot pad sec aw overflow: "<<hhotsec->Integral(bin,bin+1)<<"\t"<<hhotsec->GetBinContent(bin)<<endl;
  bin = hcoldsec->FindBin(16384);
  cout<<"Entries cold pad sec aw: "<<hcoldsec->Integral(1,bin-1)<<endl;
  cout<<"Entries cold pad sec aw overflow: "<<hcoldsec->Integral(bin,bin+1)<<"\t"<<hcoldsec->GetBinContent(bin)<<endl;
}

void GainCorrection(TH2D* hsca)
{
  int minbin = hsca->GetMinimumBin();
  int maxbin = hsca->GetMaximumBin();
  cout<<"min bin: "<<minbin<<"\tmax bin: "<<maxbin<<endl;

  int minbinx,minbiny,maxbinx,maxbiny,dummy;
  hsca->GetBinXYZ(minbin,minbinx,minbiny,dummy);
  hsca->GetBinXYZ(maxbin,maxbinx,maxbiny,dummy);
  cout<<"min bin (x,y): "<<minbinx<<","<<minbiny<<"\tmax bin (x,y): "<<maxbinx<<","<<maxbiny<<endl;
  double min = hsca->GetBinContent(minbin);
  double max = hsca->GetBinContent(maxbin);
  cout<<"min: "<<min<<"\tmax: "<<max<<endl;

  TH2D* hsca_ratio = (TH2D*) hsca->Clone();
  hsca_ratio->SetTitle("Possible SCA Gain Assignment");
  
  for(int bx=1; bx<=hsca->GetNbinsX(); ++bx)
    for(int by=1; by<=hsca->GetNbinsY(); ++by)
      {
	double bc = hsca_ratio->GetBinContent(bx,by);
	double sca_gain = round(bc/min);
	hsca_ratio->SetBinContent(bx,by,sca_gain);
	//	  hsca_ratio->SetBinContent(bx,by,(bc-min)/min);
	int bin = hsca_ratio->GetBin(bx,by);
	cout<<robin[bin].first<<"\t"<<robin[bin].second<<"\t"<<sca_gain<<endl;
      }
  
  TString cname = "AFTERamptextR";
  cname += RunNumber;
  TCanvas* c1 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hsca->Draw("coltext");
  c1->SetGrid();
  c1->SaveAs(".pdf");

  cname = "AFTERratioR";
  cname += RunNumber;
  TCanvas* c2 = new TCanvas(cname.Data(),cname.Data(),1900,1200);
  hsca_ratio->Draw("textcol");
  c2->SetGrid();
  c2->SaveAs(".pdf");
}

void deformation(TFile* fin)
{
  fin->cd();

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

  for(int b=1; b<=p->GetNbinsX(); ++b)
    {
      double amp = p->GetBinContent( b );
      int r,s;
      pads.get(b-1,s,r);
      int sca_row = r/36, sca_col = (s-1)/2;
      if( s == 0 ) sca_col = 15;

      int bin = hscamp->FindBin(sca_row,sca_col);
      robin[bin]=std::make_pair(GetPWB(s,r),GetSCA(s,r));

      hscamp->Fill(sca_row,sca_col,amp);
      hpadamp->SetBinContent(r+1,s+1,amp);
    }
  hscamp->Scale(1./72.);
  GainCorrection( hscamp );

  TString cname = "PadOccupancyR";
  cname += RunNumber;
  hpadocc->Scale(1./Nevents);
  TCanvas* c1 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hpadocc->Draw("colz");
  c1->Update();
  TPaletteAxis *pal1 = (TPaletteAxis*) hpadocc->GetListOfFunctions()->FindObject("palette");
  pal1->SetX1NDC(0.91);
  pal1->SetX2NDC(0.92);
  if(saveas) c1->SaveAs(".pdf");
  
  cname = "PadAverageAmpR";
  cname += RunNumber;
  TCanvas* c2 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hpadamp->Draw("colz");
  c2->Update();
  TPaletteAxis *pal2 = (TPaletteAxis*) hpadamp->GetListOfFunctions()->FindObject("palette");
  pal2->SetX1NDC(0.91);
  pal2->SetX2NDC(0.92);
  if(saveas) c2->SaveAs(".pdf");


  cname = "PadOverflowR";
  cname += RunNumber;
  hOF->Scale(1./Nevents);
  TCanvas* c3 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hOF->Draw("colz");
  c3->Update();
  TPaletteAxis *pal3 = (TPaletteAxis*) hOF->GetListOfFunctions()->FindObject("palette");
  pal3->SetX1NDC(0.91);
  pal3->SetX2NDC(0.92);
  if(saveas) c3->SaveAs(".pdf");

  cname = "AFTERampR";
  cname += RunNumber;
  TCanvas* c4 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hscamp->Draw("colz");
  c4->Update();
  TPaletteAxis *pal4 = (TPaletteAxis*) hscamp->GetListOfFunctions()->FindObject("palette");
  pal4->SetX1NDC(0.91);
  pal4->SetX2NDC(0.92);
  if(saveas) c4->SaveAs(".pdf");

  cname = "AFTERoverflowR";
  cname += RunNumber;
  TCanvas* c5 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hscaoverflow->Draw("colz");
  c5->Update();
  TPaletteAxis *pal5 = (TPaletteAxis*) hscaoverflow->GetListOfFunctions()->FindObject("palette");
  pal5->SetX1NDC(0.91);
  pal5->SetX2NDC(0.92);
  if(saveas) c5->SaveAs(".pdf");
  

  cname = "PadAMPR";
  cname += RunNumber;
  TCanvas* c6 = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  p->Draw();
  if(saveas) c6->SaveAs(".pdf");

  cname = "PadOverflowOverOccupancyR";
  cname += RunNumber;
  TCanvas* ca = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hofl->Draw("colz");
  ca->Update();
  TPaletteAxis *pala = (TPaletteAxis*) hofl->GetListOfFunctions()->FindObject("palette");
  pala->SetX1NDC(0.91);
  pala->SetX2NDC(0.92);
  if(saveas) ca->SaveAs(".pdf");

 
  
  cname = "AFTEROverflowOverOccupancyR";
  cname += RunNumber;
  TCanvas* cb = new TCanvas(cname.Data(),cname.Data(),1800,1200);
  hscaofl->Draw("colz");
  cb->Update();
  TPaletteAxis *palb = (TPaletteAxis*) hscaofl->GetListOfFunctions()->FindObject("palette");
  palb->SetX1NDC(0.91);
  palb->SetX2NDC(0.92);
  if(saveas) cb->SaveAs(".pdf");

}


void plotTPCdeformation()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  RunNumber = GetRunNumber( fin->GetName() );
  cout<<"Run Number: "<<RunNumber<<endl;
  ReadMap();

  deformation(fin);
  phspectrum(fin);
  phspectrum_tracks(fin);
}
