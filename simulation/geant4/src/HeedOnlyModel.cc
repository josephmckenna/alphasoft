/*
 * HeedOnlyModel.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 *
 *  Modified: Jan, 2019
 *            A Capra
 *
 */

#include "GasModelParameters.hh"
#include "HeedOnlyModel.hh"
#include "DetectorConstruction.hh"
#include "AWHit.hh"
#include "TPCSD.hh"

#include "G4VPhysicalVolume.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4FieldTrack.hh"
#include "G4FieldTrackUpdator.hh"
#include "G4PathFinder.hh"
#include "G4FastStep.hh"
#include "G4FastTrack.hh"

#include "G4SDManager.hh"

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

HeedOnlyModel::HeedOnlyModel(GasModelParameters* gmp, G4String modelName, 
			     G4Region* envelope, DetectorConstruction* dc, 
			     TPCSD* sd): HeedModel(modelName, envelope, dc, sd)	
{
  G4cout << "HeedOnlyModel::HeedOnlyModel Copying the particle map --> size: " 
	 << gmp->GetParticleNamesHeedOnly().size() << G4endl;
  fMapParticlesEnergy = gmp->GetParticleNamesHeedOnly();

  driftElectrons = gmp->GetDriftElectrons();
  createAval = gmp->GetCreateAvalancheMC();

  driftRKF = gmp->GetDriftRKF();
  trackMicro = gmp->GetTrackMicroscopic();

  generateSignals = gmp->GetGenerateSignals();

  fsg->SetAnodeNoiseLevel( gmp->GetAnodeNoiseLevel() );
  fsg->SetPadNoiseLevel( gmp->GetPadNoiseLevel() );

  fsg->PrintNoiseLevels();

  fVisualizeChamber = gmp->GetVisualizeChamber();
  fVisualizeField = gmp->GetVisualizeField();

  fName = modelName.c_str();
  G4cout<<fName<<G4endl;

  G4cout << "HeedOnlyModel::HeedOnlyModel   Initialise Physics" << G4endl;
  InitialisePhysics();
}

HeedOnlyModel::~HeedOnlyModel() {}

void HeedOnlyModel::Run(G4FastStep& fastStep,const G4FastTrack& fastTrack,
                        G4String particleName, double ekin_keV, double t, 
			double x_cm, double y_cm, double z_cm, 
			double dx, double dy, double dz)
{
  double eKin_eV = ekin_keV * 1000.;  
   uint prec=G4cout.precision();
   if( fVerboseLevel > 0 )
      {
         G4cout << "Run HeedOnly\t";
         G4cout.precision(5);
         G4cout << "Electron energy(in eV): " << eKin_eV << G4endl;  
         G4cout.precision(prec);
         fTrackHeed->EnableDebugging();
      }

  fTrackHeed->SetParticle(particleName);
  fTrackHeed->SetEnergy(eKin_eV);
  fTrackHeed->NewTrack(x_cm, y_cm, z_cm, t, dx, dy, dz);
  double xcl, ycl, zcl, tcl, ecl, extra;
  int ncl = 0;
  
   if( fVerboseLevel > 0 )
      G4cout << "HeedOnlyModel::Run  # of clusters: " << ncl << G4endl;

  while( fTrackHeed->GetCluster(xcl, ycl, zcl, tcl, ncl, ecl, extra) )
    {
       if( fVerboseLevel > 0 )
          G4cout << "Cluster #" << ncl 
                 << " (" << G4BestUnit(xcl,"Length") << "," << G4BestUnit(ycl,"Length") << "," 
                 << G4BestUnit(zcl,"Length") << "," << G4BestUnit(tcl,"Time") << ")" << G4endl;
       // Retrieve the electrons of the cluster.
      for (int i = 0; i < ncl; ++i) 
	{
	  double x, y, z, t, e, dx, dy, dz;
	  fTrackHeed->GetElectron(i, x, y, z, t, e, dx, dy, dz);
	  ChamberHit* hit = new ChamberHit();
	  hit->SetPos(G4ThreeVector(x*CLHEP::cm,y*CLHEP::cm,z*CLHEP::cm));
	  hit->SetTime(t);
	  hit->SetEnergy(e);
	  hit->SetTrackID(i);
	  hit->SetModelName(fName);
	  fTPCSD->InsertChamberHit(hit);
	  Drift(x,y,z,t);
	}
    }
  PlotTrack();

   // try to estimate the exit location of the high energy particle out of the gas volume from the initial location and momentum direction and update the track parameters: location, direction, energy
   G4Track track = * fastTrack.GetPrimaryTrack();
   G4FieldTrack aFieldTrack( '0' );
   G4FieldTrackUpdator::Update( &aFieldTrack, &track );
    
   G4double retSafety = -1.0;
   ELimited retStepLimited;
   G4FieldTrack endTrack( 'a' );
   G4double currentMinimumStep = 10.0*m;  // Temporary: change that to sth connected
   // to particle momentum.
   G4PathFinder* fPathFinder = G4PathFinder::GetInstance();
   /*G4double lengthAlongCurve = */
   fPathFinder->ComputeStep( aFieldTrack,
                             currentMinimumStep,
                             0,
                             fastTrack.GetPrimaryTrack()->GetCurrentStepNumber(),
                             retSafety,
                             retStepLimited,
                             endTrack,
                             fastTrack.GetPrimaryTrack()->GetVolume() );
    
   // Place the particle at the tracking detector exit
   // (at the place it would reach without the change of its momentum).
   fastStep.ProposePrimaryTrackFinalPosition( endTrack.GetPosition(), false );
   G4cout << "HeedOnlyModel::Run Particle location: " << endTrack.GetPosition() << G4endl;
   fastStep.SetPrimaryTrackFinalKineticEnergyAndDirection(eKin_eV*eV, 
                                                          G4ThreeVector(dx,dy,dz),false);
   fastStep.SetTotalEnergyDeposited((ekin_keV*1000-eKin_eV)*eV);
   std::cout << "HeedOnlyModel::Run Particle Tracked out of the gas volume" << std::endl;
}

bool HeedOnlyModel::Readout()
{
  return isReadout;
}

void HeedOnlyModel::ProcessEvent() { }

void HeedOnlyModel::Reset() { }


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
