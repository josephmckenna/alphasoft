{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;
  if (!getenv("AGRELEASE"))
  {
     std::cout <<"$AGRELEASE not set... Please source agconfig.sh"<<std::endl;
     exit(1);
  }
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
 
  libname="libGEM.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;
  else cout<<" Not loaded"<<endl;
  
  libname="libaged.so";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

#ifdef BUILD_AG_SIM
  libname="libG4out.so";
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
