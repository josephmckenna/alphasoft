TH1D* GetHisto( TFile* fin, double angle_cut = 90., TString tag = "fail" )
{
  TH2D* h2 = (TH2D*) fin->Get("hDtScintVsAngle");
  int gbin = h2->FindBin(0.,angle_cut),
    binx,biny,binz;
  h2->GetBinXYZ(gbin,binx,biny,binz);
  TString hname = TString::Format("hproj_cut%1.0fdeg",angle_cut);
  hname+=tag;
  TH1D* h2_proj_cut = h2->ProjectionX(hname.Data(),biny);
  h2_proj_cut->Sumw2();
  h2_proj_cut->Scale(1./h2_proj_cut->Integral());
  h2_proj_cut->Rebin(5);

  return h2_proj_cut;
}

TH1D* GetHisto( TFile* fin, TString tag = "fail" )
{
  TH1D* h1 = (TH1D*) fin->Get("hDtScint");
  h1->SetName( tag.Data() );
  h1->Sumw2();
  h1->Scale(1./h1->Integral());
  h1->Rebin(5);
  return h1;
}

double Power(TH1D* h, double cut)
{
  int bin_cut = h->FindBin(cut);
  return h->Integral(1,bin_cut);
}

double Power(TH1D* h, double cut, double& error)
{
  int bin_cut = h->FindBin(cut);
  return h->IntegralAndError(1,bin_cut, error);
}

void baranalysis(double Acut = 5.625, double Ecut = 300., double cosm_rej_cut = 0.01)
{
  double ymax = 0.19, // a.u.
    tmax = 3.; // ns
  
  TLine* timecutline[9];

  TGraphErrors* g1 = new TGraphErrors();  
  g1->SetMarkerColor(kBlack);
  g1->SetMarkerStyle(8);
  g1->SetMarkerSize(1.6);
  g1->SetLineColor(kBlack);
  int n1=0;

  cout<<"========= Energy Cut: "<<Ecut<<" keV ========="<<endl;
  cout<<"========= Angle Cut: "<<Acut<<" deg ========="<<endl;


  TString cname = TString::Format("cDtScint_Ecut%1.0fkeV_Acut%1.2fdeg_%1.2fxCosmBkg",
				  Ecut,Acut,cosm_rej_cut);
  TCanvas* cDtScint = new TCanvas(cname.Data(),cname.Data(),2600,2000);
  cDtScint->Divide(2,3);

  int nn=0;
  for(double ts=100.; ts<=600.; ts+=100.)
    {
      cout<<"Resolution: "<<ts<<" ps\t";

      //-----------------------------------------------------------------------------------------
      // get cosmic file and histo
      TString fcosmname,cosmtag;
      fcosmname = TString::Format("histoAgBars_cosm_Acut%1.2fdeg_Ecut%1.0fkeV_tres%1.0fps.root",
				  Acut,Ecut,ts);
      cosmtag = TString::Format("hDt_Acut%1.2fdeg_cosm_Ecut%1.0fkeV_tres%1.0fps",Acut,Ecut,ts);
	  
      TFile* fcosm = TFile::Open(fcosmname.Data(),"READ");
      cout<<fcosm->GetName()<<endl;
      TH1D* hcosm;
      hcosm = GetHisto( fcosm, cosmtag );
      hcosm->SetLineColor(kBlue);
      //-----------------------------------------------------------------------------------------

      //-----------------------------------------------------------------------------------------
      // find time cut 
      double timecut = -1., cosm_rej=1., cosm_rej_err = 0.1;
      for(double tcut = 5.; tcut>0.; tcut-=0.01)
	{
	  double error;
	  cosm_rej = Power( hcosm, tcut, error );
	  if( cosm_rej <= cosm_rej_cut )
	    {
	      timecut = tcut;
	      cosm_rej_err = error;
	      break;
	    }
	}// find cut loop
      //-----------------------------------------------------------------------------------------

      //-----------------------------------------------------------------------------------------
      // get pbar file and histo
      TString fpbarname,pbartag;
      fpbarname = TString::Format("histoAgBars_pbar_Acut%1.2fdeg_Ecut%1.0fkeV_tres%1.0fps.root",
				  Acut,Ecut,ts);
      pbartag = TString::Format("hDt_pbar_Acut%1.2fdeg_Ecut%1.0fkeV_tres%1.0fps",Acut,Ecut,ts);
      TFile* fpbar = TFile::Open(fpbarname.Data(),"READ");
      cout<<fpbar->GetName()<<endl;
      TH1D* hpbar;
      hpbar = GetHisto( fpbar, pbartag );
      hpbar->SetLineColor(kRed);
      //-----------------------------------------------------------------------------------------
      
      //-----------------------------------------------------------------------------------------
      // calculate signal acceptance
      double sig_err, sig = Power( hpbar, timecut , sig_err);
      //-----------------------------------------------------------------------------------------

      //-----------------------------------------------------------------------------------------
      // print and plot
      cout<<"Cut: "<<timecut
	  <<" ns\tBackground: ("<<cosm_rej<<" +/- "<<cosm_rej_err
	  <<")\tRejected: "<<1.-cosm_rej;

      cout<<"\t\tSignal: "<<sig<<" +/- "<<sig_err<<endl;

      hpbar->SetStats(kFALSE);
      TString htitle = TString::Format("TOF - E_{cut} = %1.0fkeV - Bar #Delta#phi = %1.2f^{o} - #sigma_{t} = %1.0fps",Ecut, Acut, ts);
      hpbar->SetTitle(htitle.Data());
      hpbar->GetYaxis()->SetRangeUser(0.,ymax);

      cDtScint->cd(nn+1);
      hpbar->Draw();
      hcosm->Draw("same");

      //      hpbar->GetXaxis()->SetRangeUser(0.,tmax);

      timecutline[nn] = new TLine(timecut,0.,timecut,ymax);
      timecutline[nn]->SetLineColor(kBlack);
      timecutline[nn]->SetLineStyle(2);
      timecutline[nn]->SetLineWidth(2);
      timecutline[nn]->Draw("same");
      //-----------------------------------------------------------------------------------------
      ++nn;

      if( timecut == -1. )
	continue;

      g1->SetPoint( n1, ts, sig );
      g1->SetPointError( n1, 0, sig_err );
      ++n1;
    }// time resolution loop

  cDtScint->SaveAs((cname+".pdf").Data()); cDtScint->SaveAs((cname+".pdf").Data());
  cDtScint->SaveAs((cname+".png").Data()); cDtScint->SaveAs((cname+".png").Data());
  cout<<"---------------------------------------------------------------------"<<endl;
 

  TString caname,ctname,htitle; 

  caname = TString::Format("SignalAccVsTimeRes_Acut%1.2fdeg_Ecut%1.0fkeV_%1.2fxCosmBkg",
			   Acut,Ecut,cosm_rej_cut);
  htitle = TString::Format("Signal Acceptance Vs Time Resolution [%1.0f%% Cosmic Background] [Angle Cut %1.2f^{o}] [Energy Threshold %1.0fkeV];Time Resolution [ps];Signal",
			   cosm_rej_cut*1.e2,Acut,Ecut);
 
  TCanvas* ca = new TCanvas(caname.Data(),caname.Data(),1500,1000);
  g1->SetTitle(htitle.Data());
  g1->GetXaxis()->SetRangeUser(-0.1,810.);
  g1->GetYaxis()->SetRangeUser(0.,1.02);
  ca->cd();
  g1->Draw("AP");

  ca->SetGrid();
  ca->SaveAs((caname+".pdf").Data()); ca->SaveAs((caname+".pdf").Data());
  ca->SaveAs((caname+".png").Data()); ca->SaveAs((caname+".png").Data());
}
