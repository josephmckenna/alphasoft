
void FitTails() 
{
   // List of bad bars
   int badbars[22] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,25,27,35,52,55,59};

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH1D* hZed;

   // Loads zed histogram
   fin->GetObject("/bv_tpc_matching_module/hZBV",hZed);
   cout<<"Total entries: "<<hZed->Integral()<<endl;

   // Creates output files
   TCanvas* can = new TCanvas("","");
   TString pdf = "output.pdf";
   can->Print(pdf+"[");

   hZed->Rebin(10);
   hZed->SetTitle("Zed;Zed [mm]");
   hZed->Draw();

   TF1* lfit = new TF1("lfit","(x>[3])*([0]*exp(-0.5*((x-[3])/[4])^2))+(x<[3] && x>[1])*([0])+(x<[1])*([0]*exp(-0.5*((x-[1])/[2])^2))",-3000,3000);
   lfit->SetParameters(1000.,-1000,400,1000,400);
   hZed->Fit("lfit","R");
   lfit->Draw("same");
   double Zbot = lfit->GetParameter(1);
   double Ztop = lfit->GetParameter(3);
   double sigmaZbot = lfit->GetParameter(2);
   double sigmaZtop = lfit->GetParameter(4);
   double BarL = Ztop - Zbot;
//   double halfbot = lfit->GetX(lfit->GetParameter(0)/2.,-3000,0.);
  // double halftop = lfit->GetX(lfit->GetParameter(0)/2.,0.,3000);
//   double FWHM = halftop - halfbot;
   cout<<"Bottom gaus mean = "<<Zbot<<"\t sigma = "<<sigmaZbot<<endl;
   cout<<"Top gaus mean = "<<Ztop<<"\t sigma = "<<sigmaZtop<<endl;
   cout<<"Top to bottom gaus mean = "<<BarL<<endl;
//   cout<<"Width at half max = "<<FWHM<<endl;
   
   can->Print(pdf);
   // Saves file
   can->Print(pdf+"]");

}

