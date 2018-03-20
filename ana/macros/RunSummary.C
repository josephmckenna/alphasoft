#include "TH1D.h"
#include "TH2D.h"
#include "TVector3.h"
#include "TMath.h"
#include "TString.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TROOT.h"
#include "TStyle.h"

#include "TPCBase.hh"
#include "TSpacePoint.hh"
#include "TStoreEvent.hh"
#include "TStoreLine.hh"


TH1D* hNhits=NULL;
TH1D* hNtracks=NULL;
TH1D* hPattRecEff=NULL;

void RunSummary(const char* filename="../output01238.root")
{



  TFile* fin = TFile::Open(filename,"READ");
  TTree* tin = (TTree*) fin->Get("StoreEventTree");

  hPattRecEff = new TH1D("hPattRecEff",
			      "Number of Spacepoints per Track per Event;SP/Tr [a.u.]",
			      1000,0.,1000.);
  hNhits = new TH1D("hNhits","Number of Spacepoints per Event;Points [a.u.];Events [a.u.]",
			 500,0.,500.);
  hNtracks = new TH1D("hNtracks","Number of Tracks per Event;Tracks [a.u.];Events [a.u.]",
			   10,0.,10.);


  TStoreEvent* event = new TStoreEvent();
  tin->SetBranchAddress("StoredEvent", &event);

  int Ntrack2=0;
  for(int e=0; e<tin->GetEntries(); ++e)
    {
      if(e%1000==0) cout<<"*** "<<e<<endl;
      event->Reset();
      tin->GetEntry(e);
      //      event->Print();
      int Nhits = event->GetNumberOfHits();
      int Ntracks = event->GetNumberOfTracks();
      hNhits->Fill(Nhits);
      hNtracks->Fill(Ntracks);
      hPattRecEff->Fill( double(Nhits)/double(Ntracks) );
      if( Ntracks == 2 )
        {
          Ntrack2++;
        }
    }
  cout <<"\t\tEntries\tMean\tError\tStdev\tError"<<endl;
  cout <<"Hits:\t\t"<<hNhits->GetEntries()<<"\t"<<hNhits->GetMean()<<"\t"<<hNhits->GetMeanError()<<"\t"<<hNhits->GetStdDev()<<"\t"<<hNhits->GetStdDevError()<< endl;
  cout <<"Tracks:\t\t"<<hNtracks->GetEntries()<<"\t"<<hNtracks->GetMean()<<"\t"<<hNtracks->GetMeanError()<<"\t"<<hNtracks->GetStdDev()<<"\t"<<hNtracks->GetStdDevError()<< endl;
  cout <<"PatRecEff:\t"<<hPattRecEff->GetEntries()<<"\t"<<hPattRecEff->GetMean()<<"\t"<<hPattRecEff->GetMeanError()<<"\t"<<hPattRecEff->GetStdDev()<<"\t"<<hPattRecEff->GetStdDevError()<< endl;
}  
