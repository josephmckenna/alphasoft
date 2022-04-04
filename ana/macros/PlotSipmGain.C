void PlotSipmGain(int run_num)
{
   TString filename="/afs/cern.ch/work/g/gwsmith/private/root_output_files/";
   filename+="output0";
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
   
   std::vector<double> gx = {};
   std::vector<double> ugx = {};
   std::vector<double> p0 = {};
   std::vector<double> up0 = {};
   std::vector<double> p1 = {};
   std::vector<double> up1 = {};
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
      g->SetTitle(Form("Amplitude vs TPC zed, channel %d;Zed from TPC (m);Ln amplitude from fit",i));
      g->Fit("pol1");
      TF1 *f2 = (TF1*) (g->GetListOfFunctions()->FindObject("pol1"));
      gx.push_back(i);
      ugx.push_back(0);
      p0.push_back(f2->GetParameter(0));
      up0.push_back(f2->GetParError(0));
      p1.push_back(f2->GetParameter(1));
      up1.push_back(f2->GetParError(1));
      h->Draw();
      if (f2) f2->Draw("same");
      f1->SetLineColor(4);
      f1->Draw("same");
      c->Print(outputname.Data(),"pdf");
      delete c;
      delete g;
   }
   TGraphErrors* gp0 = new TGraphErrors(128,gx.data(),p0.data(),ugx.data(),up0.data());
   gp0->SetTitle("Fit y intercept;Channel Number;Fit y intercept");
   gp0->Draw();
   c1->Print(outputname.Data(),"pdf");
   TGraphErrors* gp1 = new TGraphErrors(128,gx.data(),p1.data(),ugx.data(),up1.data());
   gp1->SetTitle("Fit y intercept;Channel Number;Fit y intercept");
   gp1->Draw();
   c1->Print(outputname.Data(),"pdf");
   c1->Print((outputname+"]").Data());
   delete c1;
   delete f;

}

