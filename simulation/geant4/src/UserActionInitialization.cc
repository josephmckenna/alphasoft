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
  PrimaryGeneratorAction* primgen = new PrimaryGeneratorAction(fDetector);
  RunAction* run_action = new RunAction( fDetector );
  run_action->SetRunName( fRunName );
  primgen->SetRunAction( run_action );
  EventAction* event_action = new EventAction( run_action );

  SetUserAction(primgen);
  SetUserAction(run_action);
  SetUserAction(event_action);
}

void UserActionInitialization::BuildForMaster() const 
{
  RunAction* run_action = new RunAction( fDetector );
  run_action->SetRunName( fRunName );
}
