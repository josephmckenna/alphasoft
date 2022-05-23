#define BV_TREE_analysis_cxx
#include "BV_TREE_analysis.h"
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
static const Double_t C        = 299792458*1.e-9; //< speed of light (m/ns)
static const Double_t V_EJ200    = C/1.93; //< Accordint to Gareth (private communication)
static const Double_t s_to_ns = 1.e9;

///< HISTOGRAMS [[2] -> 0 for cosmics, 1 for pbars]
///< single bar histos
TH1F *hnDigi[2], *hnBars[2], *hEdep[2], *hEvTotEne[2], *hBarID[2];
TH1F *ht[2], *hZ[2];
TH1F *hATop[2], *hABot[2], *hlnA[2], *hsqrtA[2], *hdt[2]; ///< histos for Atop, Abot, tTop, tBot;
///< pair of bar histos
TH2F *hTOFvsDIST[2];
TH1F *hTOF[2], *hDIST[2], *hDz[2], *hDPhi[2];
TH1F *hTOFMIN[2], *hTOFMAX[2], *hTOFMEAN[2], *hTOFSTD[2];
TH1F *hDPHIMIN[2], *hDPHIMAX[2], *hDPHIMEAN[2], *hDPHISTD[2];
TH1F *hDZETAMIN[2], *hDZETAMAX[2], *hDZETAMEAN[2], *hDZETASTD[2];
TH1F *hDISTMIN[2], *hDISTMAX[2], *hDISTMEAN[2], *hDISTSTD[2];

///< FLAGS for CANVAS to be shown
static const bool bSummary       = true;
static const bool bTopBot        = true;
static const bool bPairs         = false;
static const bool bMinMaxMeanSTD = false;
static const bool bTOFDIST       = false;
///< FLAG to write the canvas output to a file
static const bool c_save         = false;
///< ############################################################
///< HISTOS AND NUMBERS FOR THE BV BARS
///< ############################################################
void BV_TREE_analysis::ShowBV() {

   if (treeMCBV == 0) return;
   n_tot_events = treeMCBV->GetEntriesFast();
   SetParameters();
   CreateHistos();

   for (Long64_t ievt=0; ievt<n_tot_events;ievt++) { //loop on Events
      LoadMCBVTREE(ievt);
      treeMCBV->GetEntry(ievt);
      FillHistos();
   }//end loop on events
   if(n_tot_events>0) {
      PrepareHistos();
      ShowHistos(); 
   }
//________________________________________________________________________

}

void BV_TREE_analysis::ShowHistos() {

   ///< Defining canvas
   TCanvas *cTOF, *cDPHI, *cDZETA, *cDIST;
   TCanvas *cSummary, *cTOFDIST, *cPairs, *cTopBot;

   if(bSummary) {cSummary = new TCanvas("cSummary","cSummary",1600,1200); cSummary->Divide(3,2);}
   if(bPairs) {cPairs = new TCanvas("cPairs", "cPairs", 1600, 1200); cPairs->Divide(2,2);}
   if(bTopBot) {cTopBot = new TCanvas("cTopBot","cTopBot", 1600, 1200); cTopBot->Divide(3,2);}
   if(bTOFDIST) cTOFDIST = new TCanvas("cTOFDIST", "cTOFvsDIST", 1600, 1200);
   if(bMinMaxMeanSTD) {
      cTOF = new TCanvas("cTOF", "TOFs variables", 1600, 1200);   cTOF->Divide(2,2);
      cDPHI = new TCanvas("cDPHI", "DPHIs variables", 1600, 1200); cDPHI->Divide(2,2);
      cDZETA = new TCanvas("cDZETA", "DZETAs variables", 1600, 1200);cDZETA->Divide(2,2);
      cDIST = new TCanvas("cDIST", "DISTs variables", 1600, 1200); cDIST->Divide(2,2);
   }

   std::string  stat_opt = "imr";     // i = integral, m = mean, r = rms, etc. 
   gStyle->SetOptStat(stat_opt.c_str());
   std::string draw_opt = "H,SAME";
   std::ostringstream s_c_out;
   for(unsigned int i=0; i<2; i++) {
      if(hnBars[i]->GetEntries()<=0) continue;

      if(bSummary) {
         cSummary->cd(1);     hnBars[i]->Draw(draw_opt.c_str());
         cSummary->cd(2);     hBarID[i]->Draw(draw_opt.c_str());
         cSummary->cd(3);     hEdep[i]->Draw(draw_opt.c_str());
         cSummary->cd(4);     ht[i]->Draw(draw_opt.c_str());
         cSummary->cd(5);     hZ[i]->Draw(draw_opt.c_str());
         cSummary->cd(6);     hEvTotEne[i]->Draw(draw_opt.c_str());
      }

      if(bTopBot) {
         cTopBot->cd(1);      hATop[i]->Draw(draw_opt.c_str());
         cTopBot->cd(2);      hABot[i]->Draw(draw_opt.c_str());
         cTopBot->cd(3);      hsqrtA[i]->Draw(draw_opt.c_str());
         cTopBot->cd(4);      hlnA[i]->Draw(draw_opt.c_str());
         cTopBot->cd(5);      hdt[i]->Draw(draw_opt.c_str());
         cTopBot->cd(6);      hnBars[i]->Draw(draw_opt.c_str());
         // cTopBot->cd(6);      hZ[i]->Draw(draw_opt.c_str());
      }

      if(bTOFDIST) {
         cTOFDIST->cd();
         hTOFvsDIST[i]->Draw("COLZ");
      }

      if(bPairs) {
         cPairs->cd(1);       hTOF[i]->Draw(draw_opt.c_str());
         cPairs->cd(2);       hDIST[i]->Draw(draw_opt.c_str());
         cPairs->cd(3);       hDz[i]->Draw(draw_opt.c_str());
         cPairs->cd(4);       hDPhi[i]->Draw(draw_opt.c_str());
      }

      if(bMinMaxMeanSTD) {
         cTOF->cd(1);         hTOFMIN[i]->Draw(draw_opt.c_str());
         cTOF->cd(2);         hTOFMAX[i]->Draw(draw_opt.c_str());
         cTOF->cd(3);         hTOFMEAN[i]->Draw(draw_opt.c_str());
         cTOF->cd(4);         hTOFSTD[i]->Draw(draw_opt.c_str());
         
         cDPHI->cd(1);        hDPHIMIN[i]->Draw(draw_opt.c_str());
         cDPHI->cd(2);        hDPHIMAX[i]->Draw(draw_opt.c_str());
         cDPHI->cd(3);        hDPHIMEAN[i]->Draw(draw_opt.c_str());
         cDPHI->cd(4);        hDPHISTD[i]->Draw(draw_opt.c_str());
         
         cDZETA->cd(1);       hDZETAMIN[i]->Draw(draw_opt.c_str());
         cDZETA->cd(2);       hDZETAMAX[i]->Draw(draw_opt.c_str());
         cDZETA->cd(3);       hDZETAMEAN[i]->Draw(draw_opt.c_str());
         cDZETA->cd(4);       hDZETASTD[i]->Draw(draw_opt.c_str());
         
         cDIST->cd(1);        hDISTMIN[i]->Draw(draw_opt.c_str());
         cDIST->cd(2);        hDISTMAX[i]->Draw(draw_opt.c_str());
         cDIST->cd(3);        hDISTMEAN[i]->Draw(draw_opt.c_str());
         cDIST->cd(4);        hDISTSTD[i]->Draw(draw_opt.c_str());
      }
   }
   if(c_save) {
      if(bSummary) {
         cSummary->Modified(); cSummary->Update();
         s_c_out.clear(); s_c_out.str("");
         s_c_out << "root_output_files/" << filename_core << "_cSummary.pdf";
         cSummary->Print(s_c_out.str().c_str(),"pdf");
      }
      if(bTopBot) {
         cTopBot->Modified(); cTopBot->Update();
         s_c_out.clear(); s_c_out.str("");
         s_c_out << "root_output_files/" << filename_core << "_cTopBot.pdf";
         cTopBot->Print(s_c_out.str().c_str(),"pdf");
      }
   }

}

void BV_TREE_analysis::FillHistos() {
   Double_t totEne = 0.;
   hnDigi[pbar]->Fill(nDigi);
   hnBars[pbar]->Fill(nBars);
   ht[pbar]->Fill(t_event);
   for(unsigned int i = 0; i<bars_id->size(); i++) {
      totEne += bars_edep->at(i);
      hBarID[pbar]->Fill(bars_id->at(i));
      hEdep[pbar]->Fill(bars_edep->at(i));
      // ht[pbar]->Fill(bars_t->at(i));
      hZ[pbar]->Fill(bars_z->at(i));
      hATop[pbar]->Fill(bars_atop->at(i));
      hABot[pbar]->Fill(bars_abot->at(i));
      hlnA[pbar]->Fill(0.5*TMath::Log(bars_atop->at(i)/bars_abot->at(i)));
      hsqrtA[pbar]->Fill(TMath::Sqrt(bars_atop->at(i)*bars_abot->at(i)));
      hdt[pbar]->Fill(-0.5*(bars_ttop->at(i)*s_to_ns-bars_tbot->at(i)*s_to_ns)); ///< 1.e9 => 
   }
   hEvTotEne[pbar]->Fill(totEne);

   for(unsigned int i = 0; i<pairs_dist->size(); i++) {
      hTOF[pbar]->Fill(pairs_tof->at(i)*s_to_ns);
      hDIST[pbar]->Fill(pairs_dist->at(i)/C);
      hDz[pbar]->Fill(pairs_dzeta->at(i));
      hDPhi[pbar]->Fill(pairs_dphi->at(i));
      hTOFvsDIST[pbar]->Fill(pairs_dist->at(i)/C,pairs_tof->at(i)*s_to_ns); 
   }

   hTOFMIN[pbar]->Fill(tof_min*s_to_ns);
   hTOFMAX[pbar]->Fill(tof_max*s_to_ns);
   hTOFMEAN[pbar]->Fill(tof_mean*s_to_ns);
   hTOFSTD[pbar]->Fill(tof_std*s_to_ns);

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

void BV_TREE_analysis::SetParameters() {
   if(mc_file_flag) A_max = 20.; else A_max = 2.;
   t_event_max = 1.; ///< This is in seconds
   for (Long64_t ievt=n_tot_events-1; ievt>=0;ievt--) { //loop on Events
      LoadMCBVTREE(ievt);
      treeMCBV->GetEntry(ievt);
      if(t_event>t_event_max) t_event_max = t_event;
      if(t_event_max>1.) return;
   }//end loop on events
   return;
}

void BV_TREE_analysis::CreateHistos() {
   ostringstream name, title;
   Int_t colors[2]; colors[0] = 8; colors[1] = 38; ///< Color code for cosmics and pbars [38/46]
   Int_t stile[2]; stile[0] = 3001; stile[1] = 3008; ///< Fill style code for cosmics and pbars
   ///< pair of bars histos
   Float_t t_xmax       = 15.;
   Float_t tof_xmax     = 6.;
   Float_t dzeta_xmax   = 2.000;
   Float_t dphi_xmax    = 3.15;
   Float_t dist_xmax    = 2.;

   for(unsigned int i=0; i<2; i++) {
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hnDigi" << i; title << "Number of digi per event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hnDigi[i]      = new TH1F(name.str().c_str(),title.str().c_str(), 15, -0.5, 14.5);  
      hnDigi[i]->SetFillColor(colors[i]); hnDigi[i]->SetFillStyle(stile[i]);
      
      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hnBars" << i; title << "Number of bars per event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hnBars[i]      = new TH1F(name.str().c_str(),title.str().c_str(), 15, -0.5, 14.5);
      hnBars[i]->SetFillColor(colors[i]); hnBars[i]->SetFillStyle(stile[i]); 

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hEdep" << i; title << "Energy released in a bar "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hEdep[i]       = new TH1F(name.str().c_str(),title.str().c_str(), 200, 0., A_max);
      hEdep[i]->SetFillColor(colors[i]); hEdep[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hEvTotEne" << i; title << "Energy released in all the bars per event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hEvTotEne[i]   = new TH1F(name.str().c_str(),title.str().c_str(), 200, 0., 100.);
      hEvTotEne[i]->SetFillColor(colors[i]); hEvTotEne[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hBarID" << i; title << "Bar number "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hBarID[i]      = new TH1F(name.str().c_str(),title.str().c_str(), NBARS, 0, NBARS);
      hBarID[i]->SetFillColor(colors[i]); hBarID[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "ht" << i; title << "Event time "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      ht[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 500, 0., t_event_max*1.1);
      ht[i]->SetFillColor(colors[i]); ht[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hZ" << i; title << "Zeta "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hZ[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 500, -dzeta_xmax, +dzeta_xmax);
      hZ[i]->SetXTitle("m");
      hZ[i]->SetFillColor(colors[i]); hZ[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hATop" << i; title << "A_{top} "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hATop[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 200, 0., A_max);
      hATop[i]->SetXTitle("a.u.");
      hATop[i]->SetFillColor(colors[i]); hATop[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hABot" << i; title << "A_{bot} "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hABot[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 200, 0., A_max);
      hABot[i]->SetXTitle("a.u.");
      hABot[i]->SetFillColor(colors[i]); hABot[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hsqrtA" << i; title << "#sqrt{A_{top} #times A_{bot}} #propto #sqrt{e^{-L/#lambda}} "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hsqrtA[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 500, 0., A_max);
      hsqrtA[i]->SetXTitle("a.u.");
      hsqrtA[i]->SetFillColor(colors[i]); hsqrtA[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hlnA" << i; title << "#frac{1}{2} log(A_{top}/A_{bot}) = #frac{z_{hit}}{#lambda} "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hlnA[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 500, -2., +2.);
      hlnA[i]->SetXTitle("a.u.");
      hlnA[i]->SetFillColor(colors[i]); hlnA[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hdt" << i; title << "- #frac{1}{2} (t_{top}-t_{bot}) = #frac{z_{hit}}{c_{EJ200}} "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hdt[i]          = new TH1F(name.str().c_str(),title.str().c_str(), 250, -t_xmax, t_xmax);
      hdt[i]->SetXTitle("ns");
      hdt[i]->SetFillColor(colors[i]); hdt[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFvsDIST" << i; title << "hTOFvsDIST "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFvsDIST[i] = new TH2F(name.str().c_str(),title.str().c_str(), 400, 0., tof_xmax, 400, 0., tof_xmax);
      hTOFvsDIST[i]->SetTitle("TOF vs (Hit distance)/speed");
      hTOFvsDIST[i]->GetXaxis()->SetTitle("dist/speed [ns]");
      hTOFvsDIST[i]->GetYaxis()->SetTitle("TOF [ns]");

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOF" << i; title << "hTOF "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOF[i] = new TH1F(name.str().c_str(),title.str().c_str(),200,0,tof_xmax);
      hTOF[i]->SetTitle("TOF");
      hTOF[i]->GetXaxis()->SetTitle("TOF [ns]");
      hTOF[i]->SetFillColor(colors[i]); hTOF[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDz" << i; title << "hDz "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDz[i] = new TH1F(name.str().c_str(),title.str().c_str(),200,-dzeta_xmax,dzeta_xmax);
      hDz[i]->SetTitle("#Delta z");
      hDz[i]->GetXaxis()->SetTitle("m");
      hDz[i]->SetFillColor(colors[i]); hDz[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPhi" << i; title << "hDPhi "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPhi[i] = new TH1F(name.str().c_str(),title.str().c_str(),200,0.,dphi_xmax);
      hDPhi[i]->SetTitle("#Delta #Phi");
      hDPhi[i]->GetXaxis()->SetTitle("rad");
      hDPhi[i]->SetFillColor(colors[i]); hDPhi[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDIST" << i; title << "hDIST "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDIST[i] = new TH1F(name.str().c_str(),title.str().c_str(),200,0,tof_xmax);
      hDIST[i]->SetTitle("(Hit distance)/speed");
      hDIST[i]->GetXaxis()->SetTitle("dist/speed [ns]");
      hDIST[i]->SetFillColor(colors[i]); hDIST[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFMIN" << i; title << "Minimum TOF in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFMIN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., tof_xmax);
      hTOFMIN[i]->SetXTitle("ns");
      hTOFMIN[i]->SetFillColor(colors[i]); hTOFMIN[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFMAX" << i; title << "Maximum TOF in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFMAX[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., tof_xmax);
      hTOFMAX[i]->SetXTitle("ns");
      hTOFMAX[i]->SetFillColor(colors[i]); hTOFMAX[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFMEAN" << i; title << "Mean of TOFs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFMEAN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., tof_xmax);
      hTOFMEAN[i]->SetXTitle("ns");
      hTOFMEAN[i]->SetFillColor(colors[i]); hTOFMEAN[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hTOFSTD" << i; title << "STD of TOFs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hTOFSTD[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., tof_xmax);
      hTOFSTD[i]->SetXTitle("ns");
      hTOFSTD[i]->SetFillColor(colors[i]); hTOFSTD[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPHIMIN" << i; title << "Minimum DPHI in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPHIMIN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dphi_xmax);
      hDPHIMIN[i]->SetXTitle("rad");
      hDPHIMIN[i]->SetFillColor(colors[i]); hDPHIMIN[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPHIMAX" << i; title << "Maximum DPHI in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPHIMAX[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dphi_xmax);
      hDPHIMAX[i]->SetXTitle("rad");
      hDPHIMAX[i]->SetFillColor(colors[i]); hDPHIMAX[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPHIMEAN" << i; title << "Mean of DPHIs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPHIMEAN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dphi_xmax);
      hDPHIMEAN[i]->SetXTitle("rad");
      hDPHIMEAN[i]->SetFillColor(colors[i]); hDPHIMEAN[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDPHISTD" << i; title << "STD of DPHIs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDPHISTD[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dphi_xmax);
      hDPHISTD[i]->SetXTitle("rad");
      hDPHISTD[i]->SetFillColor(colors[i]); hDPHISTD[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDZETAMIN" << i; title << "Minimum DZETA in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDZETAMIN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, -dzeta_xmax, dzeta_xmax);
      hDZETAMIN[i]->SetXTitle("m");
      hDZETAMIN[i]->SetFillColor(colors[i]); hDZETAMIN[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDZETAMAX" << i; title << "Maximum DZETA in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDZETAMAX[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, -dzeta_xmax, dzeta_xmax);
      hDZETAMAX[i]->SetXTitle("m");
      hDZETAMAX[i]->SetFillColor(colors[i]); hDZETAMAX[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDZETAMEAN" << i; title << "Mean of DZETAs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDZETAMEAN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, -dzeta_xmax, dzeta_xmax);
      hDZETAMEAN[i]->SetXTitle("m");
      hDZETAMEAN[i]->SetFillColor(colors[i]); hDZETAMEAN[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDZETASTD" << i; title << "STD of DZETAs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDZETASTD[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dzeta_xmax);
      hDZETASTD[i]->SetXTitle("m");
      hDZETASTD[i]->SetFillColor(colors[i]); hDZETASTD[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDISTMIN" << i; title << "Minimum DIST in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDISTMIN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dist_xmax);
      hDISTMIN[i]->SetXTitle("m");
      hDISTMIN[i]->SetFillColor(colors[i]); hDISTMIN[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDISTMAX" << i; title << "Maximum DIST in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDISTMAX[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dist_xmax);
      hDISTMAX[i]->SetXTitle("m");
      hDISTMAX[i]->SetFillColor(colors[i]); hDISTMAX[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDISTMEAN" << i; title << "Mean of DISTs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDISTMEAN[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dist_xmax);
      hDISTMEAN[i]->SetXTitle("m");
      hDISTMEAN[i]->SetFillColor(colors[i]); hDISTMEAN[i]->SetFillStyle(stile[i]);

      name.clear(); name.str(""); title.clear(); title.str("");
      name << "hDISTSTD" << i; title << "STD of DISTs in the event "; if(i==0) {title << "(cosmic)";} else {title << "(pbar)";}
      hDISTSTD[i] = new TH1F(name.str().c_str(),title.str().c_str(), 100, 0., dist_xmax);
      hDISTSTD[i]->SetXTitle("m");
      hDISTSTD[i]->SetFillColor(colors[i]); hDISTSTD[i]->SetFillStyle(stile[i]);

   }

}

void BV_TREE_analysis::PrepareHistos() {
   ///< Corresponding variables for maximum
   Float_t hnDigiymax =0., hnBarsymax = 0., hEdepymax = 0., hEvTotEneymax = 0., hBarIDymax = 0.;
   Float_t hZymax=0., hTOFymax=0.;
   Float_t hATopymax = 0., hABotymax = 0., hlnAymax = 0., hsqrtAymax = 0., hdtymax = 0.;
   Float_t hTOFMINymax=0., hTOFMAXymax = 0., hTOFMEANymax = 0., hTOFSTDymax = 0.;
   Float_t hDISTMINymax=0., hDISTMAXymax = 0., hDISTMEANymax = 0., hDISTSTDymax = 0.;
   Float_t hDZETAMINymax=0., hDZETAMAXymax = 0., hDZETAMEANymax = 0., hDZETASTDymax = 0.;
   Float_t hDPHIMINymax=0., hDPHIMAXymax = 0., hDPHIMEANymax = 0., hDPHISTDymax = 0.;
   ///< Finding max values for some histograms
   for(unsigned int i=0; i<2; i++) {
      if(hnDigi[i]->GetMaximum()>hnDigiymax) hnDigiymax = hnDigi[i]->GetMaximum();
      if(hnBars[i]->GetMaximum()>hnBarsymax) hnBarsymax = hnBars[i]->GetMaximum();
      if(hEdep[i]->GetMaximum()>hEdepymax) hEdepymax = hEdep[i]->GetMaximum();
      if(hEvTotEne[i]->GetMaximum()>hEvTotEneymax) hEvTotEneymax = hEvTotEne[i]->GetMaximum();
      if(hBarID[i]->GetMaximum()>hBarIDymax) hBarIDymax = hBarID[i]->GetMaximum();
      if(hZ[i]->GetMaximum()>hZymax) hZymax = hZ[i]->GetMaximum();
      if(hTOF[i]->GetMaximum()>hTOFymax) hTOFymax = hTOF[i]->GetMaximum();

      if(hATop[i]->GetMaximum()>hATopymax) hATopymax = hATop[i]->GetMaximum();
      if(hABot[i]->GetMaximum()>hABotymax) hABotymax = hABot[i]->GetMaximum(); 
      if(hlnA[i]->GetMaximum()>hlnAymax) hlnAymax = hlnA[i]->GetMaximum();
      if(hsqrtA[i]->GetMaximum()>hsqrtAymax) hsqrtAymax = hsqrtA[i]->GetMaximum();
      if(hdt[i]->GetMaximum()>hdtymax) hdtymax = hdt[i]->GetMaximum();
 
      if(hTOFMIN[i]->GetMaximum()>hTOFMINymax) hTOFMINymax = hTOFMIN[i]->GetMaximum();
      if(hTOFMAX[i]->GetMaximum()>hTOFMAXymax) hTOFMAXymax = hTOFMAX[i]->GetMaximum();
      if(hTOFMEAN[i]->GetMaximum()>hTOFMEANymax) hTOFMEANymax = hTOFMEAN[i]->GetMaximum();
      if(hTOFSTD[i]->GetMaximum()>hTOFSTDymax) hTOFSTDymax = hTOFSTD[i]->GetMaximum();
      if(hDISTMIN[i]->GetMaximum()>hDISTMINymax) hDISTMINymax = hDISTMIN[i]->GetMaximum();
      if(hDISTMAX[i]->GetMaximum()>hDISTMAXymax) hDISTMAXymax = hDISTMAX[i]->GetMaximum();
      if(hDISTMEAN[i]->GetMaximum()>hDISTMEANymax) hDISTMEANymax = hDISTMEAN[i]->GetMaximum();
      if(hDISTSTD[i]->GetMaximum()>hDISTSTDymax) hDISTSTDymax = hDISTSTD[i]->GetMaximum();
      if(hDZETAMIN[i]->GetMaximum()>hDZETAMINymax) hDZETAMINymax = hDZETAMIN[i]->GetMaximum();
      if(hDZETAMAX[i]->GetMaximum()>hDZETAMAXymax) hDZETAMAXymax = hDZETAMAX[i]->GetMaximum();
      if(hDZETAMEAN[i]->GetMaximum()>hDZETAMEANymax) hDZETAMEANymax = hDZETAMEAN[i]->GetMaximum();
      if(hDZETASTD[i]->GetMaximum()>hDZETASTDymax) hDZETASTDymax = hDZETASTD[i]->GetMaximum();
      if(hDPHIMIN[i]->GetMaximum()>hDPHIMINymax) hDPHIMINymax = hDPHIMIN[i]->GetMaximum();
      if(hDPHIMAX[i]->GetMaximum()>hDPHIMAXymax) hDPHIMAXymax = hDPHIMAX[i]->GetMaximum();
      if(hDPHIMEAN[i]->GetMaximum()>hDPHIMEANymax) hDPHIMEANymax = hDPHIMEAN[i]->GetMaximum();
      if(hDPHISTD[i]->GetMaximum()>hDPHISTDymax) hDPHISTDymax = hDPHISTD[i]->GetMaximum();
   }
   ///< Scaling, setting max and min
   for(unsigned int i=0; i<2; i++) {
         hnBars[i]->Scale(1./n_tot_events); hnBars[i]->SetMinimum(0.); hnBars[i]->SetMaximum((hnBarsymax*1.1)/n_tot_events);
         hBarID[i]->Scale(1./n_tot_events); hBarID[i]->SetMinimum(0.); hBarID[i]->SetMaximum((hBarIDymax*1.1)/n_tot_events);
         hEdep[i]->Scale(1./n_tot_events); hEdep[i]->SetMinimum(0.); hEdep[i]->SetMaximum((hEdepymax*1.1)/n_tot_events);
         ht[i]->Scale(1./n_tot_events); ht[i]->SetMinimum(0.);
         hZ[i]->Scale(1./n_tot_events); hZ[i]->SetMinimum(0.); hZ[i]->SetMaximum((hZymax*1.1)/n_tot_events);
         hEvTotEne[i]->Scale(1./n_tot_events); hEvTotEne[i]->SetMinimum(0.); hEvTotEne[i]->SetMaximum((hEvTotEneymax*1.1)/n_tot_events);

         hATop[i]->Scale(1./n_tot_events); hATop[i]->SetMinimum(0.); hATop[i]->SetMaximum((hATopymax*1.1)/n_tot_events);
         hABot[i]->Scale(1./n_tot_events); hABot[i]->SetMinimum(0.); hABot[i]->SetMaximum((hABotymax*1.1)/n_tot_events);
         hlnA[i]->Scale(1./n_tot_events); hlnA[i]->SetMinimum(0.); hlnA[i]->SetMaximum((hlnAymax*1.1)/n_tot_events);
         hsqrtA[i]->Scale(1./n_tot_events); hsqrtA[i]->SetMinimum(0.); hsqrtA[i]->SetMaximum((hsqrtAymax*1.1)/n_tot_events);
         hdt[i]->Scale(1./n_tot_events); hdt[i]->SetMinimum(0.); hdt[i]->SetMaximum((hdtymax*1.1)/n_tot_events);

         hTOF[i]->Scale(1./n_tot_events); hTOF[i]->SetMinimum(0.); hTOF[i]->SetMaximum(hTOFymax/n_tot_events*1.1);
         hDIST[i]->Scale(1./n_tot_events); hDIST[i]->SetMinimum(0.);
         hDz[i]->Scale(1./n_tot_events); hDz[i]->SetMinimum(0.);
         hDPhi[i]->Scale(1./n_tot_events); hDPhi[i]->SetMinimum(0.);

         hTOFMIN[i]->Scale(1./n_tot_events); hTOFMIN[i]->SetMinimum(0.); hTOFMIN[i]->SetMaximum(hTOFMINymax/n_tot_events*1.1);
         hTOFMAX[i]->Scale(1./n_tot_events); hTOFMAX[i]->SetMinimum(0.); hTOFMAX[i]->SetMaximum(hTOFMAXymax/n_tot_events*1.1);
         hTOFMEAN[i]->Scale(1./n_tot_events); hTOFMEAN[i]->SetMinimum(0.); hTOFMEAN[i]->SetMaximum(hTOFMEANymax/n_tot_events*1.1);
         hTOFSTD[i]->Scale(1./n_tot_events); hTOFSTD[i]->SetMinimum(0.); hTOFSTD[i]->SetMaximum(hTOFSTDymax/n_tot_events*1.1);

         hDPHIMIN[i]->Scale(1./n_tot_events); hDPHIMIN[i]->SetMinimum(0.); hDPHIMIN[i]->SetMaximum(hDPHIMINymax/n_tot_events*1.1);
         hDPHIMAX[i]->Scale(1./n_tot_events); hDPHIMAX[i]->SetMinimum(0.); hDPHIMAX[i]->SetMaximum(hDPHIMAXymax/n_tot_events*1.1);
         hDPHIMEAN[i]->Scale(1./n_tot_events); hDPHIMEAN[i]->SetMinimum(0.); hDPHIMEAN[i]->SetMaximum(hDPHIMEANymax/n_tot_events*1.1);
         hDPHISTD[i]->Scale(1./n_tot_events); hDPHISTD[i]->SetMinimum(0.); hDPHISTD[i]->SetMaximum(hDPHISTDymax/n_tot_events*1.1);
         
         hDZETAMIN[i]->Scale(1./n_tot_events); hDZETAMIN[i]->SetMinimum(0.); hDZETAMIN[i]->SetMaximum(hDZETAMINymax/n_tot_events*1.1);
         hDZETAMAX[i]->Scale(1./n_tot_events); hDZETAMAX[i]->SetMinimum(0.); hDZETAMAX[i]->SetMaximum(hDZETAMAXymax/n_tot_events*1.1);
         hDZETAMEAN[i]->Scale(1./n_tot_events); hDZETAMEAN[i]->SetMinimum(0.); hDZETAMEAN[i]->SetMaximum(hDZETAMEANymax/n_tot_events*1.1);
         hDZETASTD[i]->Scale(1./n_tot_events); hDZETASTD[i]->SetMinimum(0.); hDZETASTD[i]->SetMaximum(hDZETASTDymax/n_tot_events*1.1);
         
         hDISTMIN[i]->Scale(1./n_tot_events); hDISTMIN[i]->SetMinimum(0.); hDISTMIN[i]->SetMaximum(hDISTMINymax/n_tot_events*1.1);
         hDISTMAX[i]->Scale(1./n_tot_events); hDISTMAX[i]->SetMinimum(0.); hDISTMAX[i]->SetMaximum(hDISTMAXymax/n_tot_events*1.1);
         hDISTMEAN[i]->Scale(1./n_tot_events); hDISTMEAN[i]->SetMinimum(0.); hDISTMEAN[i]->SetMaximum(hDISTMEANymax/n_tot_events*1.1);
         hDISTSTD[i]->Scale(1./n_tot_events); hDISTSTD[i]->SetMinimum(0.); hDISTSTD[i]->SetMaximum(hDISTSTDymax/n_tot_events*1.1);
   }
}