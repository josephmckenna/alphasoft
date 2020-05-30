#ifndef __RESFUNC__
#define __RESFUNC__
#include "ResolutionFunctions.C"
#endif

ofstream fout;
TString tag;

void ShowResolutions(TFile* fin)
{
  cout<<"Show Histo for: "<<tag<<endl;
  TH1D*	hNRhel = (TH1D*) fin->Get("hNhel");
  int range=9;
  hNRhel->GetXaxis()->SetRangeUser(0,range);
  hNRhel->GetXaxis()->SetNdivisions(100+range);
  hNRhel->GetXaxis()->CenterLabels();
  hNRhel->GetYaxis()->SetTitle("events");
  hNRhel->GetYaxis()->SetTitleOffset(1.5);
  hNRhel->SetTitle("Reconstructed Helices");
  TH1D*	hNUhel = (TH1D*) fin->Get("hNusedhel");
  hNUhel->GetXaxis()->SetRangeUser(0,range);
  hNUhel->SetLineColor(kRed);
  hNRhel->GetYaxis()->SetRangeUser(0.,1.1*hNUhel->GetBinContent(hNUhel->GetMaximumBin())); 
  TLegend* leghel = new TLegend(0.63,0.59,0.91,0.84);
  leghel->AddEntry(hNRhel,"good helices","l");
  leghel->AddEntry(hNUhel,"used helices","l");

  TH1D* hResR = (TH1D*) fin->Get("hResRad");
  hResR->Rebin(10);
  hResR->GetXaxis()->SetRangeUser(-25.,150.);
  TH1D* hResP = (TH1D*) fin->Get("hResPhi");
  hResP->Rebin(20);
  hResP->GetXaxis()->SetRangeUser(-TMath::Pi(),TMath::Pi());

  TH1D* hResX = (TH1D*) fin->Get("hResX");
  hResX->Rebin(20);
  hResX->SetLineColor(kRed);
  hResX->SetTitle("Vertex Resolution X,Y;x,y [mm];events");
  TH1D* hResY = (TH1D*) fin->Get("hResY");
  hResY->Rebin(20);
  hResY->SetLineColor(kBlack); 
  TLegend* legXY = new TLegend(0.78,0.65,0.91,0.84);
  legXY->AddEntry(hResX,"X","l");
  legXY->AddEntry(hResY,"Y","l");


  TH1D* hResZ = (TH1D*) fin->Get("hResZed"); 
  hResZ->Rebin(10);
  hResZ->GetXaxis()->SetRangeUser(-150.,150.);

  TH2D* hvrRC= (TH2D*) fin->Get("hvrRC"); 
  hvrRC->RebinX(5);
  hvrRC->RebinY(5);
  hvrRC->SetTitle("Vertex X-Y;x [mm];y [mm]");
  TH2D* hvrMC= (TH2D*) fin->Get("hvrMC");

  TString cvtxname("plots/cVtx");
  cvtxname.Append(tag);
  TCanvas* cvtx = new TCanvas(cvtxname.Data(),cvtxname.Data(),2500,1600);
  cvtx->Divide(3,2);

  cvtx->cd(1);
  hNRhel->Draw();
  hNUhel->Draw("same");
  leghel->Draw("same");

  cvtx->cd(2);
  hResX->Draw();
  hResY->Draw("same");
  legXY->Draw("same");

  cvtx->cd(3);
  hResZ->Draw();

  cvtx->cd(4);
  hResR->Draw();

  cvtx->cd(5);
  hResP->Draw();

  cvtx->cd(6);
  hvrRC->Draw("col");
  hvrMC->Draw("same");


  //===================================================================================
  cout<<"\nused helices = "<<hNUhel->GetMean()<<" +/- "<<hNUhel->GetMeanError()<<endl;
  cout<<"evts w/ 2 tracks used: "<<hNUhel->GetBinContent(3)<<endl;
  cout<<"evts w/ more than 2 tracks used: "<<hNUhel->Integral(4,hNRhel->GetNbinsX()+1)<<endl;
  //===================================================================================
  cout<<"*** Mean Number of Reconstructed Helices: "<<hNRhel->GetMean()
      <<" +/- "<<hNRhel->GetMeanError()<<" ***"<<endl;
  cout<<"*** Mean Number of Used Helices: "<<hNUhel->GetMean()
      <<" +/- "<<hNUhel->GetMeanError()<<" ***"<<endl;
  //===================================================================================

  double Nevt = 10000.,
    Nvtx = double(hNUhel->Integral(3,hNRhel->GetNbinsX()+1));
  double amp = Nvtx/100.;
  double eff = Nvtx/Nevt;
  double efferr = sqrt( eff*(1.-eff)/(Nevt-1.) );
  //===================================================================================
  cout<<"*** VERTICES = "<<Nvtx<<" ***"<<endl;
  cout<<"*** Efficiency = ("<<eff*1.e2<<" +/- "<<efferr*1.e2<<") % ***\n"<<endl;
  //===================================================================================
  

  //===================================================================================
  double RMSr=hResR->GetRMS(),RMSerrr=hResR->GetRMSError(); 

  hResR->Fit("gaus","Q0+","",-11.,25.);
  TF1* fR = hResR->GetFunction("gaus");
  fR->SetLineColor(kBlack);
  fR->SetLineStyle(2);
  double sr = fR->GetParameter(2), errsr = fR->GetParError(2);

  TF1 *fitR = new TF1("BiGausRad",BiGaus,-21.,35.,6);
  fitR->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  fitR->SetParameters(amp,-5.,5.,amp,-5.,15.);
  fitR->SetParameters(amp,-10.,5.,amp,-10.,15.);
  hResR->Fit("BiGausRad","Q0+");
  double resR,reserrR, chi2R = Resolution(fitR,resR,reserrR);
  double wresR,wreserrR, chi2Rw = WeightedResolution(fitR,wresR,wreserrR);
  double fwhmR = FWHM(fitR);
  //===================================================================================
  cout<<"\nRadial Projection"<<endl;
  cout<<"RMS: ("<<RMSr<<" +/- "<<RMSerrr<<") mm"<<endl;
  cout<<"gaus: ("<<sr<<" +/- "<<errsr<<") mm"<<endl;
  cout<<"bigaus: ("<<resR<<" +/- "<<reserrR<<") mm\tchi^2/ndf = "<<chi2R<<endl;
  cout<<"\t Weighted: ("<<wresR<<" +/- "<<wreserrR<<") mm\tchi^2/ndf = "<<chi2Rw<<endl;
  cout<<"\t FWHM: "<<fwhmR<<" mm\n"<<endl;
  //===================================================================================  


  //===================================================================================  
  double RMSp=hResP->GetRMS(),RMSerrp=hResP->GetRMSError();

  hResP->Fit("gaus","Q0+","",-0.7,0.7);
  TF1* fP = hResP->GetFunction("gaus");
  fP->SetLineColor(kBlack);
  fP->SetLineStyle(2);
  double sp = fP->GetParameter(2), errsp = fP->GetParError(2);

  TF1 *fitPhi = new TF1("BiGausPhi",BiGaus,-TMath::Pi(),TMath::Pi(),6);
  fitPhi->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  fitPhi->SetParameters(amp,0.,20.*TMath::DegToRad(),amp,0.,40*TMath::DegToRad());
  fitPhi->SetParameters(10.*amp,0.,30.*TMath::DegToRad(),10.*amp,0.,40*TMath::DegToRad());
  hResP->Fit("BiGausPhi","Q0+");
  double resP, reserrP, chi2P = Resolution(fitPhi,resP,reserrP);
  double wresP, wreserrP, chi2Pw = WeightedResolution(fitPhi,wresP,wreserrP);
  double fwhmP = FWHM(fitPhi);
  //===================================================================================
  cout<<"Azimuthal Projection"<<endl;
  cout<<"RMS: ("<<RMSp*TMath::RadToDeg()<<" +/- "<<RMSerrp*TMath::RadToDeg()<<") deg"<<endl;
  cout<<"gaus: ("<<sp*TMath::RadToDeg()<<" +/- "<<errsp*TMath::RadToDeg()<<") deg"<<endl;
  cout<<"bigaus: ("<<resP*TMath::RadToDeg()<<" +/- "<<reserrP*TMath::RadToDeg()<<") deg\tchi^2/ndf = "<<chi2P<<endl;
  cout<<"\t Weighted: ("<<wresP*TMath::RadToDeg()<<" +/- "<<wreserrP*TMath::RadToDeg()<<") deg\tchi^2/ndf = "<<chi2Pw<<endl;
  cout<<"\t FWHM: "<<fwhmP*TMath::RadToDeg()<<" deg\n"<<endl;
  //===================================================================================


  //===================================================================================
  double RMSz=hResZ->GetRMS(),RMSerrz=hResZ->GetRMSError();

  hResZ->Fit("gaus","Q0+","",-8.,8.);
  TF1* fZ = hResZ->GetFunction("gaus");
  fZ->SetLineColor(kBlack);
  fZ->SetLineStyle(2);
  double sz = fZ->GetParameter(2), errsz = fZ->GetParError(2);

  TF1 *fitZ = new TF1("BiGausZed",BiGaus,-75.,75.,6);
  fitZ->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  amp*=3.;
  fitZ->SetParameters(amp,0.,8.,amp,0.,12.);
  hResZ->Fit("BiGausZed","Q0+");
  double resZ, reserrZ, chi2Z = Resolution(fitZ,resZ,reserrZ); 
  double wresZ,wreserrZ, chi2Zw = WeightedResolution(fitZ,wresZ,wreserrZ);
  double fwhmZ = FWHM(fitZ);
  //===================================================================================
  cout<<"Axial Projection"<<endl;
  cout<<"RMS: ("<<RMSz<<" +/- "<<RMSerrz<<") mm"<<endl;
  cout<<"gaus: ("<<sz<<" +/- "<<errsz<<") mm"<<endl;
  cout<<"bigaus: ("<<resZ<<" +/- "<<reserrZ<<") mm\tchi^2/ndf = "<<chi2Z<<endl;
  cout<<"\t Weighted: ("<<wresZ<<" +/- "<<wreserrZ<<") mm\tchi^2/ndf = "<<chi2Zw<<endl;  
  cout<<"\t FWHM: "<<fwhmZ<<" mm\n"<<endl;
  //===================================================================================

  cvtx->cd(4);
  fR->Draw("same");
  //  fitR->Draw("same");
  
  cvtx->cd(5);
  fP->Draw("same");
  fitPhi->Draw("same");
  
  cvtx->cd(3);
  fZ->Draw("same");
  fitZ->Draw("same");


  //===================================================================================
  double RMSx=hResX->GetRMS(),RMSerrx=hResX->GetRMSError();

  hResX->Fit("gaus","Q0+","",-8.,8.);
  TF1* fX = hResX->GetFunction("gaus");
  fX->SetLineColor(kCyan);
  fX->SetLineStyle(2);
  double sx = fX->GetParameter(2), errsx = fX->GetParError(2);

  TF1 *fitX = new TF1("BiGausXed",BiGaus,-75.,75.,6);
  fitX->SetLineColor(kCyan);
  fitX->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  amp*=3.;
  fitX->SetParameters(amp,0.,8.,amp,0.,12.);
  hResX->Fit("BiGausXed","Q0+");
  double resX, reserrX, chi2X = Resolution(fitX,resX,reserrX); 
  double wresX,wreserrX, chi2Xw = WeightedResolution(fitX,wresX,wreserrX);
  double fwhmX = FWHM(fitX);
  //===================================================================================
  cout<<"X Projection"<<endl;
  cout<<"RMS: ("<<RMSx<<" +/- "<<RMSerrx<<") mm"<<endl;
  cout<<"gaus: ("<<sx<<" +/- "<<errsx<<") mm"<<endl;
  cout<<"bigaus: ("<<resX<<" +/- "<<reserrX<<") mm\tchi^2/ndf = "<<chi2X<<endl;
  cout<<"\t Weighted: ("<<wresX<<" +/- "<<wreserrX<<") mm\tchi^2/ndf = "<<chi2Xw<<endl;  
  cout<<"\t FWHM: "<<fwhmX<<" mm\n"<<endl;
  //===================================================================================


  //===================================================================================
  double RMSy=hResY->GetRMS(),RMSerry=hResY->GetRMSError();

  hResY->Fit("gaus","Q0+","",-8.,8.);
  TF1* fY = hResY->GetFunction("gaus");
  fY->SetLineColor(kBlue);
  fY->SetLineStyle(2);
  double sy = fY->GetParameter(2), errsy = fY->GetParError(2);

  TF1 *fitY = new TF1("BiGausYed",BiGaus,-75.,75.,6);
  fitY->SetLineColor(kBlue);
  fitY->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  amp*=3.;
  fitY->SetParameters(amp,0.,8.,amp,0.,12.);
  hResY->Fit("BiGausYed","Q0+");
  double resY, reserrY, chi2Y = Resolution(fitY,resY,reserrY); 
  double wresY,wreserrY, chi2Yw = WeightedResolution(fitY,wresY,wreserrY);
  double fwhmY = FWHM(fitY);
  //===================================================================================
  cout<<"Y Projection"<<endl;
  cout<<"RMS: ("<<RMSy<<" +/- "<<RMSerry<<") mm"<<endl;
  cout<<"gaus: ("<<sy<<" +/- "<<errsy<<") mm"<<endl;
  cout<<"bigaus: ("<<resY<<" +/- "<<reserrY<<") mm\tchi^2/ndf = "<<chi2Y<<endl;
  cout<<"\t Weighted: ("<<wresY<<" +/- "<<wreserrY<<") mm\tchi^2/ndf = "<<chi2Yw<<endl;  
  cout<<"\t FWHM: "<<fwhmY<<" mm\n"<<endl;
  //===================================================================================

  cvtx->cd(2);
  fX->Draw("same");
  fitX->Draw("same");
  fY->Draw("same");
  fitY->Draw("same");

  cvtx->SaveAs(".png");cvtx->SaveAs(".png");
  //  cvtx->SaveAs(".pdf");

  cout<<"*************************************************************************"<<endl;
  cout<<"DEBUG:"<<endl;
  cout<<"X resolution: ("<<hResX->GetMean()<<" +/- "<<hResX->GetMeanError()<<") mm"<<endl;
  cout<<"Y resolution: ("<<hResY->GetMean()<<" +/- "<<hResY->GetMeanError()<<") mm"<<endl;
  cout<<"-------------------------------------------------------------------------"<<endl;
  cout<<"X centre: ("<<hvrRC->GetMean(1)<<" +/- "<<hvrRC->GetMeanError(1)<<") mm"<<endl;
  cout<<"Y centre: ("<<hvrRC->GetMean(2)<<" +/- "<<hvrRC->GetMeanError(2)<<") mm"<<endl;
  cout<<"*************************************************************************"<<endl;

  //===================================================================================
  fout<<"evts w/o tracks: "<<hNRhel->GetBinContent(1)<<endl;
  fout<<"evts w/ 1 track: "<<hNRhel->GetBinContent(2)<<endl;
  int norecevts = hNRhel->GetBinContent(1)+hNRhel->GetBinContent(2);
  fout<<"Unreconstructable evts: "<<norecevts<<endl;
  int recevts = hNRhel->Integral(3,hNRhel->GetNbinsX()+1);
  fout<<"Reconstructable evts: "<<recevts<<endl;
  fout<<"total evts: "<<norecevts+recevts<<endl;
  fout<<"evts w/ 2 tracks: "<<hNRhel->GetBinContent(3)<<endl;
  fout<<"evts w/ more than 2 tracks: "<<hNRhel->Integral(4,hNRhel->GetNbinsX()+1)<<endl;
  fout<<"-----------------------------------------------------------------------"<<endl;
  fout<<"evts w/o used tracks: "<<hNUhel->GetBinContent(1)<<endl;
  fout<<"evts w/ 2 tracks used: "<<hNUhel->GetBinContent(3)<<endl;
  fout<<"evts w/ more than 2 tracks used: "<<hNUhel->Integral(4,hNRhel->GetNbinsX()+1)<<endl; 
  int rec = hNUhel->Integral(3,hNRhel->GetNbinsX()+1);
  fout<<"Reconstructed evts: "<<rec<<endl;
  fout<<"Seeding failed : "<<hNUhel->GetBinContent(1)-norecevts<<endl;
  fout<<"-----------------------------------------------------------------------"<<endl;
  fout<<"*** Mean Number of Reconstructed Helices: "<<hNRhel->GetMean()
      <<" +/- "<<hNRhel->GetMeanError()<<" ***"<<endl;
  fout<<"*** Mean Number of Used Helices: "<<hNUhel->GetMean()
      <<" +/- "<<hNUhel->GetMeanError()<<" ***"<<endl;
  fout<<"-----------------------------------------------------------------------"<<endl;
  fout<<"*** VERTICES = "<<Nvtx<<" ***"<<endl;  
  fout<<"*** Efficiency = ("<<eff*1.e2<<" +/- "<<efferr*1.e2<<") % ***\n"<<endl;
  fout<<"-----------------------------------------------------------------------"<<endl;
  fout<<"\nRadial Projection"<<endl;
  fout<<"RMS: ("<<RMSr<<" +/- "<<RMSerrr<<") mm"<<endl;
  fout<<"gaus: ("<<sr<<" +/- "<<errsr<<") mm"<<endl;
  fout<<"bigaus: ("<<resR<<" +/- "<<reserrR<<") mm\tchi^2/ndf = "<<chi2R<<endl;
  fout<<"\t Weighted: ("<<wresR<<" +/- "<<wreserrR<<") mm\tchi^2/ndf = "<<chi2Rw<<endl;
  fout<<"\t FWHM: "<<fwhmR<<" mm"<<endl;  
  fout<<"Azimuthal Projection"<<endl;
  fout<<"RMS: ("<<RMSp*TMath::RadToDeg()<<" +/- "<<RMSerrp*TMath::RadToDeg()<<") deg"<<endl;
  fout<<"gaus: ("<<sp*TMath::RadToDeg()<<" +/- "<<errsp*TMath::RadToDeg()<<") deg"<<endl;
  fout<<"bigaus: ("<<resP*TMath::RadToDeg()<<" +/- "<<reserrP*TMath::RadToDeg()<<") deg\tchi^2/ndf = "<<chi2P<<endl;
  fout<<"\t Weighted: ("<<wresP*TMath::RadToDeg()<<" +/- "<<wreserrP*TMath::RadToDeg()<<") deg\tchi^2/ndf = "<<chi2Pw<<endl;
  fout<<"\t FWHM: "<<fwhmP*TMath::RadToDeg()<<" deg"<<endl; 
  fout<<"Axial Projection"<<endl;
  fout<<"RMS: ("<<RMSz<<" +/- "<<RMSerrz<<") mm"<<endl;
  fout<<"gaus: ("<<sz<<" +/- "<<errsz<<") mm"<<endl;
  fout<<"bigaus: ("<<resZ<<" +/- "<<reserrZ<<") mm\tchi^2/ndf = "<<chi2Z<<endl;
  fout<<"\t Weighted: ("<<wresZ<<" +/- "<<wreserrZ<<") mm\tchi^2/ndf = "<<chi2Zw<<endl;  
  fout<<"\t FWHM: "<<fwhmZ<<" mm\n"<<endl;
  fout<<"X Projection"<<endl;
  fout<<"RMS: ("<<RMSx<<" +/- "<<RMSerrx<<") mm"<<endl;
  fout<<"gaus: ("<<sx<<" +/- "<<errsx<<") mm"<<endl;
  fout<<"bigaus: ("<<resX<<" +/- "<<reserrX<<") mm\tchi^2/ndf = "<<chi2X<<endl;
  fout<<"\t Weighted: ("<<wresX<<" +/- "<<wreserrX<<") mm\tchi^2/ndf = "<<chi2Xw<<endl;  
  fout<<"\t FWHM: "<<fwhmX<<" mm\n"<<endl;
  fout<<"Y Projection"<<endl;
  fout<<"RMS: ("<<RMSy<<" +/- "<<RMSerry<<") mm"<<endl;
  fout<<"gaus: ("<<sy<<" +/- "<<errsy<<") mm"<<endl;
  fout<<"bigaus: ("<<resY<<" +/- "<<reserrY<<") mm\tchi^2/ndf = "<<chi2Y<<endl;
  fout<<"\t Weighted: ("<<wresY<<" +/- "<<wreserrY<<") mm\tchi^2/ndf = "<<chi2Yw<<endl;  
  fout<<"\t FWHM: "<<fwhmY<<" mm\n"<<endl;
  //===================================================================================
  return;
}

void ShowRadialDensity(TFile* fin)
{
  //===================================================================================
  // radial profile
  TH1D* hRP = (TH1D*) fin->Get("hRadProf");
  hRP->SetTitle("Vertex Radial Profile;r [mm];E");
  hRP->SetMarkerColor(kRed);
  hRP->SetLineColor(kRed);
  hRP->SetMarkerStyle(7);
  hRP->Rebin(5);
  hRP->Sumw2();
  hRP->Scale(1./hRP->Integral());
  TH1D* hRD = new TH1D("hRadDens","Vertex Radial Density;r [mm];dE/dr [mm^{-1}]",hRP->GetNbinsX(),
  		       hRP->GetXaxis()->GetXmin(),hRP->GetXaxis()->GetXmax());
  for(int b=1; b<=hRP->GetNbinsX(); ++b)
    {
      hRD->SetBinContent(b, hRP->GetBinContent(b)/hRP->GetBinCenter(b) );
      double error = hRD->GetBinContent(b) * 
  	TMath::Sqrt( TMath::Power( hRP->GetBinError(b)/hRP->GetBinContent(b), 2.) + 
  		     TMath::Power( hRP->GetBinWidth(b)/hRP->GetBinCenter(b), 2.) );
      hRD->SetBinError(b,error);
    }
  hRD->SetMarkerColor(kBlue);
  hRD->SetLineColor(kBlue);
  hRD->SetMarkerStyle(7);
  //===================================================================================

  //===================================================================================
  TString c2name("plots/radprof");
  c2name.Append(tag);
  TCanvas* c2 = new TCanvas(c2name.Data(),c2name.Data(),1500,1200);
  c2->Divide(2,1);
  c2->cd(1);
  hRP->Draw();
  //  double yMax = hRP->GetMaximum()*1.1;
  double yMax = hRP->GetYaxis()->GetXmax()*1.1;
  TLine* trap = new TLine(22.275,0.,22.275,yMax);
  trap->SetLineColor(kBlack);
  trap->SetLineStyle(2);
  trap->SetLineWidth(2);
  trap->Draw("same");
  c2->cd(2);

  hRD->Draw();
  hRD->GetYaxis()->SetRangeUser(0.,hRD->GetMaximum()*1.1); 
  //  double NewYmax = hRD->GetMaximum()*1.1;
  double NewYmax = hRD->GetYaxis()->GetXmax()*1.1;
  TLine* NewTrap = new TLine(22.275,0.,22.275,NewYmax);
  NewTrap->SetLineColor(kBlack);
  NewTrap->SetLineStyle(2);
  NewTrap->SetLineWidth(2);
  NewTrap->Draw("same");
  c2->SaveAs(".png"); c2->SaveAs(".png");
  //===================================================================================
  return;
}

void RecAnalysis()
{
  TFile* ff = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(ff->GetName());
  //TString datadir(getenv("DATADIR"));
  TString datadir("");
  Ssiz_t strip = datadir.Length()+1;
  TString temp(fname(strip,fname.Length()));
  tag = temp(0,temp.Length()-5);
  TString sname("logs/stat");
  sname.Append(tag);
  sname.Append(".txt");
  fout.open(sname.Data());

  gStyle->SetOptStat(kFALSE);

  cout<<"\n"<<tag<<endl;
  fout<<"\n"<<tag<<endl;

  ShowResolutions(ff);

  ShowRadialDensity(ff);


  fout.close();
  cout<<"\n"<<endl;
}
