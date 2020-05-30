
void FitQDT() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* hQDT;

   // Loads zed histogram
   fin->GetObject("/bv_tpc_matching_module/hZvsDT2",hQDT);
   TF1* fit = new TF1("fit","[0]*x+[1]",-1200,1200);
   hQDT->Fit(fit,"R");

}
