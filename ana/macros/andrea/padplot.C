#include <map>
#include <string>

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

#include "SignalsType.hh"
#include "IntGetters.h"
#include "TPCconstants.hh"

#include "DeformHistoLim.h"

TH2D* hspzp=0;
TH2D* hsppad=0;
TH1D* hocc_px=0;
TH1D* hspzp_px=0;
TH1D* hsppad_px=0;


ALPHAg::padmap pm;

void plot_spacepoints(TFile* fin)
{
  hspzp = new TH2D("hspzp","Spacepoints in Tracks;z [mm];#phi [deg]",
		   600,-1200.,1200.,256,0.,360.);
  hspzp->SetStats(kFALSE);
  hsppad = new TH2D("hsppad","Spacepoints in Tracks by Pad;row;sec;N",
		    576,0.,576.,32,0.,32.);
  hsppad->SetStats(kFALSE);

  TTree* tin = (TTree*) fin->Get("StoreEventTree");
  cout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<endl;
  TStoreEvent* event = new TStoreEvent();
  tin->SetBranchAddress("StoredEvent", &event);
  for(int e=0; e<tin->GetEntries(); ++e)
    {
      if( e%10000 == 0 ) cout<<"*** "<<e<<endl;
      event->Reset();
      tin->GetEntry(e);
      const TObjArray* tracks = event->GetLineArray();
      for(int i=0; i<tracks->GetEntriesFast(); ++i)
	{
	  TStoreLine* aLine = (TStoreLine*) tracks->At(i);
	  const TObjArray* sp = aLine->GetSpacePoints();
	  for( int ip = 0; ip<sp->GetEntriesFast(); ++ip )
	    {
	      TSpacePoint* ap = (TSpacePoint*) sp->At(ip);
	      hspzp->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
	      int spad = ap->GetPad();
	      int s,r;
	      pm.get(spad,s,r);
	      hsppad->Fill(double(r),double(s));
	    }
	}
    }
  cout<<"spacepoints END"<<endl;
}


void temperature(TH2D* hocc)
{
  double n=0.,m=0.,m2=0.;
  for(int r=0; r<576; ++r)
    {
      int bx=r+1;
      for(int s=0; s<32; ++s)
	{
	  int by=s+1;
	  int bin = hocc->GetBin(bx,by);
	  double bc = hocc->GetBinContent(bin);
	  m+=bc;
	  m2+=bc*bc;
	  n++;
	}
    } 
  double mu=m/n;
  double rms=sqrt(m2/n-mu*mu);
  cout<<"total: "<<n<<"\tmean: "<<mu<<"\trms: "<<rms<<endl;

  double thr=mu+3.*rms;
  cout<<"Hot Pads:\nsec\trow\tocc"<<endl;
  int Nhot=0;
  for(int r=0; r<576; ++r)
    {
      int bx=r+1;
      for(int s=0; s<32; ++s)
	{
	  int by=s+1;
	  int bin = hocc->GetBin(bx,by);
	  double bc = hocc->GetBinContent(bin);
	  if( bc > thr) 
	    {
	      cout<<s<<"\t"<<r<<"\t"<<bc<<endl;
	      ++Nhot;
	    }
	}
    } 
  cout<<"Total Hot: "<<Nhot<<endl;

  thr=mu-5.*rms;
  cout<<"Cold Pads:\nsec\trow\tocc"<<endl;
  int Ncold=0;
  for(int r=0; r<576; ++r)
    {
      int bx=r+1;
      for(int s=0; s<32; ++s)
  	{
  	  int by=s+1;
  	  bool skip=false;
  	  if( s == 17 || s == 18 ){
  	    for(int x=432; x<468; ++x) 
  	      {
  		if( r == x ) {
  		  skip=true;
  		  break;
  		}
  	      }
  	  }
  	  if( skip ) continue;
  	  int bin = hocc->GetBin(bx,by);
  	  double bc = hocc->GetBinContent(bin);
  	  if( bc < thr) 
  	    {
  	      cout<<s<<"\t"<<r<<"\t"<<bc<<endl;
  	      ++Ncold;
  	    }
  	}
    } 
  cout<<"Total Cold: "<<Ncold<<endl;
}

void padplot()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(fin->GetName());
  cout<<fname<<" FOUND"<<endl;

  int RunNumber = GetRunNumber( fname );
  cout<<"Run # "<<RunNumber<<endl;

  SetHistoLimits( RunNumber );

  if( !gDirectory->cd("/") ) 
    {
      cout<<"something is wrong, exiting..."<<endl;
      gROOT->ProcessLine(".q");
    }

  gDirectory->cd("paddeconv");
  TH2D* hocc = (TH2D*)gROOT->FindObject("hOccPad");
  hocc->SetStats(kFALSE);
  hocc->SetMinimum(min_occ);
  hocc->SetMaximum(max_occ);
  // hocc->SetMaximum(8000.);

  TString cname=TString::Format("coccpadR%d",RunNumber);
  TCanvas* c1 = new TCanvas(cname,cname,1700,1000);
  hocc->Draw("colz");
  c1->SaveAs(".pdf");

  temperature(hocc);
  
  hocc_px=hocc->ProjectionX();
  hocc_px->SetMinimum(0.0);
  cname=TString::Format("coccpadprojR%d",RunNumber);
  TCanvas* c3 = new TCanvas(cname,cname,1700,1000);
  hocc_px->Draw();
  c3->Update();
  TPaveStats *stax3 = (TPaveStats*)hocc_px->FindObject("stats");
  stax3->SetX1NDC(0.8);
  stax3->SetX2NDC(0.99);
  stax3->SetY1NDC(0.1);
  stax3->SetY2NDC(0.4);
  c3->SaveAs(".pdf");


  // gDirectory->cd("/");  
  plot_spacepoints(fin);
  cname=TString::Format("cspZedPhiR%d",RunNumber);
  TCanvas* c2 = new TCanvas(cname,cname,2100,1900);
  c2->Divide(2,2);
  c2->cd(1);
  hspzp->Draw("colz");
  //hsp->SetMinimum(5);
  c2->cd(2);
  hsppad->Draw("colz");

  cout<<"Get Projections"<<endl;

  hspzp_px=hspzp->ProjectionX();
  hspzp_px->Scale(1.e2/hspzp_px->Integral());
  hspzp_px->SetMinimum(0.);
  hspzp_px->SetStats(kFALSE);
  hspzp_px->SetLineColor(kRed);
  hspzp_px->SetMarkerColor(kRed);
  TString htitle=hspzp->GetTitle();
  htitle+=" Averaged over #phi";
  hspzp_px->SetTitle(htitle);
  cout<<htitle<<endl;
  c2->cd(3);
  hspzp_px->Draw();
  // c2->Update();
  // TPaveStats *stax1 = (TPaveStats*)hspzp_px->FindObject("stats");
  // stax1->SetX1NDC(0.8);
  // stax1->SetX2NDC(0.99);
  // stax1->SetY1NDC(0.1);
  // stax1->SetY2NDC(0.4);

  hsppad_px=hsppad->ProjectionX();
  hsppad_px->Scale(1./hsppad_px->Integral());
  hsppad_px->SetMinimum(0.);
  hsppad_px->SetStats(kFALSE);
  hsppad_px->SetLineColor(kBlue);
  hsppad_px->SetMarkerColor(kBlue);
  htitle=hsppad->GetTitle();
  htitle+=" Averaged over Sector";
  hsppad_px->SetTitle(htitle);
  cout<<htitle<<endl;
  c2->cd(4);
  hsppad_px->Draw();
  // c2->Update();
  // TPaveStats *stax2 = (TPaveStats*)hsppad_px->FindObject("stats");
  // stax2->SetX1NDC(0.8);
  // stax2->SetX2NDC(0.99);
  // stax2->SetY1NDC(0.1);
  // stax2->SetY2NDC(0.4);

 

  Int_t MaxBin = hspzp->GetMaximumBin();
  Int_t x,y,z;
  hspzp->GetBinXYZ(MaxBin, x, y, z);
  double maxval=hspzp->GetBinContent(MaxBin);
  printf("The bin having the maximum value of %1.f is (%d,%d)\n",maxval,x,y);
  Int_t MinBin = hspzp->GetMinimumBin();
  hspzp->GetBinXYZ(MinBin, x, y, z);
  double minval=hspzp->GetBinContent(MinBin); 
  printf("The bin having the maximum value of %1.f is (%d,%d)\n",minval,x,y);
 
  c2->SaveAs(".pdf");


  cname=TString::Format("cspoccpadprojR%d",RunNumber);
  TCanvas* c4 = new TCanvas(cname,cname,1700,1000);
  c4->cd();
  hsppad_px->Draw();
  cname=TString::Format("cspoccR%d",RunNumber);
  TCanvas* c5 = new TCanvas(cname,cname,1700,1000);
  c5->cd();
  hspzp_px->Draw();

}
