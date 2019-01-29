#include <vector>
#include <stdio.h>

#include "HeedModel.hh"
#include "DriftLineTrajectory.hh"
#include "TPC.hh"
#include "DetectorConstruction.hh"

#include "G4VPhysicalVolume.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"

#include "G4RunManager.hh"
#include "G4TrackingManager.hh"
#include "G4EventManager.hh"
#include "G4VVisManager.hh"

#include "Helpers.hh"

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

const static G4double torr = 1. / 760. * atmosphere;

HeedModel::HeedModel(G4String modelName, G4Region* envelope, 
		     DetectorConstruction* dc, TPCSD* sd): G4VFastSimulationModel(modelName, envelope), 
							   fDet(dc), fTPCSD(sd), fName(modelName)
{
  fMaxRad = fDet->GetTPC()->GetROradius();
  fMinRad = fDet->GetTPC()->GetCathodeRadius();
  fLen = fDet->GetTPC()->GetFullLengthZ();
}

HeedModel::~HeedModel() {}

G4bool HeedModel::IsApplicable(const G4ParticleDefinition& particleType) {
  G4String particleName = particleType.GetParticleName();
  return FindParticleName(particleName);
}

G4bool HeedModel::ModelTrigger(const G4FastTrack& fastTrack) {
  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4String particleName =
    fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();
  return FindParticleNameEnergy(particleName, ekin / keV);
}

void HeedModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) {

  G4ThreeVector localdir = fastTrack.GetPrimaryTrackLocalDirection();

  G4ThreeVector worldPosition = fastTrack.GetPrimaryTrack()->GetPosition();

  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4double time = fastTrack.GetPrimaryTrack()->GetGlobalTime();
  G4String particleName =
    fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();

  fastStep.KillPrimaryTrack();
  fastStep.SetPrimaryTrackPathLength(0.0);
  Run(particleName, ekin/keV, time, worldPosition.x() / CLHEP::cm,
      worldPosition.y() / CLHEP::cm, worldPosition.z() / CLHEP::cm,
      localdir.x(), localdir.y(), localdir.z());
  fastStep.SetTotalEnergyDeposited(ekin);
}

G4bool HeedModel::FindParticleName(G4String name) {
  MapParticlesEnergy::iterator it;
  it = fMapParticlesEnergy.find(name);
  if (it != fMapParticlesEnergy.end()) {
    return true;
  }
  return false;
}

G4bool HeedModel::FindParticleNameEnergy(G4String name, double ekin_keV) 
{
  MapParticlesEnergy::iterator it;
  for( it=fMapParticlesEnergy.begin(); it!=fMapParticlesEnergy.end(); ++it )
    {
      if(it->first == name)
	{
	  EnergyRange_keV range = it->second;
	  if (range.first <= ekin_keV && range.second >= ekin_keV) return true;
	}
    }
  return false;
}

void HeedModel::InitialisePhysics()
{
  G4cout << "HeedModel::InitialisePhysics()  create Sensor" << G4endl;
  AddSensor();
  
  G4cout << "HeedModel::InitialisePhysics()  Tracking" << G4endl;
  SetTracking();
  
  if(fVisualizeChamber) CreateChamberView();
  if(fVisualizeSignal) CreateSignalView();
  if(fVisualizeField) CreateFieldView();
}

void HeedModel::AddSensor()
{
  fSensor = new Garfield::Sensor();
  fSensor->DisableDebugging();
  // Calculate the electric field
  fSensor->AddComponent(fDet->GetTPC());

  // Request signal calculation for the electrode named with labels above,
  // using the weighting field provided by the Component object cmp.
  std::vector<std::string> anodes = fDet->GetTPC()->GetAnodeReadouts();
  for(unsigned int a=0; a < anodes.size(); ++a)
    fSensor->AddElectrode(fDet->GetTPC(),anodes[a]);

  fSensor->AddElectrode(fDet->GetTPC(), "ro");

  // Set Time window for signal integration, units in [ns]
  fSensor->SetTimeWindow(0., 16., 411);
  
  fSensor->SetTransferFunction(Hands);
}

void HeedModel::SetTracking()
{
  if(driftRKF)
    {
      fDriftRKF = new Garfield::DriftLineRKF();
      fDriftRKF->SetSensor(fSensor);
      const double maxStepSize=0.03;// cm
      fDriftRKF->SetMaximumStepSize(maxStepSize);
      fDriftRKF->EnableStepSizeLimit();
      //      fDriftRKF->EnableDebugging();
    }
  else if(trackMicro)
    {
      fAvalanche = new Garfield::AvalancheMicroscopic();
      fAvalanche->SetSensor(fSensor);
      fAvalanche->EnableSignalCalculation();
    }
  else
    {  
      fDrift = new Garfield::AvalancheMC();
      fDrift->SetSensor(fSensor);
      fDrift->EnableSignalCalculation();
      fDrift->SetDistanceSteps(2.e-3);
      if(createAval) fDrift->EnableAttachment();
      else fDrift->DisableAttachment();
    }
  
  fTrackHeed = new Garfield::TrackHeed();
  fTrackHeed->SetSensor(fSensor);
  fTrackHeed->SetParticle("e-");
  fTrackHeed->EnableDeltaElectronTransport();
}
  
void HeedModel::CreateChamberView()
{
  char str[30];
  strcpy(str,fName);
  strcat(str,"_chamber");
  fChamber = new TCanvas(str, "Chamber View", 700, 700);
  cellView = new Garfield::ViewCell();
  cellView->SetComponent(fDet->GetTPC());
  cellView->SetCanvas(fChamber);
  cellView->Plot2d();
  fChamber->Update();
  char str2[30];
  strcpy(str2,fName);
  strcat(str2,"_chamber.pdf");
  fChamber->Print(str2);
  //  gSystem->ProcessEvents();
  G4cout << "CreateCellView()" << G4endl;
  
  viewDrift = new Garfield::ViewDrift();
  viewDrift->SetCanvas(fChamber);
  if(driftRKF) fDriftRKF->EnablePlotting(viewDrift);
  else if(trackMicro) fAvalanche->EnablePlotting(viewDrift);
  else fDrift->EnablePlotting(viewDrift);
  fTrackHeed->EnablePlotting(viewDrift);
}

void HeedModel::CreateSignalView()
{
  char str[30];
  strcpy(str,fName);
  strcat(str,"_signal");
  fSignal = new TCanvas(str, "Signal on the wire", 700, 700);
  viewSignal = new Garfield::ViewSignal();
  viewSignal->SetSensor(fSensor);
  viewSignal->SetCanvas(fSignal);
}

void HeedModel::CreateFieldView()
{
  char str[30];
  strcpy(str,fName);
  strcat(str,"_efield");
  fField = new TCanvas(fName, "Electric field", 700, 700);
  viewField = new Garfield::ViewField();
  viewField->SetCanvas(fField);
  viewField->SetComponent(fDet->GetTPC());
  viewField->SetNumberOfContours(40);
  viewField->PlotContour();
  fField->Update();
  char str2[30];
  strcpy(str2,fName);
  strcat(str2,"_efield.pdf");
  fField->Print(str2);
}

void HeedModel::Drift(double x, double y, double z, double t)
{
  if(driftElectrons)
    {
      DriftLineTrajectory* dlt = new DriftLineTrajectory();
      G4TrackingManager* fpTrackingManager = G4EventManager::GetEventManager()->GetTrackingManager();
      fpTrackingManager->SetTrajectory(dlt);
      if(driftRKF)
	{
	  bool stat = fDriftRKF->DriftElectron(x,y,z,t);
	  if( !stat )
	    {
	      std::cerr<<"x = "<<x<<" y = "<<y<<" z = "<<z<<" t = "<<t<<std::endl;
	      return;
	    }
	  unsigned int n = fDriftRKF->GetNumberOfDriftLinePoints();
	  double xi,yi,zi,ti;
	  for(uint i=0;i<n;i++)
	    {
	      fDriftRKF->GetDriftLinePoint(i,xi,yi,zi,ti);
	      if(G4VVisManager::GetConcreteInstance() && i % 1000 == 0)
		dlt->AppendStep(G4ThreeVector(xi*CLHEP::cm,yi*CLHEP::cm,zi*CLHEP::cm),ti);
            }
        }
      else if(trackMicro)
	{
	  fAvalanche->AvalancheElectron(x,y,z,t,0,0,0,0);
	  unsigned int nLines = fAvalanche->GetNumberOfElectronEndpoints();
	  for(uint i=0;i<nLines;i++)
	    {
	      unsigned int n = fAvalanche->GetNumberOfElectronDriftLinePoints(i);
	      double xi,yi,zi,ti;
	      for(uint j=0;j<n;j++)
		{
		  fAvalanche->GetElectronDriftLinePoint(xi,yi,zi,ti,j,i);
		  if(G4VVisManager::GetConcreteInstance() && i % 1000 == 0)
		    dlt->AppendStep(G4ThreeVector(xi*CLHEP::cm,yi*CLHEP::cm,zi*CLHEP::cm),ti);
                }
            }
        }
      else
	{
	  fDrift->DriftElectron(x,y,z,t);
	  unsigned int n = fDrift->GetNumberOfDriftLinePoints();
	  double xi,yi,zi,ti;
	  for(uint i=0;i<n;i++)
	    {
	      fDrift->GetDriftLinePoint(i,xi,yi,zi,ti);
	      if(G4VVisManager::GetConcreteInstance() && i % 1000 == 0)
		dlt->AppendStep(G4ThreeVector(xi*CLHEP::cm,yi*CLHEP::cm,zi*CLHEP::cm),ti);
            }
        }
    }
}

void HeedModel::PlotTrack(G4String fileName)
{
  if(fVisualizeChamber)
    {
      G4cout << "HeedModel::PlotTrack " << fName << G4endl;
      viewDrift->Plot(true,false);
      fChamber->Update();
      fChamber->Print(fileName.c_str());
    }
}

// void HeedModel::ProcessEvent()
// {
//   G4cout << "HeedModel::ProcessEvent()" << G4endl;
   
//   fSensor->ConvoluteSignal();
     
//   double Tstart, BinWidth;
//   unsigned int nBins;
//   fSensor->GetTimeWindow(Tstart, BinWidth, nBins);
   
//   double a;
//   for(int w=0; w<fDet->GetTPC()->GetNumberOfAnodeWires(); ++w)
//     {
//       G4String wname="a"+std::to_string(w);
//       std::vector<double> data;
//       for(uint b=1; b<=nBins; ++b)
// 	{
// 	  a = fSensor->GetSignal(wname.c_str(),b);
// 	  data.push_back( a );
// 	}
//       AWHit* hit = new AWHit();
//       hit->SetAnode( w );
//       hit->SetWaveform( data );
//       fTPCSD->InsertAWHit(hit);
//     }
// }

// void HeedModel::Reset()
// {
//   G4cout << "HeedModel::Reset()" << G4endl;
//   fSensor->ClearSignal();
//   fSensor->NewSignal();
// }
