{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;
  gSystem->AddIncludePath("-I../reco");
  //gSystem->AddLinkedLibs("..");
  //  gSystem->AddDynamicPath("..");
  gSystem->Load("libMinuit");
  gSystem->Load("libGeom");
  gSystem->Load("libAGTPC");
  //  cout<<gSystem->Now().AsString()<<endl;
}


