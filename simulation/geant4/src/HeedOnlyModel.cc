/*
 * HeedOnlyModel.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
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

  fVisualizeChamber = gmp->GetVisualizeChamber();
  fVisualizeField = gmp->GetVisualizeField();

  fName = modelName.c_str();
  G4cout<<fName<<G4endl;

  G4cout << "HeedOnlyModel::HeedOnlyModel   Initialise Physics" << G4endl;
  InitialisePhysics();
}

HeedOnlyModel::~HeedOnlyModel() {}

void HeedOnlyModel::Run(G4String particleName, double ekin_keV, double t, 
			double x_cm, double y_cm, double z_cm, 
			double dx, double dy, double dz)
{
  double eKin_eV = ekin_keV * 1000.;  
  G4cout << "Run HeedOnly" << G4endl;
  //  int nc = 0, ni=0;
  fTrackHeed->SetParticle(particleName);
  fTrackHeed->SetEnergy(eKin_eV);
  fTrackHeed->NewTrack(x_cm, y_cm, z_cm, t, dx, dy, dz);
  double xcl, ycl, zcl, tcl, ecl, extra;
  int ncl = 0;
  
  while( fTrackHeed->GetCluster(xcl, ycl, zcl, tcl, ncl, ecl, extra) )
    {
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
}

bool HeedOnlyModel::Readout()
{
  return isReadout;
}
