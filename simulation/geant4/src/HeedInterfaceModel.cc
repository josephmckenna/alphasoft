/*
 * HeedInterfaceModel.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 */
#include <iostream>
#include "HeedInterfaceModel.hh"
#include "GasModelParameters.hh"
#include "TPCSD.hh"

#include "G4VPhysicalVolume.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"

#include "G4VVisManager.hh"

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}


HeedInterfaceModel::HeedInterfaceModel(GasModelParameters* gmp, G4String modelName,
				       G4Region* envelope, DetectorConstruction* dc,
				       TPCSD* sd): HeedModel(modelName, envelope, dc, sd)
{ 
  G4cout << "HeedInterfaceModel::HeedInterfaceModel Copying the particle map" << G4endl;
  G4cout << gmp->GetParticleNamesHeedInterface().size() << G4endl;
  fMapParticlesEnergy = gmp->GetParticleNamesHeedInterface();

  G4cout << "HeedInterfaceModel::HeedInterfaceModel set the gas file" << G4endl;
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

  //  name="HeedInterfaceModel";
  name = modelName.c_str();
  G4cout<<name<<G4endl;
 
  InitialisePhysics();
}

HeedInterfaceModel::~HeedInterfaceModel() {}

void HeedInterfaceModel::Run(G4String particleName, double ekin_keV, double t, 
			     double x_cm, double y_cm, double z_cm,
			     double dx, double dy, double dz)
{
  double eKin_eV = ekin_keV * 1000.;
  int nc = 0, ni=0;
  G4cout << "Run Interface" << G4endl;
  G4cout << "Electron energy(in eV): " << eKin_eV << G4endl;
  if(particleName == "e-"){
    G4AutoLock lock(&aMutex);
    fTrackHeed->TransportDeltaElectron(x_cm, y_cm, z_cm, t, eKin_eV, dx, dy,
				       dz, nc, ni);
  }
  else{
    G4AutoLock lock(&aMutex);
    fTrackHeed->TransportPhoton(x_cm, y_cm, z_cm, t, eKin_eV, dx, dy,
				dz, nc);
  }
  for( int cl = 0; cl < nc; cl++ )
    {
    double xe, ye, ze, te;
    double ee, dxe, dye, dze;
    fTrackHeed->GetElectron(cl, xe, ye, ze, te, ee, dxe, dye, dze);
    ChamberHit* hit = new ChamberHit();
    hit->SetPos(G4ThreeVector(xe*CLHEP::cm,ye*CLHEP::cm,ze*CLHEP::cm));
    hit->SetTime(te);
    hit->SetModelName(name);
    fTPCSD->InsertChamberHit(hit);
    if(G4VVisManager::GetConcreteInstance() && cl % 100 == 0)
      Drift(xe,ye,ze,te);
    }
  PlotTrack();

}

// void HeedInterfaceModel::ProcessEvent(){

// }

// void HeedInterfaceModel::Reset(){
  
// }


