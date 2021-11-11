void PlotTOF(const char* filename)
{
  TFile *f = new TFile(filename);
  gDirectory->cd("bsc_histo_module");
  gDirectory->cd("bar_histos");
  TString cname = "cTOF";
  TCanvas* c = new TCanvas(cname.Data(),cname.Data(), 1200, 800);
  c->Divide(4,2);
  std::vector<std::string> histos {"hTwoBarTOF","hTwoBarTOF2d","hNBarTOF","hNBarTOF2d","hTwoBarExpectedTOFvsTOF","hNBarExpectedTOFvsTOF","hTwoBarExpectedTOFminusTOF","hNBarExpectedTOFminusTOF"};
  int i=0;
  for(auto it = histos.begin(); it!=histos.end(); ++it)
    {
      TH1D* h = (TH1D*) gDirectory->Get(it->c_str());
      c->cd(++i);
      h->Draw("colz");
    }
  return;
}

