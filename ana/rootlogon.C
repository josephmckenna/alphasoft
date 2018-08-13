{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;
  gSystem->AddIncludePath("-I../reco");
  gSystem->AddIncludePath("-I../analib");
  //gSystem->AddLinkedLibs("..");
  //  gSystem->AddDynamicPath("..");
  gSystem->Load("libMinuit");
  gSystem->Load("libGeom");
  gSystem->Load("libAGTPC");
  gSystem->Load("libagana");
  //  cout<<gSystem->Now().AsString()<<endl;
}


