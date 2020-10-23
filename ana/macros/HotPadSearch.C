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

#include "SignalsType.hh"
#include "IntGetters.h"
#include "TPCconstants.hh"

TH2D* plot_spacepoints(TFile* fin)
{
  TH2D* hspzp = new TH2D("hspzp","Spacepoints in Tracks;z [mm];#phi [deg]",
			 600,-1200.,1200.,256,0.,360.);
  TTree* tin = (TTree*) fin->Get("StoreEventTree");
  cout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<endl;
  TStoreEvent* event = new TStoreEvent();
  tin->SetBranchAddress("StoredEvent", &event);
  for(int e=0; e<tin->GetEntries(); ++e)
    {
      if( e%5000 == 0 ) cout<<"*** "<<e<<endl;
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
	    }
	}
    }
  return hspzp;
}

void HotPadSearch()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(fin->GetName());
  cout<<fname<<" FOUND"<<endl;

  int RunNumber = GetRunNumber( fname );
  cout<<"Run # "<<RunNumber<<endl;

  if( !gDirectory->cd("/") ) 
    {
      cout<<"something is wrong, exiting..."<<endl;
      gROOT->ProcessLine(".q");
    }

  // TObjString* sett = (TObjString*) gROOT->FindObject("ana_settings");
  // cout<<sett->GetString()<<endl;

  gDirectory->cd("paddeconv");
  TH2D* hocc = (TH2D*)gROOT->FindObject("hOccPad");
  hocc->SetStats(kFALSE);
  // hocc->SetMinimum(2000);
  //  hocc->SetMaximum(4500.);
  //  hocc->Scale(1./hocc->Integral());

  TString cname=TString::Format("coccpadR%d",RunNumber);
  TCanvas* c1 = new TCanvas(cname,cname,1700,1000);
  //hocc->Draw("surf2");
  //hocc->Draw();
  hocc->Draw("colz");
  c1->SaveAs(".pdf");

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


  gDirectory->cd("/");
  TH2D* hsp = (TH2D*)gROOT->FindObject("hspzp");
  if( !hsp )
    {
      hsp = plot_spacepoints(fin);
    }
   
  hsp->SetStats(kFALSE);
  cname=TString::Format("cspZedPhiR%d",RunNumber);
  TCanvas* c2 = new TCanvas(cname,cname,1700,1000);
  hsp->Draw("colz");
  //hsp->SetMinimum(5);
  Int_t MaxBin = hsp->GetMaximumBin();
  Int_t x,y,z;
  hsp->GetBinXYZ(MaxBin, x, y, z);
  double maxval=hsp->GetBinContent(MaxBin);
  printf("The bin having the maximum value of %1.f is (%d,%d)\n",maxval,x,y);
  Int_t MinBin = hsp->GetMinimumBin();
  hsp->GetBinXYZ(MinBin, x, y, z);
  double minval=hsp->GetBinContent(MinBin); 
  printf("The bin having the maximum value of %1.f is (%d,%d)\n",minval,x,y);
  
  c2->SaveAs(".pdf");
}
