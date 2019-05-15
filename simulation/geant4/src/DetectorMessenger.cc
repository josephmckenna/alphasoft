#include "DetectorMessenger.hh"

#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::DetectorMessenger(DetectorConstruction* det): fDetector(det)
{
  AGTPCDir = new G4UIdirectory("/AGTPC/");
  AGTPCDir->SetGuidance("UI commands for ALPHA-g TPC");

  garfVerb = new G4UIcmdWithAnInteger("/AGTPC/garfVerb",this);
  garfVerb->SetGuidance("Verbosity for Garfield++ interface. 0: none, 1: clusters, 2: driftlines");
  garfVerb->SetDefaultValue(0);
  garfVerb->AvailableForStates(G4State_PreInit, G4State_Idle);

  geometryDir = new G4UIdirectory("/AGTPC/geom/");
  geometryDir->SetGuidance("AGTPC geometry specific controls");

  setQuencherFraction = new G4UIcmdWithADouble("/AGTPC/geom/QuencherFraction",this);
  setQuencherFraction->SetGuidance("Set quencher (CO2) fraction: 0-1");
  setQuencherFraction->SetDefaultValue(0.3);
  setQuencherFraction->AvailableForStates(G4State_PreInit, G4State_Idle);

  setMagneticField = new G4UIcmdWithADoubleAndUnit("/AGTPC/geom/MagneticField",this);
  setMagneticField->SetGuidance("Set Uniform Magnetic Field Value");
  setMagneticField->SetDefaultValue(1.*tesla);
  setMagneticField->SetUnitCategory("Magnetic flux density");
  setMagneticField->AvailableForStates(G4State_PreInit, G4State_Idle);

  setPrototype = new G4UIcmdWithoutParameter("/AGTPC/geom/EnablePrototype",this);
  setPrototype->SetGuidance("Enable Simulation of the rTPC Prototype");
  setPrototype->AvailableForStates(G4State_PreInit, G4State_Idle);

  setMaterial = new G4UIcmdWithoutParameter("/AGTPC/geom/DisableMaterial",this);
  setMaterial->SetGuidance("Construct ALPHA-g Cryostat and Traps");
  setMaterial->AvailableForStates(G4State_PreInit, G4State_Idle);

  setupFieldMap = new G4UIcmdWithAString("/AGTPC/geom/FieldMap",this);
  setupFieldMap->SetGuidance("Specify Magnetic Field map path");
  G4String path = G4String(getenv("AGRELEASE")) + "simulation/common/fieldmaps/";
  path += "Babcock_Field_Map.csv";
  setupFieldMap->SetDefaultValue(path);
  setupFieldMap->AvailableForStates(G4State_PreInit, G4State_Idle);

  setCADverb = new G4UIcmdWithoutParameter("/AGTPC/geom/CADverbose",this);
  setCADverb->SetGuidance("Print list of CAD parts");
  setCADverb->AvailableForStates(G4State_PreInit, G4State_Idle);
}


DetectorMessenger::~DetectorMessenger() 
{
  delete AGTPCDir;
  delete geometryDir;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) 
{
  if( command == setQuencherFraction )
    fDetector->SetQuencherFraction(setQuencherFraction->GetNewDoubleValue(newValues));
  else if( command == setMagneticField )
    fDetector->SetMagneticFieldValue(setMagneticField->GetNewDoubleValue(newValues));
  else if( command == setMaterial )
    fDetector->TurnOffMaterial();
  else if( command == setPrototype )
    fDetector->SetPrototype();
  else if( command == setupFieldMap )
    G4cout << "/AGTPC/geom/FieldMap NOT YET IMPLEMENTED" << G4endl;
  else if( command == setCADverb )
    fDetector->SetVerboseCAD();
  else if( command == garfVerb )
     fDetector->SetVerboseGarf(garfVerb->GetNewIntValue(newValues));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
