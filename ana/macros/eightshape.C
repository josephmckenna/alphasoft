#include "SignalsType.h"  

map<int,double> ReadDataFile()
{
  char fname[128];
  sprintf(fname,"%s/ana/eightshape.dat",getenv("AGRELEASE"));
  ifstream fin(fname);
  string head;
  getline(fin,head);
  cout<<head<<endl;
  getline(fin,head);
  cout<<head<<endl;
  getline(fin,head);
  double pos,col8,col24;
  int meas1=8,meas2=24;
  padmap pads;
  map<int,double> defl;
  while(1)
    {
      fin>>pos>>col8>>col24;
      if( !fin.good() ) break;
      int row = int(pos);
      defl[pads.index(meas1,row)] = col8;
      defl[pads.index(meas2,row)] = col24;
      //cout<<pos<<"\t"<<col8<<"\t"<<col24<<endl;
    }
  fin.close();

  return defl;
}

// void ReadRootFile(TProfile* pofPwbAmp, TH2D* hpadocc)
// {
//   TFile* ff = TFile::Open("/daq/alpha_data0/acapra/alphag/output5/output03873.root","READ");
//   gDirectory->cd("paddeconv/pwbwf");
//   gDirectory->pwd();
//   pofPwbAmp = (TProfile*) gROOT->FindObject("hPwbAmp_prox");
//   cout<<pofPwbAmp->GetName()<<"\t"<<pofPwbAmp->GetEntries()<<endl;

//   gDirectory->cd("/");
//   gDirectory->cd("paddeconv");
//   gDirectory->pwd();
//   hpadocc = (TH2D*) gROOT->FindObject("hOccPad");
//   cout<<hpadocc->GetName()<<"\t"<<hpadocc->GetEntries()<<endl;
// }

double CalculateBase(TProfile* h, map<int,double> defl, bool verb=false)
{
  padmap pads;
  double base = 0.0,nb=0.0;
  for( auto d: defl )
    {
      double amp = h->GetBinContent( d.first + 1 );

      if( verb )
	{
	  int s,r;
	  pads.get( d.first, s, r );
	  cout<<"( "<<setw(2)<<s<<" "<<setw(3)<<r<<" ) --> "
	      <<d.second<<" mm\t";
	  cout<<"amp: "<<amp;
	  if( d.second == 0. )
	    cout<<"\t@!@!@";
	  cout<<"\n";
	}
      base+=amp;
      ++nb;
    }
  if( nb > 0. )
    return base/nb;
  else
    return 0.;
}

double CalculateBase(TH2D* h, map<int,double> defl, bool verb=false)
{
  padmap pads;
  double base = 0.0,nb=0.0;
  for( auto d: defl )
    {
      int s,r;
      pads.get( d.first, s, r );
      int bin = h->GetBin(r+1,s+1);
      double amp = h->GetBinContent( bin );

      if( verb )
	{
	  cout<<"( "<<setw(2)<<s<<" "<<setw(3)<<r<<" ) --> "
	      <<d.second<<" mm\t";
	  cout<<"amp: "<<amp;
	  if( d.second == 0. )
	    cout<<"\t@!@!@";
	  cout<<"\n";
	}
      base+=amp;
      ++nb;
    }
  if( nb > 0. )
    return base/nb;
  else
    return 0.;
}

vector<int> FindZeros(map<int,double> defl)
{
  vector<int> nodefl;
  for( auto d: defl )
    {
      if( d.second == 0. ) nodefl.push_back( d.first );
    }
  return nodefl;
}

vector<int> PrintZeros(TProfile* h, map<int,double> defl)
{
  padmap pads;
  vector<int> nodefl;
  for( auto d: defl )
    {
      if( d.second == 0. ) nodefl.push_back( d.first );
    }

  for( auto n: nodefl )
	{
	  double bc = h->GetBinContent( h->FindBin( double(n) ) );
	  double bc1 = h->GetBinContent( n+1 );
      
	  cout<<n<<" sec: "<<pads.getsector(n)<<"\t"<<bc<<"\t"<<bc1<<endl;
	  //cout<<n<<" sec: "<<pads.getsector(n)<<"\t"<<bc1<<endl;
	}
  return nodefl;
}

vector<int> PrintZeros(TH2D* h, map<int,double> defl)
{
  padmap pads;
  vector<int> nodefl;
  for( auto d: defl )
    {
      if( d.second == 0. ) nodefl.push_back( d.first );
    }

  for( auto n: nodefl )
	{
	  int s,r;
	  pads.get( n, s, r );
	  double bc = h->GetBinContent( h->FindBin( r,s ) );
	  int bin = h->GetBin(r+1,s+1);
	  double bc1 = h->GetBinContent( bin );
      
	  cout<<n<<" sec: "<<pads.getsector(n)<<"\t"<<bc<<"\t"<<bc1<<endl;
	  //cout<<n<<" sec: "<<pads.getsector(n)<<"\t"<<bc1<<endl;
	}
  return nodefl;
}

TF1* Plot(TProfile* h, map<int,double> defl, double base, string tag)
{
  padmap pads;
  TGraph* gmm8 = new TGraph();
  gmm8->SetLineColor(kRed);
  gmm8->SetMarkerColor(kRed);
  gmm8->SetLineWidth(2);
  gmm8->SetMarkerStyle(8);
  TGraph* gamp8 = new TGraph();
  gamp8->SetLineColor(kOrange);
  gamp8->SetMarkerColor(kOrange);
  gamp8->SetLineWidth(2);
  gamp8->SetMarkerStyle(8);
  TGraph* gmm24 = new TGraph();
  gmm24->SetLineColor(kBlue);
  gmm24->SetMarkerColor(kBlue);
  gmm24->SetLineWidth(2);
  gmm24->SetMarkerStyle(8);
  TGraph* gamp24 = new TGraph();
  gamp24->SetLineColor(kCyan);
  gamp24->SetMarkerColor(kCyan);
  gamp24->SetLineWidth(2);
  gamp24->SetMarkerStyle(8);

  TLegend* leg8 = new TLegend(0.7,0.75,0.87,0.87);
  leg8->AddEntry(gmm8,"Deflection","l");
  leg8->AddEntry(gamp8,"-Amplitude+Baseline","l");

  TLegend* leg24 = new TLegend(0.7,0.75,0.87,0.87);
  leg24->AddEntry(gmm24,"Deflection","l"); 
  leg24->AddEntry(gamp24,"-Amplitude+Baseline","l");

  TGraph* gampmm = new TGraph();
  gampmm->SetMarkerColor(kBlack);
  gampmm->SetMarkerStyle(8);
  gampmm->SetTitle("Amplitude-Deflection Relation;Amplitude [a.u.];Deflection [mm]");

  int i8,i24,ii;
  i8=i24=ii=0;
  for( auto d: defl )
    {
      int s,r;
      pads.get( d.first, s, r );
      double amp = h->GetBinContent( d.first + 1 );
      if( amp == 0. ) continue;
      if( s == 8 )
	{
	  gmm8->SetPoint(i8,r,d.second*1.e3);
	  // gamp8->SetPoint(i8,r,amp);
	  //gmm8->SetPoint(i8,r,d.second);
	  //gamp8->SetPoint(i8,r,amp-base);
	  gamp8->SetPoint(i8,r,-amp+base);
	  ++i8;
	}
      else if( s == 24 )
	{
	  gmm24->SetPoint(i24,r,d.second*1.e3);
	  // gamp24->SetPoint(i24,r,amp);
	  //gmm24->SetPoint(i24,r,d.second);
	  //	  gamp24->SetPoint(i24,r,amp-base);
	  gamp24->SetPoint(i24,r,-amp+base);
	  ++i24;
	}

      gampmm->SetPoint(ii,amp,d.second);
      ++ii;
    }

  TCanvas* campdefl = new TCanvas("campdefl","campdefl",1600,2500);
  campdefl->Divide(1,3);
  
  TH1D* h8 = new TH1D("h8","TPC deformation Sector 8;pad row;Amplitude[a.u.]/Deflection[um]",
		      1,0.,576.);
  h8->SetLineColor(0);
  h8->SetStats(kFALSE);
  h8->GetYaxis()->SetRangeUser(-1000.,1000.);

  campdefl->cd(1);
  h8->Draw();
  //  gmm8->Draw("Csame");
  //  gamp8->Draw("Csame");
  gmm8->Draw("PCsame");
  gamp8->Draw("PCsame");
  leg8->Draw("same");

  TH1D* h24 = new TH1D("h24","TPC deformation Sector 24;pad row;Amplitude[a.u.]/Deflection[um]",
		       1,0.,576.);
  h24->SetLineColor(0);
  h24->SetStats(kFALSE);
  h24->GetYaxis()->SetRangeUser(-1000.,1000.);
  campdefl->cd(2);
  h24->Draw();
  //  gmm24->Draw("Csame");
  //  gamp24->Draw("Csame");
  gmm24->Draw("PCsame");
  gamp24->Draw("PCsame");
  leg24->Draw("same"); 

  campdefl->cd(3);
  gampmm->Draw("AP");
  
  gampmm->Fit("pol1");
  return gampmm->GetFunction("pol1");
}

void Plot(TH2D* h, map<int,double> defl, double base, string tag)
{
  padmap pads;
  TGraph* gmm8 = new TGraph();
  gmm8->SetLineColor(kRed);
  gmm8->SetMarkerColor(kRed);
  gmm8->SetLineWidth(2);
  gmm8->SetMarkerStyle(8);
  TGraph* gocc8 = new TGraph();
  gocc8->SetLineColor(kOrange);
  gocc8->SetMarkerColor(kOrange);
  gocc8->SetLineWidth(2);
  gocc8->SetMarkerStyle(8);
  TGraph* gmm24 = new TGraph();
  gmm24->SetLineColor(kBlue);
  gmm24->SetMarkerColor(kBlue);
  gmm24->SetLineWidth(2);
  gmm24->SetMarkerStyle(8);
  TGraph* gocc24 = new TGraph();
  gocc24->SetLineColor(kCyan);
  gocc24->SetMarkerColor(kCyan);
  gocc24->SetLineWidth(2);
  gocc24->SetMarkerStyle(8);

  TLegend* leg8 = new TLegend(0.7,0.75,0.87,0.87);
  leg8->AddEntry(gmm8,"Deflection","l");
  leg8->AddEntry(gocc8,"-Occupancy+Baseline","l");

  TLegend* leg24 = new TLegend(0.7,0.75,0.87,0.87);
  leg24->AddEntry(gmm24,"Deflection","l"); 
  leg24->AddEntry(gocc24,"-Occupancy+Baseline","l");

  TGraph* goccmm = new TGraph();
  goccmm->SetMarkerColor(kBlack);
  goccmm->SetMarkerStyle(8);
  goccmm->SetTitle("Occupancy-Deflection Relation;Occupancy [a.u.];Deflection [mm]");

  int i8,i24,ii;
  i8=i24=0,ii;
  for( auto d: defl )
    {
      int s,r;
      pads.get( d.first, s, r );
      int bin = h->GetBin(r+1,s+1);
      double occ = h->GetBinContent( bin );
      if( occ == 0. ) continue;
      if( s == 8 )
	{
	  gmm8->SetPoint(i8,r,d.second*1.e3);
	  // gocc8->SetPoint(i8,r,occ);
	  //gmm8->SetPoint(i8,r,d.second);
	  //gocc8->SetPoint(i8,r,occ-base);
	  gocc8->SetPoint(i8,r,-occ+base);
	  ++i8;
	}
      else if( s == 24 )
	{
	  gmm24->SetPoint(i24,r,d.second*1.e3);
	  // gocc24->SetPoint(i24,r,occ);
	  //gmm24->SetPoint(i24,r,d.second);
	  //	  gocc24->SetPoint(i24,r,occ-base);
	  gocc24->SetPoint(i24,r,-occ+base);
	  ++i24;
	}
      
      goccmm->SetPoint(ii,occ,d.second);
      ++ii;
    }

  TCanvas* coccdefl = new TCanvas("coccdefl","coccdefl",1600,2100);
  coccdefl->Divide(1,3);
  
  TH1D* h8 = new TH1D("h8","TPC deformation Sector 8;pad row;Occupancy[a.u.]/Deflection[um]",
		      1,0.,576.);
  h8->SetLineColor(0);
  h8->SetStats(kFALSE);
  h8->GetYaxis()->SetRangeUser(-5000.,5000.);

  coccdefl->cd(1);
  h8->Draw();
  //  gmm8->Draw("Csame");
  //  gocc8->Draw("Csame");
  gmm8->Draw("PCsame");
  gocc8->Draw("PCsame");
  leg8->Draw("same");

  TH1D* h24 = new TH1D("h24","TPC deformation Sector 24;pad row;Occupancy[a.u.]/Deflection[um]",
		       1,0.,576.);
  h24->SetLineColor(0);
  h24->SetStats(kFALSE);
  h24->GetYaxis()->SetRangeUser(-5000.,5000.);
  coccdefl->cd(2);
  h24->Draw();
  //  gmm24->Draw("Csame");
  //  gocc24->Draw("Csame");
  gmm24->Draw("PCsame");
  gocc24->Draw("PCsame");
  leg24->Draw("same"); 

  coccdefl->cd(3);
  goccmm->Draw("AP");
}

void PrintCal(TF1* f, TProfile* h, double b, vector<int> zeros)
{
  cout<<"cal: f(base) = "<<f->Eval(b)<<endl;
   for( auto z: zeros )
    {
      double amp = h->GetBinContent( z+1 );
      cout<<"cal: zero = "<<amp<<"\tf(zero) = "<<f->Eval(amp)<<endl;
    }
}

TH2D* UseCal(TF1* f, TProfile* h)
{
  // TH2D* hdefl = new TH2D("hdefl","TPC Deformation;Row;Sector;Deflection [mm]",
  // 			 576,0.,576.,32,0.,32.);
  TH2D* hdefl = new TH2D("hdefl","TPC Deformation;#phi [rad];z [mm];Deflection [mm]",
			 32,0.,TMath::TwoPi(),576,-_halflength,_halflength);
  hdefl->SetStats(kFALSE);
  ofstream fpotato("tpcpotato.dat");
  fpotato<<"# row\tsec\tz\t\tphi\tdefl [mm]\n";
  double PadWidthPhi = 1./_padcol;
  double phi_c = 2.*M_PI*PadWidthPhi;
  double phi_ch = M_PI*PadWidthPhi;
  padmap pads;
  for( int s=0; s<32; ++s)
    {
      for(int r=0; r<576; ++r)
	{
	  int bin = pads.index(s,r) + 1;
	  if( bin == 160 || bin == 18423 || 
	      bin == 6617 || bin == 6265 || 
	      bin == 5337 || bin == 9329) continue;
	  double amp = h->GetBinContent( bin );
	  //	  if( amp <= 0. || amp > 2550.) continue;
	  // if( amp <= 0. || amp > 2000.) continue;
	  if( amp <= 700. || amp > 2550.) continue;
	  double d = f->Eval( amp );
	  //hdefl->Fill(r,s,d);
	  double phi = phi = double(s) * phi_c + phi_ch ;
	  if(phi > 2.*M_PI) phi -= 2.*M_PI;
	  double z = ( double(r) + 0.5 ) * _padpitch;
	  z -= _halflength;
	  hdefl->Fill(phi,z,d);
	  fpotato<<r<<"\t"<<s<<"\t"<<z<<"\t"<<phi<<"\t"<<d<<endl;
	}
    }
  fpotato.close();
  return hdefl;
}

void eightshape()
{
  map<int,double> defl = ReadDataFile();
  cout<<"defl size: "<<defl.size()<<endl;

  //TFile* ff = TFile::Open("/daq/alpha_data0/acapra/alphag/output5/output03873.root","READ");
  //TFile* ff = TFile::Open("output03875.root","READ");
  TFile* ff = (TFile*) gROOT->GetListOfFiles()->First();
  gDirectory->cd("paddeconv/pwbwf");
  gDirectory->pwd();
  TProfile* pofPwbAmp = (TProfile*) gROOT->FindObject("hPwbAmp_prox");
  cout<<pofPwbAmp->GetName()<<"\t"<<pofPwbAmp->GetEntries()<<endl;
  TCanvas* cpadamp = new TCanvas("cpadamp","cpadamp",1600,1200);
  pofPwbAmp->Draw();

  double base = CalculateBase(pofPwbAmp,defl);
  cout<<"baseline: "<<base<<endl;
  vector<int> zeros = PrintZeros(pofPwbAmp,defl);
  string name = "Amplitude";
  TF1* fcal = Plot( pofPwbAmp, defl, base, name );
  PrintCal(fcal, pofPwbAmp, base, zeros);
 

  gDirectory->cd("/");
  gDirectory->cd("paddeconv");
  gDirectory->pwd();
  TH2D* hpadocc = (TH2D*) gROOT->FindObject("hOccPad");
  cout<<hpadocc->GetName()<<"\t"<<hpadocc->GetEntries()<<endl;

  base = CalculateBase(hpadocc,defl);
  cout<<"baseline: "<<base<<endl;
  PrintZeros(hpadocc, defl);
  name = "Occupancy";
  Plot( hpadocc, defl, base, name );

 
  TH2D* hdefl = UseCal(fcal, pofPwbAmp);
  TCanvas* cpaddefl = new TCanvas("cpaddefl","cpaddefl",1600,1200);
  hdefl->SetContour(51);
  //hdefl->Draw("colz"); 
  hdefl->Draw("SURF2CYL");
}
