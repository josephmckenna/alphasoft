TString tag("_R");
int RunNumber=0;
TString savFolder;
ofstream fout;

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
TH1D* hprad;
TH1D* hpphi;
TH1D* hpzed;

// track finding
TH2D* hsptrp;
TH1D* hsplen;
TH2D* hsprlen;
TH2D* hspNlen;

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

TH1D* hlchi2;

// line intercept
TH1D* hqphi;
TH1D* hqz;
TH1D* hqr;
TH2D* hqxy;
TH2D* hqzr;
TH2D* hqzphi;
TH2D* hqrphi;

// reco helices
TH1D* hNhel;
TH1D* hhdist;
TH1D* hhD;
TH1D* hhc;
TH1D* hhchi2R;
TH1D* hhchi2Z;

TH1D* hpt;
TH1D* hpz;
TH1D* hpp;
TH2D* hptz;

// reco helices spacepoints
TH1D* hhpattreceff;
TH2D* hhspxy;
TH2D* hhspzr;
TH2D* hhspzp;
TH2D* hhsprp;

// reco vertex
TH1D* hvr;
TH1D* hvphi;
TH1D* hvz;
TH2D* hvxy;

// used helices
TH1D* hNusedhel;
TH1D* huhD;
TH1D* huhc;
TH1D* huhchi2R;
TH1D* huhchi2Z;

TH1D* huhpt;
TH1D* huhpz;
TH1D* huhpp;
TH2D* huhptz;

// used helices spacepoints
TH1D* huhpattreceff;
TH2D* huhspxy;
TH2D* huhspzr;
TH2D* huhspzp;
TH2D* huhsprp;

// cosmic time distribution
TH1D* hpois;

// z axis intersection
// lines
TVector3 zaxis(0.,0.,1.);
TH1D* hldz;
TH1D* hlr;
TH1D* hlz;
TH1D* hlp;
TH2D* hlzp;
TH2D* hlzr;
TH2D* hlrp;
TH2D* hlxy;
// helices
TH1D* hhr;
TH1D* hhz;
TH1D* hhp;
TH2D* hhzp;
TH2D* hhzr;
TH2D* hhrp;
TH2D* hhxy;

// cosmics
TH1D* hcosaw;
TH2D* hcospad;
TH1D* hRes2min;
TH1D* hdeltaT;
TH1D* hDCAeq2;
TH1D* hDCAgr2;
TH1D* hAngeq2;
TH1D* hAnggr2;
TH2D* hAngDCAeq2;
TH2D* hAngDCAgr2;
TH1D* hcosphi;
TH1D* hcostheta;

void MakeHistos()
{
  // spacepoints
  hpxy = new TH2D("hpxy","Spacepoints;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);
  hpxy->SetStats(kFALSE);
  hpzr = new TH2D("hpzr","Spacepoints;z [mm];r [mm]",600,-1200.,1200.,61,109.,174.);
  hpzr->SetStats(kFALSE);
  hpzp = new TH2D("hpzp","Spacepoints;z [mm];#phi [deg]",600,-1200.,1200.,100,0.,360.);
  hpzp->SetStats(kFALSE);
  // hprp = new TH2D("hprp","Spacepoints;r [mm];#phi [deg]",100,108.,175.,180,0.,360.);
  hprp = new TH2D("hprp","Spacepoints;#phi [deg];r [mm]",100,0.,TMath::TwoPi(),61,109.,174.);
  hprp->SetStats(kFALSE);

  hprad = new TH1D("hprad","Spacepoints;r [mm]",61,109.,174.);
  hprad->SetStats(kFALSE);
  hpphi = new TH1D("hpphi","Spacepoints;#phi [deg]",256,0.,360.);
  hpphi->SetMinimum(0.);
  hpphi->SetStats(kFALSE);
  hpzed = new TH1D("hpzed","Spacepoints;zed [mm]",600,-1200.,1200.);
  hpzed->SetStats(kFALSE);

  // spacepoints from tracks
  hpattreceff = new TH1D("hpattreceff","Track Finding Efficiency",300,-1.,600.);
  hpattreceff->SetLineWidth(2);
  hspxy = new TH2D("hspxy","Spacepoints in Tracks;x [mm];y [mm]",
		   100,-190.,190.,100,-190.,190.);
  hspxy->SetStats(kFALSE);
  hspzr = new TH2D("hspzr","Spacepoints in Tracks;z [mm];r [mm]",
		   600,-1200.,1200.,61,109.,174.);
  hspzr->SetStats(kFALSE);
  hspzp = new TH2D("hspzp","Spacepoints in Tracks;z [mm];#phi [deg]",
		   600,-1200.,1200.,100,0.,360.);
  hspzp->SetStats(kFALSE);

  hsprp = new TH2D("hsprp","Spacepoints in Tracks;#phi [deg];r [mm]",
		   100,0.,TMath::TwoPi(),61,109.,174.);
  hsprp->SetStats(kFALSE);

  int blen = 200;
  double maxlen = 400.;
  hsplen = new TH1D("hsplen","Distance between First and Last Spacepoint;[mm]",blen,0.,maxlen);
  hsprlen = new TH2D("hsprlen","Distance between First and Last Spacepoint;r [mm]; d [mm]",
		     100,108.,175.,blen,0.,maxlen);
  hspNlen = new TH2D("hspNlen","Distance between First and Last Spacepoint;Number of Points; d [mm]",
		     200,0.,200.,blen,0.,maxlen);

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

  hlchi2 = new TH1D("hlchi2","Line #chi^{2}",100,0.,200.);

  hqr = new TH1D("hqr","Intercept Point;r [mm]",200,100.,190.);
  hqphi = new TH1D("hqphi","Intercept Point;#phi [deg]",180,-180.,180.);
  hqphi->SetMinimum(0.);
  hqz = new TH1D("hqz","Intercept Point;z [mm]",600,-1200.,1200.);
  hqxy = new TH2D("hqxy","Intercept Point;x [mm];y [mm]",
		   100,-190.,190.,100,-190.,190.);
  hqxy->SetStats(kFALSE);
  hqzr = new TH2D("hqzr","Intercept Point;z [mm];r [mm]",
		   600,-1200.,1200.,200,100.,190.);
  hqzr->SetStats(kFALSE);
  hqzphi = new TH2D("hqzphi","Intercept Point;z [mm];#phi [deg]",
		   600,-1200.,1200.,180,-180.,180.);
  hqzphi->SetStats(kFALSE);
  hqrphi = new TH2D("hqrphi","Intercept Point;r [mm];#phi [deg];",
		    200,100.,190.,180,-180.,180.);
  hqzphi->SetStats(kFALSE);

  // reco helices
  hNhel = new TH1D("hNhel","Reconstructed Helices",10,0.,10.);
  //  hhdist = new TH1D("hhdist","Distance between  2 helices;s [mm]",200,0.,20.);

  hhD = new TH1D("hhD","Hel D;[mm]",200,0.,200.);
  hhc = new TH1D("hhc","Hel c;[mm^{-1}]",200,-1.e-1,1.e-1);
  hhchi2R = new TH1D("hhchi2R","Hel #chi^{2}_{R}",100,0.,50.);
  hhchi2Z = new TH1D("hhchi2Z","Hel #chi^{2}_{Z}",100,0.,50.);

  hpt = new TH1D("hpt","Helix Transverse Momentum;p_{T} [MeV/c]",1000,0.,2000.);
  hpz = new TH1D("hpz","Helix Longitudinal Momentum;p_{Z} [MeV/c]",2000,-1000.,1000.);
  hpp = new TH1D("hpp","Helix Total Momentum;p_{tot} [MeV/c]",1000,0.,2000.);
  hptz = new TH2D("hptz","Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
		  100,0.,2000.,200,-1000.,1000.);

  // reco helices spacepoints
  hhpattreceff = new TH1D("hhpattreceff","Track Finding Efficiency",300,-1.,600.);
  hhpattreceff->SetLineWidth(2);;
  hhspxy = new TH2D("hhspxy","Spacepoints in Helices;x [mm];y [mm]",
		    100,-190.,190.,100,-190.,190.);
  hhspxy->SetStats(kFALSE);
  hhspzr = new TH2D("hhspzr","Spacepoints in Helices;z [mm];r [mm]",
		    600,-1200.,1200.,61,109.,174.);
  hhspzr->SetStats(kFALSE);
  hhspzp = new TH2D("hhspzp","Spacepoints in Helices;z [mm];#phi [deg]",
		    600,-1200.,1200.,100,0.,360.);
  hhspzp->SetStats(kFALSE);

  hhsprp = new TH2D("hhsprp","Spacepoints in Helices;#phi [deg];r [mm]",
		    100,0.,TMath::TwoPi(),61,109.,174.);
  hhsprp->SetStats(kFALSE);


  // used helices
  hNusedhel = new TH1D("hNusedhel","Used Helices",10,0.,10.);

  huhD = new TH1D("huhD","Used Hel D;[mm]",200,0.,200.);
  huhc = new TH1D("huhc","Used Hel c;[mm^{-1}]",200,-1.e-1,1.e-1);
  huhchi2R = new TH1D("huhchi2R","Used Hel #chi^{2}_{R}",100,0.,50.);
  huhchi2Z = new TH1D("huhchi2Z","Used Hel #chi^{2}_{Z}",100,0.,50.);

  huhpt = new TH1D("huhpt","Used Helix Transverse Momentum;p_{T} [MeV/c]",1000,0.,2000.);
  huhpz = new TH1D("huhpz","Used Helix Longitudinal Momentum;p_{Z} [MeV/c]",2000,-1000.,1000.);
  huhpp = new TH1D("huhpp","Used Helix Total Momentum;p_{tot} [MeV/c]",1000,0.,2000.);
  huhptz = new TH2D("huhptz","Used Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
		    100,0.,2000.,200,-1000.,1000.);

  // used helices spacepoints
  huhspxy = new TH2D("huhspxy","Spacepoints in Used Helices;x [mm];y [mm]",
		     100,-190.,190.,100,-190.,190.);
  huhspxy->SetStats(kFALSE);
  huhspzr = new TH2D("huhspzr","Spacepoints in Used Helices;z [mm];r [mm]",
		     600,-1200.,1200.,61,109.,174.);
  huhspzr->SetStats(kFALSE);
  huhspzp = new TH2D("huhspzp","Spacepoints in Used Helices;z [mm];#phi [deg]",
		     600,-1200.,1200.,100,0.,360.);
  huhspzp->SetStats(kFALSE);

  huhsprp = new TH2D("huhsprp","Spacepoints in Used Helices;#phi [deg];r [mm]",
		     100,0.,TMath::TwoPi(),90,109.,174.);
  huhsprp->SetStats(kFALSE);

  // reco vertex
  hvr = new TH1D("hvr","Vertex Radius;r [mm]",190,0.,190.);
  hvphi = new TH1D("hvphi","Vertex #phi; [deg]",360,-180.,180.);
  hvphi->SetMinimum(0.);
  hvz = new TH1D("hvz","Vertex Z;z [mm]",1000,-1152.,1152.);
  hvxy = new TH2D("hvxy","Vertex X-Y;x [mm];y [mm]",200,-190.,190.,200,-190.,190.);

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

  hhr = new TH1D("hhr","Helix Minimum Radius;r [mm]",200,0.,190.);
  hhz = new TH1D("hhz","Helix Z intersection with min rad;z [mm]",300,-1200.,1200.);
  hhp = new TH1D("hhp","Helix #phi intersection with min rad;#phi [deg]",100,-180.,180.);
  hhzp = new TH2D("hhzp","Helix Z-#phi intersection with min rad;z [mm];#phi [deg]",
		  100,-1200.,1200.,90,-180.,180.);
  hhzp->SetStats(kFALSE);
  hhzr = new TH2D("hhzr","Helix Z-R intersection with min rad;z [mm];r [mm]",
		  100,-1200.,1200.,100,0.,190.);
  hhrp = new TH2D("hhrp","Helix R-#phi intersection with min rad;r [mm];#phi [deg]",
		  100,0.,190.,90,-180.,180.);
  hhxy = new TH2D("hhxy","Helix X-Y intersection with min rad;x [mm];y [mm]",
		  100,-190.,190.,100,-190.,190.);

  // cosmics
  hRes2min = new TH1D("hRes2","Minimum Residuals Squared Divide by Number of Spacepoints from 2 Helices;#delta [mm^{2}]",
		      1000,0.,1000.);
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
    if( hhpad->GetEntries() > 0. )
      {
	hhpad->Draw("same");
	hht->GetXaxis()->SetRangeUser(0.,2100.);
	hht->GetYaxis()->SetRangeUser(0.,
				      hht->GetBinContent(hht->GetMaximumBin()) > hhpad->GetBinContent(hhpad->GetMaximumBin()) ?
				      hht->GetBinContent(hht->GetMaximumBin())*1.1 : hhpad->GetBinContent(hhpad->GetMaximumBin())*1.1
				      );
      }
    else
      hht->GetYaxis()->SetRangeUser(0.,hht->GetBinContent(hht->GetMaximumBin())*1.1);

    TLegend* legdec = new TLegend(0.65,0.62,0.84,0.73);
    legdec->AddEntry(hht,"AW","l");
    legdec->AddEntry(hhpad,"pad","l");
    legdec->Draw("same");

    if( hmatch->GetEntries() > 0 )
      {
	cdec->cd(4);
	hmatch->Draw();
	hmatch->GetXaxis()->SetRangeUser(0.,2100.);
      }

    cdec->cd(2);
    hot->Scale(1./hot->Integral());
    //  hot->Draw("HIST");
    if( hocol->GetEntries() > 0 )
      {
	hot->Draw("HIST");
	hocol->Scale(1./hocol->Integral());
	hocol->Draw("HISTsame");
	hot->GetYaxis()->SetRangeUser(0.,
				      hot->GetBinContent(hot->GetMaximumBin()) > hocol->GetBinContent(hocol->GetMaximumBin()) ?
				      hot->GetBinContent(hot->GetMaximumBin())*1.1 : hocol->GetBinContent(hocol->GetMaximumBin())*1.1
				      );

      }
    else
      {
	cout<<hot->GetName()<<"\t"<<hot->GetBinContent(hot->GetMaximumBin())<<endl;
	hot->Draw();
	hot->GetYaxis()->SetRangeUser( 0.,hot->GetBinContent(hot->GetMaximumBin())*1.1 );
      }

    cdec->cd(3);
    htt->GetXaxis()->SetRangeUser(0.,4200.);
    htt->Draw();
    if( htpad->GetEntries() > 0 )
      {
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
      }

    cdec->SaveAs(savFolder+cname+TString(".pdf"));
    cdec->SaveAs(savFolder+cname+TString(".pdf"));
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
      cpnt->SaveAs(savFolder+cname+TString(".pdf"));
      cpnt->SaveAs(savFolder+cname+TString(".pdf"));

      cname = "spacepoints_coord";
      cname+=tag;
      TCanvas* cpntcoord = new TCanvas(cname.Data(),cname.Data(),1800,1400);
      cpntcoord->Divide(1,3);
      cpntcoord->cd(1);
      hprad->Draw();
      cpntcoord->cd(2);
      hpphi->Draw();
      cpntcoord->cd(3);
      hpzed->Draw();
      cpntcoord->SaveAs(savFolder+cname+TString(".pdf"));
      cpntcoord->SaveAs(savFolder+cname+TString(".pdf"));
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
      csp->SaveAs(savFolder+cname+TString(".pdf"));
      csp->SaveAs(savFolder+cname+TString(".pdf"));

      cname = "spacepoint_lines";
      cname+=tag;
      TCanvas* csprphi = new TCanvas(cname.Data(),cname.Data(),1600,1000);
      csprphi->Divide(2,2);
      csprphi->cd(1);
      hsprp->Draw("pol surf2");
      csprphi->cd(2);
      //if(hsptrp) hsptrp->Draw("pol surf2");
      hsplen->Draw("colz");
      csprphi->cd(3);
      hsprlen->Draw("colz");
      csprphi->cd(4);
      hspNlen->Draw("colz");
      csprphi->SaveAs(savFolder+cname+TString(".pdf"));
      csprphi->SaveAs(savFolder+cname+TString(".pdf"));
    }

  //  if( hNlines->GetEntries() )
  if( hlphi->GetEntries() )
    {
      cname = "lines";
      cname+=tag;
      TCanvas* cl = new TCanvas(cname.Data(),cname.Data(),1800,1400);
      cout<<cname<<endl;
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
      hlcosang->GetXaxis()->SetRangeUser(-1.,-0.9);
      cl->cd(5);
      hldist->Draw();
      cl->cd(6);
      hlcosangdist->Draw("colz");
      cl->SaveAs(savFolder+cname+TString(".pdf"));
      cl->SaveAs(savFolder+cname+TString(".pdf"));

      cname = "lines_intercept";
      cname+=tag;
      TCanvas* cq = new TCanvas(cname.Data(),cname.Data(),2600,1400);
      cout<<cname<<endl;
      cq->Divide(4,2);
      cq->cd(1);
      hlchi2->Draw("HIST");
      cq->cd(2);
      hqr->Draw("HIST");
      cq->cd(3);
      hqphi->Draw("HIST");
      cq->cd(4);
      hqz->Draw("HIST");
      cq->cd(5);
      hqxy->Draw("colz");
      cq->cd(6);
      hqzr->Draw("colz");
      cq->cd(7);
      hqzphi->Draw("colz");
      cq->cd(8);
      hqrphi->Draw("colz");
      cq->SaveAs(savFolder+cname+TString(".pdf"));  
      cq->SaveAs(savFolder+cname+TString(".pdf"));
    }

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
      czint->SaveAs(savFolder+cname+TString(".pdf"));
      czint->SaveAs(savFolder+cname+TString(".pdf"));
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
      cpois->SaveAs(savFolder+cname+TString(".pdf"));
      cpois->SaveAs(savFolder+cname+TString(".pdf"));

    }

  // reco helices
  //  if(hNhel->GetEntries())
  if(hhD->GetEntries())
    {
      cname = "chel";
      cname+=tag;
      TCanvas* chel = new TCanvas(cname.Data(),cname.Data(),1000,800);
      chel->Divide(2,1);
      chel->cd(1);
      hNhel->Draw();
      chel->cd(2);
      //      hhdist->Draw();
      hhpattreceff->Draw();
      chel->SaveAs(savFolder+cname+TString(".pdf"));
      chel->SaveAs(savFolder+cname+TString(".pdf"));

      cname ="chelprop";
      cname+=tag;
      TCanvas* chelprop = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      chelprop->Divide(2,2);
      chelprop->cd(1);
      hhD->Draw();
      chelprop->cd(2);
      hhc->Draw();
      chelprop->cd(3);
      hhchi2R->Draw();
      chelprop->cd(4);
      hhchi2Z->Draw();
      chelprop->SaveAs(savFolder+cname+TString(".pdf"));
      chelprop->SaveAs(savFolder+cname+TString(".pdf"));

      cname ="chelmom";
      cname+=tag;
      TCanvas* chelmom = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      chelmom->Divide(2,2);
      chelmom->cd(1);
      hpt->Draw();
      chelmom->cd(2);
      hpz->Draw();
      chelmom->cd(3);
      hpp->Draw();
      chelmom->cd(4);
      hptz->Draw("colz");
      chelmom->SaveAs(savFolder+cname+TString(".pdf"));
      chelmom->SaveAs(savFolder+cname+TString(".pdf"));

      cname = "spacepoints_helices";
      cname+=tag;
      TCanvas* chsp = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      chsp->Divide(2,2);
      chsp->cd(1);
      hhspxy->Draw("colz");
      chsp->cd(2);
      hhspzr->Draw("colz");
      chsp->cd(3);
      hhspzp->Draw("colz");
      chsp->cd(4);
      hhsprp->Draw("colz");
      chsp->SaveAs(savFolder+cname+TString(".pdf"));
      chsp->SaveAs(savFolder+cname+TString(".pdf"));
    }

  // vertex
  if( hvr->GetEntries() )
    {
      cname="cvtx";
      cname+=tag;
      TCanvas* cvtx = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      cvtx->Divide(2,2);
      cvtx->cd(1);
      hvr->Draw();
      cvtx->cd(2);
      hvphi->Draw();
      cvtx->cd(3);
      hvz->Draw();
      cvtx->cd(4);
      hvxy->Draw("colz");
      cvtx->SaveAs(savFolder+cname+TString(".pdf"));
      cvtx->SaveAs(savFolder+cname+TString(".pdf"));
    }

  // used helices
  //  if(hNusedhel->GetEntries())
  if(huhD->GetEntries())
    {
      cname = "cusehel";
      cname+=tag;
      TCanvas* cusehel = new TCanvas(cname.Data(),cname.Data(),1000,800);
      hNusedhel->Draw();
      cusehel->SaveAs(savFolder+cname+TString(".pdf"));
      cusehel->SaveAs(savFolder+cname+TString(".pdf"));

      cname ="cusehelprop";
      cname+=tag;
      TCanvas* cusehelprop = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      cusehelprop->Divide(2,2);
      cusehelprop->cd(1);
      huhD->Draw();
      cusehelprop->cd(2);
      huhc->Draw();
      cusehelprop->cd(3);
      huhchi2R->Draw();
      cusehelprop->cd(4);
      huhchi2Z->Draw();
      cusehelprop->SaveAs(savFolder+cname+TString(".pdf"));
      cusehelprop->SaveAs(savFolder+cname+TString(".pdf"));

      cname ="cusehelmom";
      cname+=tag;
      TCanvas* cusehelmom = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      cusehelmom->Divide(2,2);
      cusehelmom->cd(1);
      huhpt->Draw();
      cusehelmom->cd(2);
      huhpz->Draw();
      cusehelmom->cd(3);
      huhpp->Draw();
      cusehelmom->cd(4);
      huhptz->Draw("colz");
      cusehelmom->SaveAs(savFolder+cname+TString(".pdf"));
      cusehelmom->SaveAs(savFolder+cname+TString(".pdf"));

      cname = "spacepoints_usedhelices";
      cname+=tag;
      TCanvas* chsp = new TCanvas(cname.Data(),cname.Data(),1400,1400);
      chsp->Divide(2,2);
      chsp->cd(1);
      huhspxy->Draw("colz");
      chsp->cd(2);
      huhspzr->Draw("colz");
      chsp->cd(3);
      huhspzp->Draw("colz");
      chsp->cd(4);
      huhsprp->Draw("colz");
      chsp->SaveAs(savFolder+cname+TString(".pdf"));
      chsp->SaveAs(savFolder+cname+TString(".pdf"));
    }

  // if(hcosaw->GetEntries())
  //   {
  //     cname = "ccos";
  //     cname+=tag;
  //     TCanvas* ccos = new TCanvas(cname.Data(),cname.Data(),1400,1200);
  //     ccos->Divide(2,2);
  //     ccos->cd(1);
  //     hcosaw->Draw();
  //     ccos->cd(2);
  //     hcospad->Draw("colz");
  //     ccos->cd(3);
  //     hRes2min->Draw();
  //     ccos->cd(4);
  //     hdeltaT->Draw("P");
  //     //TF1* fdeltaT = new TF1("fdeltaT","[0]*exp([1]*x+[2])",0.,300.);
  //     //fdeltaT->SetParameters(hdeltaT->GetBinContent(4),20.,-3.);
  //     //hdeltaT->Fit(fdeltaT,"0EMW");
  //     hdeltaT->Fit("expo","Q0EMW");
  //     TF1* fdeltaT = hdeltaT->GetFunction("expo");
  //     if( fdeltaT )
  // 	{
  // 	  double rate = fabs( fdeltaT->GetParameter(1) )*1.e3,
  // 	    rate_err = fabs( fdeltaT->GetParError(1) )*1.e3;
  // 	  TString srate = TString::Format("Cosmic Rate R%d: (%1.1f#pm%1.1f) Hz",
  // 					  RunNumber,rate,rate_err);
  // 	  cout<<srate<<endl;
  // 	  fdeltaT->Draw("same");
  // 	  TPaveText* trate = new TPaveText(0.5,0.53,0.87,0.6,"NDC");
  // 	  trate->AddText(srate.Data());
  // 	  trate->SetFillColor(0);
  // 	  trate->Draw();
  // 	}
  //     ccos->SaveAs(savFolder+cname+TString(".pdf"));
  //     ccos->SaveAs(savFolder+cname+TString(".pdf"));

  //     cname="ccosdir";
  //     cname+=tag;
  //     TCanvas* ccosdir = new TCanvas(cname.Data(),cname.Data(),1200,1000);
  //     ccosdir->Divide(1,2);
  //     ccosdir->cd(1);
  //     hcosphi->Draw();
  //     ccosdir->cd(2);
  //     hcostheta->Draw();
  //     ccosdir->SaveAs(savFolder+cname+TString(".pdf"));
  //     ccosdir->SaveAs(savFolder+cname+TString(".pdf"));

  //     cname="ccosres";
  //     cname+=tag;
  //     TCanvas* ccosres = new TCanvas(cname.Data(),cname.Data(),1600,1200);
  //     ccosres->Divide(3,2);
  //     ccosres->cd(1);
  //     hDCAeq2->Draw();
  //     ccosres->cd(2);
  //     hAngeq2->Draw();
  //     ccosres->cd(3);
  //     hAngDCAeq2->Draw("colz");
  //     ccosres->cd(4);
  //     hDCAgr2->Draw();
  //     ccosres->cd(5);
  //     hAnggr2->Draw();
  //     ccosres->cd(6);
  //     hAngDCAgr2->Draw("colz");
  //     ccosres->SaveAs(savFolder+cname+TString(".pdf"));
  //     ccosres->SaveAs(savFolder+cname+TString(".pdf"));
  //   }

}

void ProcessLine(TStoreLine* aLine)
{
  TVector3 u = *(aLine->GetDirection());
  TVector3 p = *(aLine->GetPoint());

  hlphi->Fill(u.Phi()*TMath::RadToDeg());
  hltheta->Fill(u.Theta()*TMath::RadToDeg());

  // z axis intersection
  // TVector3 c = -1.*p;
  // double num = c.Cross(zaxis) * u.Cross(zaxis);
  double num = zaxis.Cross(p) * u.Cross(zaxis);
  double den = u.Cross(zaxis).Mag2();

  if( den > 0. )
    {
      double t = num/den;
      TVector3 zint = p + t*u;
      hlr->Fill( zint.Perp() );
      hlz->Fill( zint.Z() );
      hlp->Fill( zint.Phi()*TMath::RadToDeg() );
      hlzp->Fill( zint.Z(), zint.Phi()*TMath::RadToDeg() );
      hlzr->Fill( zint.Z(), zint.Perp() );
      hlrp->Fill( zint.Perp(), zint.Phi()*TMath::RadToDeg() );
      hlxy->Fill( zint.X(), zint.Y() );
    }

  hlchi2->Fill(aLine->GetChi2());

  hqr->Fill(p.Perp());
  hqz->Fill(p.Z());
  hqphi->Fill(p.Phi()*TMath::RadToDeg());
  hqxy->Fill(p.X(),p.Y());
  hqzr->Fill(p.Z(),p.Perp());
  hqzphi->Fill(p.Z(),p.Phi()*TMath::RadToDeg());
  hqrphi->Fill(p.Perp(),p.Phi()*TMath::RadToDeg());


  const TObjArray* sp = aLine->GetSpacePoints();
  for( int ip = 0; ip<sp->GetEntries(); ++ip )
    {
      TSpacePoint* ap = (TSpacePoint*) sp->At(ip);
      hspxy->Fill( ap->GetX(), ap->GetY() );
      hspzp->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
      hspzr->Fill( ap->GetZ(), ap->GetR() );
      hsprp->Fill( ap->GetPhi(), ap->GetR() );
    }
  double maxd= ((TSpacePoint*)sp->Last())->Distance( (TSpacePoint*)sp->First() );
  hsplen->Fill( maxd );
  hsprlen->Fill( ((TSpacePoint*)sp->Last())->GetR(), maxd );
  hspNlen->Fill( double(sp->GetEntries()), maxd );
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

void ProcessHelix(TStoreHelix* hel)
{
  hhD->Fill(hel->GetD());
  hhc->Fill(hel->GetC());
  hhchi2R->Fill(hel->GetRchi2());
  hhchi2Z->Fill(hel->GetZchi2());

  //  hel->GetMomentumV().Print();

  hpt->Fill(hel->GetMomentumV().Perp());
  hpz->Fill(hel->GetMomentumV().Z());
  hpp->Fill(hel->GetMomentumV().Mag());
  hptz->Fill(hel->GetMomentumV().Perp(),hel->GetMomentumV().Z());

  const TObjArray* sp = hel->GetSpacePoints();
  for( int ip = 0; ip<sp->GetEntries(); ++ip )
    {
      TSpacePoint* ap = (TSpacePoint*) sp->At(ip);
      hhspxy->Fill( ap->GetX(), ap->GetY() );
      hhspzp->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
      hhspzr->Fill( ap->GetZ(), ap->GetR() );
      hhsprp->Fill( ap->GetPhi(), ap->GetR() );
    }
}

void ProcessUsed(TFitHelix* hel)
{
  huhD->Fill(hel->GetD());
  huhc->Fill(hel->GetC());
  huhchi2R->Fill(hel->GetRchi2());
  huhchi2Z->Fill(hel->GetZchi2());

  //  hel->GetMomentumV().Print();

  huhpt->Fill(hel->GetMomentumV().Perp());
  huhpz->Fill(hel->GetMomentumV().Z());
  huhpp->Fill(hel->GetMomentumV().Mag());
  huhptz->Fill(hel->GetMomentumV().Perp(),hel->GetMomentumV().Z());

  const vector<TSpacePoint*> *sp = hel->GetPointsArray();
  for( int ip = 0; ip<sp->size(); ++ip )
    {
        TSpacePoint* ap = sp->at(ip);
      huhspxy->Fill( ap->GetX(), ap->GetY() );
      huhspzp->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
      huhspzr->Fill( ap->GetZ(), ap->GetR() );
      huhsprp->Fill( ap->GetPhi(), ap->GetR() );
    }
}

void ProcessVertex(TVector3* v)
{
  hvr->Fill(v->Perp());
  hvphi->Fill(v->Phi()*TMath::RadToDeg());
  hvz->Fill(v->Z());
  hvxy->Fill(v->X(),v->Y());
}

void ProcessTree( TTree* tin, int idx=0 )
{
  TStoreEvent* event = new TStoreEvent();
  tin->SetBranchAddress("StoredEvent", &event);
  double temp=0.;
  double Nvtx=0.;
  for(int e=0; e<tin->GetEntries(); ++e)
    {
      if( e%1000 == 0 ) cout<<"*** "<<e<<endl;
      event->Reset();
      tin->GetEntry(e);
      //      cout<<event->GetEventNumber()<<"\t"<<event->GetTimeOfEvent()<<endl;
      const TObjArray* points = event->GetSpacePoints();
      //      cout<<"Number of Points: "<<points->GetEntries()<<endl;
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

	      hprad->Fill( ap->GetR() );
	      hpphi->Fill( ap->GetPhi()*TMath::RadToDeg() );
	      hpzed->Fill( ap->GetZ() );
	    }
	}
      //      event->Print();
      //      continue;
      const TObjArray* tracks = event->GetLineArray();
      int Ntracks = tracks->GetEntries();
      //      cout<<"Number of Tracks: "<<Ntracks<<endl;

      double Npoints = 0.;
      for(int i=0; i<Ntracks; ++i)
	{
	  TStoreLine* aLine = (TStoreLine*) tracks->At(i);
	  ProcessLine( aLine );
	  Npoints += double(aLine->GetNumberOfPoints());
	}
      hNlines->Fill( double(Ntracks) );
      if( Ntracks )
	{
	  hpattreceff->Fill(Npoints/double(Ntracks));
	  //	  cout<<"PattRecEff: "<<Npoints/double(Ntracks)<<endl;
	}

      const TObjArray* helices = event->GetHelixArray();
      int Nhelices = helices->GetEntries();
      //      cout<<"Number of Helices: "<<Nhelices<<endl;
      Npoints = 0.;
      for(int i=0; i<Nhelices; ++i)
	{
	  TStoreHelix* aHelix = (TStoreHelix*) helices->At(i);
	  ProcessHelix( aHelix );
	  Npoints += double(aHelix->GetNumberOfPoints());
	}
      hNhel->Fill( double(Nhelices) );
      if( Nhelices )
	{
	  hhpattreceff->Fill(Npoints/double(Nhelices));
	}

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
      //      if( Ntracks >= 2 && Ntracks < 4 )
      if( Nhelices >= 2 && Nhelices < 4 )
	{
	  double delta = (event->GetTimeOfEvent() - temp)*1.e3;
	  hpois->Fill( delta );
	  temp = event->GetTimeOfEvent();
	}
      TVector3 vtx = event->GetVertex();
      if(event->GetVertexStatus()>0)
	{
	  const TObjArray* used_hel = event->GetUsedHelices();
	  hNusedhel->Fill( double(used_hel->GetEntries()) );
	  for(int ih=0; ih<used_hel->GetEntries(); ++ih) ProcessUsed((TFitHelix*) used_hel->At(ih));
	  ProcessVertex(&vtx);
	  ++Nvtx;
	}
      //      cout<<"End of Event"<<endl;
    }
  cout<<"Number of Events Processed: "<<tin->GetEntries()<<endl;
  cout<<"Number of Reconstructed Vertexes: "<<Nvtx<<endl;
  cout<<"Total Runtime: "<<temp<<" s"<<endl;
  cout<<"Cosmic Rate: "<<Nvtx/temp<<" s^-1"<<endl;
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
  else
    {
      cout<<"skipping signals"<<endl;
      return;
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
		if( hpadcol->GetEntries() == 0 ) break;
		double bc = hpadcol->GetBinContent(b);
		// cout<<b-1<<"\t";
		for( int s=0; s<8; ++s )
		  {
		    hocol->Fill((b-1)*8+s,bc);
		    // cout<<(b-1)*8+s<<" ";
		  }
		// cout<<"\n";
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

void GetCosmicHistos(TFile* fin)
{
  if( fin->cd("cosmics") )
    {
      hcosaw = (TH1D*) gROOT->FindObject("hcosaw");
      hcospad = (TH2D*) gROOT->FindObject("hcospad");
      hcospad->SetStats(kFALSE);
      hRes2min = (TH1D*) gROOT->FindObject("hRes2min");
      hdeltaT = (TH1D*) gROOT->FindObject("hpois");
      hdeltaT->SetMarkerColor(kBlack);
      hdeltaT->SetMarkerStyle(8);
      hdeltaT->SetLineColor(kBlack);

      hDCAeq2 = (TH1D*) gROOT->FindObject("hDCAeq2");
      hDCAgr2 = (TH1D*) gROOT->FindObject("hDCAgr2");
      hAngeq2 = (TH1D*) gROOT->FindObject("hAngeq2");
      hAnggr2 = (TH1D*) gROOT->FindObject("hAnggr2");
      hAngDCAeq2 = (TH2D*) gROOT->FindObject("hAngDCAeq2");
      hAngDCAeq2->SetStats(kFALSE);
      hAngDCAgr2 = (TH2D*) gROOT->FindObject("hAngDCAgr2");
      hAngDCAgr2->SetStats(kFALSE);

      hcosphi = (TH1D*) gROOT->FindObject("hcosphi");
      hcosphi->SetMinimum(0);
      hcostheta = (TH1D*) gROOT->FindObject("hcostheta");

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

void WriteRunStats()
{
  fout<<"===== Run Stats ====="<<endl;
  fout<<"Title\t\tEntries\tMean\tRMS\n";
  fout<<"----------------------------------------------------------------------------------------\n";
  fout<<hht->GetTitle()<<"\t"<<setw(8)<<hht->GetEntries()<<"\t"<<setw(5)<<hht->GetMean()<<"\t"<<setw(5)<<hht->GetRMS()<<endl;
  fout<<hhpad->GetTitle()<<"\t"<<setw(8)<<hhpad->GetEntries()<<"\t"<<setw(5)<<hhpad->GetMean()<<"\t"<<setw(5)<<hhpad->GetRMS()<<endl;  
  fout<<hmatch->GetTitle()<<"\t"<<setw(8)<<hmatch->GetEntries()<<"\t"<<setw(5)<<hmatch->GetMean()<<"\t"<<setw(5)<<hmatch->GetRMS()<<endl;
  fout<<hpzed->GetTitle()<<"\t"<<setw(8)<<hpzed->GetEntries()<<"\t"<<setw(5)<<hpzed->GetMean()<<"\t"<<setw(5)<<hpzed->GetRMS()<<endl;
  fout<<"\n";
  fout<<hpattreceff->GetTitle()<<"\t"<<setw(8)<<hpattreceff->GetEntries()<<"\t"<<setw(5)<<hpattreceff->GetMean()<<"\t"<<setw(5)<<hpattreceff->GetRMS()<<endl;
  fout<<hNlines->GetTitle()<<"\t"<<setw(8)<<hNlines->GetEntries()<<"\t"<<setw(5)<<hNlines->GetMean()<<"\t"<<setw(5)<<hNlines->GetRMS()<<endl;
  fout<<hlcosang->GetTitle()<<"\t"<<setw(8)<<hlcosang->GetEntries()<<"\t"<<setw(5)<<hlcosang->GetMean()<<"\t"<<setw(5)<<hlcosang->GetRMS()<<endl;
  fout<<hldist->GetTitle()<<"\t"<<setw(8)<<hldist->GetEntries()<<"\t"<<setw(5)<<hldist->GetMean()<<"\t"<<setw(5)<<hldist->GetRMS()<<endl;
  fout<<hlchi2->GetTitle()<<"\t"<<setw(8)<<hlchi2->GetEntries()<<"\t"<<setw(5)<<hlchi2->GetMean()<<"\t"<<setw(5)<<hlchi2->GetRMS()<<endl;
  fout<<"\n";
  fout<<hhpattreceff->GetTitle()<<"\t"<<setw(8)<<hhpattreceff->GetEntries()<<"\t"<<setw(5)<<hhpattreceff->GetMean()<<"\t"<<setw(5)<<hhpattreceff->GetRMS()<<endl;
  fout<<hNhel->GetTitle()<<"\t"<<setw(8)<<hNhel->GetEntries()<<"\t"<<setw(5)<<hNhel->GetMean()<<"\t"<<setw(5)<<hNhel->GetRMS()<<endl;
  fout<<hhchi2R->GetTitle()<<"\t"<<setw(8)<<hhchi2R->GetEntries()<<"\t"<<setw(5)<<hhchi2R->GetMean()<<"\t"<<setw(5)<<hhchi2R->GetRMS()<<endl;
  fout<<hhchi2Z->GetTitle()<<"\t"<<setw(8)<<hhchi2Z->GetEntries()<<"\t"<<setw(5)<<hhchi2Z->GetMean()<<"\t"<<setw(5)<<hhchi2Z->GetRMS()<<endl;
  fout<<hpp->GetTitle()<<"\t"<<setw(8)<<hpp->GetEntries()<<"\t"<<setw(5)<<hpp->GetMean()<<"\t"<<setw(5)<<hpp->GetRMS()<<endl;
  fout<<"\n";
  fout<<hNusedhel->GetTitle()<<"\t"<<setw(8)<<hNusedhel->GetEntries()<<"\t"<<setw(5)<<hNusedhel->GetMean()<<"\t"<<setw(5)<<hNusedhel->GetRMS()<<endl;
  fout<<huhchi2R->GetTitle()<<"\t"<<setw(8)<<huhchi2R->GetEntries()<<"\t"<<setw(5)<<huhchi2R->GetMean()<<"\t"<<setw(5)<<huhchi2R->GetRMS()<<endl;
  fout<<huhchi2Z->GetTitle()<<"\t"<<setw(8)<<huhchi2Z->GetEntries()<<"\t"<<setw(5)<<huhchi2Z->GetMean()<<"\t"<<setw(5)<<huhchi2Z->GetRMS()<<endl;
  fout<<huhpp->GetTitle()<<"\t"<<setw(8)<<huhpp->GetEntries()<<"\t"<<setw(5)<<huhpp->GetMean()<<"\t"<<setw(5)<<huhpp->GetRMS()<<endl;
  fout<<"\n";
  fout<<hvr->GetTitle()<<"\t"<<setw(8)<<hvr->GetEntries()<<"\t"<<setw(5)<<hvr->GetMean()<<"\t"<<setw(5)<<hvr->GetRMS()<<endl;
  fout<<hvz->GetTitle()<<"\t"<<setw(8)<<hvz->GetEntries()<<"\t"<<setw(5)<<hvz->GetMean()<<"\t"<<setw(5)<<hvz->GetRMS()<<endl;
  fout<<"\n";
  
  fout<<"\n";
  fout<<hNlines->GetTitle()<<"\t0 line: "<<hNlines->GetBinContent(1)<<"\t1 line: "<<hNlines->GetBinContent(2)<<"\t2 lines: "<<hNlines->GetBinContent(3)<<"\t>2 lines: "<<hNlines->Integral(4,10)<<endl;
  fout<<"\n";
  fout<<hNhel->GetTitle()<<"\t0 helixs: "<<hNhel->GetBinContent(1)<<"\t1 helix: "<<hNhel->GetBinContent(2)<<"\t2 helixs: "<<hNhel->GetBinContent(3)<<"\t>2 helixs: "<<hNhel->Integral(4,10)<<endl;
}

void ProcessData( TFile* fin )
{
  cout<<"ProcessData --> Run Number: "<<RunNumber<<endl;
  fout<<"ProcessData --> Run Number: "<<RunNumber<<endl;
  MakeHistos();

  TTree* tin = (TTree*) fin->Get("StoreEventTree");
  cout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<endl;
  fout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<endl;
  ProcessTree( tin );

  GetSignalHistos(fin);
  //  GetRecoHistos(fin);
  //  GetCosmicHistos(fin);

  cout<<"DisplayHisto"<<endl;
  DisplayHisto();

  cout<<"Write Run Stats"<<endl;
  WriteRunStats();
}

// int GetRunNumber( TString fname )
// {
//   TRegexp re("[0-9][0-9][0-9][0-9][0-9]");
//   int pos = fname.Index(re);
//   int run = TString(fname(pos,5)).Atoi();
//   return run;
// }

void copy_file( const char* srce_file, const char* dest_file )
{
  ifstream srce( srce_file, std::ios::binary ) ;
  ofstream dest( dest_file, std::ios::binary ) ;
  dest << srce.rdbuf() ;
}
void ReadEventTree()
{
  cout<<"DATA"<<endl;
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(fin->GetName());
  cout<<fname<<" FOUND"<<endl;

  RunNumber = GetRunNumber( fname );
  cout<<"Run # "<<RunNumber<<endl;
  tag+=RunNumber;

  savFolder=MakeAutoPlotsFolder("time");

  TString foutname(savFolder+"/statR"+RunNumber+".txt");
  fout.open(foutname.Data());
  //fout<<"Hello!\nThis is a test for run: "<<RunNumber<<"\nBye!"<<endl;
  fout<<"Filename: "<<fname<<endl;

  ProcessData( fin );

  fout.close();

  TString logfile = TString::Format("%s/R%d.log",
				    getenv("AGRELEASE"),RunNumber);
  TString bkpfile = TString::Format("%s/ana/%s/R%d.log",
				    getenv("AGRELEASE"),savFolder.Data(),
				    RunNumber);
  copy_file(logfile.Data(),bkpfile.Data());
}
