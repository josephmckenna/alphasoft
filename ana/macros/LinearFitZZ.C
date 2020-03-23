
void LinearFitZZ() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* hZVZ;

   // Loads zed histogram
   fin->GetObject("/bv_tpc_matching_module/hZBVvZTPC",hZVZ);

   TF1* fit = new TF1("fit","pol1",-1500,1500);
   hZVZ->Fit(fit,"R");


}
