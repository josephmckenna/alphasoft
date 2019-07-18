
void FitTails() 
{
   // List of bad bars
   int badbars[22] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,25,27,35,52,55,59};

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH1D* hZedProfile[64];
   TH1D* hZedProfileFull = new TH1D("Z profile","Z profile",6000,-3,3);
   TH2D* hZed;
   TH1D* hBarL = new TH1D("Calculated Length of Bar","Calculated Length of Bar",64,-0.5,63.5);
   TH1D* hZtop = new TH1D("Z of Bar Top","Z of Top of Bar;Bar Number;Zed [m]",64,-0.5,63.5);
   TH1D* hZbot = new TH1D("Z of Bar Bottom","Z of Bottom of Bar;Bar Number;Zed [m]",64,-0.5,63.5);

   // Loads zed histogram
   fin->GetObject("/bsc_tdc_module/hTdcZed",hZed);
   cout<<"Total entries: "<<hZed->Integral()<<endl;

   // Creates output files
   TFile* outf = new TFile("FitTailsOutput.root", "recreate");
   TCanvas* can = new TCanvas("","");
   TString pdf = "output.pdf";
   can->Print(pdf+"[");

   // _____________
   // For each bar:
   for (int i_bar = 0; i_bar<64; i_bar++)
      {
         // Generates projections
         char name[128];
         sprintf(name,"hZedProfile%d",i_bar);
         hZedProfile[i_bar] = hZed->ProjectionY(name,i_bar+1,i_bar+1); // Individual bars

         // Adds to main histogram
         int* p = std::find(std::begin(badbars), std::end(badbars), i_bar);
         if (p==std::end(badbars)) { hZedProfileFull->Add(hZedProfile[i_bar]); }
         
         // Rebins and retitles histogram
         hZedProfile[i_bar]->Rebin(60);
         char title[128];
         sprintf(title,"Zed of events on Bar %d",i_bar);
         hZedProfile[i_bar]->SetTitle(title);
         hZedProfile[i_bar]->Draw();
         
         // Fits distribution
         TF1* lfit = new TF1("lfit","(x>[3])*([0]*exp(-0.5*((x-[3])/[4])^2))+(x<[3] && x>[1])*([0])+(x<[1])*([0]*exp(-0.5*((x-[1])/[2])^2))",-3.,3.);
         lfit->SetParameters(100.,-2.0,1.0,2.0,1.0);
         lfit->SetParLimits(1,-2.0,-0.5);
         lfit->SetParLimits(3,0.5,2.0);
         hZedProfile[i_bar]->Fit("lfit","R");
         lfit->Draw("same");

         // Calculates top and bottom position using the mean and sigma of the gaussians
         double Zbot = lfit->GetParameter(1);
         double Ztop = lfit->GetParameter(3);
         double sigmaZbot = lfit->GetParameter(2);
         double sigmaZtop = lfit->GetParameter(4);

         // Calculates the top and bottom position using 90% max and 10%-90% max.
         //double max = lfit->GetParameter(0);
         //double Zbot = lfit->GetX(max*0.9,-3.,0.);
         //double Ztop = lfit->GetX(max*0.9,0.,3.);
         //double sigmaZbot = lfit->GetX(max*0.9,-3.,0.)-lfit->GetX(max*0.1,-3.,0.);
         //double sigmaZtop = lfit->GetX(max*0.1,0.,3.)-lfit->GetX(max*0.9,0.,3.);

         // Calculates bar length
         double BarL = Ztop - Zbot;
         double sigmaBarL = TMath::Sqrt(TMath::Power(sigmaZbot,2)+TMath::Power(sigmaZtop,2));

         // Only writes for bars whose values make sense
         if (p==std::end(badbars))
            {
               // Fills histograms
               hZtop->Fill(i_bar,Ztop);
               hZtop->SetBinError(i_bar+1,sigmaZtop);
               hZbot->Fill(i_bar,Zbot);
               hZbot->SetBinError(i_bar+1,sigmaZbot);
               hBarL->Fill(i_bar,BarL);
               hBarL->SetBinError(i_bar+1,sigmaBarL);
            }

         can->Print(pdf);
         delete lfit;

      }
      
      // Repeats for the combined data from all bars
      //hZedProfileFull = hZed->ProjectionY("Zed (all bars)",16,64); // All bars
      hZedProfileFull->Rebin(10);
      hZedProfileFull->SetTitle("Zed of events on all bars;Zed [m]");
      hZedProfileFull->Draw();
      TF1* lfit = new TF1("lfit","(x>[3])*([0]*exp(-0.5*((x-[3])/[4])^2))+(x<[3] && x>[1])*([0])+(x<[1])*([0]*exp(-0.5*((x-[1])/[2])^2))",-3.,3.);
      lfit->SetParameters(100.,-2.0,1.0,2.0,1.0);
      lfit->SetParLimits(1,-2.0,-0.5);
      lfit->SetParLimits(3,0.5,2.0);
      hZedProfileFull->Fit("lfit","R");
      lfit->Draw("same");
      double Zbot = lfit->GetParameter(1);
      double Ztop = lfit->GetParameter(3);
      double sigmaZbot = lfit->GetParameter(2);
      double sigmaZtop = lfit->GetParameter(4);
      //double max = lfit->GetParameter(0);
      //double Zbot = lfit->GetX(max*0.9,-3.,0.);
      //double Ztop = lfit->GetX(max*0.9,0.,3.);
      //double sigmaZbot = lfit->GetX(max*0.9,-3.,0.)-lfit->GetX(max*0.1,-3.,0.);
      //double sigmaZtop = lfit->GetX(max*0.1,0.,3.)-lfit->GetX(max*0.9,0.,3.);
      double BarL = Ztop - Zbot;
      double sigmaBarL = TMath::Sqrt(TMath::Power(sigmaZbot,2)+TMath::Power(sigmaZtop,2));
      cout<<"Z of bottom:"<<Zbot<<"\tError: "<<sigmaZbot<<endl;
      cout<<"Z of top:"<<Ztop<<"\tError: "<<sigmaZtop<<endl;
      cout<<"Bar Length: "<<BarL<<"\tError: "<<sigmaBarL<<endl;
      can->Print(pdf);
      delete lfit;

   // Writes histograms
   hZtop->Draw();
   can->Print(pdf);
   hZbot->Draw();
   can->Print(pdf);
   hBarL->Draw();
   can->Print(pdf);

   // Saves file
   outf->Write();
   can->Print(pdf+"]");

   // Cleanup
   delete can;
   delete outf;
   delete hBarL;
   delete hZtop;
   delete hZbot;

}

