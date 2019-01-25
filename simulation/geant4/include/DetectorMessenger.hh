#ifndef DetectorMessenger_h
#define DetectorMessenger_h 1

#include "G4SystemOfUnits.hh"
#include "G4UImessenger.hh"

class DetectorConstruction;
class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithADouble;
class G4UIcmdWithoutParameter;
class G4UIcmdWith3Vector;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorMessenger : public G4UImessenger 
{
public:
  DetectorMessenger(DetectorConstruction*);
  ~DetectorMessenger();
  
  void SetNewValue(G4UIcommand*, G4String);

 private:
  DetectorConstruction* fDetector;

  G4UIdirectory* AGTPCDir; 
  G4UIdirectory* geometryDir;

  G4UIcmdWithADouble* setQuencherFraction;
  G4UIcmdWithADoubleAndUnit* setMagneticField;
  G4UIcmdWithABool* setPrototype;
  G4UIcmdWithABool* setMaterial;
  G4UIcmdWithAString* setupFieldMap;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
