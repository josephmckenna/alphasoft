#ifndef __RESFUNC__
#define __RESFUNC__
#include "ResolutionFunctions.C"
#endif

ofstream fout;
TString tag;

void ShowRes(TFile* fin)
{
  cout<<"Show Histo for: "<<tag<<endl;
  TH1D* hResR = (TH1D*) fin->Get("hResRad");
  hResR->Rebin(10);
  TH1D* hResP = (TH1D*) fin->Get("hResPhi");
  hResP->Rebin(10);   
  TH1D* hResX = (TH1D*) fin->Get("hResX");
  hResX->Rebin(10);
  hResX->SetLineColor(kRed);
  hResX->SetTitle("Vertex Resolution X,Y;x,y [mm];events");
  TH1D* hResY = (TH1D*) fin->Get("hResY");
  hResY->Rebin(10);
  hResY->SetLineColor(kBlack);  
  TH1D* hResZ = (TH1D*) fin->Get("hResZed"); 
  //  hResZ->Rebin(5); 
  //hResZ->Rebin(4);
  hResZ->GetXaxis()->SetRangeUser(-150.,150.);


  double Nevt = 10000.,
    Nvtx = double(hResR->GetEntries());
  double amp = Nvtx/50.;
  double eff = Nvtx/Nevt;
  double efferr = sqrt( eff*(1.-eff)/(Nevt-1.) );
  //===================================================================================
  cout<<"*** VERTICES = "<<Nvtx<<" ***"<<endl;
  cout<<"*** Efficiency = ("<<eff*1.e2<<" +/- "<<efferr*1.e2<<") % ***\n"<<endl;
  //===================================================================================

  TF1 *fitR = new TF1("BiGausRad",BiGaus,0.,75.,6);
  fitR->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  fitR->SetParameters(amp,-5.,5.,amp,-5.,15.);
  fitR->SetParameters(amp,0.,5.,amp,1.,15.);
  hResR->Fit("BiGausRad","Q0+");
  double resR,reserrR, chi2R = Resolution(fitR,resR,reserrR);
  cout<<"\nRadial Projection bigaus: ("<<resR<<" +/- "<<reserrR<<") mm\tchi^2/ndf = "<<chi2R<<endl;

  
  TF1 *fitPhi = new TF1("BiGausPhi",BiGaus,-TMath::Pi(),TMath::Pi(),6);
  fitPhi->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  fitPhi->SetParameters(amp,0.,20.*TMath::DegToRad(),amp,0.,40*TMath::DegToRad());
  fitPhi->SetParameters(amp,0.,30.*TMath::DegToRad(),amp,0.,40*TMath::DegToRad());
  hResP->Fit("BiGausPhi","Q0+");
  double resP, reserrP, chi2P = Resolution(fitPhi,resP,reserrP);
  cout<<"Azimuthal Projection bigaus: ("<<resP*TMath::RadToDeg()<<" +/- "<<reserrP*TMath::RadToDeg()<<") deg\tchi^2/ndf = "<<chi2P<<endl;


  TF1 *fitZ = new TF1("BiGausZed",BiGaus,-75.,75.,6);
  fitZ->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  amp*=3.;
  fitZ->SetParameters(amp,0.,8.,amp,0.,12.);
  // fitZ->FixParameter(1,0.);
  // fitZ->FixParameter(4,0.);
  hResZ->Fit("BiGausZed","Q0+");
  double resZ, reserrZ, chi2Z = Resolution(fitZ,resZ,reserrZ); 
  cout<<"Axial Projection bigaus: ("<<resZ<<" +/- "<<reserrZ<<") mm\tchi^2/ndf = "<<chi2Z<<endl;


  TF1 *fitX = new TF1("BiGausXed",BiGaus,-75.,75.,6);
  fitX->SetLineColor(kCyan);
  fitX->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  amp*=3.;
  fitX->SetParameters(amp,0.,8.,amp,0.,12.);
  hResX->Fit("BiGausXed","Q0+");
  double resX, reserrX, chi2X = Resolution(fitX,resX,reserrX);
  cout<<"X Projection bigaus: ("<<resX<<" +/- "<<reserrX<<") mm\tchi^2/ndf = "<<chi2X<<endl;


  TF1 *fitY = new TF1("BiGausYed",BiGaus,-75.,75.,6);
  fitY->SetLineColor(kBlue);
  fitY->SetParNames("Amplitude1","Mean1","Sigma1","Amplitude2","Mean2","Sigma2");
  //  amp*=3.;
  fitY->SetParameters(amp,0.,8.,amp,0.,12.);
  hResY->Fit("BiGausYed","Q0+");
  double resY, reserrY, chi2Y = Resolution(fitY,resY,reserrY);
  cout<<"Y Projection bigaus: ("<<resY<<" +/- "<<reserrY<<") mm\tchi^2/ndf = "<<chi2Y<<endl;



  TString cvtxname("cResVtx");
  cvtxname.Append(tag);
  TCanvas* cvtx = new TCanvas(cvtxname.Data(),cvtxname.Data(),2500,1600);
  cvtx->Divide(2,2);

  cvtx->cd(1);
  hResR->Draw(); 
  fitR->Draw("same");
  hResR->GetXaxis()->SetRangeUser(-80.,80.);
 
  cvtx->cd(2);
  hResX->Draw();
  hResY->Draw("same");
  // legXY->Draw("same");
  fitX->Draw("same");
  fitY->Draw("same");
  hResX->GetXaxis()->SetRangeUser(-80.,80.);

  cvtx->cd(3);
  hResP->Draw();
  fitPhi->Draw("same");

  cvtx->cd(4);
  hResZ->Draw();
  fitZ->Draw("same");
  hResZ->GetXaxis()->SetRangeUser(-80.,80.);
}

void PlotRes()
{
  TFile* ff = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(ff->GetName());
  //TString datadir(getenv("DATADIR"));
  TString datadir(getenv("./"));
  Ssiz_t strip = datadir.Length()+1;
  TString temp(fname(strip,fname.Length()));
  tag = temp(0,temp.Length()-5);
  TString sname("stat");
  sname.Append(tag);
  sname.Append(".txt");
  fout.open(sname.Data());

  gStyle->SetOptStat(kFALSE);

  cout<<"\n"<<tag<<endl;
  fout<<"\n"<<tag<<endl;

  ShowRes(ff);

  fout.close();
  cout<<"\n"<<endl;
}
