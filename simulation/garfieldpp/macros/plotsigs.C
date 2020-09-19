{
  TH1D* h = (TH1D*) _file0->Get("ha029");
  h->GetXaxis()->SetRangeUser(0.,500.);
  h->Draw();

  TH1D* h = (TH1D*) _file0->Get("hpad00579sec03row018");
  h->GetXaxis()->SetRangeUser(0.,500.);
  h->Draw();
 
}
