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

  TString inclib("-I"); inclib += basedir; inclib += "/analib/include";
  cout<<"Including: "<<inclib<<endl;
  gSystem->AddIncludePath(inclib.Data());

  TString inca2lib("-I"); inca2lib += basedir; inca2lib += "/a2lib/include";
  cout<<"Including: "<<inca2lib<<endl;
  gSystem->AddIncludePath(inca2lib.Data());
  
  TString incalpha2("-I"); incalpha2 += basedir; incalpha2 += "/alpha2/include";
  cout<<"Including: "<<incalpha2<<endl;
  gSystem->AddIncludePath(incalpha2.Data());

  TString incroot("-I"); incroot += basedir; incroot += "/rootUtils/include";
  cout<<"Including: "<<incroot<<endl;
  gSystem->AddIncludePath(incroot.Data());
  
  
  gSystem->Load("libMinuit2");
  gSystem->Load("libGeom");

  // TString libreco(basedir); libreco += "/recolib/libagtpc";
  // gSystem->Load(libreco.Data());

  // TString libana(basedir); libana += "/analib/libanalib";
  // gSystem->Load(libana.Data());

  TString libname("libAGTPC.so");
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  int s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

<<<<<<< HEAD
=======
  libname="libagana.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

>>>>>>> A2
  libname="libalpha2.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;
 
<<<<<<< HEAD
  libname="libanalib.so";
=======
  libname="librootUtils.so";
>>>>>>> A2
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;
<<<<<<< HEAD

  gROOT->ProcessLine("#include \"analib/RootUtils/RootUtils.h\"");
=======

  gROOT->ProcessLine("#include \"rootUtils/include/A2RootUtils.h\"");

  gROOT->ProcessLine("#include \"rootUtils/include/RootUtils.h\"");
>>>>>>> A2
  
  gStyle->SetOptStat(1011111);
  //gStyle->SetPalette(kRainBow);
  //gStyle->SetPalette(kAurora);
  gStyle->SetPalette(kCool);
  //gStyle->SetPalette(kNeon);
  //gStyle->SetPalette(1);
  //gStyle->SetPalette(kRedBlue);
}
