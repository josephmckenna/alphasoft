
void FitTails() 
{
   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH1D* hZedProfile[64];
   TH2D* hZed;
   TH1D* hBarL = new TH1D("Calculated Length of Bar","Calculated Length of Bar",64,-0.5,63.5);
   TH1D* hZtop = new TH1D("Z of Bar Top","Z of Top of Bar;Bar Number;Zed [m]",64,-0.5,63.5);
   TH1D* hZbot = new TH1D("Z of Bar Bottom","Z of Bottom of Bar;Bar Number;Zed [m]",64,-0.5,63.5);

   // Loads tree
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
         
         // Rebins and retitles histogram
         hZedProfile[i_bar]->Rebin(60);
         char title[128];
         sprintf(title,"Zed of events on Bar %d",i_bar);
         hZedProfile[i_bar]->SetTitle(title);
         hZedProfile[i_bar]->Draw();
         
         // Fits distribution
         TF1* lfit = new TF1("lfit","(x>[3])*([0]*exp(-0.5*((x-[3])/[4])^2))+(x<[3] && x>[1])*([0])+(x<[1])*([0]*exp(-0.5*((x-[1])/[2])^2))",-3.,3.);
         lfit->SetParameters(100.,-2.0,1.0,2.0,1.0);
         lfit->SetParLimits(1,-2.5,-1.5);
         lfit->SetParLimits(3,1.5,2.5);
         hZedProfile[i_bar]->Fit("lfit","R");
         lfit->Draw("same");

         // Calculates top and bottom position, and bar length
         double Zbot = lfit->GetParameter(1);
         double Ztop = lfit->GetParameter(3);
         double sigmaZbot = lfit->GetParameter(2);
         double sigmaZtop = lfit->GetParameter(4);
         double BarL = Ztop - Zbot;
         double sigmaBarL = TMath::Sqrt(TMath::Power(sigmaZbot,2)+TMath::Power(sigmaZtop,2));

         double max = lfit->GetParameter(0);
         double Zbot10 = lfit->GetX(max*.1, -3.,0);
         double Zbot90 = lfit->GetX(max*.9, -3.,0);
         double Ztop10 = lfit->GetX(max*.1, 0,3.);
         double Ztop90 = lfit->GetX(max*.9, 0,3.);
         double deltaZbot = Zbot90 - Zbot10;
         double deltaZtop = Ztop10 - Ztop90;

         // Only writes for bars whose values make sense
         if (sigmaBarL < 5.)
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

