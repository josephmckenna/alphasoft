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
							   fDet(dc), fTPCSD(sd), fName(modelName)
{
  fMaxRad = fDet->GetTPC()->GetROradius();
  fMinRad = fDet->GetTPC()->GetCathodeRadius();
  fLen = fDet->GetTPC()->GetFullLengthZ();

  fBinWidth = 16.; // ns
  fNbins = 411;
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

  TestSensor();

  // Request signal calculation for the electrode named with labels above,
  // using the weighting field provided by the Component object cmp.
  std::vector<std::string> anodes = fDet->GetTPC()->GetAnodeReadouts();
  for(unsigned int a=0; a < anodes.size(); ++a)
    fSensor->AddElectrode(fDet->GetTPC(),anodes[a]);
  fSensor->AddElectrode(fDet->GetTPC(), "ro");

  G4cout << "HeedModel::AddSensor() --> # of Electrodes: " << fSensor->GetNumberOfElectrodes() << G4endl;

  // Set Time window for signal integration, units in [ns]
  fSensor->SetTimeWindow(0., fBinWidth, fNbins);
  
  fSensor->SetTransferFunction(Hands);
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
      const double maxStepSize=0.03;// cm
      fDriftRKF->SetMaximumStepSize(maxStepSize);
      fDriftRKF->EnableStepSizeLimit();
    }
  else if(trackMicro)
    {
      fAvalanche = new Garfield::AvalancheMicroscopic();
      fAvalanche->SetSensor(fSensor);
      fAvalanche->EnableMagneticField();
    }
  else
    {  
      fDrift = new Garfield::AvalancheMC();
      fDrift->SetSensor(fSensor);
      fDrift->EnableMagneticField();
      fDrift->SetDistanceSteps(2.e-3);
      if(createAval) fDrift->EnableAttachment();
      else fDrift->DisableAttachment();
    }

  if( generateSignals )
    {
      fIonDrift = new Garfield::AvalancheMC();
      fIonDrift->SetSensor(fSensor);
      fIonDrift->EnableMagneticField();
      fIonDrift->EnableSignalCalculation();
      const double dist_step = 2.e-4;// cm = 2 um
      fIonDrift->SetDistanceSteps(dist_step);
      G4cout << "HeedModel::SetTracking() Signal Calculation Enabled" << G4endl;
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

  if( generateSignals )
    fIonDrift->EnablePlotting(viewDrift);
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
  G4cout << "CreateSignalView()" << G4endl;
}

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

void HeedModel::Drift(double x, double y, double z, double t)
{
  if(driftElectrons)
    { 
      double xi,yi,zi,ti,ei,
	xf,yf,zf,tf,ef;
      int status;
      uint prec=G4cout.precision();
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
	  double gain = fDriftRKF->GetGain();
	  G4cout.precision(5);
	  G4cout << "HeedModel::Drift -- DriftRKF: drift time = " << drift_time 
		 << " ns  gain = " << gain << G4endl;
	  fDriftRKF->GetEndPoint(xi,yi,zi,ti,status);
	  G4cout << "\tEndpoint: status: "<<status
		 <<"\t"<<sqrt(xi*xi+yi*yi)<<"\t"<<atan2(yi,xi)*rad_to_deg
		 <<"\t"<<zi<<"\t"<<ti<<G4endl;
	  G4cout.precision(prec);
	  if( generateSignals && gain > 0. )
	    {
	      fIonDrift->SetIonSignalScalingFactor(gain/double(fNions));
	      for(int j=0; j<fNions; ++j)
		{  
		  fIonDrift->DriftIon(xi, yi, zi, ti);
		}
	    }
        }
      else if(trackMicro)
	{
	  fAvalanche->AvalancheElectron(x,y,z,t,0,0,0,0);
	  int ne,ni;
	  fAvalanche->GetAvalancheSize(ne,ni);
	  G4cout<<"HeedModel::Drift -- AvalacheMicro Avalanche Size: #ions = "
		<<ni<<"; #e- "<<ne<<G4endl;
	  if( generateSignals )
	    fIonDrift->SetIonSignalScalingFactor(double(ni)/double(fNions));
	  for(uint i=0; i<fAvalanche->GetNumberOfElectronEndpoints(); ++i)
	    {
	      fAvalanche->GetElectronEndpoint(i, 
					      xi, yi, zi, ti, ei,
					      xf, yf, zf, tf, ef,
					      status);	      
	      G4cout.precision(5);
	      G4cout<<i<<"\tstatus: "<<status
		    <<"\n\tinit: "
		    <<sqrt(xi*xi+yi*yi)<<"\t"<<atan2(yi,xi)*rad_to_deg<<"\t"
		    <<zi<<"\t"<<ti<<"\t"<<ei
		    <<"\n\tfinal: "
		    <<sqrt(xf*xf+yf*yf)<<"\t"<<atan2(yf,xf)*rad_to_deg<<"\t"
		    <<zf<<"\t"<<tf<<"\t"<<ef<<G4endl;
	      G4cout.precision(prec);
	      // signal calculation
	      if( generateSignals )
		{
		  if( int(i) >= fNions ) break;
		  fIonDrift->DriftIon(xi,yi,zi,ti);
		  if( i%100 == 0 ) 
		    G4cout<<"HeedModel::Drift -- AvalancheMicro Generate Signal"<<G4endl;
		}
	    }
        }
      else // AvalancheMC - default if driftElectrons is true
	{
	  fDrift->DriftElectron(x,y,z,t);
	  uint ne,ni;
	  fDrift->GetAvalancheSize(ne,ni);
	  G4cout<<"HeedModel::Drift -- AvalacheMC Avalanche Size: #ions = "
		<<ni<<"; #e- "<<ne<<G4endl;
	  fDrift->GetElectronEndpoint(0, 
				      xi, yi, zi, ti,
				      xf, yf, zf, tf,
				      status);	      
	  G4cout.precision(5);
	  G4cout<<"\tstatus: "<<status
		<<"\n\tinit: "
		<<sqrt(xi*xi+yi*yi)<<"\t"<<atan2(yi,xi)*rad_to_deg<<"\t"
		<<zi<<"\t"<<ti
		<<"\n\tfinal: "
		<<sqrt(xf*xf+yf*yf)<<"\t"<<atan2(yf,xf)*rad_to_deg<<"\t"
		<<zf<<"\t"<<tf<<G4endl;
	  G4cout.precision(prec);
	  if( createAval )
	    {
	      fIonDrift->SetIonSignalScalingFactor(double(ni)/double(fNions));
	      if( generateSignals && ni )
		{
		  double rmin = 0.5*fDet->GetTPC()->GetDiameterAnodeWires();
		  double rmax = rmin*5.;
		  //		  double phi_f = atan2(yf,xf)+0.5*pi;
		  //		  double phi_f = atan2(yf,xf);
		  G4cout<<"HeedModel::Drift -- AvalancheMC Generate Signal"<<G4endl;
		  for(int j=0; j<fNions; ++j)
		    {  
		      //double phi = twopi*G4UniformRand();
		      //double phi = G4RandFlat::shoot( -phi_f,phi_f );
		      //double phi = G4RandGauss::shoot( phi_f, 2.3e-5 );
		      double phi = G4RandGauss::shoot( 0., 2.3e-5 );
		      double rad = G4RandomRadiusInRing(rmin,rmax);
		      double xx = rad*cos(phi) + xf,
		       	yy = rad*sin(phi) + yf,
		       	zz = G4RandGauss::shoot(zf,7.e-4);
		      fIonDrift->DriftIon(xx,yy,zz,tf);
		      if( j%10 == 0 )
			{
			  G4cout.precision(5);
			  G4cout<<"\t ion: ("<<xx<<","<<yy<<","<<zz
				<<")\t r: "<<sqrt(xx*xx+yy*yy)
				<<" phi: "<<atan2(yy,xx)*rad_to_deg<<G4endl;
			  G4cout.precision(prec);
			}
		      // fIonDrift->DriftIon(xi, yi, zi, ti);
		    }
		}
	    }
	  else
	    G4cerr<<"HeedModel::Drift -- AvalacheMC cannot estimate avalanche size"<<G4endl;
        }
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

void HeedModel::PlotSignal(G4String electrode, G4String fileName)
{
  if(fVisualizeSignal)
    {
      G4cout << "HeedModel::PlotSignal " << fName << G4endl;
      viewSignal->PlotSignal(electrode);
      fSignal->Print(fileName.c_str());  
    }
}

void HeedModel::ProcessEvent()
{
  G4cout << "HeedModel::ProcessEvent()" << G4endl;
  if( G4VVisManager::GetConcreteInstance() )
    AddTrajectories();

  G4String plotName = G4String(fName) + "_track.pdf";
  PlotTrack(plotName);

  if( !generateSignals ) return;
   
  fSensor->ConvoluteSignal();
  double Tstart, BinWidth;
  unsigned int nBins;
  fSensor->GetTimeWindow(Tstart, BinWidth, nBins);
  G4cout << "HeedModel::ProcessEvent() # of bins: " << nBins << G4endl;

  double a, min=9.e9;
  G4String plotElectrode="";
  for(int w=0; w<fDet->GetTPC()->GetNumberOfAnodeWires(); ++w)
    {
      G4String wname="a"+std::to_string(w);
      std::vector<double> data;
      for(uint b=0; b<nBins; ++b)
	{
	  a = fSensor->GetSignal(wname.c_str(),b);
	  data.push_back( a );
	}
      AWHit* hit = new AWHit();
      hit->SetAnode( w );
      hit->SetWaveform( data );
      hit->SetModelName( fName );
      // if( fTPCSD->InsertAWHit(hit) ) G4cout << "HeedModel::ProcessEvent() New Hit" << G4endl;
      // else G4cerr << "HeedModel::ProcessEvent() problem with AWhits" << G4endl;
      fTPCSD->InsertAWHit(hit);
      double temp_min = *std::min_element(data.begin(),data.end());
      if( temp_min<min )
	{
	  plotElectrode = wname;
	  min = temp_min;
	}
      // G4cout << "\t" << wname << " size: " << data.size() 
      // 	     << " min: " << temp_min
      // 	     << " max: " << *std::max_element(data.begin(),data.end())
      // 	     << G4endl;
    }

  if( plotElectrode == "" )
    plotElectrode = "ro";
  plotName = G4String(fName) + "_" + plotElectrode + "_sig.pdf";

  PlotSignal(plotElectrode,plotName);
}

void HeedModel::Reset()
{
  G4cout << "HeedModel::Reset()" << G4endl;
  fSensor->ClearSignal();
  fSensor->NewSignal();
}
