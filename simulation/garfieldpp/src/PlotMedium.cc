#include <string>

#include <TMath.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TApplication.h>
#include <TFile.h>

#include "MediumMagboltz.hh"
#include "ViewMedium.hh"

using namespace Garfield;
using std::string;

int main(int argc, char * argv[])
{
  string path2gasfile(getenv("AGRELEASE"));
  path2gasfile+="/simulation/common/gas_files/";
  int len = path2gasfile.size();
  string gasfile("ar_70_co2_30_725Torr_20E200000_4B1.10.gas");
  path2gasfile+=gasfile;
  std::cout<<"GAS file: "<<path2gasfile<<std::endl;

  string gastag(gasfile.substr(0,gasfile.length()-4));
  std::cout<<"gas tag: "<<gastag<<std::endl;

  TApplication app("app", &argc, argv);

  TString fname = TString::Format("magboltz_%s.root",gastag.c_str());
  // I will store the histogram in this file
  TFile* fout = TFile::Open(fname.Data(),"RECREATE");

  // Create the medium
  MediumMagboltz* gas = new MediumMagboltz();
  // Gas file created with other software
  if(!gas->LoadGasFile(path2gasfile.c_str())) return -2;

  string ionfile(getenv("GARFIELD_HOME"));
  ionfile+="/Data/IonMobility_Ar+_Ar.txt";
  if(!gas->LoadIonMobility(ionfile.c_str())) return -1;

  gas->PrintGas();

  TString cname = TString::Format("magboltzgas_%s",gastag.c_str());
  TCanvas* c = new TCanvas(cname.Data(),cname.Data(),1400,1200);
  c->Divide(2,2);

  double EF = 1.e3; // V/cm
  double Emax = 300.e3; // V/cm
  double BF = 1.; // T
  double theta = TMath::PiOver2(); // rad

  ViewMedium* mviewV = new ViewMedium();
  mviewV->SetMedium(gas);
  //  mviewV->SetElectricFieldRange(100.,1.1e5);
  mviewV->SetCanvas((TCanvas*)c->cd(1));
  //mviewV->PlotElectronVelocity('e',EF,BF,theta);
  mviewV->PlotElectronVelocity('e');

  TH1F* h1 = new TH1F("h1","h1",1,0.,1.);
  h1->SetMarkerStyle(20);
  h1->SetMarkerColor(kOrange - 3);
  TH1F* h2 = new TH1F("h2","h2",1,0.,1.);
  h2->SetMarkerStyle(20);
  h2->SetMarkerColor(kGreen);
  TH1F* h3 = new TH1F("h3","h3",1,0.,1.);
  h3->SetMarkerStyle(20);
  h3->SetMarkerColor(kRed);
  TLegend* legV = new TLegend(0.2,0.7,0.65,0.9);
  legV->AddEntry(h1,"Electron drift velocity along E","p");
  legV->AddEntry(h2,"Electron drift velocity along B","p");
  legV->AddEntry(h3,"Electron drift velocity along ExB","p");
  c->cd(1);
  legV->Draw("same");

  ViewMedium* mviewD = new ViewMedium();
  mviewD->SetMedium(gas);
  //  mviewD->SetElectricFieldRange(100.,1.1e5);
  mviewD->SetCanvas((TCanvas*)c->cd(2));
  //  mviewD->PlotElectronDiffusion('e',EF,BF,theta);
  mviewD->PlotElectronDiffusion('e');

  TH1F* h4 = new TH1F("h4","h4",1,0.,1.);
  h4->SetMarkerStyle(20);
  h4->SetMarkerColor(kBlue + 3);
  TH1F* h5 = new TH1F("h5","h5",1,0.,1.);
  h5->SetMarkerStyle(20);
  h5->SetMarkerColor(kOrange - 3);
  TLegend* legD = new TLegend(0.5,0.7,0.95,0.95);
  legD->AddEntry(h5,"Electron transverse diffusion","p");
  legD->AddEntry(h4,"Electron longitudinal diffusion","p");
  c->cd(2);
  legD->Draw("same");


  ViewMedium* mviewT = new ViewMedium();
  mviewT->SetMedium(gas);
  //  mviewT->SetElectricFieldRange(100.,Emax);
  mviewT->SetCanvas((TCanvas*)c->cd(3));
  //  mviewT->PlotElectronTownsend('e',EF,BF,theta);
  mviewT->PlotElectronTownsend('e');
  c->cd(3)->SetLogy();


  if( gastag.find("ar_70_co2_30") == 0 )
    {
      TCanvas* cg = new TCanvas("gasgain_comparison_ar_co2_70-30","gasgain_comparison_ar_co2_70-30",1800,1400);
      mviewT->SetCanvas(cg);
      //      mviewT->PlotElectronTownsend('e',EF,BF,theta);
      mviewT->PlotElectronTownsend('e');
      cg->SetLogy();
      cg->SetGrid();

      // for Ar-CO2 70:30 only
      double a0 = 2.2e5, // cm^-1
	E0 = 550.e3; // V/cm
      TF1* fa = new TF1("fa","[0]*exp(-[1]/x)",100.,Emax);
      fa->SetParameters(a0,E0);
      fa->SetLineColor(kRed);
      cg->cd();
      fa->Draw("same");

      TH1F* h6 = new TH1F("h6","h6",1,100.,Emax);
      h6->SetMarkerStyle(20);
      h6->SetMarkerColor(kOrange - 3);
      TLegend* legT = new TLegend(0.5,0.2,0.95,0.45);
      legT->AddEntry(h6,"Magboltz","pl");
      legT->AddEntry(fa,"elog:101","l");
      legT->Draw("same");
      std::cout<<"alpha from elog: "<<fa->Eval(221.)<<" cm^-1"<<std::endl;
    }


  ViewMedium* mviewA = new ViewMedium();
  mviewA->SetMedium(gas);
  //  mviewA->SetElectricFieldRange(100.,Emax);
  mviewA->SetCanvas((TCanvas*)c->cd(4));
  //  mviewA->PlotElectronAttachment('e',EF,BF,theta);
  mviewA->PlotElectronAttachment('e');

  ViewMedium* mviewTb = new ViewMedium();
  mviewTb->SetMedium(gas);
  //mviewTb->SetElectricFieldRange(100.,5.e5);
  //mviewTb->SetMagneticFieldRange(0.,2.);
  mviewTb->SetCanvas((TCanvas*)c->cd(3));
  //  mviewTb->PlotElectronTownsend('b',EF,BF,theta);
  mviewTb->PlotElectronTownsend('b');

  c->SaveAs(".png");

  cname=TString::Format("IonVelocity_%s",gastag.c_str());
  TCanvas* cIon = new TCanvas(cname.Data(),cname.Data(),1200,1000);
  ViewMedium* mviewI = new ViewMedium();
  mviewI->SetMedium(gas);
  //mviewI->SetElectricFieldRange(100.,Emax);
  mviewI->SetCanvas(cIon);
  //  mviewI->PlotIonVelocity('e',EF,BF,theta);
  mviewI->PlotIonVelocity('e');
  cIon->SaveAs(".png");


  cname=TString::Format("LorentzAngle_%s",gastag.c_str());
  TCanvas* cLor = new TCanvas(cname.Data(),cname.Data(),1200,1000);
  ViewMedium* mviewL = new ViewMedium();
  mviewL->SetMedium(gas);
  //mviewL->SetElectricFieldRange(100.,Emax);
  mviewL->SetCanvas(cLor);
  //  mviewL->PlotElectronLorentzAngle('e',EF,BF,theta);
  mviewL->PlotElectronLorentzAngle('e');
  cLor->SaveAs(".png");

  
  app.Run(kTRUE);

  fout->cd();
  c->Write();
  cIon->Write();
  fout->Close();

  return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
