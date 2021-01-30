{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;
  TString basedir(getenv("AGRELEASE"));
  if (basedir.Sizeof()<3)
  {
     std::cout <<"$AGRELEASE not set... Please source agconfig.sh"<<std::endl;
     exit(1);
  }


  TString incana("-I"); incana += basedir; incana += "/bin/include";
  cout<<"Including: "<<incana<<endl;
  gSystem->AddIncludePath(incana.Data());
  #include "BuildConfig.h"
  
  
  gSystem->Load("libMinuit2");
  gSystem->Load("libGeom");



  TString libname("libagtpc.so");
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  int s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

  libname="libanalib.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

  libname="libalpha2.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;
 
  libname="librootUtils.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;


  gStyle->SetOptStat(1011111);
  //gStyle->SetPalette(kRainBow);
  //gStyle->SetPalette(kAurora);
  gStyle->SetPalette(kCool);
  //gStyle->SetPalette(kNeon);
  //gStyle->SetPalette(1);
  //gStyle->SetPalette(kRedBlue);
}
