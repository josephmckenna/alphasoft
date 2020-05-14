
void MakeProfile() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* htop;
   TH2D* hbot;

   // Loads zed histogram
   fin->GetObject("/bsc_tdc_module/ZedVsAmpTop",htop);
   fin->GetObject("/bsc_tdc_module/ZedVsAmpBot",hbot);

   TProfile* ptop = htop->ProfileX();
   TProfile* pbot = hbot->ProfileX();
   TH1D* hptop = (TH1D*)ptop->Rebin(20);
   TH1D* hpbot = (TH1D*)pbot->Rebin(20);
   hptop->SetTitle("Average top peak amplitude across bar Z;Zed [m];Average amplitude");
   hpbot->SetTitle("Average bottom peak amplitude across bar Z;Zed [m];Average amplitude");

}
