
void LinearFitZZ() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* hZVZ;
   TH2D* hPhiVPhi;

   // Loads zed histogram
   fin->GetObject("/bv_tpc_matching_module/hZVZ",hZVZ);
   fin->GetObject("/bv_tpc_matching_module/hPhiVPhi",hPhiVPhi);

   TF1* fitphi = new TF1("fitphi","pol1",-0.92*TMath::Pi(),0.92*TMath::Pi());
   hPhiVPhi->Fit(fitphi,"R");
   TF1* fit = new TF1("fit","pol1",-1500,1500);
   //TF1* fit = new TF1("fit","x+[0]",-1500,1500);
   hZVZ->Fit(fit,"R");


}
