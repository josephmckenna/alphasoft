#define MC_BV_TREE_analysis_cxx
#include "MC_BV_TREE_analysis.h"
#include <TH2.h>
#include <TRandom.h>
#include <TMath.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TStyle.h>
#include <TCanvas.h>

///< CONSTANTS
static const Int_t   NBARS=64;
// static const V_EJ200  = 3.e10/1.58; //< Speed of light in cm/s in the EJ-200 (https://eljentechnology.com/products/plastic-scintillators/ej-200-ej-204-ej-208-ej-212)
static const Float_t V_EJ200    = 299.792458/1.93; //< Accordint to Gareth (private communication)
static const Float_t C        = 299.792458; //< speed of light

///< HISTOGRAMS [[2] -> 0 for cosmics, 1 for pbars]
///< single bar histos
TH1F *hnDigi[2], *hnBars[2], *hEdep[2], *hEvTotEne[2], *hBarID[2];
TH1F *ht[2], *hZ[2];
///< pair of bar histos
TH1F *hTOF[2], *hDIST[2];
TH2F *hTOFvsDIST[2];
TH1F *hTOFMIN[2], *hTOFMAX[2], *hTOFMEAN[2], *hTOFSTD[2];
TH1F *hDPHIMIN[2], *hDPHIMAX[2], *hDPHIMEAN[2], *hDPHISTD[2];
TH1F *hDZETAMIN[2], *hDZETAMAX[2], *hDZETAMEAN[2], *hDZETASTD[2];
TH1F *hDISTMIN[2], *hDISTMAX[2], *hDISTMEAN[2], *hDISTSTD[2];
///< ############################################################
///< HISTOS AND NUMBERS FOR THE BV BARS
///< ############################################################
void MC_BV_TREE_analysis::ShowBV() {

   if (treeMCBV == 0) return;
   CreateHistos();
   n_tot_events = treeMCBV->GetEntriesFast();

   for (Long64_t ievt=0; ievt<n_tot_events;ievt++) { //loop on Events
      LoadMCBVTREE(ievt);
      treeMCBV->GetEntry(ievt);
      FillHistos();
   }//end loop on events
   if(n_tot_events>0) ShowHistos(pbar_file_flag); ///< 0 = cosmics, 1 = pbars, -1 = both
//________________________________________________________________________

}

void MC_BV_TREE_analysis::ShowHistos(Int_t code=0) {

   TCanvas *c = new TCanvas("c","c",1200,800);                          c->Divide(3,2);
   TCanvas *cTOFDIST = new TCanvas("cTOFDIST", "cTOFvsDIST", 1200, 1200);
   TCanvas *cProj = new TCanvas("cProj", "cProj", 1200, 600);           cProj->Divide(2,1);
   TCanvas *cTOF = new TCanvas("cTOF", "TOFs variables", 1200,1200);    cTOF->Divide(2,2);
   TCanvas *cDPHI = new TCanvas("cDPHI", "DPHIs variables", 1200,1200); cDPHI->Divide(2,2);
   TCanvas *cDZETA = new TCanvas("cDZETA", "DZETAs variables", 1200,1200);cDZETA->Divide(2,2);
   TCanvas *cDIST = new TCanvas("cDIST", "DISTs variables", 1200,1200); cDIST->Divide(2,2);

   std::string  stat_opt = "imr";      // i = integral, m = mean, r = rms, etc. 
   gStyle->SetOptStat(stat_opt.c_str());
   std::string draw_opt = "H,SAME";
   for(unsigned int i=0; i<2; i++) {
      if(code!=-1&&i!=code) continue;
      c->cd(1);
      hnBars[i]->Scale(1./n_tot_events);  hnBars[i]->SetMinimum(0.);
      hnBars[i]->Draw(draw_opt.c_str());
      // hnDigi[i]->Draw(draw_opt.c_str());
      c->cd(2);
      hBarID[i]->Scale(1./n_tot_events);  hBarID[i]->SetMinimum(0.);
      hBarID[i]->Draw(draw_opt.c_str());
      c->cd(3);
      hEdep[i]->Scale(1./n_tot_events);  hEdep[i]->SetMinimum(0.);
      hEdep[i]->Draw(draw_opt.c_str());
      c->cd(4);
      ht[i]->Scale(1./n_tot_events);  ht[i]->SetMinimum(0.);
      ht[i]->Draw(draw_opt.c_str());
      c->cd(5);
      hZ[i]->Scale(1./n_tot_events);  hZ[i]->SetMinimum(0.);
      hZ[i]->Draw(draw_opt.c_str());
      c->cd(6);
      hEvTotEne[i]->Scale(1./n_tot_events);  hEvTotEne[i]->SetMinimum(0.);
      hEvTotEne[i]->Draw(draw_opt.c_str());

      cTOFDIST->cd();
      hTOFvsDIST[i]->Draw("COLZ");

      cProj->cd(1); 
      hTOF[i]->Scale(1./n_tot_events);  hTOF[i]->SetMinimum(0.);
      hTOF[i]->Draw(draw_opt.c_str());
      cProj->cd(2); 
      hDIST[i]->Scale(1./n_tot_events);  hDIST[i]->SetMinimum(0.);
      hDIST[i]->Draw(draw_opt.c_str());

      cTOF->cd(1); 
      hTOFMIN[i]->Scale(1./n_tot_events);  hTOFMIN[i]->SetMinimum(0.);
      hTOFMIN[i]->Draw(draw_opt.c_str());
      cTOF->cd(2); 
      hTOFMAX[i]->Scale(1./n_tot_events);  hTOFMAX[i]->SetMinimum(0.);
      hTOFMAX[i]->Draw(draw_opt.c_str());
      cTOF->cd(3); 
      hTOFMEAN[i]->Scale(1./n_tot_events);  hTOFMEAN[i]->SetMinimum(0.);
      hTOFMEAN[i]->Draw(draw_opt.c_str());
      cTOF->cd(4); 
      hTOFSTD[i]->Scale(1./n_tot_events);  hTOFSTD[i]->SetMinimum(0.);
      hTOFSTD[i]->Draw(draw_opt.c_str());
      
      cDPHI->cd(1); 
      hDPHIMIN[i]->Scale(1./n_tot_events);  hDPHIMIN[i]->SetMinimum(0.);
      hDPHIMIN[i]->Draw(draw_opt.c_str());
      cDPHI->cd(2); 
      hDPHIMAX[i]->Scale(1./n_tot_events);  hDPHIMAX[i]->SetMinimum(0.);
      hDPHIMAX[i]->Draw(draw_opt.c_str());
      cDPHI->cd(3); 
      hDPHIMEAN[i]->Scale(1./n_tot_events);  hDPHIMEAN[i]->SetMinimum(0.);
      hDPHIMEAN[i]->Draw(draw_opt.c_str());
      cDPHI->cd(4); 
      hDPHISTD[i]->Scale(1./n_tot_events);  hDPHISTD[i]->SetMinimum(0.);
      hDPHISTD[i]->Draw(draw_opt.c_str());
      
      cDZETA->cd(1); 
      hDZETAMIN[i]->Scale(1./n_tot_events);  hDZETAMIN[i]->SetMinimum(0.);
      hDZETAMIN[i]->Draw(draw_opt.c_str());
      cDZETA->cd(2); 
      hDZETAMAX[i]->Scale(1./n_tot_events);  hDZETAMAX[i]->SetMinimum(0.);
      hDZETAMAX[i]->Draw(draw_opt.c_str());
      cDZETA->cd(3); 
      hDZETAMEAN[i]->Scale(1./n_tot_events);  hDZETAMEAN[i]->SetMinimum(0.);
      hDZETAMEAN[i]->Draw(draw_opt.c_str());
      cDZETA->cd(4); 
      hDZETASTD[i]->Scale(1./n_tot_events);  hDZETASTD[i]->SetMinimum(0.);
      hDZETASTD[i]->Draw(draw_opt.c_str());
      
      cDIST->cd(1); 
      hDISTMIN[i]->Scale(1./n_tot_events);  hDISTMIN[i]->SetMinimum(0.);
      hDISTMIN[i]->Draw(draw_opt.c_str());
      cDIST->cd(2); 
      hDISTMAX[i]->Scale(1./n_tot_events);  hDISTMAX[i]->SetMinimum(0.);
      hDISTMAX[i]->Draw(draw_opt.c_str());
      cDIST->cd(3); 
      hDISTMEAN[i]->Scale(1./n_tot_events);  hDISTMEAN[i]->SetMinimum(0.);
      hDISTMEAN[i]->Draw(draw_opt.c_str());
      cDIST->cd(4); 
      hDISTSTD[i]->Scale(1./n_tot_events);  hDISTSTD[i]->SetMinimum(0.);
      hDISTSTD[i]->Draw(draw_opt.c_str());
   }
   
}

void MC_BV_TREE_analysis::FillHistos() {
   Float_t totEne = 0.;
   hnDigi[pbar]->Fill(nDigi);
   hnBars[pbar]->Fill(nBars);
   for(unsigned int i = 0; i<bars_id->size(); i++) {
      totEne += bars_edep->at(i);
      hBarID[pbar]->Fill(bars_id->at(i));
      hEdep[pbar]->Fill(bars_edep->at(i));
      ht[pbar]->Fill(bars_t->at(i));
      hZ[pbar]->Fill(bars_z->at(i));
   }
   hEvTotEne[pbar]->Fill(totEne);

   for(unsigned int i = 0; i<pairs_dist->size(); i++) {
      hTOF[pbar]->Fill(pairs_tof->at(i));
      hDIST[pbar]->Fill(pairs_dist->at(i)/C);
      hTOFvsDIST[pbar]->Fill(pairs_dist->at(i)/C,pairs_tof->at(i)); 
   }

   hTOFMIN[pbar]->Fill(tof_min);
   hTOFMAX[pbar]->Fill(tof_max);
   hTOFMEAN[pbar]->Fill(tof_mean);
   hTOFSTD[pbar]->Fill(tof_std);

   hDPHIMIN[pbar]->Fill(dphi_min);
   hDPHIMAX[pbar]->Fill(dphi_max);
   hDPHIMEAN[pbar]->Fill(dphi_mean);
   hDPHISTD[pbar]->Fill(dphi_std);

   hDZETAMIN[pbar]->Fill(dzeta_min);
   hDZETAMAX[pbar]->Fill(dzeta_max);
   hDZETAMEAN[pbar]->Fill(dzeta_mean);
   hDZETASTD[pbar]->Fill(dzeta_std);

   hDISTMIN[pbar]->Fill(dist_min);
   hDISTMAX[pbar]->Fill(dist_max);
   hDISTMEAN[pbar]->Fill(dist_mean);
   hDISTSTD[pbar]->Fill(dist_std);
}

void MC_BV_TREE_analysis::CreateHistos() {
   ostringstream name, title;
   Int_t colors[2]; colors[0] = 8; colors[1] = 38; ///< Color code for cosmics and pbars
   for(unsigned int i=0; i<2; i++) {
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hnDigi" << i; title << "Number of digi per event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hnDigi[i]      = new TH1F(name.str().c_str(),title.str().c_str(), 15, -0.5, 14.5);   
      hnDigi[i]->SetFillColor(colors[i]);
      
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hnBars" << i; title << "Number of bars per event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hnBars[i]      = new TH1F(name.str().c_str(),title.str().c_str(), 15, -0.5, 14.5);
      hnBars[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hEdep" << i; title << "Energy released in a bar "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hEdep[i]       = new TH1F(name.str().c_str(),title.str().c_str(), 200, 0., 20.);
      hEdep[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hEvTotEne" << i; title << "Energy released in all the bars per event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hEvTotEne[i]   = new TH1F(name.str().c_str(),title.str().c_str(), 200, 0., 100.);
      hEvTotEne[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hBarID" << i; title << "Bar number "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hBarID[i]      = new TH1F(name.str().c_str(),title.str().c_str(), NBARS, 0, NBARS);
      hBarID[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "ht" << i; title << "Time "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      ht[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 500, 0., 15.);
      ht[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hZ" << i; title << "Zeta "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hZ[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 500, -1350., +1350.);
      hZ[i]->SetFillColor(colors[i]);

      ///< pair of bars histos
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFvsDIST" << i; title << "hTOFvsDIST "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFvsDIST[i] = new TH2F(name.str().c_str(),title.str().c_str(), 400, 0., 10., 400, 0., 10.);
      hTOFvsDIST[i]->SetTitle("TOF vs Geometric distance/c");
      hTOFvsDIST[i]->GetXaxis()->SetTitle("distance/c [ns]");
      hTOFvsDIST[i]->GetYaxis()->SetTitle("TOF [ns]");

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOF" << i; title << "hTOF "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOF[i] = new TH1F(name.str().c_str(),title.str().c_str(),200,0,10 );
      hTOF[i]->SetTitle("TOF");
      hTOF[i]->GetXaxis()->SetTitle("TOF [ns]");
      hTOF[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDIST" << i; title << "hDIST "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDIST[i] = new TH1F(name.str().c_str(),title.str().c_str(),200,0,10 );
      hDIST[i]->SetTitle("Distance");
      hDIST[i]->GetXaxis()->SetTitle("distance/c [ns]");
      hDIST[i]->SetFillColor(colors[i]);

      Float_t tof_xmax = 6.;
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFMIN" << i; title << "Minimum TOF in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFMIN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., tof_xmax);
      hTOFMIN[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFMAX" << i; title << "Maximum TOF in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFMAX[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., tof_xmax);
      hTOFMAX[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFMEAN" << i; title << "Mean of TOFs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFMEAN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., tof_xmax);
      hTOFMEAN[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFSTD" << i; title << "STD of TOFs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFSTD[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., tof_xmax);
      hTOFSTD[i]->SetFillColor(colors[i]);

      Float_t dphi_xmax = 3.15;
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPHIMIN" << i; title << "Minimum DPHI in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPHIMIN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dphi_xmax);
      hDPHIMIN[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPHIMAX" << i; title << "Maximum DPHI in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPHIMAX[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dphi_xmax);
      hDPHIMAX[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPHIMEAN" << i; title << "Mean of DPHIs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPHIMEAN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dphi_xmax);
      hDPHIMEAN[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPHISTD" << i; title << "STD of DPHIs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPHISTD[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dphi_xmax);
      hDPHISTD[i]->SetFillColor(colors[i]);

      Float_t dzeta_xmax = 2000;
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDZETAMIN" << i; title << "Minimum DZETA in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDZETAMIN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dzeta_xmax);
      hDZETAMIN[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDZETAMAX" << i; title << "Maximum DZETA in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDZETAMAX[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dzeta_xmax);
      hDZETAMAX[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDZETAMEAN" << i; title << "Mean of DZETAs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDZETAMEAN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dzeta_xmax);
      hDZETAMEAN[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDZETASTD" << i; title << "STD of DZETAs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDZETASTD[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dzeta_xmax);
      hDZETASTD[i]->SetFillColor(colors[i]);

      Float_t dist_xmax = 2000;
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDISTMIN" << i; title << "Minimum DIST in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDISTMIN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dist_xmax);
      hDISTMIN[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDISTMAX" << i; title << "Maximum DIST in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDISTMAX[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dist_xmax);
      hDISTMAX[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDISTMEAN" << i; title << "Mean of DISTs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDISTMEAN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dist_xmax);
      hDISTMEAN[i]->SetFillColor(colors[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDISTSTD" << i; title << "STD of DISTs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDISTSTD[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dist_xmax);
      hDISTSTD[i]->SetFillColor(colors[i]);

   }

}
