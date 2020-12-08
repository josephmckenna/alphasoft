void ReadSettings()
{
  TFile* ff = (TFile*) gROOT->GetListOfFiles()->First();
  TObjString* str = (TObjString*) ff->Get("ana_settings");
  cout<<str->GetString()<<endl;
}
