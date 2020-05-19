
void FitConv() 
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

   TF1* step = new TF1("step","(-[0]<x)*(x<[0])*[1]",-3000,3000);
   TF1Convolution* f_conv = new TF1Convolution("step","gaus",-3000,3000);
   f_conv->SetRange(-3000,3000);
   TF1* fit = new TF1("fit",*f_conv, -3000,3000, f_conv->GetNpar());
   fit->SetParameters(1000.,1000.,1.,0.,500);
   hZed->Fit("fit","R");


   can->Print(pdf);
   // Saves file
   can->Print(pdf+"]");

}

