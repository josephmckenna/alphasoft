
void PlotProjections() 
{

   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* h;

   // Loads zed histogram
   fin->GetObject("/bsc/hBsc_AmplitudeVsChannel",h);

   TH1 *hbins[16];
   int color = 0;
   for (int ii=0;ii<16;ii++)
      {
         hbins[ii] = h->ProjectionY(Form("bin%d",ii+1),ii+1,ii+2);
         if (hbins[ii]->GetEntries()>0)
            {
               color++;
               hbins[ii]->Rebin(20);
               hbins[ii]->SetLineColor(color);
               hbins[ii]->GetXaxis()->SetRangeUser(0,3);
               hbins[ii]->Draw("same");
            }
      }

}
