//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "G4SystemOfUnits.hh"

#include "GasModelParameters.hh"

class G4VPhysicsConstructor;
class PhysicsListMessenger;
class DetectorConstruction;
class G4FastSimulationPhysics;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class PhysicsList : public G4VModularPhysicsList {
 public:
  PhysicsList(GasModelParameters*);
  virtual ~PhysicsList();

  void SetCuts();
  void SetCutForGamma(G4double);
  void SetCutForElectron(G4double);
  void SetCutForPositron(G4double);

  void SetLowerProductionLimit(G4double e){lowE=e;};

  void InitializePhysicsList(const G4String& name);
  void AddParametrisation();

 private:
  void AddIonGasModels();
 

  G4double cutForGamma;
  G4double cutForElectron;
  G4double cutForPositron;
  G4double lowE;

  PhysicsListMessenger* pMessenger;
  G4FastSimulationPhysics* fastSimulationPhysics;
  GasModelParameters* fGasModelParameters;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
