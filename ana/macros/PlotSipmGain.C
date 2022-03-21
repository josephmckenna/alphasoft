void PlotSipmGain(int run_num)
{
   TString filename=getenv("AGRELEASE");
   filename+="/root_output_files/output0";
   filename+=std::to_string(run_num);
   filename+=".root";
   TString outputname = "SipmGain_run" + std::to_string(run_num) + ".pdf";
   TString cname = "BV_canvas_"+std::to_string(run_num);
   gROOT->SetBatch(kTRUE);
   TFile *f = new TFile(filename.Data());
   gDirectory->cd("bsc_histo_module");
   gDirectory->cd("tpc_matching_histos");
   gDirectory->cd("amp_vs_zed");

   TCanvas* c1 = new TCanvas("cname","cname", 1200, 800);
   c1->Print((outputname+"[").Data());
   for (int i=0;i<128;i++) {
      TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1200, 800);
      TH2D* h = (TH2D*) gDirectory->Get(Form("hAmpVsZed%d",i));
      if (!h) continue;
      h->Draw("colz");
      c->Print(outputname.Data(),"pdf");
      delete c;
   }

   std::vector<double> x0 = {};
   std::vector<double> y0 = {};
   int count0 = 0;
   for (int i=0;i<128;i++) {
      TH2D* h = (TH2D*) gDirectory->Get(Form("hLnAmpVsZed%d",i));
      if (!h) continue;
      for (int ix=0;ix<200;ix++) {
         for (int iy=0;iy<200;iy++) {
            for (int n=0;n<h->GetBinContent(ix,iy);n++) {
               x0.push_back(((TAxis*)h->GetXaxis())->GetBinCenter(ix));
               y0.push_back(((TAxis*)h->GetYaxis())->GetBinCenter(iy));
               count0++;
            }
         }
      }
   }
   TGraph* g0 = new TGraph(count0,&x0[0],&y0[0]);
   g0->Fit("pol1");
   TF1 *f1 = (TF1*) (g0->GetListOfFunctions()->FindObject("pol1"));
   
   for (int i=0;i<128;i++) {
      TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1200, 800);
      TH2D* h = (TH2D*) gDirectory->Get(Form("hLnAmpVsZed%d",i));
      if (!h) continue;
      std::vector<double> x = {};
      std::vector<double> y = {};
      int count = 0;
      for (int ix=0;ix<200;ix++) {
         for (int iy=0;iy<200;iy++) {
            for (int n=0;n<h->GetBinContent(ix,iy);n++) {
               x.push_back(((TAxis*)h->GetXaxis())->GetBinCenter(ix));
               y.push_back(((TAxis*)h->GetYaxis())->GetBinCenter(iy));
               count++;
            }
         }
      }
      TGraph* g = new TGraph(count,&x[0],&y[0]);
      g->SetTitle(Form("Amplitude vs TPC zed, channel %d;Zed from TPC (m);Natural logarithm of amplitude from fit",i));
      g->Fit("pol1");
      TF1 *f2 = (TF1*) (g->GetListOfFunctions()->FindObject("pol1"));
      h->Draw();
      if (f2) f2->Draw("same");
      //f1->Draw("same");
      //f1->SetLineColor(4);
      c->Print(outputname.Data(),"pdf");
      delete c;
      delete g;
   }
   c1->Print((outputname+"]").Data());
   delete c1;
   delete f;

}

