#include "geant4/include/TMChit.hh"
#include "geant4/include/TWaveform.hh"

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TGraph.h"

#include <iostream>
using namespace std;

TCanvas* cTPCHits;

int GetPadIndexes(string padname, int& col, int& row)
{
  string delimiter = "_";

  size_t pos = padname.find(delimiter);
  std::string p = padname.substr(0, pos);
  if( p != "p" ) 
    std::cerr<<"GetPadIndexes: Wrong Electrode? "<<p<<std::endl;
  padname = padname.erase(0, pos + delimiter.length());

  pos = padname.find(delimiter);
  col = std::stoi( padname.substr(0, pos) );
  assert(col<32&&col>=0);
  //std::cout<<"DeconvModule::FindPadTimes() col: "<<col<<std::endl;
  padname = padname.erase(0, pos + delimiter.length());

  pos = padname.find(delimiter);
  row = std::stoi( padname.substr(0, pos) );
  return col + 32 * row;;
}

void plotMCgen( TTree* tMC )
{
  TClonesArray* vtx = new TClonesArray("TVector3");
  tMC->SetBranchAddress("MCvertex",&vtx);
  TClonesArray* pic = new TClonesArray("TLorentzVector");
  tMC->SetBranchAddress("MCpions",&pic);
  int Npoints = 100;
  for( int i=0; i<tMC->GetEntries(); ++i )
    {
      tMC->GetEntry(i);
      int Npions = pic->GetEntries();
      cout<<i<<" #pions "<<Npions<<endl;
      if( Npions == 0 ) continue;
      TGraph* gpicxy = new TGraph(Npoints*Npions);
      gpicxy->SetMarkerStyle(34);
      gpicxy->SetMarkerColor(kRed);
      gpicxy->SetTitle("MC #pi X-Y;x [mm];y [mm]");
      TGraph* gpicrz = new TGraph(Npoints*Npions);
      gpicrz->SetMarkerStyle(34);
      gpicrz->SetMarkerColor(kRed);
      gpicrz->SetTitle("MC #pi R-Z;r [mm];z [mm]");
      TVector3 mcvtx = *((TVector3*) vtx->ConstructedAt(0));
      mcvtx.Print();
      int n=0;
      double step = 80. / double(Npoints);
      for(int k=0; k<Npions; ++k)
      	{
      	  TLorentzVector* mom4 = (TLorentzVector*) pic->ConstructedAt(k);
      	  TVector3 dir = mom4->Vect().Unit();
      	  double tmin = 0., rad = 0.;
      	  while( rad < 109. )
      	    {
      	      TVector3 test = mcvtx + (++tmin) * dir;
      	      rad = test.Perp();
      	    }
      	  for(int j=0; j<Npoints; ++j )
      	    {
      	      double t = j*step + tmin;
      	      TVector3 apoint = mcvtx + t * dir;
      	      //  apoint.Print();
      	      gpicxy->SetPoint(n,apoint.X(),apoint.Y());
      	      gpicrz->SetPoint(n,apoint.Perp(),apoint.Z());
      	      ++n;
      	    }
      	}

      cTPCHits->cd(1);
      gpicxy->Draw("Psame");
      cTPCHits->cd(2);
      gpicrz->Draw("Psame");
    }
}

void plotTPChits( TTree* tMC )
{
  TClonesArray* tpc_hits = new TClonesArray("TMChit");
  tMC->SetBranchAddress("TPCHits",&tpc_hits);

  for( int i=0; i<tMC->GetEntries(); ++i )
    {
      tMC->GetEntry(i);

      int Npoints = tpc_hits->GetEntries();
      cout<<i<<" --> "<<Npoints<<endl;
      if( Npoints == 0 ) continue;
      TGraph* gxy = new TGraph(Npoints);
      gxy->SetMarkerStyle(7);
      gxy->SetMarkerColor(kBlue);
      gxy->SetTitle("TPCHits X-Y;x [mm];y [mm]");
      TGraph* grz = new TGraph(Npoints);
      grz->SetMarkerStyle(7);
      grz->SetMarkerColor(kBlue);
      grz->SetTitle("TPCHits R-Z;r [mm];z [mm]");
      TGraph* grphi = new TGraph(Npoints);
      grphi->SetMarkerStyle(7);
      grphi->SetMarkerColor(kBlue);
      grphi->SetTitle("TPCHits R-#phi;r [mm];#phi [rad]");
      TGraph* gzphi = new TGraph(Npoints);
      gzphi->SetMarkerStyle(7);
      gzphi->SetMarkerColor(kBlue);
      gzphi->SetTitle("TPCHits Z-#phi;z [mm];#phi [rad]");

      for( int j=0; j<Npoints; ++j )
	{
	  TMChit* h = (TMChit*) tpc_hits->ConstructedAt(j);
	  gxy->SetPoint(j,h->GetX(),h->GetY());
	  grz->SetPoint(j,h->GetRadius(),h->GetZ());
	  grphi->SetPoint(j,h->GetRadius(),h->GetPhi()*TMath::RadToDeg());
	  gzphi->SetPoint(j,h->GetZ(),h->GetPhi()*TMath::RadToDeg());
	}
	  
      cTPCHits->cd(1);
      gxy->Draw("Psame");
      cTPCHits->cd(2);
      grz->Draw("Psame");
      cTPCHits->cd(3);
      grphi->Draw("Psame");
      cTPCHits->cd(4);
      gzphi->Draw("Psame");
    }
}

void plotGarfhits( TTree* tGarf )
{
  TClonesArray* garfpp_hits = new TClonesArray("TMChit");
  tGarf->SetBranchAddress("GarfHits",&garfpp_hits);
  for( int i=0; i<tGarf->GetEntries(); ++i )
    {
      tGarf->GetEntry(i);
      int Npoints = garfpp_hits->GetEntries();
      cout<<i<<" --> "<<Npoints<<endl;
      TGraph* gxy = new TGraph(Npoints);
      gxy->SetMarkerStyle(6);
      gxy->SetMarkerColor(kGreen+2);
      gxy->SetTitle("Garfield++ Hits X-Y;x [mm];y [mm]");
      TGraph* grz = new TGraph(Npoints);
      grz->SetMarkerStyle(6);
      grz->SetMarkerColor(kGreen+2);
      grz->SetTitle("Garfield++ Hits R-Z;r [mm];z [mm]");
      TGraph* grphi = new TGraph(Npoints);
      grphi->SetMarkerStyle(6);
      grphi->SetMarkerColor(kGreen+2);
      grphi->SetTitle("Garfield++ Hits R-#phi;r [mm];#phi [deg]");
      TGraph* gzphi = new TGraph(Npoints);
      gzphi->SetMarkerStyle(6);
      gzphi->SetMarkerColor(kGreen+2);
      gzphi->SetTitle("Garfield++ Hits Z-#phi;z [mm];#phi [deg]");
      for( int j=0; j<Npoints; ++j )
	{
	  TMChit* h = (TMChit*) garfpp_hits->ConstructedAt(j);
	  gxy->SetPoint(j,h->GetX(),h->GetY());
	  grz->SetPoint(j,h->GetRadius(),h->GetZ());
	  grphi->SetPoint(j,h->GetRadius(),h->GetPhi()*TMath::RadToDeg());
	  gzphi->SetPoint(j,h->GetZ(),h->GetPhi()*TMath::RadToDeg());
	}
      cTPCHits->cd(1);
      gxy->Draw("AP");
      gxy->GetXaxis()->SetRangeUser(109.,190.);
      gxy->GetYaxis()->SetRangeUser(-0.,190.);
      cTPCHits->cd(2);
      grz->Draw("AP");
      grz->GetXaxis()->SetRangeUser(109.,190.);
      grz->GetYaxis()->SetRangeUser(-10.,10.);
      cTPCHits->cd(3);
      grphi->Draw("AP");
      grphi->GetXaxis()->SetRangeUser(109.,190.);
      grphi->GetYaxis()->SetRangeUser(0.,40.);
      cTPCHits->cd(4);
      gzphi->Draw("AP");
      gzphi->GetXaxis()->SetRangeUser(-10.,10.);
      gzphi->GetYaxis()->SetRangeUser(0.,40.);
    }
}

void plotAWhits( TTree* tGarf )
{
  TClonesArray* garfpp_hits = new TClonesArray("TMChit");
  tGarf->SetBranchAddress("AnodeHits",&garfpp_hits);
  for( int i=0; i<tGarf->GetEntries(); ++i )
    {
      tGarf->GetEntry(i);
      int Npoints = garfpp_hits->GetEntries();
      cout<<i<<" --> "<<Npoints<<endl;
      TGraph* gxy = new TGraph(Npoints);
      gxy->SetMarkerStyle(6);
      gxy->SetMarkerColor(kBlack);
      gxy->SetTitle("Garfield++ Hits X-Y;x [mm];y [mm]");
      TGraph* grz = new TGraph(Npoints);
      grz->SetMarkerStyle(6);
      grz->SetMarkerColor(kBlack);
      grz->SetTitle("Garfield++ Hits R-Z;r [mm];z [mm]");
      TGraph* grphi = new TGraph(Npoints);
      grphi->SetMarkerStyle(6);
      grphi->SetMarkerColor(kBlack);
      grphi->SetTitle("Garfield++ Hits R-#phi;r [mm];#phi [deg]");
      TGraph* gzphi = new TGraph(Npoints);
      gzphi->SetMarkerStyle(6);
      gzphi->SetMarkerColor(kBlack);
      gzphi->SetTitle("Garfield++ Hits Z-#phi;z [mm];#phi [deg]");
      for( int j=0; j<Npoints; ++j )
	{
	  TMChit* h = (TMChit*) garfpp_hits->ConstructedAt(j);
	  gxy->SetPoint(j,h->GetX(),h->GetY());
	  grz->SetPoint(j,h->GetRadius(),h->GetZ());
	  grphi->SetPoint(j,h->GetRadius(),h->GetPhi()*TMath::RadToDeg());
	  gzphi->SetPoint(j,h->GetZ(),h->GetPhi()*TMath::RadToDeg());
	}
      cTPCHits->cd(1);
      gxy->Draw("Psame");
      cTPCHits->cd(2);
      grz->Draw("Psame");
      cTPCHits->cd(3);
      grphi->Draw("Psame");
      cTPCHits->cd(4);
      gzphi->Draw("Psame");
    }
}

void plotPads(TTree* tSig)
{
  TClonesArray* PADsignals = new TClonesArray("TWaveform");
  tSig->SetBranchAddress("PAD",&PADsignals);
  for( int i=0; i<tSig->GetEntries(); ++i )
    {
      tSig->GetEntry(i);
      int Npoints = PADsignals->GetEntries(); 
      cout<<"plotPads(TTree* tSig) "<<Npoints<<endl;
      double min=9.e9,max=-min;
      int col=-1,row=-1;
      for( int j=0; j<Npoints; ++j )
	{
	  TWaveform* w = (TWaveform*) PADsignals->ConstructedAt(j);
	  vector<int> data(w->GetWaveform());
	  string padname = w->GetElectrode();

	  int temp_min = *std::min_element(data.begin(),data.end());
	  if( temp_min<min ) 
	    {
	      GetPadIndexes(padname,col,row);
	      min = temp_min;
	    }
	  
	  int temp_max = *std::max_element(data.begin(),data.end());
	  if( temp_max > max )
	    {
	      max = temp_max; 
	    }
	}

      if( col < 0 || row < 0 ) 
	continue;
      else
	cout<<"Event# "<<i
	    <<" pad col: "<<col<<" row: "<<row
	    <<"  min: "<<min<<" max: "<<max<<endl;

      int mincol=col-1>=0?col-1:31;
      int maxcol=col+1<32?col+1:0;
      int col_list[3] = {mincol,col,maxcol};
      
      int row_list[5];
      for( int n=0; n<5; ++n )
	{
	  int curr_row = row-2+n;
	  if( curr_row < 0 || curr_row >= 576 ) curr_row = -1;
	  row_list[n] = curr_row;
	}

      TH1D* hsig[15];
      TString cname = TString::Format("LargestPadSignalsEvent%03d",i);
      TCanvas* cSig = new TCanvas(cname.Data(),cname.Data(),1800,1500);
      cSig->Divide(3,5);
      int k=0;
      for(int j=0; j<Npoints; ++j)
	{
	  TWaveform* w = (TWaveform*) PADsignals->ConstructedAt(j);
	  string padname = w->GetElectrode();
	  int pcol=-1, prow=-1, x=-1;
	  GetPadIndexes(padname,pcol,prow);
	  for( int m=0; m<3; ++m )
	    {
	      if( pcol != col_list[m] ) continue;
	      for( int n=0; n<5; ++n )
		{
		  if( prow == row_list[n] )
		    {
		      x = m + 3 * n + 1;
		      break;
		    }
		}
	    }

	  if( x<=0 ) continue;	  

	  string htitle = w->GetElectrode() + " " + w->GetModel() + ";bin [16ns];ADC";
	  cout<<k<<"\t"<<htitle<<endl;
	  hsig[k] = new TH1D(w->GetElectrode().c_str(),htitle.c_str(),411,0.,411.*16.);
	  int b=1;
	  for(int v: w->GetWaveform())
	    {
	      hsig[k]->SetBinContent(b,double(v));
	      ++b;
	    }
	  cSig->cd( x );
	  hsig[k]->Draw();
	  hsig[k]->GetYaxis()->SetRangeUser(min*1.01,max*1.1);
	  ++k;
	}
    }
}

void plotAnodes(TTree* tSig)
{
  TClonesArray* AWsignals = new TClonesArray("TWaveform");
  tSig->SetBranchAddress("AW",&AWsignals);
  for( int i=0; i<tSig->GetEntries(); ++i )
    {
      tSig->GetEntry(i);
      int Npoints = AWsignals->GetEntries(); 
      double min=9.e9,max=-min;
      int ee=-1;
      for( int j=0; j<Npoints; ++j )
	{
	  TWaveform* w = (TWaveform*) AWsignals->ConstructedAt(j);
	  vector<int> data(w->GetWaveform());
	  string wname = w->GetElectrode(); 
	  // cout<<"plotAnodes "<<j<<" wire: "<<wname<<" size: "<<data.size()<<std::endl;
	  int temp_min = *std::min_element(data.begin(),data.end());
	  // cout << "\t" << wname << " size: " << data.size() 
	  //      << " min: " << *std::min_element(data.begin(),data.end())
	  //      << " max: " << *std::max_element(data.begin(),data.end())
	  //      << endl; 

	  if( temp_min<min ) 
	    {
	      ee = j;
	      min = temp_min;
	    }
	  
	  int temp_max = *std::max_element(data.begin(),data.end());
	  if( temp_max > max )
	    {
	      max = temp_max; 
	    }
	}

      if(ee<0) continue;
      int first = ee-4;//,last=ee+4;
      if(first<0) first += 256;
      //      if(last>255) last -= 256;
      int n=first;
      TH1D* hsig[9];
      TString cname = TString::Format("LargestAwSignalsEvent%03d",i);
      TCanvas* cSig = new TCanvas(cname.Data(),cname.Data(),1200,1300);
      cSig->Divide(3,3);
      for(int k=0; k<9; ++k)
	{
	  TWaveform* w = (TWaveform*) AWsignals->ConstructedAt(n);
	  string htitle = w->GetElectrode() + " " + w->GetModel() + ";bins [16ns];ADC";
	  //cout<<htitle<<endl;
	  hsig[k] = new TH1D(w->GetElectrode().c_str(),htitle.c_str(),411,0.,411.*16.);
	  int b=1;
	  for(int v: w->GetWaveform())
	    {
	      hsig[k]->SetBinContent(b,double(v));
	      ++b;
	    }
	  cSig->cd(k+1);
	  hsig[k]->Draw();
	  hsig[k]->GetYaxis()->SetRangeUser(min*1.01,max*1.1);
	  
	  ++n;
	  if(n>255) n -= 256;
	}
    }
}

void plotPoints()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();

  cTPCHits = new TCanvas("cTPChits","cTPChits",1800,1900);
  cTPCHits->Divide(2,2);


  TTree* tGarf = (TTree*) fin->Get("Garfield");
  cout<<"plot garfield"<<endl;
  plotGarfhits(tGarf);
  plotAWhits(tGarf);


  TTree* tMC = (TTree*) fin->Get("MCinfo");
  cout<<"plot MC info"<<endl;
  plotTPChits(tMC);


  TEllipse* TPCcath = new TEllipse(0.,0.,109.,109.);
  TPCcath->SetFillStyle(0);
  cTPCHits->cd(1);
  TPCcath->Draw();
  TEllipse* TPCpads = new TEllipse(0.,0.,190.,190.);
  TPCpads->SetFillStyle(0);
  cTPCHits->cd(1);
  TPCpads->Draw("same");



  TTree* tSig =  (TTree*) fin->Get("Signals");
  cout<<"plot Signals"<<endl;
  plotAnodes(tSig);
  plotPads(tSig);
}
