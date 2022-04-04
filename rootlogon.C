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

  #include "BuildConfig.h"
  

  TString incana("-I"); incana += basedir; incana += "/bin/include";
  cout<<"Including: "<<incana<<endl;
  gSystem->AddIncludePath(incana.Data());
  

   std::vector<TString> LibsToLoad = {"libMinuit2","libGeom"};

#if BUILD_AG
   LibsToLoad.emplace_back("libagtpc");
   LibsToLoad.emplace_back("libaglib");
#endif
#if BUILD_A2
  LibsToLoad.emplace_back("libalpha2");
#endif 
  LibsToLoad.emplace_back("libanalib");
  LibsToLoad.emplace_back("librootUtils");

#if BUILD_AG_SIM
   LibsToLoad.emplace_back("libG4out");
#endif

   for ( TString& libname: LibsToLoad)
   {
      gSystem->FindDynamicLibrary(libname);
      std::cout<<"Loading: "<<libname;
      int s = gSystem->Load( libname );
      if(s==0)
         std::cout<<"... ok"<<std::endl;
      else
         std::cout<<"...ERROR!"<<std::endl;
   }

  gStyle->SetOptStat(1011111);
  //gStyle->SetPalette(kRainBow);
  //gStyle->SetPalette(kAurora);
  gStyle->SetPalette(kCool);
  //gStyle->SetPalette(kNeon);
  //gStyle->SetPalette(1);
  //gStyle->SetPalette(kRedBlue);
}
