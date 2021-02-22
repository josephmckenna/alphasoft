///< ##############################################
///< Based on the VMC framework by I. Hřivnáčová 
///< https://vmc-project.github.io/
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "TG4RunConfiguration.h"
#include "TGeant4.h"
#include "TThread.h"

#include "a2mcVirtualMC.h"
#include "a2mcUtility.h"

int main(int argc, char** argv) {
    TThread::Initialize();
    gInterpreter->SetProcessLineLock(false);
  
    Utility utils(argc, argv);
    int nEvents = utils.GetNEvents();
    int runNumber = utils.GetRun();
    string runTime = utils.GetRunTime();
  
// Check if the "root" and "output" folder already exist otherwise create them
    if(!utils.checkDir("./","root"))  {
        cout << "Creating root subdirectory " << endl;
        gSystem->Exec("mkdir root");
    }
    if(!utils.checkDir("./","output"))  {
        cout << "Creating output subdirectory " << endl;
        gSystem->Exec("mkdir output");
    }
  
// seed generator
    TDatime dt;
    UInt_t curtime=dt.Get();
    UInt_t procid=gSystem->GetPid();
    UInt_t runSeed=curtime-procid;
    gRandom->SetSeed(runSeed);
//    gRandom->SetSeed(0);
    ///< Creating the Virtual MC object
    a2mcVirtualMC* virtualMC = new a2mcVirtualMC("a2mcVirtualMC", "The Alpha2 MC", runNumber, runTime, runSeed);
  
    ostringstream sgeo;
    sgeo << "geomRootToGeant4";
    TG4RunConfiguration* runConfiguration = new TG4RunConfiguration(sgeo.str().c_str(), "FTFP_BERT_TRV","stepLimiter+specialCuts+specialControls");
    TGeant4* geant4 = new TGeant4("TGeant4", "The Geant4 Monte Carlo", runConfiguration, argc, argv);
    geant4->ProcessGeantMacro("g4config.in");
  
    virtualMC->InitMC("");
    virtualMC->RunMC(nEvents);
  
    delete virtualMC;
    return 0;
}
