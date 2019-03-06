TH2D* hrt;
TH2D* hh;
TF1* fit_func;
TGraphErrors* str_raw;
TGraph* str_err;
TF1* str_fit;
TGraph* gRes;

bool fTrace = false;
double MagneticField=0.;

double tmax=4100.;

void CalculateSTR(std::vector<double>& outtime,
		  std::vector<double>& outrad,
		  std::vector<double>& outdrad
		  )
{
  hh = (TH2D*) hrt->Clone();
  //      hh->RebinY(15); // <-- HARD-CODED: arbitrary
  double entries = double(hh->GetEntries());

  TH2D *hchi2 = new TH2D("hchi2_calib","Gaussian fit chi2",hh->GetNbinsX(),1.,double(hh->GetNbinsX()),2000,0.,200.);

  ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(500);

  outdrad.clear();
  outrad.clear();
  outtime.clear();

  outdrad.push_back(4.); // <-- HARD-CODED: arbitrary
  outrad.push_back( _anoderadius );
  outtime.push_back(0.);
  int n=0; // number of points

  for(int b = 1; b <= hh->GetNbinsX(); ++b)
    {
      if( hh->GetXaxis()->GetBinCenter(b) < 0. )
	continue;

      // get me a slice of STR
      TString hname = TString::Format("py%04d",b);
      TH1D *h = hh->ProjectionY(hname.Data(), b, b);
      h->SetBinContent( h->FindBin( _anoderadius ), 0. );

      // ignore slices with too few events
      double Nproj = h->Integral();
      if( Nproj < 1.e-3 * entries )
	continue;

      // initialize gaus fit with peak finding results
      TSpectrum s(1, 0.001); // <-- HARD-CODED: arbitrary and irrelevant
      int error_level_save = gErrorIgnoreLevel;
      gErrorIgnoreLevel = kFatal;
      if( s.Search(h, 2., "nodraw") )
	{
	  //	    s.Print();
	  double *r = s.GetPositionX();
	  double *A = s.GetPositionY();
	  int rbin = h->FindBin( r[0] );

	  // estimate sigma by estimating FWHM
	  double fwhm = h->GetBinWidth( rbin );
	  for(int ib=rbin; ib<h->GetNbinsX(); ++ib)
	    {
	      double bc = h->GetBinContent( ib );
	      if( bc < 0.5*A[0] )
		{
		  fwhm = 2.* TMath::Abs( r[0] - h->GetBinCenter( ib-1 ) );
		  break;
		}
	    }
	  double sigma = fwhm/2.355;

	  fit_func->SetParameter(0, A[0]);
	  fit_func->SetParameter(1, r[0]);
	  fit_func->SetParameter(2, sigma);
	  if( fTrace )
	    std::cout<<"CalibRun::CalculateSTR()  bin: "<<b
		     <<"  r: "<<r[0]
		     <<"mm    s: "<<sigma<<" mm"<<std::endl;

	  // TFitResultPtr fptr = h->Fit(fit_func,"QME0S","",
	  //                             r[0]-5.*sigma,r[0]+5.*sigma);
	  TFitResultPtr fptr = h->Fit(fit_func,"QME0S","",
				      r[0]-sigma,r[0]+sigma);

	  if(!fptr->IsValid()) {
	    if( fTrace )
	      std::cout<<"CalibRun::CalculateSTR() fit failed for slice "<< b << std::endl;
	    continue; // skip slices in which gaussian fit fails
	  }
	  hchi2->Fill(b, fptr->Chi2()/fptr->Ndf());

	  double time = hh->GetXaxis()->GetBinCenter(b),
	    radius = fit_func->GetParameter(1),
	    error = fit_func->GetParError(1);
	  sigma = fit_func->GetParameter(2);
	  if( fTrace ){
	    std::cout<<"CalibRun::CalculateSTR() fit slice result  t: "<<time
		     <<"ns   r: "<<radius
		     <<"mm   s: "<<sigma<<" mm"<<std::endl;
	    // fptr->Print();
	  }
	  if( time < 0. ||
	      radius < _cathradius || radius > _anoderadius ||
	      sigma < 2. || sigma > 10. ||
	      error > 1.5) // <-- HARD-CODED: arbitrary
	    continue;

	  outdrad.push_back(sigma);
	  outrad.push_back(radius);
	  outtime.push_back(time);
	  str_raw->SetPoint(n,time,radius);
	  str_raw->SetPointError(n,_timebin,sigma);

	  str_err->SetPoint(n,time,sigma);

	  ++n;
	}// peak found
      gErrorIgnoreLevel = error_level_save;
    }// bins loop

  if( n )
    {
      // str_fit->FixParameter(0, _anoderadius);
      // str_raw->Fit(str_fit,"QME0");
      str_fit->SetParameter(0, _anoderadius);
      str_raw->Fit(str_fit,"QME0","",200.);// cut off induction region for fit
      std::cout<<"CalibRun::CalculateSTR(...) STR function chi^2: "
	       <<str_fit->GetChisquare()/double(str_fit->GetNDF())<<std::endl;
    }
}

// Get current date/time
const std::string currentDateTime()
{
  time_t     now = time(0);
  struct tm  tstruct;
  char       buf[80];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  //    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
  strftime(buf, sizeof(buf), "%d %B %Y, %X", &tstruct);
  return buf;
}

void MakeLookUpTable( int run )
{
  TString flookupname = TString::Format("%s/ana/calib/LookUp_%1.2fT_STRR%d_fit.dat",getenv("AGRELEASE"),MagneticField,run);
  //Catch invalid loopup tables (thus don't write corrupt ones)
  if (str_fit->Eval(0.)<0)
    {
      std::cerr<<"Error in calib_module, avoiding writing corrupt file:"<< flookupname<<std::endl;
    }
  std::ofstream flookup(flookupname.Data());
  flookup<<"# B = "<<MagneticField<<" T, TPC data (run "<<run<<"), "<<currentDateTime()<<std::endl;
  flookup<<"# t\tr\tphi"<<std::endl;
  double phi=0.;
  for(double t=0.; t<7000.; t+=8.)
    {
      double rad = str_fit->Eval(t);
      flookup<<t<<"\t"<<rad<<"\t"<<phi<<std::endl;
      if( rad < _cathradius ) break;
    }
  flookup.close();
}

void Residuals(std::vector<double> &time, std::vector<double> &radius)
{
  int n=0;
  for(auto it=time.begin(); it!=time.end(); ++it)
    // hRes->Fill( *it,
    //             str_fit->Eval(*it) - radius.at( std::distance(time.begin(),it) ) );
    gRes->SetPoint( n++, *it,
		    str_fit->Eval(*it) - radius.at( std::distance(time.begin(),it) ) );
}

void PlotMSRcalib()
{
  gStyle->SetOptStat(kFALSE);

  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(fin->GetName());
  cout<<fname<<" FOUND"<<endl;
  int idx=fname.Index( "output" )+13;
  cout<<idx<<"\t"<<fname(idx,5)<<endl;
  int run = TString(fname(idx,5)).Atoi();
  cout<<fname<<"\tRun # "<<run<<endl;


  gDirectory->cd("Calibration");
  gDirectory->pwd();

  hrt = (TH2D*) gROOT->FindObject("hRofT_straight");
  hrt->RebinX(3);
  TCanvas* ccalib = new TCanvas("ccalib","ccalib",1800,1200);
  ccalib->Divide(2,2);
  ccalib->cd(1);
  hrt->Draw();
  hrt->GetXaxis()->SetRangeUser(0.,tmax);

  fit_func = new TF1("fRofT","gaus(0)", 109., 190.);

  str_raw = new TGraphErrors();
  str_raw->SetName("STRraw");
  str_raw->SetTitle("STR;Drift Time [ns];TPC Radius [mm]");

  str_fit = new TF1("fSTR","pol3(0)", 0., 5000.);
  str_fit->SetLineColor(kBlue);
  
  str_err = new TGraph();
  str_err->SetName("STRerr");
  str_err->SetTitle("STR radial error;Drift Time [ns];Radial Error [mm]");
  str_err->SetMarkerStyle(7);
  str_err->SetMarkerColor(kRed);

  std::vector<double> time,rad,drad;
  CalculateSTR(time,rad,drad);

  MakeLookUpTable( run );

  gRes = new TGraph;
  gRes->SetName("STR residuals");
  gRes->SetTitle("STR residuals;t [ns];res [mm]");
  gRes->SetMarkerStyle(7);
  gRes->SetMarkerColor(kBlue);
  Residuals( time, rad );

  int n=1;
  while(1)
    {
      TString name = TString::Format("fSTR;%d",n);
      TF1* f = (TF1*) gROOT->FindObject(name.Data());
      if( f )
  	{
	  ccalib->cd(1);
  	  f->Draw("same");
  	  ++n;
  	}
      else
  	break;
    }

  ccalib->cd(1);
  str_fit->Draw("same");

  ccalib->cd(2);
  str_raw->Draw("AP");
  str_fit->Draw("same");

  ccalib->cd(3);
  TH1D* h1 = new TH1D("h1","h1",1,0.,tmax);
  h1->SetLineColor(0);
  h1->Draw();
  h1->GetYaxis()->SetRangeUser(-9,9.);
  str_err->Draw("Psame");
  gRes->Draw("Psame");
  TLegend* leg = new TLegend(0.8,0.8,0.95,0.95);
  leg->AddEntry(str_err,"fit error","P");
  leg->AddEntry(gRes,"fit residual","P");
  leg->Draw("same");

  ccalib->cd(4);
  hh->Draw();
  str_fit->Draw("same");


  // //  TGraphErrors* raw = (TGraphErrors*) gROOT->FindObject("STRraw");
  // TGraph* raw = (TGraph*) gROOT->FindObject("STRraw");
  // TCanvas* ccalib2 = new TCanvas("ccalib2","ccalib2",1800,1200);
  // raw->Draw("AP");
}
