{
///< Checking if the environment is set correctly
    TString basedir(getenv("AGRELEASE"));
    if (basedir.Sizeof()<3) {
        std::cout <<"$AGRELEASE not set... Please source agconfig.sh"<<std::endl;
        exit(1);
    }

///< Loading the a2MC library
    TString libname="../lib/libvmc_a2MC.dylib";
    libname=gSystem->FindDynamicLibrary(libname);
    cout<<"Loading: "<<libname;
    s=gSystem->Load( libname );
    if(s==0) cout<<" ... ok"<<endl;

    gStyle->SetOptStat(1001111);
    gStyle->SetPalette(kRainBow);
    gStyle->SetPalette(kCool);
}
