/*
 * HeedInterfaceModel.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 */
#include "GasModelParameters.hh"
#include "HeedInterfaceModel.hh"
#include "DetectorConstruction.hh"
#include "AWHit.hh"
#include "TPCSD.hh"

#include "G4VPhysicalVolume.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4VVisManager.hh"

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}


HeedInterfaceModel::HeedInterfaceModel(GasModelParameters* gmp, G4String modelName,
				       G4Region* envelope, DetectorConstruction* dc,
				       TPCSD* sd): HeedModel(modelName, envelope, dc, sd)
{ 
  G4cout << "HeedInterfaceModel::HeedInterfaceModel Copying the particle map --> size: "
	 << gmp->GetParticleNamesHeedInterface().size() << G4endl;
  fMapParticlesEnergy = gmp->GetParticleNamesHeedInterface();

  driftElectrons = gmp->GetDriftElectrons();
  driftRKF = gmp->GetDriftRKF();
  trackMicro = gmp->GetTrackMicroscopic();
  createAval = gmp->GetCreateAvalancheMC();

  fVisualizeChamber = gmp->GetVisualizeChamber();
  fVisualizeSignal = gmp->GetVisualizeSignals();
  fVisualizeField = gmp->GetVisualizeField();

  fName = modelName.c_str();
  G4cout<<fName<<G4endl;
 
  InitialisePhysics();
}

HeedInterfaceModel::~HeedInterfaceModel() {}

void HeedInterfaceModel::Run(G4String particleName, double ekin_keV, double t, 
			     double x_cm, double y_cm, double z_cm,
			     double dx, double dy, double dz)
{
  double eKin_eV = ekin_keV * 1000.;
  int nc = 0, ni=0;
  G4cout << "Run HeedInterface" << G4endl;
  G4cout << "Electron energy(in eV): " << eKin_eV << G4endl;  
  if(particleName == "e-")
    {
      G4AutoLock lock(&aMutex);
      fTrackHeed->TransportDeltaElectron(x_cm, y_cm, z_cm, t, eKin_eV, dx, dy,
					 dz, nc, ni);
    }
  else
    {
      G4AutoLock lock(&aMutex);
      fTrackHeed->TransportPhoton(x_cm, y_cm, z_cm, t, eKin_eV, dx, dy,
				  dz, nc);
    }
  G4cout << "HeedInterfaceModel::Run  # of e-: " << nc << " # of ions: " << ni <<G4endl;
  for( int cl = 0; cl < nc; cl++ )
    {
      double xe, ye, ze, te;
      double ee, dxe, dye, dze;
      fTrackHeed->GetElectron(cl, xe, ye, ze, te, ee, dxe, dye, dze);
      G4cout << "Electron #" << cl 
	     << " r: " << G4BestUnit(sqrt(xe*xe + ye*ye)*CLHEP::cm,"Length") 
	     << " phi: "<< G4BestUnit(atan2(ye,xe),"Angle")
	     << " z:" << G4BestUnit(ze*CLHEP::cm,"Length") 
	     << " time: " << G4BestUnit(te,"Time") << G4endl;
      ChamberHit* hit = new ChamberHit();
      hit->SetPos(G4ThreeVector(xe*CLHEP::cm,ye*CLHEP::cm,ze*CLHEP::cm));
      hit->SetTime(te);
      hit->SetEnergy(ee);
      hit->SetTrackID(cl);
      hit->SetModelName(fName);
      fTPCSD->InsertChamberHit(hit);
      if(G4VVisManager::GetConcreteInstance() && cl % 100 == 0)
	Drift(xe,ye,ze,te);
    }
}

void HeedInterfaceModel::ProcessEvent()
{
  G4cout << "HeedInterfaceModel::ProcessEvent()" << G4endl;

  PlotTrack();
   
  fSensor->ConvoluteSignal();
     
  double Tstart, BinWidth;
  unsigned int nBins;
  fSensor->GetTimeWindow(Tstart, BinWidth, nBins);
   
  double a;
  for(int w=0; w<fDet->GetTPC()->GetNumberOfAnodeWires(); ++w)
    {
      G4String wname="a"+std::to_string(w);
      std::vector<double> data;
      for(uint b=1; b<=nBins; ++b)
	{
	  a = fSensor->GetSignal(wname.c_str(),b);
	  data.push_back( a );
	}
      AWHit* hit = new AWHit();
      hit->SetModelName( fName );
      hit->SetAnode( w );
      hit->SetWaveform( data );
      fTPCSD->InsertAWHit(hit);
    }
}

void HeedInterfaceModel::Reset()
{
  G4cout << "HeedInterfaceModel::Reset()" << G4endl;
  fSensor->ClearSignal();
  fSensor->NewSignal();
}


