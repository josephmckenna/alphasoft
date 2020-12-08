
void LinearFitZZ() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* hDTVZ;

   // Loads zed histogram
   fin->GetObject("/bv_tpc_matching_module/hDTvZTPC",hDTVZ);

   TF1* fit = new TF1("fit","pol1",-40,40);
   hDTVZ->Fit(fit,"R");


}
