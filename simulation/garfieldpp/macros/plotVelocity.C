#include "TFile.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TString.h"

void plotVelocity()
{
  TFile* f1=TFile::Open("/daq/alpha_data0/acapra/alphag/GPPdata/chamber_drift/drift_root/ChamberDrift_phi1.0000_Z0.0cm_Ar70-CO230_Cathode-4000V_Anode3100V_Field-110V_B0.00T.root","READ");
  TH2D* h1 = (TH2D*) gDirectory->Get("hrv");
  TString hname = TString::Format("%s_3100",h1->GetName());
  h1->SetName(hname);
  h1->SetLineColor(kRed);
  h1->SetMarkerColor(kRed);
  h1->SetStats(0);
  h1->SetTitle("rTPC Radius Vs Drift Velocity");

  TFile* f2=TFile::Open("/daq/alpha_data0/acapra/alphag/GPPdata/chamber_drift/drift_root/ChamberDrift_phi1.0000_Z0.0cm_Ar70-CO230_Cathode-4000V_Anode3200V_Field-110V_B0.00T.root","READ");
  TH2D* h2 = (TH2D*) gDirectory->Get("hrv");
  hname = TString::Format("%s_3200",h2->GetName());
  h2->SetName(hname);
  h2->SetLineColor(kBlue);
  h2->SetMarkerColor(kBlue);
  h2->SetStats(0);


  TCanvas* c1 = new TCanvas("DriftVelocity","DriftVelocity",1600,1000);
  h1->Draw("HISTC");
  h2->Draw("HISTCsame");
  c1->SetGridx();
  c1->SetGridy();
  h1->GetYaxis()->SetRangeUser(0.,0.008);
  h1->GetYaxis()->SetLabelSize(0.02);


  TLegend* leg = new TLegend(0.2,0.7,0.4,0.85);
  leg->AddEntry(h1,"AW 3100V","pl");
  leg->AddEntry(h2,"AW 3200V","pl");
  leg->Draw("same");
}
