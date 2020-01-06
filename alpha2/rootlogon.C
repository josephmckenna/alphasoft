{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;
  TString basedir(getenv("AGRELEASE"));
  if (basedir.Sizeof()<3)
  {
     std::cout <<"$AGRELEASE not set... Please source agconfig.sh"<<std::endl;
     exit(1);
  }


  TString incana("-I"); incana += basedir; incana += "/ana/include";
  cout<<"Including: "<<incana<<endl;
  gSystem->AddIncludePath(incana.Data());

  TString increco("-I"); increco += basedir; increco += "/recolib/include";
  cout<<"Including: "<<increco<<endl;
  gSystem->AddIncludePath(increco.Data());

  TString inclib("-I"); inclib += basedir; inclib += "/analib/include";
  cout<<"Including: "<<inclib<<endl;
  gSystem->AddIncludePath(inclib.Data());

  TString inca2lib("-I"); inca2lib += basedir; inca2lib += "/a2lib/include";
  cout<<"Including: "<<inca2lib<<endl;
  gSystem->AddIncludePath(inca2lib.Data());

  TString inca2("-I"); inca2 += basedir; inca2 += "/alpha2/include";
  cout<<"Including: "<<inca2<<endl;
  gSystem->AddIncludePath(inca2.Data());



  gSystem->Load("libMinuit2");
  gSystem->Load("libGeom");

  TString a2lib(basedir); a2lib += "/a2lib/libalpha2";
  gSystem->Load(a2lib);


  TString libreco(basedir); libreco += "/recolib/libAGTPC";
  gSystem->Load(libreco.Data());

  TString libana(basedir); libana += "/analib/libagana";
  gSystem->Load(libana.Data());

  gROOT->ProcessLine("#include \"RootUtils/RootUtils.h\"");
  
  gStyle->SetOptStat(1011111);
  //gStyle->SetPalette(kRainBow);
  //gStyle->SetPalette(kAurora);
  gStyle->SetPalette(kCool);
  //gStyle->SetPalette(kNeon);
  //gStyle->SetPalette(1);
  //gStyle->SetPalette(kRedBlue);
}
