{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;
  if (!getenv("AGRELEASE"))
  {
     std::cout <<"AGRELEASE not set... Please source agconfig.sh"<<std::endl;
     exit(1);
  }
  TString basedir(getenv("AGRELEASE"));
  if (basedir.Sizeof()<3)
  {
     std::cout <<"AGRELEASE not set... Please source agconfig.sh"<<std::endl;
     exit(1);
  }


  TString incana("-I"); incana += basedir; incana += "/bin/include";
  cout<<"Including: "<<incana<<endl;
  gSystem->AddIncludePath(incana.Data());
  //gInterpreter->ProcessLine("#include \"BuildConfig.h\"");
  #include "bin/include/BuildConfig.h"
  
  
  gSystem->Load("libMinuit2");
  gSystem->Load("libGeom");



  TString libname;
  int s=-1;
#ifdef BUILD_AG
  libname="libagtpc";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;
#endif

  libname="libanalib";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

#ifdef BUILD_A2
  libname="libalpha2";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;
#endif 
 
  libname="librootUtils";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

#ifdef BUILD_AG_SIM
  libname="libG4out";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

  libname="libG4dict";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;
#endif

  gInterpreter->ProcessLine("#include \"RootUtils.h\"");
  gStyle->SetOptStat(1011111);
  //gStyle->SetPalette(kRainBow);
  //gStyle->SetPalette(kAurora);
  gStyle->SetPalette(kCool);
  //gStyle->SetPalette(kNeon);
  //gStyle->SetPalette(1);
  //gStyle->SetPalette(kRedBlue);
}
