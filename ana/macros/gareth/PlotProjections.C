
void PlotProjections() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH3D* h;

   // Loads zed histogram
   fin->GetObject("/bsc_tdc_module/hAmpVIntVChan",h);

   for (int ii=0;ii<128;ii++)
      {
         h->GetZaxis()->SetRange(ii+1,ii+1);
         TH2D* h2 = (TH2D*)h->Project3D("yx");
         h2->SetName(Form("hAmpVInt-Chan%d",ii));
      }

}
