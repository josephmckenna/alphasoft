#define ReadScintBarHits_cxx
#include "ReadScintBarHits.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <bitset>
                               //WELAAAAAAAAA
void ReadScintBarHits::Loop(Float_t EnergyCut=-999.0,
                            Float_t DeltaPhiCut = -999.0, 
                            Int_t   MultCut = 999, 
                            Float_t smearingTime = -999.0,
                            Int_t   PDGcode = -999) //TAGLIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
{
//   In a ROOT session, you can do:
//      root> .L ReadScintBarHits.C
//      root> ReadScintBarHits t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
//_________________________________________________________________________________________

   const Float_t Radius=223; //[mm]
   const Float_t speedC=299.792458; //[mm/ns]
   const Int_t   NBARS=64;
//_________________________________________________________________________________________   

   TH1F *hNbarsHitperEvent = new TH1F("hNbarsHitperEvent", "hNbarsHitperEvent",20,0,20);
   TH1F *hBarNumber = new TH1F("hBarNumber", "hBarNumber", NBARS,0,NBARS);
   TH1F *hNdigiperEvent = new TH1F("hNdigiperEvent","hNdigiperEvent", 20,0,20);
   TH1F *hNdigiMinusNbarsHitperEvent =  new TH1F("hNdigiMinusNbarsHitperEvent","hNdigiMinusNbarsHitperEvent",40,-20,20);
   TH1F *hEnergyperEvent = new TH1F("hEnergyperEvent","hEnergyperEvent", 200,0,100);
   TH1F *hEnergyperBar = new TH1F("hEnergyperBar", "hEnergyperBar", 200,0,20);
   TH1F *hEnergyperBarMult = new TH1F("hEnergyperBarMult", "hEnergyperBarMult", 200,0,20);
   TH2F *hEnergyperDigivsPDGcode = new TH2F("hEnergyperDigitvsPDGcode", "hEnergyperDigivsPDGcode", 100,0,100,600,-300,300);
   
   TH1F *hzin = new TH1F("hzin","hzin",500,-1500,+1500); //[mm]
   hzin->GetXaxis()->SetTitle("z [mm]");
   TH1F *hzout = new TH1F("hzout","hzout",500,-1500,+1500);
   TH1F *hzinMinuszout = new TH1F("hzinMinuszout","hzinMinuszout",1000,-1500,+1500);
   hzinMinuszout->GetXaxis()->SetTitle("z_in-z_out [mm]");
   
   TH1F *hTOF = new TH1F("hTOF","hTOF",200,0,10 );
   hTOF->GetXaxis()->SetTitle("TOF [ns]");
   TH1F *hdeltaz = new TH1F("hdeltaz","hdeltaz",500,0,2000);
   hdeltaz->GetXaxis()->SetTitle("#Deltaz [mm]");
   TH2F *hTOFvsDistance = new TH2F("hTOFvsDistance","hTOFvsDistance",400,0,10, 400,0, 10 );
   hTOFvsDistance->GetXaxis()->SetTitle("distance/c [ns]");
   hTOFvsDistance->GetYaxis()->SetTitle("TOF [ns]");


//_________________________________________________________________________________________

   gRandom->SetSeed(0);
   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) //loop on Events
   {
      //std::cout<<jentry<<")"<<std::endl;

      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      Int_t ndigi = ScintBarHits->GetEntriesFast();
      if(ndigi==0) continue;
      
      hNdigiperEvent->Fill(ndigi);//****************

      Int_t barFlag[NBARS]={0}; //quante volte ho un hit sulla barra
      Int_t barFlag_in[NBARS]={0}; //quante volte ho un incoming hit sulla barra
      Int_t NbarsHit =0;

      Float_t Energy[NBARS]={0};
      Float_t TotEnergy=0;
      Float_t time[NBARS]={0};
      Float_t z[NBARS]={0};
      Float_t phi[NBARS]={0};
      for (Int_t i=0;i<ndigi;i++) //loop on digi //NO CUTS HERE!!!
      {
         TScintDigi *Scdigi = (TScintDigi*) ScintBarHits->At(i);
         if(PDGcode>0 && (Scdigi->GetTrackPDG()!=PDGcode && Scdigi->GetTrackPDG()!=-PDGcode)) continue;

         Int_t barN = Scdigi->GetBar();
        
         //std::cout<<barN<<std::endl;
         hEnergyperDigivsPDGcode->Fill(Scdigi->GetEnergy(),Scdigi->GetTrackPDG());//****************

         Energy[barN]+=Scdigi->GetEnergy();
         TotEnergy +=Scdigi->GetEnergy();

         double p_in[3];
         double p_out[3];
         Scdigi->GetPos_in(p_in);
         Scdigi->GetPos_out(p_out);

         if(!isnan(p_in[2])) hzin->Fill(p_in[2]);//****************
         if(!isnan(p_out[2])) hzout->Fill(p_out[2]);//****************
         if(!isnan(p_in[2]) && !isnan(p_out[2])) hzinMinuszout->Fill(p_in[2]-p_out[2]);//****************

         
         if( !isnan(Scdigi->GetTimeIn()) && !isnan(p_in[2]))
         {
            time[barN]+=Scdigi->GetTimeIn(); //poi divido per nro di digi in quella barra
            z[barN]+=p_in[2];
            phi[barN]=barN*TMath::Pi()/32;

            barFlag_in[barN]+=1; //aggiungo un 1 all'elemento barN-esimo
         }  
         
         barFlag[barN]+=1;
         if(barFlag[barN]==1) //se la barraN è appena stata colpita
         {
            NbarsHit++;
            hBarNumber->Fill(barN);//****************
         }
          
      }//end loop on digi
      
      hEnergyperEvent->Fill(TotEnergy);//****************
      
      Float_t TOF=nan("");
      Float_t deltaz=nan("");
      Float_t deltaphi=nan("");
      Float_t distance=nan("");
      
      for(Int_t i=0;i<NBARS;i++) //loop on bars
      {
         if(barFlag[i]==0) continue; //if bar not hit
         if(Energy[i]<EnergyCut) //riduce il numero di barre colpine nell'evento
         {
            barFlag[i]=0;
            barFlag_in[i]=0;
            NbarsHit-=1;
         }
      }
      if(NbarsHit == 0 || NbarsHit>MultCut) continue; //Cut on Multiplicity

      for(Int_t i=0;i<NBARS;i++) //loop on bars
      {
         //cout<<barFlag[i];
         if(barFlag[i]==0) continue; //if bar not hit
         if(Energy[i]<EnergyCut) continue; //Cut on energy released in the bar
         
         hEnergyperBar->Fill(Energy[i]);//****************

         if(barFlag_in[i]==0) continue; //if bar not hit
         time[i]/=barFlag_in[i]; //media dei tempi se ho multiple hit in one bar
         if(smearingTime>0) time[i] += gRandom->Gaus(0,smearingTime);
         z[i]/=barFlag_in[i];
         for (Int_t j = 0; j < i; j++)
         {
            if(barFlag_in[j]==0) continue; //if bar not hit
            TOF =  TMath::Abs(time[j]-time[i]);
            deltaz = TMath::Abs(z[j]-z[i]);
            deltaphi = TMath::Abs(phi[j]-phi[i]);

            if(deltaphi<DeltaPhiCut) continue; //cut on deltaphi

            distance = TMath::Sqrt(deltaz*deltaz+(2*Radius*TMath::Sin(deltaphi/2))*(2*Radius*TMath::Sin(deltaphi/2)));

            hTOF->Fill(TOF);//****************
            hdeltaz->Fill(deltaz);//****************
            hTOFvsDistance->Fill(distance/speedC,TOF);//****************
         }
      }//end loop on bars

      //cout<<endl;
      hNbarsHitperEvent->Fill(NbarsHit);//****************
      hNdigiMinusNbarsHitperEvent->Fill(ndigi-NbarsHit);//****************
   }//end loop on events
//________________________________________________________________________

   TCanvas *c = new TCanvas("c","c",1200,800);
   c->Divide(3,2);
   c->cd(1);
   hNdigiperEvent->Draw();
   c->cd(2);
   hNbarsHitperEvent->Draw();
   c->cd(3);
   hNdigiMinusNbarsHitperEvent->Draw();
   c->cd(4);
   hBarNumber->Draw();
   c->cd(5);
   hEnergyperBar->Draw();
   c->cd(6);
   hEnergyperDigivsPDGcode->Draw("colz");
   
   
   
   TCanvas *c1 = new TCanvas("c1","c1",1200,800);
   c1->Divide(3,2);
   c1->cd(1);
   hzin->Draw();
   hzout->SetLineColor(kSpring);
   hzout->Draw("same");
   c1->cd(2);
   hzinMinuszout->Draw();
   c1->cd(3);
   hTOF->Draw();
   c1->cd(4);
   hdeltaz->Draw();
   c1->cd(5);
   hTOFvsDistance->Draw("colz");

   TFile *fOut = new TFile("fHistoOut"+filename+Form("EnergyCut%1.1f_DeltaPhiCut%1.1f_MultCut%d_timesmeaaring%1.1f_PDGcode%d.root",EnergyCut,DeltaPhiCut,MultCut,smearingTime,PDGcode),"RECREATE");
   fOut->cd();
   hNdigiperEvent->Write();
   hNbarsHitperEvent->Write();
   hNdigiMinusNbarsHitperEvent->Write();
   hBarNumber->Write();
   hEnergyperBar->Write();
   hEnergyperBarMult->Write();
   hEnergyperDigivsPDGcode->Write();
   hEnergyperEvent->Write();
   hzin->Write();
   hzout->Write();
   hzinMinuszout->Write();
   hTOFvsDistance->Write();
   fOut->Close();
}
