/*
 * HeedOnlyModel.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 */
#include <iostream>
#include "HeedOnlyModel.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "GasModelParameters.hh"
#include "GasBoxSD.hh"
#include "GasBoxHit.hh"
#include "G4SDManager.hh"

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

HeedOnlyModel::HeedOnlyModel(GasModelParameters* gmp, G4String modelName, 
			     G4Region* envelope, DetectorConstruction* dc, 
			     TPCSD* sd): HeedModel(modelName, envelope, dc, sd)	
{
  G4cout << "HeedOnlyModel::HeedOnlyModel Copying the particle map" << G4endl;
  G4cout << gmp->GetParticleNamesHeedOnly().size() << G4endl;
  fMapParticlesEnergy = gmp->GetParticleNamesHeedOnly();

  G4cout << "HeedOnlyModel::HeedOnlyModel set the gas file" << G4endl;
  gasFile = gmp->GetGasFile();
  ionMobFile = gmp->GetIonMobilityFile();

  driftElectrons = gmp->GetDriftElectrons();
  driftRKF = gmp->GetDriftRKF();
  trackMicro = gmp->GetTrackMicroscopic();
  createAval = gmp->GetCreateAvalancheMC();

  fVisualizeChamber = gmp->GetVisualizeChamber();
  fVisualizeSignal = gmp->GetVisualizeSignals();
  fVisualizeField = gmp->GetVisualizeField();

  vAnodeWires = gmp->GetVoltageAnode();
  vCathode = gmp->GetVoltageCathode();
  vFieldWires = gmp->GetVoltageField();

  //  name="HeedOnlyModel";
  name = modelName.c_str();
  G4cout<<name<<G4endl;

  InitialisePhysics();
}

HeedOnlyModel::~HeedOnlyModel() {}

void HeedOnlyModel::Run(G4String particleName, double ekin_keV, double t, 
			double x_cm, double y_cm, double z_cm, 
			double dx, double dy, double dz)
{
  double eKin_eV = ekin_keV * 1000.;
  int nc = 0, ni=0;
  fTrackHeed->SetParticle(particleName);
  fTrackHeed->SetEnergy(eKin_eV);
  fTrackHeed->NewTrack(x_cm, y_cm, z_cm, t, dx, dy, dz);
  double xcl, ycl, zcl, tcl, ecl, extra;
  int ncl = 0;
  while( fTrackHeed->GetCluster(xcl, ycl, zcl, tcl, ncl, ecl, extra) )
    {
      // Retrieve the electrons of the cluster.
      for (int i = 0; i < ncl; ++i) 
	{
	  double x, y, z, t, e, dx, dy, dz;
	  fTrackHeed->GetElectron(i, x, y, z, t, e, dx, dy, dz);
	  ChamberHit* hit = new GasBoxHit();
	  hit->SetPos(G4ThreeVector(x*CLHEP::cm,y*CLHEP::cm,z*CLHEP::cm));
	  hit->SetTime(t);
	  hit->SetModelName(name);
	  fTPCSD->InsertChamberHit(hit);
	  Drift(x,y,z,t);
	}
    }
  PlotTrack();
}



// void HeedOnlyModel::ProcessEvent(){

// }

// void HeedOnlyModel::Reset(){

// }
