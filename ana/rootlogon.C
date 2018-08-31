{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;
  gSystem->AddIncludePath("-I../reco");
  gSystem->AddIncludePath("-I../analib");
  //gSystem->AddLinkedLibs("..");
  //  gSystem->AddDynamicPath("..");
  gSystem->Load("libMinuit");
  gSystem->Load("libGeom");
  gSystem->Load("../reco/libAGTPC");
  gSystem->Load("../analib/libagana");
  //  cout<<gSystem->Now().AsString()<<endl;
  gROOT->ProcessLine("#include \"RootUtils/RootUtils.h\"");
}


