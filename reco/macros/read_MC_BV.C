#define read_MC_BV_cxx
#include "read_MC_BV.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

///< ############################################################
///< HISTOS AND NUMBERS FOR THE VERTEX AND MOMENTUM AT GENERATION
///< ############################################################
void read_MC_BV::AnalyzeMCinfo()
{
   //std::string  stat_opt = "im";      // i = integral, m = mean, r = rms, etc. 
   gStyle->SetOptStat(1);
   //##########################################################
   // histograms
   int nbin = 1000, nbinr = 100;
   float rlim = 250., zleft = -130., zright = +130.;
   TH2F *hr = new TH2F("hr", "Antiproton annihilation radius (X vs Y)", nbinr, -rlim, rlim, nbinr, -rlim, rlim);
   hr->SetXTitle("cm"); hr->SetYTitle("cm");
   TH1F *hz = new TH1F("hz", "Antiproton annihilation z", nbin, zleft, zright);
   hz->SetXTitle("cm"); hz->SetYTitle("#");
   float p_tot_origin_max = 20.;
   TH1F *hp = new TH1F("hp","Momentum",100, 0., p_tot_origin_max*1.2);
   hp->SetXTitle("GeV/c"); 
   TH1F *hcos2 = new TH1F("hcos2","cos^2(#theta)",101, -0.05, 1.05); hcos2->SetXTitle("cos^2(#theta)");

   //##########################################################

   if (tMCinfo == 0) return;

   Long64_t nentries = tMCinfo->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   double xvtx, yvtx, zvtx;
   double px, py, pz;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadMCInfoTree(jentry);
      tMCinfo->GetEntry(jentry);   

      TVector3 *v = (TVector3*)MCvertex->At(0); ///< Only 1 vertex per event
      TLorentzVector *p = (TLorentzVector*)MCpions->At(0); ///< Muon momentum
      if(v==nullptr) {
         cout << "NO VERTEX?" << endl;
      } else {
         xvtx = v->x()/10.; yvtx = v->y()/10.; zvtx = v->z()/10.; ///< To have them in cm
         double r = sqrt(xvtx*xvtx+yvtx*yvtx);
         hr->Fill(yvtx, xvtx);
         hz->Fill(zvtx);
      }
      if(p==nullptr) {
         cout << "NO MOMENTUM?" << endl;
      } else {
         px = p->Px()/1000.; py = p->Py()/1000.; pz = p->Pz()/1000.; ///< To have them in GeV
         double ptot = sqrt(px*px+py*py+pz*pz);
         hp->Fill(ptot);
         if(ptot!=0) {
            hcos2->Fill(pow(fabs(pz)/ptot,2.));
         }
      }
   }

   TCanvas *cgen = new TCanvas("cgen","Vertex and momentum",1200,1200);
   cgen->Divide(2,2);
   cgen->cd(1); //cpos_1->SetFillColor(30);
   hz->SetFillColor(10);
   hz->Draw();
   cgen->cd(2); //cpos_2->SetFillColor(38);
   hr->SetFillColor(10);
   hr->Draw("BOX,COLZ");
   cgen->cd(3); //cpos_1->SetFillColor(30);
   hp->SetFillColor(10);
   hp->Draw();
   cgen->cd(4); //cpos_2->SetFillColor(38);
   hcos2->Draw();

}

///< ############################################################
///< HISTOS AND NUMBERS FOR THE BV BARS
///< ############################################################
void read_MC_BV::AnalyzeBVBars(Float_t EnergyCut=-999.0, Float_t DeltaPhiCut = -999.0, Int_t MultCut = -999, Float_t smearingTime = -999.0, Float_t v_reluncertainty = -999.0) //TAGLIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
{
//_________________________________________________________________________________________

   bool  BExcludeBars = 0; //OCCHIO escludi  barre escluse nei dati
   const Float_t Radius=223; //[mm]
   const Float_t speedC=299.792458; //[mm/ns]
   const Float_t Veff=speedC/1.93;
   const Int_t   NBARS=64;
   Float_t nEle = 0., nMu = 0., nPi = 0., nOther = 0.;
//_________________________________________________________________________________________   

   TH1F *hNbarsHitperEvent = new TH1F("hNbarsHitperEvent", "hNbarsHitperEvent",15,-0.5,14.5);
   hNbarsHitperEvent->SetTitle("Number of hit bars per event");
   hNbarsHitperEvent->SetXTitle("N. hit bars"); //hNbarsHitperEvent->SetYTitle("%");
   TH1F *hBarNumber = new TH1F("hBarNumber", "hBarNumber", NBARS,0,NBARS);
   TH1F *hNdigiperEvent = new TH1F("hNdigiperEvent","hNdigiperEvent", 15,-0.5,14.5);
   TH1F *hEnergyperEvent = new TH1F("hEnergyperEvent","hEnergyperEvent", 200,0,100);
   TH1F *hEnergyperBar = new TH1F("hEnergyperBar", "hEnergyperBar", 200,0,20);
   hEnergyperBar->SetTitle("Energy released in a single bar");
   TH1F *hEnergyperBarMult = new TH1F("hEnergyperBarMult", "hEnergyperBarMult", 200,0,20);
   TH2F *hEnergyperDigivsPDGcode = new TH2F("hEnergyperDigitvsPDGcode", "hEnergyperDigivsPDGcode", 100,0,100,600,-300,300);
   
   TH1F *hzin = new TH1F("hzin","hzin",300,-1500,+1500); //[mm]
   hzin->GetXaxis()->SetTitle("z [mm]");
   TH1F *hzout = new TH1F("hzout","hzout",300,-1500,+1500);
   TH1F *hzinMinuszout = new TH1F("hzinMinuszout","hzinMinuszout",1000,-1500,+1500);
   hzinMinuszout->GetXaxis()->SetTitle("z_in-z_out [mm]");
   hzin->SetTitle("Entrance Z"); hzout->SetTitle("Exit X");
   
   TH1F *hTOF = new TH1F("hTOF","hTOF",200,0,10 );
   hTOF->SetTitle("TOF");
   hTOF->GetXaxis()->SetTitle("TOF [ns]");
   TH1F *hDistance = new TH1F("hDistance","hDistance",200,0,10 );
   hDistance->GetXaxis()->SetTitle("distance/c [ns]");
   hDistance->SetTitle("Distance");
   TH1F *hdeltaz = new TH1F("hdeltaz","hdeltaz",500,0,2000);
   hdeltaz->GetXaxis()->SetTitle("#Deltaz [mm]");

   TH2F *hTOFvsDistance = new TH2F("hTOFvsDistance","hTOFvsDistance", 400, 0., 10., 400, 0., 10.);
   hTOFvsDistance->SetTitle("TOF vs Geometric distance/c");
   hTOFvsDistance->GetXaxis()->SetTitle("distance/c [ns]");
   hTOFvsDistance->GetYaxis()->SetTitle("TOF [ns]");

///< Germano
   TH1F *hDigi_Bars = new TH1F("hDigi_Bars", "N. of DIGI - N. of Hit Bars", 10, -0.5, 9.5);

//_________________________________________________________________________________________

   gRandom->SetSeed(0);
   if (tBVBars == 0) return;

   Long64_t nentries = tBVBars->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) //loop on Events
   {
      //std::cout<<jentry<<")"<<std::endl;

      Long64_t ientry = LoadBVBarsTree(jentry);
      if (ientry < 0) break;
      nb = tBVBars->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      Int_t ndigi = ScintBarDigiMCTruth->GetEntriesFast();
      if(ndigi==0) continue;
      
      hNdigiperEvent->Fill(ndigi);//****************

      Int_t barFlag[NBARS]={0}; //quante volte ho un hit sulla barra
      Int_t barFlag_in[NBARS]={0}; //quante volte ho un incoming hit sulla barra
      Int_t NbarsHit =0;
      Float_t TotEnergy=0;

      std::vector<Int_t>   BarNumber;
      std::vector<Float_t> Energy;
      std::vector<Float_t> time;
      std::vector<Float_t> z;
      std::vector<Float_t> phi;
      for (Int_t i=0;i<ndigi;i++)  //loop on digi //NO CUTS HERE!!!
      { 
         TBSCHit *Scdigi = (TBSCHit*) ScintBarDigiMCTruth->At(i);
         Int_t barN = Scdigi->GetBar();

         if(BExcludeBars)
         {
            if( barN==0 || barN==36 || barN==37 || (barN>=48 && barN<=55) ) continue; //OCCHIO  barre escluse nei dati
         }

         if(fabs(Scdigi->GetTrackPDG())==11) {nEle++;} 
         else if(fabs(Scdigi->GetTrackPDG())==13) {nMu++;} 
         else if(fabs(Scdigi->GetTrackPDG())==211) {nPi++;} 
         else {nOther++;}

         // if(fabs(Scdigi->GetTrackPDG())==11) continue; ///< Show only electrons
         // if(fabs(Scdigi->GetTrackPDG())==13) continue; ///< Show only muons
         // if(fabs(Scdigi->GetTrackPDG())==211) continue; ///< Show only pions
         //std::cout<<barN<<std::endl;
         hEnergyperDigivsPDGcode->Fill(Scdigi->GetEnergy(),Scdigi->GetTrackPDG());//****************
         TotEnergy +=Scdigi->GetEnergy();

         double p_in[3];
         double p_out[3];
         Scdigi->GetPos_in(p_in);
         Scdigi->GetPos_out(p_out);

         if(!isnan(p_in[2])) hzin->Fill(p_in[2]);//****************
         if(!isnan(p_out[2])) hzout->Fill(p_out[2]);//****************
         if(!isnan(p_in[2]) && !isnan(p_out[2])) hzinMinuszout->Fill(p_in[2]-p_out[2]);//****************
         
         if(barFlag[barN]) //if barN was already hit
         {
            for (Int_t j = BarNumber.size()-1; j >=0; j--)
            {
               if (BarNumber.at(j)==barN)
               {
                  Energy.at(j)+=Scdigi->GetEnergy();
                  if( !isnan(Scdigi->GetTimeIn()) && !isnan(p_in[2]))
                  {
                     time.at(j)+=Scdigi->GetTimeIn(); //poi divido per nro di digi in quella barra
                     z.at(j)+=p_in[2];
                     barFlag_in[barN]+=1;
                  }
                  break;
               }
               
            }
            
         }
         else
         {
            BarNumber.push_back(barN);
            phi.push_back(barN*TMath::Pi()/32);
            Energy.push_back(Scdigi->GetEnergy());
            if( !isnan(Scdigi->GetTimeIn()) && !isnan(p_in[2]))
               {
                  time.push_back(Scdigi->GetTimeIn());
                  z.push_back(p_in[2]);
                  barFlag_in[barN]+=1;
               }
            else
               {
                  time.push_back(0.0);
                  z.push_back(0.0);
               }
         }
         
         barFlag[barN]+=1;
         if(barFlag[barN]==1) //se la barraN è appena stata colpita
         {
            NbarsHit++;
            hBarNumber->Fill(barN);//****************
         }
      }//end loop on digi
      
      if(ndigi!=0) hDigi_Bars->Fill(ndigi-NbarsHit);
      hEnergyperEvent->Fill(TotEnergy);//****************
      
      Float_t TOF=nan("");
      Float_t deltaz=nan("");
      Float_t deltaphi=nan("");
      Float_t distance=nan("");
      
      // cout<<jentry<<")"<<endl;
      // cout<<BarNumber.size()<<endl;
      // cout<<Energy.size()<<endl;
      // cout<<time.size()<<endl;
      // cout<<z.size()<<endl;
      // cout<<phi.size()<<endl;
      // cout<<endl;

      for(Int_t i=0;i<BarNumber.size();i++) //loop on hits
      {
         if(Energy.at(i)<EnergyCut) //riduce il numero di barre colpine nell'evento
         {
            barFlag[BarNumber.at(i)]=0;
            barFlag_in[BarNumber.at(i)]=0;
            NbarsHit-=1;
         }
      }

      if(NbarsHit == 0 || (MultCut>0 && NbarsHit!=MultCut)) continue; //Cut on Multiplicity

      for(Int_t i=0;i<BarNumber.size();i++) //loop on hits
      {
         //cout<<barFlag[i];
         if(barFlag[BarNumber.at(i)]==0) continue; //if bar not hit
         hEnergyperBar->Fill(Energy.at(i));//****************

         if(barFlag_in[BarNumber.at(i)]==0) continue; //if bar not hit
         time.at(i)/=barFlag_in[BarNumber.at(i)]; //media dei tempi se ho multiple hit in one bar
         if(smearingTime>0) time.at(i) += gRandom->Gaus(0,smearingTime/TMath::Sqrt(2)); //TOF = T1-T2 = (t1top+t1bot)/2 - (t2top+t2bot)/2, so sigmaT1 = sigmat/sqrt(2)
         z.at(i)/=barFlag_in[BarNumber.at(i)];
         if(smearingTime>0 || v_reluncertainty>0)
         {
            Float_t sigmat = 0.0;
            Float_t sigmav = 0.0;
            if(smearingTime>0) sigmat = smearingTime;
            if(v_reluncertainty>0) sigmav = v_reluncertainty;
            Float_t sigmaz = TMath::Sqrt(sigmat*sigmat*Veff*Veff/2 + sigmav*sigmav*z.at(i)*z.at(i));
            z.at(i) += gRandom->Gaus(0,sigmaz);
         }

         for (Int_t j = 0; j < i; j++)
         {
            if(barFlag_in[BarNumber.at(j)]==0) continue; //if bar not hit
            TOF =  TMath::Abs(time.at(j)-time.at(i));
            deltaz = TMath::Abs(z.at(j)-z.at(i));
            deltaphi = TMath::Abs(phi.at(j)-phi.at(i));
            if(deltaphi>TMath::Pi()) deltaphi = 2*TMath::Pi() - deltaphi;

            if(deltaphi<DeltaPhiCut)  continue; //cut on deltaphi

            distance = TMath::Sqrt(deltaz*deltaz+(2*Radius*TMath::Sin(deltaphi/2))*(2*Radius*TMath::Sin(deltaphi/2)));

            hTOF->Fill(TOF);//****************
            hDistance->Fill(distance/speedC);//****************
            hdeltaz->Fill(deltaz);//****************
            hTOFvsDistance->Fill(distance/speedC,TOF);//****************
         }
      }//end loop on bars

      //cout<<endl;
      hNbarsHitperEvent->Fill(NbarsHit);//****************

      BarNumber.clear();
      Energy.clear();
      time.clear();
      z.clear();
      phi.clear();
   }//end loop on events
//________________________________________________________________________

   TCanvas *c = new TCanvas("c","c",1200,800);
   c->Divide(3,2);
   c->cd(1);
   hNdigiperEvent->Draw();
   c->cd(2);
   hNbarsHitperEvent->Draw();
   c->cd(3);
   hBarNumber->Draw();
   c->cd(4);
   hEnergyperBar->Draw();
   hEnergyperBarMult->SetLineColor(kRed);
   hEnergyperBarMult->Draw("same");
   c->cd(5);
   hEnergyperDigivsPDGcode->Draw("colz");
   c->cd(6);
   hEnergyperEvent->Draw();
   
   
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

   TCanvas *cTOF = new TCanvas("cTOF", "cTOF", 1200, 1200);
   hTOFvsDistance->Draw("COLZ");
   // hDigi_Bars->Draw();

   TCanvas *cSummary = new TCanvas("cSummary", "cSummary", 1200, 1200);
   cSummary->Divide(2,2);

   //hNbarsHitperEvent->Scale(1./hNbarsHitperEvent->Integral()*100.);
   cSummary->cd(1); hNbarsHitperEvent->Draw("H");

   hEnergyperBar->SetFillColor(46);
   cSummary->cd(2); hEnergyperBar->Draw();

   cSummary->cd(3);
   hzin->SetLineColor(46); hzout->SetLineColor(38);
   hzin->Draw(""); hzout->Draw("SAME");
   Float_t nAll = nEle + nMu + nPi + nOther;
   cout << "nEle " << nEle*100./nAll << " nMu " << nMu*100./nAll << " nPi " << nPi*100./nAll << " nOther " << nOther*100/nAll << endl;

   TCanvas *cProj = new TCanvas("cProj", "cProj", 1200, 600);
   cProj->Divide(2,1);
   hTOF->SetFillColor(38);
   hDistance->SetFillColor(46);
   cProj->cd(1); hTOF->Draw();
   cProj->cd(2); hDistance->Draw();

   TString snamefileout="simulation/fHistoOut"+filename+Form("EnergyCut%1.1f_DeltaPhiCut%1.1f_MultCut%d_timesmeaaring%1.1f_v_reluncertainty%1.1f",EnergyCut,DeltaPhiCut,MultCut,smearingTime,v_reluncertainty);
   if(BExcludeBars) snamefileout+="_excludedbars";

   snamefileout+=".root";
         
   TFile *fOut = new TFile(snamefileout,"RECREATE");
   fOut->cd();
   hNdigiperEvent->Write();
   hNbarsHitperEvent->Write();
   hBarNumber->Write();
   hEnergyperBar->Write();
   hEnergyperBarMult->Write();
   hEnergyperDigivsPDGcode->Write();
   hEnergyperEvent->Write();
   hzin->Write();
   hzout->Write();
   hzinMinuszout->Write();
   hTOFvsDistance->Write();
   hTOF->Write();
   hDistance->Write();
   fOut->Close();
}
