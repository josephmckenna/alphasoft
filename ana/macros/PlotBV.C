void PlotBV(int run_num)
{
   TString filename=getenv("AGRELEASE");
   filename+="/root_output_files/output0";
   filename+=std::to_string(run_num);
   filename+=".root";
   TString outputname = "BV_run" + std::to_string(run_num) + ".pdf";
   TString cname = "BV_canvas_"+std::to_string(run_num);
   gROOT->SetBatch(kTRUE);
   TFile *f = new TFile(filename.Data());
   gDirectory->cd("bsc_histo_module");

   std::vector<std::vector<TString>> histos = {
      { "adc_histos/hAdcOccupancy","adc_histos/hAdcCorrelation","adc_histos/hAdcMultiplicity","adc_histos/hAdcFitAmp"},
      { "tdc_histos/hTdcOccupancy","tdc_histos/hAdcTdcOccupancy","tdc_histos/hTdcCorrelation","tdc_histos/hTdcMultiplicity"},
      { "tdc_histos/hTdcSingleChannelMultiplicity","tdc_histos/hTdcSingleChannelMultiplicity2d","tdc_histos/hTdcSingleChannelHitTime","tdc_histos/hTdcSingleChannelHitTime2d"},
      { "bar_histos/hBarOccupancy","bar_histos/hBarMultiplicity","bar_histos/hBarCorrelation"},
      { "bar_histos/hTopBotDiff","bar_histos/hTopBotDiff2d","bar_histos/hZed","bar_histos/hZed2d"},
      { "bar_histos/hTwoBarTOF","bar_histos/hTwoBarTOF2d","bar_histos/hNBarTOF","bar_histos/hNBarTOF2d"},
      { "bar_histos/hTwoBarDPhi","bar_histos/hTwoBarDZed","bar_histos/hTwoBarDPhiDZed","bar_histos/hNBarDPhi","bar_histos/hNBarDZed","bar_histos/hNBarDPhiDZed"},
      { "bar_histos/hTwoBarExpectedTOF","bar_histos/hTwoBarExpectedTOFvsTOF","bar_histos/hTwoBarExpectedTOFminusTOF","bar_histos/hNBarExpectedTOF","bar_histos/hNBarExpectedTOFvsTOF","bar_histos/hNBarExpectedTOFminusTOF"}
   };
   TCanvas* c1 = new TCanvas("cname","cname", 1200, 800);
   c1->Print((outputname+"[").Data());

   for (auto list:histos) {
      TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1200, 800);
      int s = list.size();
      if (s>1 and s<=4) c->Divide(2,2);
      if (s>4 and s<=6) c->Divide(3,2);
      int i = 0;
      for (auto name:list) {
         TH1D* h = (TH1D*) gDirectory->Get(name.Data());
         c->cd(++i);
         if (!h) continue;
         h->Draw("colz");
      }
      c->Print(outputname.Data(),"pdf");
      delete c;
   }
   TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1200, 800);
   TH1D* h = (TH1D*) gDirectory->Get("bar_histos/hTwoBarDPhiDZed");
   h->Draw("");
   c->Print(outputname.Data(),"pdf");
   h = (TH1D*) gDirectory->Get("bar_histos/hNBarDPhiDZed");
   h->Draw("");
   c->Print(outputname.Data(),"pdf");
   delete c;
   c1->Print((outputname+"]").Data());
   delete c1;
   delete f;

}

