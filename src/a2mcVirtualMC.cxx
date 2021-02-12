///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcVirtualMC.h"

ClassImp(a2mcVirtualMC)

//_____________________________________________________________________________
a2mcVirtualMC::a2mcVirtualMC(const char *name, const char *title, Int_t run_number, std::string& run_time, Int_t run_seed)
    : TVirtualMCApplication(name,title),
    runNumber(0),
    runSeed(0),
    nEvents(0),
    fStack(0),
    fDetConstruction(0),
    fSilSD("SilSD"),
    fVerbose(0),    
    fPrimaryGenerator(0),
    fMagField(0),
    fRootManager(0)
{

    runNumber   = run_number;
    runTime     = run_time;
    runSeed     = run_seed;
    // Create a user stack
    fStack = new a2mcStack(a2mcConf.GetTracksLim());
    verbose = a2mcConf.GetVerbose();
    // Create detector construction
    fDetConstruction = new a2mcApparatus(runNumber);
    
    if(a2mcConf.GetMagField()==1) {
        Double_t rMax = 50.;
        Double_t zMin = -444.;
        Double_t zMax = +444.;
        Double_t BzMax = 10.; // B field along Z (in kG)
        fMagField = new a2mcFieldConstant(0., 0., BzMax, rMax, zMin, zMax);
    }
    if(a2mcConf.GetMagField()==2) fMagField = new a2mcFieldFromMap("input/berkeley_and_octupole_and_mirrors_extended_shifted-24mm_mm.mag");

        
    FileMode fileMode = kWrite;
    fRootManager = new a2mcRootManager(runNumber, runTime, "a2MC", fileMode);

    fPrimary = new a2mcPrimary();
    fRootManager->Register("Primary", "a2mcPrimary", &fPrimary);   

    // Create a primary generator
    fPrimaryGenerator = new a2mcGenerator(fStack,fDetConstruction); 
}

//_____________________________________________________________________________
a2mcVirtualMC::a2mcVirtualMC()
: TVirtualMCApplication(),
    nEvents(0),     
    fStack(0),
    fDetConstruction(0),
    fSilSD(),
    fVerbose(0),
    fPrimaryGenerator(0),
    fMagField(0),
    fRootManager()
{    
    /// Default constructor
}

//_____________________________________________________________________________
a2mcVirtualMC::~a2mcVirtualMC() 
{
    /// Destructor  
    delete fStack;
    delete fDetConstruction;
    delete fPrimaryGenerator;
    delete fRootManager;
    delete gMC;
}

//_____________________________________________________________________________
void a2mcVirtualMC::RegisterStack()
{
    /// Register stack in the Root manager
    fRootManager->Register("stack", "a2mcStack", &fStack);   
}  

//_____________________________________________________________________________
void a2mcVirtualMC::InitMC(const char* setup)
{    
    if ( TString(setup) != "" ) {
         gROOT->LoadMacro(setup);
        gInterpreter->ProcessLine("Config()");
        if (!gMC) Fatal("InitMC", "Processing Config() has failed. (No MC is instantiated.)");
    }

   // This need to be uncommented to allow the step limits set in defining
   // the geometry media (stemax) in a2mcApparatus.cxx
   Bool_t isUserParameters = true;
   gMC->SetUserParameters(isUserParameters);
    //gMC->SetProcess("MULS" ,0); // Turn OFF multiple scattering
    //gMC->SetCut("CUTHAD",1.e-6); //
        gMC->SetCut("CUTELE",1.e-2); // GeV
        gMC->SetCut("CUTGAM",1.e-2); // GeV
    ///< These cuts could be customized according to gen_type and gen_mode
//    if(a2mcConf.GetGen_mode()==10||a2mcConf.GetGen_mode()==11) { // positrons or positrons-like pbars
//        gMC->SetCut("CUTELE",1.e-5); // GeV
//        gMC->SetCut("CUTGAM",1.e-5); // GeV        
//    } else { // all the others
//        gMC->SetCut("CUTELE",1.e-2); // GeV
//        gMC->SetCut("CUTGAM",1.e-2); // GeV
//    }

    gMC->SetStack(fStack);
    if(a2mcConf.GetMagField()!=0) gMC->SetMagField(fMagField);
    gMC->Init();
    gMC->BuildPhysics(); 
    Int_t store_tracks = fDetConstruction->GetStoreTracks();
    if(fStack&&store_tracks==1) fStack->RegisterMCTrack();
}

//_____________________________________________________________________________
void a2mcVirtualMC::RunMC(Int_t nofEvents)
{    
    /// Run MC.
    /// \param nofEvents Number of events to be processed

    if(verbose) fVerbose.RunMC(nofEvents);

    gMC->ProcessRun(nofEvents);

    FinishRun();

}

//_____________________________________________________________________________
void a2mcVirtualMC::FinishRun()
{    
    /// Finish MC run.

    if(verbose) fVerbose.FinishRun();

    fRootManager->WriteAll();
    fPrimaryGenerator->EquivalentTimeMin();
    WriteLog();
}

//_____________________________________________________________________________
void a2mcVirtualMC::ConstructGeometry()
{    
    /// Construct geometry using detector contruction class.
    /// The detector contruction class is using TGeo functions or

    if(verbose) fVerbose.ConstructGeometry();

    // Cannot use Root geometry if not supported with 
    // selected MC
    if (  !gMC->IsRootGeometrySupported() ) {
        cerr << "Selected MC does not support TGeo geometry"<< endl;
        cerr << "Exiting program"<< endl;
        exit(1);
    } 

    cout << "Geometry will be defined via TGeo" << endl;
    fDetConstruction->ConstructMaterials();
    fDetConstruction->ConstructGeometry();

} 

//_____________________________________________________________________________
void a2mcVirtualMC::InitGeometry()
{    
    /// Initialize geometry
    if(verbose) fVerbose.InitGeometry();

//    // Adding the SD Class to the RootManager 
    if(a2mcConf.GetSilDet()) fSilSD.Initialize();


    //  fDetConstruction->SetCuts();
}

//_____________________________________________________________________________
void a2mcVirtualMC::GeneratePrimaries()
{    
    /// Fill the user stack (derived from TVirtualMCStack) with primary particles.

    fPrimaryGenerator->Generate();
    fPrimary->SetPdgCode(fPrimaryGenerator->GetPdgCode());
    fPrimary->SetVox(fPrimaryGenerator->GetVx());
    fPrimary->SetVoy(fPrimaryGenerator->GetVy());
    fPrimary->SetVoz(fPrimaryGenerator->GetVz());
    fPrimary->SetPox(fPrimaryGenerator->GetPx());
    fPrimary->SetPoy(fPrimaryGenerator->GetPy());
    fPrimary->SetPoz(fPrimaryGenerator->GetPz());
    fPrimary->SetEo(fPrimaryGenerator->GetTotE());
    fPrimary->SetGenMode(a2mcConf.GetGenMode());
}

//_____________________________________________________________________________
void a2mcVirtualMC::BeginEvent()
{    
    /// User actions at beginning of event.
    /// Nothing to be done this example

    if(verbose) fVerbose.BeginEvent();

    nEvents++;

    if(a2mcConf.GetSilDet())  fSilSD.BeginOfEvent(); 
}

//_____________________________________________________________________________
void a2mcVirtualMC::BeginPrimary()
{    
    /// User actions at beginning of a primary track.
    if(verbose) fVerbose.BeginPrimary();
}

//_____________________________________________________________________________
void a2mcVirtualMC::PreTrack()
{    
    /// User actions at beginning of each track.
    if(verbose) fVerbose.PreTrack();
}

//_____________________________________________________________________________
void a2mcVirtualMC::Stepping()
{    

//
// Called at every step during transport
//
    if(verbose) fVerbose.Stepping();
    if(a2mcConf.GetSilDet()) fSilSD.ProcessHits();

}

//_____________________________________________________________________________
void a2mcVirtualMC::PostTrack()
{    
    /// User actions at each step.
    if(verbose) fVerbose.PostTrack();
}

//_____________________________________________________________________________
void a2mcVirtualMC::FinishPrimary()
{    
    /// User actions after finishing of a primary track.
    if(verbose) fVerbose.FinishPrimary();
}

//_____________________________________________________________________________
void a2mcVirtualMC::FinishEvent()
{    
    /// User actions after finishing of an event

    if(verbose) fVerbose.FinishEvent();
    Int_t iev = gMC->CurrentEvent();
    if(iev%1000==0) {printf("Processing event %d \n",iev);}

    ///< Fill the information about the Primary Decay Vertex
    ///< Registering the "origin vertex" of a primary daughter as the "decay/annihilaiton" vertex of the primary
    for(UInt_t i=0; i<fStack->GetNtrack(); i++) {
        if(fStack->GetParticle(i)->GetMother(0)!=0) continue; ///< Not a daughter from the primary
        fPrimary->SetVdx(fStack->GetParticle(i)->Vx());
        fPrimary->SetVdy(fStack->GetParticle(i)->Vy());
        fPrimary->SetVdz(fStack->GetParticle(i)->Vz());
        break;
    }
    ///< Storing the hits and the DIGI 
    if(a2mcConf.GetSilDet()) fSilSD.Digitalize();
    fRootManager->Fill();
    if(a2mcConf.GetSilDet()) fSilSD.EndOfEvent();

    if(a2mcConf.GetVerbose()) {
        ostringstream titolo;
        titolo << "Dumping event " << iev;
        mess.TitleFrame(titolo.str());
        fPrimaryGenerator->DumpGenInfo();
    }
    ///< Resent and prepare for the next event
    fPrimary->Reset();
    fStack->Reset();
}

//_____________________________________________________________________________
void a2mcVirtualMC::AddParticles()
{    
  
  fVerbose.AddParticles();
  
/// Example of user defined particle with user defined decay mode
// Define particle
//  gMC->DefineParticle(9999, "XYZ", kPTUndefined,
//                      0.938783 , 0.000000000001, 0.0 , 
//                      "Atom", 0.0, 0, 
//                      1, 0, 0, 
//                      0, 0, 0, 
//                      1, kTRUE, kFALSE, "", 0, 0.0, 0.0); 
  
}

void a2mcVirtualMC::WriteLog() {

    ///< SAVING INFO INTO THE LOG FILE
    ostringstream sf;
    sf << "output/a2mc_" << runNumber << ".log";
    ostringstream ss; 

    ss.clear(); ss.str("");
    ss << "echo '---> Log file for run " << runNumber << " <---' >>" << sf.str();;
    gSystem->Exec(ss.str().c_str());

    ss.clear(); ss.str("");
    ss << "echo '_____________________________________________\n' >>" << sf.str();;
    gSystem->Exec(ss.str().c_str());

    ss.clear(); ss.str("");
    ss << "echo 'Run started: " << runTime << " ' >> " << sf.str();
    gSystem->Exec(ss.str().c_str());
    
    ss.clear(); ss.str("");
    ss << "echo 'Run seed   : " << runSeed << " ' >> " << sf.str();
    gSystem->Exec(ss.str().c_str());
    
    ss.clear(); ss.str("");
    ss << "echo '########### HERE BELOW: a2MC.ini/a2mcApparatus.cxx/a2mcGenerator.cxx/a2mcVirtualMC.cxx ############' >> " << sf.str();; 
    gSystem->Exec(ss.str().c_str());        

    ss.clear(); ss.str("");
    ss << "cat "<<INI_INSTALL_PATH<<"/a2MC.ini >> " << sf.str();; 
    gSystem->Exec(ss.str().c_str());        

    ss.clear(); ss.str("");
    ss << "cat "<<A2_MC_SRC_PATH<<"/src/a2mcApparatus.cxx >> " << sf.str();; 
    gSystem->Exec(ss.str().c_str());        

    ss.clear(); ss.str("");
    ss << "cat "<<A2_MC_SRC_PATH<<"/src/a2mcGenerator.cxx >> " << sf.str();; 
    gSystem->Exec(ss.str().c_str());        

    ss.clear(); ss.str("");
    ss << "cat "<<A2_MC_SRC_PATH<<"/src/a2mcVirtualMC.cxx >> " << sf.str();; 
    gSystem->Exec(ss.str().c_str());        

    cout << "Writing log into file " << sf.str() << endl;
}
