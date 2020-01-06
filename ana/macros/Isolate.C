
void Isolate() 
{

   for (int ifile=0;ifile<24;ifile++)
      {
         TFile* fin = TFile::Open(Form("./output/R3875/sub0%.2d/output03875.root",ifile));
         TH2D* h;
         fin->GetObject("/bsc_tdc_module/hTrigEventMinusTDCTrig",h);
         cout<<Form("Subrun %d integral = ",ifile)<<h->Integral()<<endl;;
      }

}

