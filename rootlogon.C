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

  TString incag("-I"); incag += basedir; incag += "/agana";
  cout<<"Including: "<<incag<<endl;
  gSystem->AddIncludePath(incag.Data());

  TString increco("-I"); increco += basedir; increco += "/recolib/include";
  cout<<"Including: "<<increco<<endl;
  gSystem->AddIncludePath(increco.Data());

  TString inclib("-I"); inclib += basedir; inclib += "/analib";
  cout<<"Including: "<<inclib<<endl;
  gSystem->AddIncludePath(inclib.Data());
  inclib += "/include";
  cout<<"Including: "<<inclib<<endl;
  gSystem->AddIncludePath(inclib.Data());

  gSystem->Load("libMinuit2");
  gSystem->Load("libGeom");

  // TString libreco(basedir); libreco += "/recolib/libagtpc";
  // gSystem->Load(libreco.Data());

  // TString libana(basedir); libana += "/analib/libanalib";
  // gSystem->Load(libana.Data());

  TString libname("libagtpc.so");
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  int s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

  libname="libalpha2.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;
 
  libname="libanalib.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

  gROOT->ProcessLine("#include \"analib/RootUtils/RootUtils.h\"");
  
  gStyle->SetOptStat(1011111);
  //gStyle->SetPalette(kRainBow);
  //gStyle->SetPalette(kAurora);
  gStyle->SetPalette(kCool);
  //gStyle->SetPalette(kNeon);
  //gStyle->SetPalette(1);
  //gStyle->SetPalette(kRedBlue);
}
