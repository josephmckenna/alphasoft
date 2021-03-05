{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;

  gSystem->AddIncludePath("-I../include");
    
  gSystem->Load("libMinuit2");
  gSystem->Load("libGeom");

//  TString libname("/home/acapra/packages/vmc/install/lib64/libVMCLibrary.so");
//  libname=gSystem->FindDynamicLibrary(libname);
//  cout<<"Loading: "<<libname;
//  int s=gSystem->Load( libname );
//  if(s==0) cout<<"... ok"<<endl;

  TString arch=gSystem->GetBuildArch();
  TString libname;
  if( arch.Contains("linux") )
    libname="../lib/libvmc_a2MC.so";
  else
    libname="../lib/libvmc_a2MC.dylib";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

 TString basedir(getenv("AGRELEASE"));
  if (basedir.Sizeof()<3)
  {
     std::cout <<"$AGRELEASE not set... Please source agconfig.sh"<<std::endl;
     exit(1);
  }

  TString incana("-I"); incana += basedir; incana += "/bin/include";
  cout<<"Including: "<<incana<<endl;
  gSystem->AddIncludePath(incana.Data());
  
  libname="libalpha2.dylib";
  libname=gSystem->FindDynamicLibrary(libname);
  cout<<"Loading: "<<libname;
  s=gSystem->Load( libname );
  if(s==0) cout<<"... ok"<<endl;

  gStyle->SetOptStat(1011111);
  gStyle->SetPalette(kRainBow);
  gStyle->SetPalette(kCool);
}
