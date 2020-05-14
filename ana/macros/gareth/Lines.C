
void Lines() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* h0;

   // Loads zed histogram
   fin->GetObject("/bsc_tdc_module/hTrigEventVTDCTrig",h0);

   TH2D* h1 = (TH2D*)h0->Clone("h1");
   TH2D* h2 = (TH2D*)h0->Clone("h2");
   for (int ix=1; ix<=2000; ix++)
      {
         for (int iy=1;iy<=2000;iy++)
            {
               double x = h1->GetXaxis()->GetBinCenter(ix);
               double y = h1->GetYaxis()->GetBinCenter(iy);
               if (x-y>0.1) h1->SetBinContent(ix,iy,0);
               else h2->SetBinContent(ix,iy,0);
            }
      }
   TF1* f1 = new TF1("f1","pol1");
   TF1* f2 = new TF1("f2","pol1");
   h1->Fit(f1);
   h2->Fit(f2);
   TH2D* hdiff = new TH2D("hdiff","Difference between the two lines;Time [s];Time difference [s]",100,0.,25.,100000,0.,1.);
   for (int xi=0;xi<10000;xi++)
      {
         double x = xi*25./10000;
         hdiff->Fill(x,f1->Eval(x)-f2->Eval(x));
      }
   TF1* fdiff = new TF1("fdiff","pol1");
   hdiff->Fit(fdiff);

}
