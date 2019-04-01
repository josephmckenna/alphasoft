#ifndef UserActionInitialization_hh
#define UserActionInitialization_hh

#include "G4VUserActionInitialization.hh"
#include "G4String.hh"

class DetectorConstruction;
class UserActionInitialization : public G4VUserActionInitialization
{
public:
  UserActionInitialization(DetectorConstruction*);
  ~UserActionInitialization();
  
  void Build() const;
  void BuildForMaster() const;

private:
  DetectorConstruction* fDetector;
  G4String fRunName;
};

#endif
