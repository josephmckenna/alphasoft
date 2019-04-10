#ifndef GasModelParametersMessenger_h
#define GasModelParametersMessenger_h 1

#include "G4SystemOfUnits.hh"
#include "G4UImessenger.hh"

class G4UIcommand;
class GasModelParameters;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithoutParameter;
class G4UIcmdWithADouble;
class G4UIcmdWithAnInteger;
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
/*! \class GasModelParametersMessenger*/
/*! class derived from G4UImessenger*/
/*! List of available commands*/

class GasModelParametersMessenger : public G4UImessenger {
 public:
  GasModelParametersMessenger(GasModelParameters*);
  ~GasModelParametersMessenger();

  void SetNewValue(G4UIcommand*, G4String);

 private:

  void AddParticleHeedInterfaceCommand(G4String newValues);
  void AddParticleHeedOnlyCommand(G4String newValues);
  void ConvertParameters(G4String newValues);

  GasModelParameters* fGasModelParameters;
  G4UIdirectory* GasModelParametersDir;
  G4UIdirectory* HeedDir;
  G4UIdirectory* HeedOnlyDir;
  G4UIdirectory* HeedInterfaceDir;

  G4UIcommand* addParticleHeedInterfaceCmd;
  G4UIcommand* addParticleHeedOnlyCmd;

  G4UIcmdWithAString* gasFileCmd;
  G4UIcmdWithAString* ionMobFileCmd;

  G4UIcmdWithABool* driftElectronsCmd;
  G4UIcmdWithABool* createAvalCmd;

  G4UIcmdWithABool* driftRKFCmd;
  G4UIcmdWithABool* trackMicroCmd;

  G4UIcmdWithABool* genSignalsCmd;
  G4UIcmdWithADouble* noiseAnodeCmd;
  G4UIcmdWithADouble* noisePadCmd;

  G4UIcmdWithABool* visualizeChamberCmd;
  G4UIcmdWithABool* visualizeFieldCmd;

  G4UIcmdWithADouble* voltageAnodeCmd;
  G4UIcmdWithADouble* voltageCathodeCmd;
  G4UIcmdWithADouble* voltageFieldCmd;

  G4String fParticleName;
  G4double fEmin;
  G4double fEmax;  
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
