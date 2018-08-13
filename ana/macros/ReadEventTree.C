#include "../reco/TPCconstants.hh"

TString tag("_R");
int RunNumber=0;

// aw deconv histos
TH1D* hht;
TH1D* hot;
TH1D* htt;

// pad deconv histos
TH1D* hhpad;
TH1D* hocol;
TH1D* htpad;
TH2D* hopad;

// aw*pad
TH1D* hmatch;
TH2D* hawpadsector;

// spacepoints
TH2D* hpxy;
TH2D* hpzr;
TH2D* hpzp;
TH2D* hprp;

// track finding
TH2D* hsptrp;

// reco lines spacepoints
TH1D* hpattreceff;
TH2D* hspxy;
TH2D* hspzr;
TH2D* hspzp;
TH2D* hsprp;

// reco lines
TH1D* hNlines;
TH1D* hlphi;
TH1D* hltheta;
TH1D* hlcosang;
TH1D* hldist;
TH2D* hlcosangdist;

// cosmic time distribution
TH1D* hpois;

// z axis intersection
TVector3 zaxis(0.,0.,1.); 
TH1D* hldz;
TH1D* hlr;
TH1D* hlz;
TH1D* hlp;
TH2D* hlzp;
TH2D* hlzr;
TH2D* hlrp;
TH2D* hlxy;

void MakeHistos()
{      
  // spacepoints
  hpxy = new TH2D("hpxy","Spacepoints;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);
  hpxy->SetStats(kFALSE);
  hpzr = new TH2D("hpzr","Spacepoints;z [mm];r [mm]",600,-1200.,1200.,100,108.,175.);
  hpzr->SetStats(kFALSE);
  hpzp = new TH2D("hpzp","Spacepoints;z [mm];#phi [deg]",600,-1200.,1200.,180,0.,360.);
  hpzp->SetStats(kFALSE);
  // hprp = new TH2D("hprp","Spacepoints;r [mm];#phi [deg]",100,108.,175.,180,0.,360.);
  hprp = new TH2D("hprp","Spacepoints;#phi [deg];r [mm]",180,0.,TMath::TwoPi(),200,0.,175.);
  hprp->SetStats(kFALSE);

  // spacepoints from tracks
  hpattreceff = new TH1D("hpattreceff","Track Finding Efficiency",300,-1.,600.);
  hpattreceff->SetLineWidth(2);
  hspxy = new TH2D("hspxy","Spacepoints in Tracks;x [mm];y [mm]",
		   100,-190.,190.,100,-190.,190.);
  hspxy->SetStats(kFALSE);
  hspzr = new TH2D("hspzr","Spacepoints in Tracks;z [mm];r [mm]",
		   600,-1200.,1200.,100,108.,175.);
  hspzr->SetStats(kFALSE);
  hspzp = new TH2D("hspzp","Spacepoints in Tracks;z [mm];#phi [deg]",
		   600,-1200.,1200.,180,0.,360.);
  hspzp->SetStats(kFALSE);

  hsprp = new TH2D("hsprp","Spacepoints in Tracks;#phi [deg];r [mm]",
		   180,0.,TMath::TwoPi(),200,0.,175.);
  hsprp->SetStats(kFALSE);

  //    hdsp = new TH1D("hdsp","Distance Spacepoints;d [mm]",100,0.,50.);

  // line properties
  hNlines = new TH1D("hNlines","Reconstructed Lines",10,0.,10.);
  hlphi = new TH1D("hphi","Direction #phi;#phi [deg]",200,-180.,180.);
  hltheta = new TH1D("htheta","Direction #theta;#theta [deg]",200,0.,180.);

  hlcosang = new TH1D("hcosang","Cosine of Angle Formed by 2 Lines;cos(#alpha)",2000,-1.,1.);
  hldist = new TH1D("hdist","Distance between  2 Lines;s [mm]",200,0.,20.);

  hlcosangdist = new TH2D("hcosangdist",
			 "Correlation Angle-Distance;cos(#alpha);s [mm]",
			 200,-1.,1.,200,0.,20.);

  // cosmic time distribution
  hpois = new TH1D("hpois","Delta t between cosmics;#Delta t [ms]",200,0.,111.);
  hpois->SetMarkerColor(kBlack);
  hpois->SetMarkerStyle(8);
  hpois->SetLineColor(kBlack);
  
  // z axis intersection
  hldz = new TH1D("hldz","Minimum Radius;r [mm]",200,0.,190.);
  hldz->SetLineColor(kRed);
  hlr = new TH1D("hlr","Minimum Radius;r [mm]",200,0.,190.);
  hlz = new TH1D("hlz","Z intersection with min rad;z [mm]",300,-1200.,1200.);
  hlp = new TH1D("hlp","#phi intersection with min rad;#phi [deg]",100,-180.,180.);
  hlzp = new TH2D("hlzp","Z-#phi intersection with min rad;z [mm];#phi [deg]",
		  100,-1200.,1200.,90,-180.,180.);
  hlzp->SetStats(kFALSE);
  hlzr = new TH2D("hlzr","Z-R intersection with min rad;z [mm];r [mm]",
		  100,-1200.,1200.,100,0.,190.);
  hlrp = new TH2D("hlrp","R-#phi intersection with min rad;r [mm];#phi [deg]",
		  100,0.,190.,90,-180.,180.);
  hlxy = new TH2D("hlxy","X-Y intersection with min rad;x [mm];y [mm]",
		  100,-190.,190.,100,-190.,190.);
}

void DisplayHisto()
{
  TString cname;

  if( hht ) {
  // deconv signals histos
  cname = "deconv";
  cname+=tag;
  TCanvas* cdec = new TCanvas(cname.Data(),cname.Data(),1600,1000);
  cdec->Divide(3,2);

  cdec->cd(1);
  //  hht->Scale(1./hht->Integral());
  hht->Draw();
  //  hhpad->Scale(1./hhpad->Integral());
  hhpad->Draw("same");
  hht->GetXaxis()->SetRangeUser(0.,2100.);
  hht->GetYaxis()->SetRangeUser(0., 
				hht->GetBinContent(hht->GetMaximumBin()) > hhpad->GetBinContent(hhpad->GetMaximumBin()) ? 
				hht->GetBinContent(hht->GetMaximumBin())*1.1 : hhpad->GetBinContent(hhpad->GetMaximumBin())*1.1
				);

  TLegend* legdec = new TLegend(0.65,0.62,0.84,0.73);
  legdec->AddEntry(hht,"AW","l");
  legdec->AddEntry(hhpad,"pad","l");
  legdec->Draw("same");

  cdec->cd(4);
  hmatch->Draw();
  hmatch->GetXaxis()->SetRangeUser(0.,2100.);

  cdec->cd(2);
  hot->Scale(1./hot->Integral());
  hot->Draw("HIST");
  hocol->Scale(1./hocol->Integral());
  hocol->Draw("HISTsame");
  hot->GetYaxis()->SetRangeUser(0., 
				hot->GetBinContent(hot->GetMaximumBin()) > hocol->GetBinContent(hocol->GetMaximumBin()) ? 
				hot->GetBinContent(hot->GetMaximumBin())*1.1 : hocol->GetBinContent(hocol->GetMaximumBin())*1.1
				);

  cdec->cd(3);
  htt->GetXaxis()->SetRangeUser(0.,4200.);
  htt->Draw();
  htpad->Draw("same");

  cdec->cd(5);
  hawpadsector->SetStats(kFALSE);
  //  hawpadsector->RebinX();
  hawpadsector->GetXaxis()->SetRangeUser(300.,4200.);
  hawpadsector->GetYaxis()->SetRangeUser(300.,4200.);
  hawpadsector->Draw("colz");
 
  cdec->cd(6);
  hopad->SetStats(kFALSE);
  hopad->Draw("colz");

  cdec->SaveAs(TString("plots/")+cname+TString(".pdf"));  
  cdec->SaveAs(TString("plots/")+cname+TString(".pdf"));
  }

  // spacepoints
  if( hpxy->GetEntries() > 0 )
    {
      cname = "spacepoints";
      cname+=tag;
      TCanvas* cpnt = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      cpnt->Divide(2,2);
      cpnt->cd(1);
      //      hprp->Draw("colz");
      hprp->Draw("pol surf2");
      cpnt->cd(2);
      hpxy->Draw("colz");
      cpnt->cd(3);
      hpzr->Draw("colz");
      cpnt->cd(4);
      hpzp->Draw("colz");
      cpnt->SaveAs(TString("plots/")+cname+TString(".pdf"));  
      cpnt->SaveAs(TString("plots/")+cname+TString(".pdf"));

    }

  // spacepoints in tracks
  if( hspxy->GetEntries() > 0 )
    {
      cname = "spacepoints_tracks";
      cname+=tag;
      TCanvas* csp = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      csp->Divide(2,2);
      csp->cd(1);
      hpattreceff->Draw();
      csp->cd(2);
      hspxy->Draw("colz");
      csp->cd(3);
      hspzr->Draw("colz");
      csp->cd(4);
      hspzp->Draw("colz");
      csp->SaveAs(TString("plots/")+cname+TString(".pdf"));  
      csp->SaveAs(TString("plots/")+cname+TString(".pdf"));

      cname = "spacepointsrphi_tracks";
      cname+=tag;
      TCanvas* csprphi = new TCanvas(cname.Data(),cname.Data(),1600,800);
      csprphi->Divide(2,1);
      csprphi->cd(1);
      hsprp->Draw("pol surf2");
      csprphi->cd(2);
      if(hsptrp) hsptrp->Draw("pol surf2");
    }

  cname = "lines";
  cname+=tag;
  TCanvas* cl = new TCanvas(cname.Data(),cname.Data(),1800,1400);
  cl->Divide(3,2);
  cl->cd(1);
  hNlines->Draw();      
  hNlines->GetXaxis()->SetNdivisions(110);
  hNlines->GetXaxis()->CenterLabels();
  cl->cd(2);
  hlphi->Draw();
  hlphi->GetYaxis()->SetRangeUser(0.,hlphi->GetBinContent(hlphi->GetMaximumBin())*1.1);
  cl->cd(3);
  hltheta->Draw();
  cl->cd(4);
  hlcosang->Draw();
  hlcosang->GetXaxis()->SetRangeUser(-1.,-0.75);
  cl->cd(5);
  hldist->Draw();
  cl->cd(6);
  hlcosangdist->Draw("colz");
  cl->SaveAs(TString("plots/")+cname+TString(".pdf"));  
  cl->SaveAs(TString("plots/")+cname+TString(".pdf"));



  // z axis intersection
  if( hlz->GetEntries() > 0 )
    {
      cname = "z_axis_intersection";
      cname+=tag;
      TCanvas* czint = new TCanvas(cname.Data(),cname.Data(),1600,1000);
      czint->Divide(3,2);
      czint->cd(1);
      hlr->Draw();
      //      hldz->Draw("same");
      czint->cd(2);
      hlz->Draw();
      czint->cd(3);
      hlp->Draw();
      hlp->GetYaxis()->SetRangeUser(0.,hlp->GetBinContent(hlp->GetMaximumBin())*1.1);
      czint->cd(4);
      hlzp->Draw("colz");
      czint->cd(5);
      //hlzr->Draw("colz");
      hlxy->Draw("colz");
      gPad->SetLogz();
      czint->cd(6);
      hlrp->Draw("colz");
      czint->SaveAs(TString("plots/")+cname+TString(".pdf"));  
      czint->SaveAs(TString("plots/")+cname+TString(".pdf"));
    }

  // cosmic time distribution
  if( hpois->GetEntries() > 0 )
    {
      // cosmic time distribution
      cname = "time_distribution_between_cosmics";
      cname+=tag;
      TCanvas* cpois = new TCanvas(cname.Data(),cname.Data(),1300,1000);
      hpois->Draw("P");
      hpois->Fit("expo","Q0EMW");
      TF1* fcosrate = hpois->GetFunction("expo");
      if( fcosrate )
	{
	  double rate = fabs( fcosrate->GetParameter(1) )*1.e3,
	    rate_err = fabs( fcosrate->GetParError(1) )*1.e3;
	  TString srate = TString::Format("Cosmic Rate R%d: (%1.1f#pm%1.1f) Hz",
					  RunNumber,rate,rate_err);
	  cout<<srate<<endl;
	  fcosrate->Draw("same");
	  TPaveText* trate = new TPaveText(0.5,0.53,0.87,0.6,"NDC");
	  trate->AddText(srate.Data());
	  trate->SetFillColor(0);
	  trate->Draw();
	}
      cpois->SaveAs(TString("plots/")+cname+TString(".pdf"));  
      cpois->SaveAs(TString("plots/")+cname+TString(".pdf"));

    }
}

void ProcessLine(TStoreLine* aLine)
{
  TVector3 u = *(aLine->GetDirection());
  TVector3 p = *(aLine->GetPoint());

  hlphi->Fill(u.Phi()*TMath::RadToDeg());
  hltheta->Fill(u.Theta()*TMath::RadToDeg());

  // z axis intersection
  TVector3 c = zaxis - p;
  //  TVector3 c = - p;
  double num = c.Cross(zaxis) * u.Cross(zaxis),
    den = u.Cross(zaxis).Mag2();

  // double skew = c * u.Cross(zaxis);
  // if( int(skew) )
  //   {
  //     cout<<"skew: "<<skew<<"\t";
  //   }

  if( den > 0. )
    {
      // double distz = TMath::Abs(c * u.Cross(zaxis)) / u.Cross(zaxis).Mag();
      // hldz->Fill( distz );

      double t = num/den;
      TVector3 zint = p + t*u;
      hlr->Fill( zint.Perp() );
      hlz->Fill( zint.Z() );
      hlp->Fill( zint.Phi()*TMath::RadToDeg() );
      hlzp->Fill( zint.Z(), zint.Phi()*TMath::RadToDeg() );
      hlzr->Fill( zint.Z(), zint.Perp() );
      hlrp->Fill( zint.Perp(), zint.Phi()*TMath::RadToDeg() );
      hlxy->Fill( zint.X(), zint.Y() );
      // if( int(skew) ) cout<<"distz: "<<distz<<" r: "<<zint.Perp()<<" z: "<<zint.Z();
    }
  //  if( int(skew) ) cout<<"\n";

  const TObjArray* sp = aLine->GetSpacePoints();
  for( int ip = 0; ip<sp->GetEntries(); ++ip )
    {
      TSpacePoint* ap = (TSpacePoint*) sp->At(ip);
      //      if( TMath::Abs( ap->GetX() ) > 0. && TMath::Abs( ap->GetY() ) > 0. )
      hspxy->Fill( ap->GetX(), ap->GetY() );
      hspzp->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
      hspzr->Fill( ap->GetZ(), ap->GetR() );
      hsprp->Fill( ap->GetPhi(), ap->GetR() );
    }
}

double LineDistance(TStoreLine* l0, TStoreLine* l1)
{
  TVector3 u0 = *(l0->GetDirection());
  TVector3 u1 = *(l1->GetDirection());
  TVector3 p0 = *(l0->GetPoint());
  TVector3 p1 = *(l1->GetPoint());

  TVector3 n0 = u0.Cross( u1 ); // normal to lines
  TVector3 c =  p1 - p0;
  if( n0.Mag() == 0. ) return -1.;
  
  TVector3 n1 = n0.Cross( u1 ); // normal to plane formed by n0 and line1

  double tau = c.Dot( n1 ) / u0.Dot( n1 ); // intersection between
  TVector3 q0 = tau * u0 + p0;             // plane and line0

  double t1 = ( (q0-p0).Cross(n0) ).Dot( u0.Cross(n0) ) / ( u0.Cross(n0) ).Mag2();
  TVector3 q1 = t1 * u0 + p0;

  double t2 = ( (q0-p1).Cross(n0) ).Dot( u1.Cross(n0) ) / ( u1.Cross(n0) ).Mag2();
  TVector3 q2 = t2*u1+p1;

  TVector3 Q = q2 - q1;

  return Q.Mag();
}

void ProcessTree( TTree* tin, int idx=0 )
{
  TStoreEvent* event = new TStoreEvent();
  tin->SetBranchAddress("StoredEvent", &event);
  double temp=0.;
  for(int e=0; e<tin->GetEntries(); ++e)
    {
      if( e%1000 == 0 ) cout<<"*** "<<e<<endl;
      event->Reset();
      tin->GetEntry(e);
      // cout<<event->GetEventNumber()<<"\t"<<event->GetTimeOfEvent()<<endl;
      const TObjArray* points = event->GetSpacePoints();
      for(int i=0; i<points->GetEntries(); ++i)
      	{
	  TSpacePoint* ap = (TSpacePoint*) points->At(i);
	  if( ap->IsGood(_cathradius, _fwradius) )
	    {
	      hpxy->Fill( ap->GetX(), ap->GetY() );
	      hpzr->Fill( ap->GetZ(), ap->GetR() );
	      hpzp->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
	      //hprp->Fill( ap->GetR(), ap->GetPhi()*TMath::RadToDeg() );
	      hprp->Fill( ap->GetPhi(), ap->GetR() );
	    }
      	  //cout<<"o";
      	}
      // cout<<"\n";
      //      cout<<points->GetEntries()<<" : ";
      const TObjArray* tracks = event->GetLineArray();
      int Ntracks = tracks->GetEntries();
      double Npoints = 0.;
      for(int i=0; i<Ntracks; ++i)
	{
	  //cout<<"+";
	  TStoreLine* aLine = (TStoreLine*) tracks->At(i);
	  ProcessLine( aLine );
	  Npoints += double(aLine->GetNumberOfPoints());
	}
      //      cout<<"\n";
      hNlines->Fill( double(Ntracks) );
      if( Ntracks )
	hpattreceff->Fill(Npoints/double(Ntracks));

      if( Ntracks == 2 )
	{
	  TStoreLine* l0 = (TStoreLine*) tracks->At(0);
	  TVector3 u0 = *(l0->GetDirection());
	  TStoreLine* l1 = (TStoreLine*) tracks->At(1);
	  TVector3 u1 = *(l1->GetDirection());
	  double cang = u0.Dot(u1);
	  hlcosang->Fill( cang );
	  double dist = LineDistance(l0,l1);
	  hldist->Fill( dist );
	  hlcosangdist->Fill( cang, dist );
	}

      // cosmic time distribution
      if( Ntracks >= 2 && Ntracks < 4 )
	{
	  double delta = (event->GetTimeOfEvent() - temp)*1.e3;
	  hpois->Fill( delta );
	  temp = event->GetTimeOfEvent();
	}
    }
} 

void GetSignalHistos(TFile* fin)
{
  if( fin->cd("awdeconv") )
    {  
      hht = (TH1D*)gROOT->FindObject("hNhitTop");
      if( hht ) {
      hht->SetStats(kFALSE);
      hht->SetLineColor(kOrange);
      hht->SetLineWidth(2);
      hht->SetTitle("Number of Hits per Event");
      }

      hot = (TH1D*)gROOT->FindObject("hOccTop");
      if( hot ) {
      hot->SetStats(kFALSE);
      hot->SetLineColor(kOrange);
      hot->SetLineWidth(2);
      hot->SetTitle("Occupancy per Channel");
      }

      htt = (TH1D*)gROOT->FindObject("hTimeTop");
      if( htt ) {
      htt->SetStats(kFALSE);
      htt->SetLineColor(kOrange);
      htt->SetLineWidth(2);
      htt->SetTitle("Drift Time Spectrum after Deconvolution");
      }
    }

  if( fin->cd("paddeconv") )
    { 
      hhpad = (TH1D*)gROOT->FindObject("hNhitPad");
      if( hhpad ) {
      hhpad->SetStats(kFALSE);
      hhpad->SetLineColor(kBlack);
      hhpad->SetLineWidth(2);
      }

      TH1D* hpadcol = (TH1D*)gROOT->FindObject("hOccCol");
      if( hpadcol ) {
      hocol = new TH1D("hOcccCol2",hpadcol->GetTitle(),256,0.,256.);
      hocol->SetStats(kFALSE);
      for(int b=1; b<=hpadcol->GetNbinsX(); ++b)
	{
	  double bc = hpadcol->GetBinContent(b);
	  // cout<<b-1<<"\t";
	  for( int s=0; s<8; ++s )
	    {
	      hocol->Fill((b-1)*8+s,bc);
	      // cout<<(b-1)*8+s<<" ";
	    }
	  // cout<<"\n";
	}
      hocol->SetLineColor(kBlack);
      hocol->SetLineWidth(2);

      htpad = (TH1D*)gROOT->FindObject("hTimePad");
      htpad->SetStats(kFALSE);
      htpad->SetLineColor(kBlack);
      htpad->SetLineWidth(2);

      hopad = (TH2D*)gROOT->FindObject("hOccPad");
      hopad->SetTitle("Pad channels Occupancy");
      }
    }
  
  if( fin->cd("match_el") )
    {
      // aw * pad
      hmatch = (TH1D*)gROOT->FindObject("hNmatch");
      hmatch->SetLineColor(kBlue);
      hmatch->SetLineWidth(2);

      hawpadsector = (TH2D*)gROOT->FindObject("hawcol_sector_time");
    }
}

void GetRecoHistos(TFile* fin)
{
  if( fin->cd("reco") )
    {
      hsptrp =  (TH2D*) gROOT->FindObject("hsprp");
      if(hsptrp) hsptrp->SetStats(kFALSE);
    }
}

void ProcessData( int idx = 0 )
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(fin->GetName());
  cout<<fname<<" FOUND"<<endl;
  RunNumber = TString(fname(6,5)).Atoi();
  tag+=RunNumber;
  //  tag+="_new";
  MakeHistos();

  TTree* tin = (TTree*) fin->Get("StoreEventTree");
  cout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<endl;
  ProcessTree( tin );

  GetSignalHistos(fin);
  //  GetRecoHistos(fin);

  DisplayHisto();
}

void ReadEventTree()
{
  cout<<"DATA"<<endl;
  cout<<"Run # "<<RunNumber<<endl;
  ProcessData( );
}
