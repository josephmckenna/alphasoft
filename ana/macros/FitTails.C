
void FitTails() 
{
   // List of bad bars
   int badbars[22] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,25,27,35,52,55,59};

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* hZed;

   // Loads zed histogram
   fin->GetObject("/bsc_tdc_module/hZed",hZed);
   cout<<"Total entries: "<<hZed->Integral()<<endl;

   // Creates output files
   TCanvas* can = new TCanvas("","");
   TString pdf = "output.pdf";
   can->Print(pdf+"[");

   TH1D* hZedProfile = hZed->ProjectionY("Zed Profile"); // All bars
   hZedProfile->Rebin(10);
   hZedProfile->SetTitle("Zed;Zed [m]");
   hZedProfile->Draw();

   /*
   TF1* lfit = new TF1("lfit","(x>[3])*([0]*exp(-0.5*((x-[3])/[4])^2))+(x<[3] && x>[1])*([0])+(x<[1])*([0]*exp(-0.5*((x-[1])/[2])^2))",-3.,3.);
   lfit->SetParameters(100.,-2.0,1.0,2.0,1.0);
   lfit->SetParLimits(1,-2.0,-0.5);
   lfit->SetParLimits(3,0.5,2.0);
   hZedProfile->Fit("lfit","R");
   lfit->Draw("same");
   double Zbot = lfit->GetParameter(1);
   double Ztop = lfit->GetParameter(3);
   double sigmaZbot = lfit->GetParameter(2);
   double sigmaZtop = lfit->GetParameter(4);
   double BarL = Ztop - Zbot;
   double halfbot = lfit->GetX(lfit->GetParameter(0)/2.,-3.,0.);
   double halftop = lfit->GetX(lfit->GetParameter(0)/2.,0.,3.);
   double FWHM = halftop - halfbot;
   cout<<"Bottom gaus mean = "<<Zbot<<"\t sigma = "<<sigmaZbot<<endl;
   cout<<"Top gaus mean = "<<Ztop<<"\t sigma = "<<sigmaZtop<<endl;
   cout<<"Top to bottom gaus mean = "<<BarL<<endl;
   cout<<"Width at half max = "<<FWHM<<endl;
   */
   
   TF1* step = new TF1("step","(-[0]<x)*(x<[0])*[1]",-3,3);
   TF1Convolution* f_conv = new TF1Convolution("step","gaus",-3,3);
   f_conv->SetRange(-3,3);
   TF1* fit = new TF1("fit",*f_conv, -3,3, f_conv->GetNpar());
   fit->SetParameters(1.,1000.,1.,0.,0.5);
   hZedProfile->Fit("fit","R");


   can->Print(pdf);
   // Saves file
   can->Print(pdf+"]");

}

