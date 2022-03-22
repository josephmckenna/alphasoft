void PlotZed(const char* filename)
{
  TFile *f = new TFile(filename);
  gDirectory->cd("bsc_histo_module");
  gDirectory->cd("bar_histos");
  TString cname = "cTOF";
  TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1200, 800);
  c->Divide(2,2);
  std::vector<std::string> histos {"hZed","hZed2d","hTwoBarDZed","hNBarDZed"};
  int i=0;
  for(auto it = histos.begin(); it!=histos.end(); ++it)
    {
      TH1D* h = (TH1D*) gDirectory->Get(it->c_str());
      c->cd(++i);
      h->Draw("colz");
    }
  return;
}

