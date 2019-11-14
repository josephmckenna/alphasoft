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

#include "G4RandomTools.hh"

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

const static G4double torr = 1. / 760. * atmosphere;
const static double rad_to_deg = 180./pi;

HeedModel::HeedModel(G4String modelName, G4Region* envelope, 
		     DetectorConstruction* dc, TPCSD* sd): G4VFastSimulationModel(modelName, envelope), 
							   fDet(dc), fTPCSD(sd), fName(modelName),
                                                           fVerboseLevel(0)
{
  fMaxRad = fDet->GetTPC()->GetROradius();
  fMinRad = fDet->GetTPC()->GetCathodeRadius();
  fLen = fDet->GetTPC()->GetFullLengthZ();

  fsg = new SignalsGenerator(1.,1.);
}

HeedModel::~HeedModel() 
{
  delete fsg;
}

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

  G4ThreeVector dir = fastTrack.GetPrimaryTrack()->GetMomentumDirection();

  G4ThreeVector worldPosition = fastTrack.GetPrimaryTrack()->GetPosition();

  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4double time = fastTrack.GetPrimaryTrack()->GetGlobalTime();
  G4String particleName =
    fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();

  Run(fastStep, fastTrack, particleName, ekin/keV, time, worldPosition.x() / CLHEP::cm,
      worldPosition.y() / CLHEP::cm, worldPosition.z() / CLHEP::cm,
      dir.x(), dir.y(), dir.z());
}

G4bool HeedModel::FindParticleName(G4String name) {
  MapParticlesEnergy::iterator it;
  it = fMapParticlesEnergy.find(name);
  if (it != fMapParticlesEnergy.end()) {
    return true;
  }
  return false;
}

//Checks if the energy condition of the particle is in the list of conditions for which the model shoould be triggered (called by ModelTrigger)
G4bool HeedModel::FindParticleNameEnergy(G4String name,
                                             double ekin_keV) {
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
  if(fVisualizeField) CreateFieldView();
}

void HeedModel::AddSensor()
{
  fSensor = new Garfield::Sensor();
  //  fSensor->DisableDebugging();
  // Calculate the electric field
  fSensor->AddComponent(fDet->GetTPC());

  TestSensor();

  // Request signal calculation for the electrode named with labels above,
  // using the weighting field provided by the Component object cmp.
  std::vector<std::string> anodes = fDet->GetTPC()->GetAnodeReadouts();
  for(unsigned int a=0; a < anodes.size(); ++a)
    fSensor->AddElectrode(fDet->GetTPC(),anodes[a]);
  fSensor->AddElectrode(fDet->GetTPC(), "ro");

  G4cout << "HeedModel::AddSensor() --> # of Electrodes: " << fSensor->GetNumberOfElectrodes() << G4endl;

}

void HeedModel::TestSensor()
{
  G4double x=110.*mm, y=110.*mm, z=0.;
  double Ex,Ey,Ez, Bx,By,Bz;
  int status;
  Medium* med;
  fSensor->ElectricField(x/cm,y/cm,z/cm,Ex,Ey,Ez,med,status);
  if( status == 0 )
    G4cout << "HeedModel::TestSensor() Electric Field @ ("<<x/cm<<","<<y/cm<<","<<z/cm
	   << ") cm is ("<<Ex<<","<<Ey<<","<<Ez
	   << ") V/cm\tMedium: "<<med->GetName()<<G4endl;
  else
    G4cout << "HeedModel::TestSensor() Electric Field @ ("<<x/cm<<","<<y/cm<<","<<z/cm
	   << ") cm is ("<<Ex<<","<<Ey<<","<<Ez<<") V/cm\t status: "<<status<<G4endl;
 
  fSensor->MagneticField(x/cm,y/cm,z/cm,Bx,By,Bz,status);
  if( status == 0 )
    G4cout << "HeedModel::TestSensor() MagnetiField @ ("<<x/cm<<","<<y/cm<<","<<z/cm
	   <<") cm is ("<<Bx<<","<<By<<","<<Bz<<") T"<<G4endl;
  else
    G4cout << "HeedModel::TestSensor() MagnetiField @ ("<<x/cm<<","<<y/cm<<","<<z/cm
	   <<") cm is ("<<Bx<<","<<By<<","<<Bz<<") T\t status: "<<status<<G4endl;
  double vx,vy,vz;
  bool stat = med->IonVelocity(Ex, Ey, Ez, Bx, By, Bz, vx, vy, vz);
  if( stat )
    G4cout << "HeedModel::TestSensor() Ion Velocity @ ("<<x/cm<<","<<y/cm<<","<<z/cm
	   << ") cm is ("<<vx<<","<<vy<<","<<vz<<") cm/ns"<<G4endl;
  else
    G4cout << "HeedModel::TestSensor() Ion Velocity Error" << G4endl;
  stat = med->ElectronVelocity(Ex, Ey, Ez, Bx, By, Bz, vx, vy, vz);
  if( stat )
    G4cout << "HeedModel::TestSensor() Electron Velocity @ ("<<x/cm<<","<<y/cm<<","<<z/cm
	   << ") cm is ("<<vx<<","<<vy<<","<<vz<<") cm/ns"<<G4endl;
  else
    G4cout << "HeedModel::TestSensor() Electron Velocity Error" << G4endl;
  
}

void HeedModel::SetTracking()
{
  if(driftRKF)
    {
      fDriftRKF = new Garfield::DriftLineRKF();
      fDriftRKF->SetSensor(fSensor);
      // const double maxStepSize=0.03;// cm
      // fDriftRKF->SetMaximumStepSize(maxStepSize);
      //      fDriftRKF->EnableStepSizeLimit();
    }
  else if(trackMicro)
    {
      fAvalanche = new Garfield::AvalancheMicroscopic();
      fAvalanche->SetSensor(fSensor);
      fAvalanche->EnableMagneticField();
      fAvalanche->EnableSignalCalculation();
    }
  else
    {  
      fDrift = new Garfield::AvalancheMC();
      fDrift->SetSensor(fSensor);
      fDrift->EnableMagneticField();
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
  G4cout << "CreateCellView()" << G4endl;
  
  viewDrift = new Garfield::ViewDrift();
  viewDrift->SetCanvas(fChamber);
  if(driftRKF) fDriftRKF->EnablePlotting(viewDrift);
  else if(trackMicro) fAvalanche->EnablePlotting(viewDrift);
  else fDrift->EnablePlotting(viewDrift);
  fTrackHeed->EnablePlotting(viewDrift);
}

// void HeedModel::CreateSignalView()
// {
//   char str[30];
//   strcpy(str,fName);
//   strcat(str,"_signal");
//   fSignal = new TCanvas(str, "Signal on the wire", 700, 700);
//   viewSignal = new Garfield::ViewSignal();
//   viewSignal->SetSensor(fSensor);
//   viewSignal->SetCanvas(fSignal);
//   G4cout << "CreateSignalView()" << G4endl;
// }

void HeedModel::CreateFieldView()
{
  char str[30];
  strcpy(str,fName);
  strcat(str,"_efield");
  fField = new TCanvas(fName, "Electric field", 800, 700);
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

void HeedModel::Drift(double &x, double &y, double &z, double &t, int& ions)
{
 if(driftElectrons)
    { 
      double xi,yi,zi,ti,ei,
	xf,yf,zf,tf,ef;
      int status;
      uint prec=G4cout.precision();
      double gain=1.0;

      if(driftRKF)
	{
	  bool stat = fDriftRKF->DriftElectron(x,y,z,t);
	  if( !stat )
	    {
	      std::cerr<<"HeedModel::Drift ERROR: x = "<<x<<" y = "<<y
		       <<" z = "<<z<<" t = "<<t<<std::endl;
	      return;
	    }

          double ne, ni;
          fDriftRKF->GetAvalancheSize(ne,ni);

          if( fVerboseLevel > 1 )
             {
                G4cout.precision(5);
                G4cout << "HeedModel::Drift -- DriftRKF: drift time = " << fDriftRKF->GetDriftTime() 
                       << " ns  gain = " << fDriftRKF->GetGain()
                       << "\tAval Param: " << ne <<","<< ni << G4endl;
                fDriftRKF->GetEndPoint(xf,yf,zf,tf,status);
                G4cout << "\tEndpoint: status: "<<status
                       <<"\t"<<sqrt(xf*xf+yf*yf)<<"\t"<<atan2(yf,xf)*rad_to_deg
                       <<"\t"<<zf<<"\t"<<tf<<G4endl;
                G4cout.precision(prec);
             }
        } // RKF
      else if(trackMicro)
	{
	  fAvalanche->AvalancheElectron(x,y,z,t,0,0,0,0);
	  int ne,ni;
	  fAvalanche->GetAvalancheSize(ne,ni);
	  uint n_endpoints = fAvalanche->GetNumberOfElectronEndpoints();
          if( fVerboseLevel > 1 )
             {
                G4cout<<"HeedModel::Drift -- AvalacheMicro Avalanche Size: #ions = "
                      <<ni<<"; #e- "<<ne
                      <<"\t #endpoints: "<<n_endpoints<<G4endl;
             }
	  gain = 1.;
	  for(uint i=0; i<n_endpoints; ++i)
	    {
	      fAvalanche->GetElectronEndpoint(i, 
					      xi, yi, zi, ti, ei,
					      xf, yf, zf, tf, ef,
					      status);
              if( fVerboseLevel > 2 )
                 {
                    G4cout.precision(5);
                    G4cout<<i<<"\tstatus: "<<status
                          <<"\n\tinit: "
                          <<sqrt(xi*xi+yi*yi)<<"\t"<<atan2(yi,xi)*rad_to_deg<<"\t"
                          <<zi<<"\t"<<ti<<"\t"<<ei
                          <<"\n\tfinal: "
                          <<sqrt(xf*xf+yf*yf)<<"\t"<<atan2(yf,xf)*rad_to_deg<<"\t"
                          <<zf<<"\t"<<tf<<"\t"<<ef<<G4endl;
                    G4cout.precision(prec);
                 }
            }
        }// micro
      else // AvalancheMC - default if driftElectrons is true
	{
	  fDrift->DriftElectron(x,y,z,t);
	  uint ne,ni;
	  fDrift->GetAvalancheSize(ne,ni);
          if( fVerboseLevel > 1 )
             {
                G4cout<<"HeedModel::Drift -- AvalacheMC Avalanche Size: #ions = "
                      <<ni<<"; #e- "<<ne<<G4endl;
             }
          fDrift->GetElectronEndpoint(0, 
				      xi, yi, zi, ti,
				      xf, yf, zf, tf,
				      status);
          if( fVerboseLevel > 1 )
             {   
                G4cout.precision(5);
                G4cout<<"\tstatus: "<<status
                      <<"\n\tinit: "
                      <<sqrt(xi*xi+yi*yi)<<"\t"<<atan2(yi,xi)*rad_to_deg<<"\t"
                      <<zi<<"\t"<<ti
                      <<"\n\tfinal: "
                      <<sqrt(xf*xf+yf*yf)<<"\t"<<atan2(yf,xf)*rad_to_deg<<"\t"
                      <<zf<<"\t"<<tf<<G4endl;
                G4cout.precision(prec);
             }
        }// MC

      if( G4VVisManager::GetConcreteInstance() ) AddTrajectories();

      if( generateSignals && ions > 0) 
	GenerateSignal(xf, yf, zf, tf, gain); 

      AWHit* hit = new AWHit(xf, yf, zf, tf);
      hit->SetGain( gain );
      hit->SetModelName( fName );
      
      fTPCSD->InsertAWHit(hit);
    }
}

void HeedModel::Drift(double &x, double &y, double &z, double &t)
{
  if(driftElectrons)
    { 
      double xi,yi,zi,ti,ei,
	xf,yf,zf,tf,ef;
      int status;
      uint prec=G4cout.precision();
      double gain;
      if(driftRKF)
	{
	  bool stat = fDriftRKF->DriftElectron(x,y,z,t);
	  if( !stat )
	    {
	      std::cerr<<"HeedModel::Drift ERROR: x = "<<x<<" y = "<<y
		       <<" z = "<<z<<" t = "<<t<<std::endl;
	      return;
	    }

	  double drift_time = fDriftRKF->GetDriftTime();
	  //gain = fDriftRKF->GetGain();
          double ne, ni;
          fDriftRKF->GetAvalancheSize(ne,ni);
          gain=ni;
          if( fVerboseLevel > 1 )
             {
                G4cout.precision(5);
                G4cout << "HeedModel::Drift -- DriftRKF: drift time = " << drift_time 
                       << " ns  gain = " << gain 
                       << "\tAval Param: " << ne <<","<< ni << G4endl;
                fDriftRKF->GetEndPoint(xf,yf,zf,tf,status);
                G4cout << "\tEndpoint: status: "<<status
                       <<"\t"<<sqrt(xf*xf+yf*yf)<<"\t"<<atan2(yf,xf)*rad_to_deg
                       <<"\t"<<zf<<"\t"<<tf<<G4endl;
                G4cout.precision(prec);
             }
        }
      else if(trackMicro)
	{
	  fAvalanche->AvalancheElectron(x,y,z,t,0,0,0,0);
	  int ne,ni;
	  fAvalanche->GetAvalancheSize(ne,ni);
	  uint n_endpoints = fAvalanche->GetNumberOfElectronEndpoints();
          if( fVerboseLevel > 1 )
             {
                G4cout<<"HeedModel::Drift -- AvalacheMicro Avalanche Size: #ions = "
                      <<ni<<"; #e- "<<ne
                      <<"\t #endpoints: "<<n_endpoints<<G4endl;
             }
	  gain = 1.;
	  for(uint i=0; i<n_endpoints; ++i)
	    {
	      fAvalanche->GetElectronEndpoint(i, 
					      xi, yi, zi, ti, ei,
					      xf, yf, zf, tf, ef,
					      status);
              if( fVerboseLevel > 2 )
                 {
                    G4cout.precision(5);
                    G4cout<<i<<"\tstatus: "<<status
                          <<"\n\tinit: "
                          <<sqrt(xi*xi+yi*yi)<<"\t"<<atan2(yi,xi)*rad_to_deg<<"\t"
                          <<zi<<"\t"<<ti<<"\t"<<ei
                          <<"\n\tfinal: "
                          <<sqrt(xf*xf+yf*yf)<<"\t"<<atan2(yf,xf)*rad_to_deg<<"\t"
                          <<zf<<"\t"<<tf<<"\t"<<ef<<G4endl;
                    G4cout.precision(prec);
                 }
	      GenerateSignal(xf, yf, zf, tf, gain);
	      AWHit* hit = new AWHit(xf, yf, zf, tf);
	      hit->SetGain( gain );
	      hit->SetModelName( fName );
	      fTPCSD->InsertAWHit(hit);
	    }
        }
      else // AvalancheMC - default if driftElectrons is true
	{
	  fDrift->DriftElectron(x,y,z,t);
	  uint ne,ni;
	  fDrift->GetAvalancheSize(ne,ni);
          if( fVerboseLevel > 1 )
             {
                G4cout<<"HeedModel::Drift -- AvalacheMC Avalanche Size: #ions = "
                      <<ni<<"; #e- "<<ne<<G4endl;
             }
	  gain = double(ni);
	  fDrift->GetElectronEndpoint(0, 
				      xi, yi, zi, ti,
				      xf, yf, zf, tf,
				      status);
          if( fVerboseLevel > 1 )
             {   
                G4cout.precision(5);
                G4cout<<"\tstatus: "<<status
                      <<"\n\tinit: "
                      <<sqrt(xi*xi+yi*yi)<<"\t"<<atan2(yi,xi)*rad_to_deg<<"\t"
                      <<zi<<"\t"<<ti
                      <<"\n\tfinal: "
                      <<sqrt(xf*xf+yf*yf)<<"\t"<<atan2(yf,xf)*rad_to_deg<<"\t"
                      <<zf<<"\t"<<tf<<G4endl;
                G4cout.precision(prec);
             }
        }

      if( G4VVisManager::GetConcreteInstance() ) AddTrajectories();

      if(trackMicro) return;

      if( generateSignals ) 
	GenerateSignal(xf, yf, zf, tf, gain); 
      AWHit* hit = new AWHit(xf, yf, zf, tf);
      hit->SetGain( gain );
      hit->SetModelName( fName );
      // if( fTPCSD->InsertAWHit(hit) ) G4cout << "HeedModel::ProcessEvent() New Hit" << G4endl;
      // else G4cerr << "HeedModel::ProcessEvent() problem with AWhits" << G4endl;
      fTPCSD->InsertAWHit(hit);
    }
}

void HeedModel::AddTrajectories()
{
  if(driftElectrons)
    {
      DriftLineTrajectory* dlt = new DriftLineTrajectory();
      G4TrackingManager* fpTrackingManager = G4EventManager::GetEventManager()->GetTrackingManager();
      fpTrackingManager->SetTrajectory(dlt);
      if(driftRKF)
	{
	  unsigned int n = fDriftRKF->GetNumberOfDriftLinePoints();
	  G4cout << "HeedModel::AddTrajectories driftRKF: " << n << G4endl;
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
	  unsigned int nLines = fAvalanche->GetNumberOfElectronEndpoints();
	  G4cout << "HeedModel::AddTrajectories trackMicro: " << nLines << G4endl;
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
	  unsigned int n = fDrift->GetNumberOfDriftLinePoints();
	  G4cout << "HeedModel::AddTrajectories AvalancheMC: " << n << G4endl;
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

void HeedModel::GenerateSignal(double &x, double &y, double &z, double &t, double& g)
{
  double radmm = sqrt(x*x+y*y)*10.;
  if( fabs( radmm - 182.) > 0.1 ) return;

  double phi = atan2(y,x);
  uint aw = fDet->GetTPC()->FindAnode(phi);
  fsg->AddAnodeSignal(aw,t,g);

  std::pair<int,int> pad = fDet->GetTPC()->FindPad( z, phi );
  double zmm = z*10.;
  //  std::pair<int,int> pad = fDet->GetTPC()->FindPad( zmm, phi );
  fsg->AddPadSignal(pad,t,g,zmm);
  //fsg->AddPadSignal(pad,t,g,z);
  
  if( fVerboseLevel > 1 )
     G4cout<<"HeedModel::GenerateSignal aw: "<<aw<<" pad: ("<<pad.first<<","<<pad.second<<")\tgain: "<<g<<G4endl;

  isReadout = true;
}

void HeedModel::Reset()
{
  G4cout << "HeedModel::Reset()" << G4endl;
  fsg->Reset();
}

bool HeedModel::Readout()
{
  G4cout << "HeedModel::Readout() Forbidden" << G4endl;
  return false;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
