
void SaveHistos() 
{
   // Loads file
   TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
   TString fname(fin->GetName());
   cout<<fname<<" FOUND"<<endl;

   // Makes histos
   TH2D* h;
   TH2D* hZ;
   TH3D* hbar;
   TH2D* hAmp;

   // Loads histograms
   fin->GetObject("/bsc_tdc_module/hTdcTimeTopVsBot",h);
   fin->GetObject("/bsc_tdc_module/hTdcTimeFromTrigger",hZ);
   fin->GetObject("/bsc_tdc_module/hTdcTimeTopVsBotVsBar",hbar);
   fin->GetObject("/bsc_tdc_module/hTdcTimeVsAmp",hAmp);

   // Creates output files
   TCanvas* can = new TCanvas("","");
   TString pdf = "histo.pdf";
   can->Print(pdf+"[");

   // Edits and writes histogram
   TH1D* hZp = hZ->ProjectionY("Event time minus trigger time;Time-trigger [ps]",17,64);
   hZp->Add( hZ->ProjectionY("Event time minus trigger time;Time-trigger [ps]",81,128) );
   hZp->Draw();
   can->Print(pdf);

   // Fits projection
   TF1* fit2 = new TF1("fit2","[0]*exp(-0.5*((x-[1])/[2])^2)",8600000,9000000);
   fit2->SetParameters(100000,8800000,200000);
   hZp->Fit("fit2","R");
   cout<<"Small peak mean = "<<fit2->GetParameter(1)<<", sigma = "<<fit2->GetParameter(2)<<endl;
   //fit2->Draw("same");
   //cout<<"Relative peak size = "<<fit1->GetParameter(0)/fit2->GetParameter(0)<<endl;
   hZp->GetXaxis()->SetRangeUser(-1600000,-1100000);
   gPad->Update();
   can->Print(pdf);
   TF1* fit1 = new TF1("fit1","[0]*exp(-0.5*((x-[1])/[2])^2)",-1600000,-1200000);
   fit1->SetParameters(100000,-1400000,200000);
   hZp->Fit("fit1","R");
   cout<<"Main peak mean = "<<fit1->GetParameter(1)<<", sigma = "<<fit1->GetParameter(2)<<endl;
   fit1->Draw("same");
   can->Print(pdf);

   // Prints projections
   if (0) {
   for (int bar=0;bar<64;bar++) {
      hbar->GetZaxis()->SetRange(bar,bar+1);
      TH2D* hbarp = (TH2D*) hbar->Project3DProfile("xy");
      char name[128];
      sprintf(name,"Time top vs bottom for bar %d",bar);
      hbarp->SetTitle(name);
      hbarp->Draw();
      can->Print(pdf);
   }
   }

   h->SetTitle("Event time minus trigger time;Time (Bottom) [ps];Time (Top) [ps]");
   can->SetLeftMargin(0.15);
   h->SetStats(0);
   h->Draw();
   can->Print(pdf);
   h->GetXaxis()->SetRangeUser(-1600000,-1100000);
   h->GetYaxis()->SetRangeUser(-1600000,-1100000);
   gPad->Update();
   can->Print(pdf);

   // Prints amplitude plot
   hAmp->Draw();
   can->Print(pdf);
   hAmp->GetXaxis()->SetRangeUser(-1600000,-1100000);
   gPad->Update();
   can->Print(pdf);
   cout<<"Time/Amplitude Correlation: "<<hAmp->GetCorrelationFactor()<<endl;

   // Saves file
   can->Print(pdf+"]");

   // Cleanup
   delete can;

}

