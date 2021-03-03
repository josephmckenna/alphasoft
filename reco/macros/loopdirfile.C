/// Example of script to loop on all the objects of a ROOT file
/// and print on PDF all TH1 derived objects
/// Checks if they are unique
///
/// \author Rene Brun
/// \modified A.Capra

void loopdirfile() 
{
  TFile* f1= (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(f1->GetName());
  cout<<fname<<" FOUND"<<endl;
  TString cname=fname(0,fname.Length()-5);
  cname+=".pdf";
  cout<<cname<<endl;

  std::list<string> hlist;
  //  TString savFolder=MakeAutoPlotsFolder("time");
  TCanvas c1;
  c1.Print(cname+"[");
  for(auto k : *f1->GetListOfKeys()) {
    TKey *key = static_cast<TKey*>(k);
    TClass *cl = gROOT->GetClass(key->GetClassName());
    if (!cl->InheritsFrom("TH1")) continue;
    TH1 *h = key->ReadObject<TH1>();
    string hname(h->GetName());
    if( std::find(hlist.begin(),hlist.end(),hname) != hlist.end() )
      continue;
    cout<<hname<<endl;
    hlist.push_back(hname);
    //   c1.cd();
    h->Draw();
    // TString hname=TString::Format("%s%s.pdf",savFolder.Data(),h->GetName());
    // c1.Print(hname);
    c1.Print(cname);
  }
  c1.Print(cname+"]");
}
