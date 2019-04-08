#include "UserActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh"


UserActionInitialization::UserActionInitialization(DetectorConstruction* dt):
  fDetector(dt),fRunName("")
{}

UserActionInitialization::~UserActionInitialization(){}

void UserActionInitialization::Build() const 
{
  G4cout<<"PrimaryGeneratorAction..."<<G4endl;
  PrimaryGeneratorAction* primgen = new PrimaryGeneratorAction(fDetector);
  G4cout<<"PrimaryGeneratorAction DONE"<<G4endl;

  G4cout<<"RunAction..."<<G4endl;
  RunAction* run_action = new RunAction( fDetector );
  run_action->SetRunName( fRunName );
  primgen->SetRunAction( run_action );
  G4cout<<"RunAction DONE"<<G4endl;

  G4cout<<"EventAction..."<<G4endl;
  EventAction* event_action = new EventAction( run_action );
  G4cout<<"EventAction DONE"<<G4endl;

  SetUserAction(primgen);
  SetUserAction(run_action);
  SetUserAction(event_action);
}

void UserActionInitialization::BuildForMaster() const 
{
  RunAction* run_action = new RunAction( fDetector );
  run_action->SetRunName( fRunName );
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
