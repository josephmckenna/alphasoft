#include <iostream>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TMath.h>

#include "Plotting.hh"

#include "ComponentAnalyticField.hh"
#include "MediumMagboltz.hh"
#include "SolidTube.hh"
#include "GeometrySimple.hh"
#include "Sensor.hh"
#include "FundamentalConstants.hh"
#include "ViewField.hh"
#include "ViewCell.hh"

#include "DriftLineRKF.hh"
#include "AvalancheMC.hh"
#include "ViewDrift.hh"

using namespace Garfield;

int main(int argc, char * argv[]) 
{ 
  double InitialPhi = 0., // rad
    InitialZed = 0.; // cm

  string tracking="driftMC";

  if( argc == 3 )
    {
      InitialPhi     = atof(argv[1])*TMath::DegToRad();
      InitialZed     = atof(argv[2]);
    }
  else if( argc == 4 )
    {
      InitialPhi     = atof(argv[1])*TMath::DegToRad();
      InitialZed     = atof(argv[2]);

      tracking = argv[3];
    }

  TApplication app("app", &argc, argv);
  plottingEngine.SetDefaultStyle();
 
  // Make a gas medium.
  MediumMagboltz gas;
  TString gasdata = TString::Format("%s/simulation/common/gas_files/ar_70_co2_30_725Torr_20E200000_4B1.10.gas",getenv("AGRELEASE"));
  gas.LoadGasFile(gasdata.Data());
  TString iondata = TString::Format("%s/Data/IonMobility_Ar+_Ar.txt",
				     getenv("GARFIELD_HOME"));
  gas.LoadIonMobility(iondata.Data());

  // Build the geometry.
  constexpr double lZ = 115.2;
  //  constexpr double rRO = 19.03;
  constexpr double rRO = 19.;
  GeometrySimple geo;
  SolidTube tube(0, 0, 0, 0, rRO, lZ);
  geo.AddSolid(&tube, &gas);

  ComponentAnalyticField cmp;
  cmp.SetGeometry(&geo);
  cmp.SetMagneticField(0.,0.,1.);
  cmp.SetPolarCoordinates();
  // Outer wall.
  cmp.AddPlaneR(rRO, 0, "ro");
  // Inner wall.
  constexpr double rCathode = 10.925;
  cmp.AddPlaneR(rCathode, -4000., "c");
  // Phi periodicity.
  const double sphi = 360. / 256.;

  // Field wires.
  // Radius [cm]
  constexpr double rFW = 17.4;
  // Diameter [cm]
  constexpr double dFW = 152.e-4;
  // Potential [V]
  constexpr double vFW = -99.;
  // Tension [g]
  constexpr double tFW = 120.;

  // Anode wires.
  constexpr double rAW = 18.2;
  constexpr double dAW = 30.e-4;
  constexpr double vAW = 3100.;
  constexpr double tAW = 40.;

  cmp.AddWire(rFW, 0, dFW, vFW, "f", 2 * lZ, tFW);
  cmp.AddWire(rAW, 0.5 * sphi, dAW, vAW, "a", 2 * lZ, tAW, 19.25); 
  cmp.AddReadout("a");
  cmp.SetPeriodicityPhi(sphi);
  cmp.PrintCell();

  TCanvas cDrift;

  ViewCell cellView;
  cellView.SetCanvas(&cDrift);
  cellView.SetComponent(&cmp);
  cellView.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
                    1.1 * rRO,  1.1 * rRO,  1.);
  cellView.EnableWireMarkers(false);


  // Finally assembling a Sensor object
  Sensor sensor;
  // Calculate the electric field
  sensor.AddComponent(&cmp);
  sensor.AddElectrode(&cmp,"a");

  // Construct object to visualise drift lines
  ViewDrift viewdrift;
  viewdrift.SetCanvas(&cDrift);
  viewdrift.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
                    1.1 * rRO,  1.1 * rRO,  1.);

  cout<<"I'M SETUP to USE "<<tracking<<endl;

  //----------------------------------------------------
  // Transport Class for Electrons drift
  // Runge-Kutta
  DriftLineRKF edrift;
  edrift.SetSensor(&sensor);
  // const double maxStepSize=0.03;// cm
  // edrift.SetMaximumStepSize(maxStepSize);
  edrift.EnablePlotting(&viewdrift);
  //  edrift.EnableDebugging();
  //----------------------------------------------------
  // Avalanche MC
  AvalancheMC eaval;
  eaval.SetSensor(&sensor);
  eaval.EnableMagneticField();
  // const double distanceStep = 2.e-3; // cm
  // eaval.SetDistanceSteps(distanceStep);
  eaval.EnablePlotting(&viewdrift);

  // Electron initial point
  double ri=rCathode;
  double xi,yi,zi=InitialZed,ti=0.0,phii = InitialPhi;
  double Rstep = 0.05, Rministep = 0.01;
  int ie = 0;
  cout<<"\nBEGIN"<<endl;
  TString fname = TString::Format("./PolarDriftLine_%s_phi%1.4f_Z%2.1fcm.dat",
				  tracking.c_str(),
				  InitialPhi,InitialZed);
  ofstream fout(fname.Data());
  fout<<"I'M SETUP to USE "<<tracking<<endl;
  // variables to identify the aw
  int aw=-1;
  double AnodeWiresPitch = sphi*TMath::DegToRad();
  while(ri<rRO)
    {
      ++ie;
      xi=ri*TMath::Cos(phii);
      yi=ri*TMath::Sin(phii);
      cout<<ie<<")\tstart @ ("<<xi<<","<<yi<<","<<zi<<") cm\t";

      bool ok = false;
      if( !tracking.compare("driftMC") )
	ok = eaval.DriftElectron(xi,yi,zi,ti);
      else if( !tracking.compare("driftRKF") )
	ok = edrift.DriftElectron(xi,yi,zi,ti);
      else if( !tracking.compare("avalMC") )
	ok = eaval.AvalancheElectron(xi,yi,zi,ti);
      else
	{
	  cerr<<tracking<<" UNKNOWN"<<endl;
	  break;
	}

      if( !ok ) 
	{
	  cerr<<tracking<<" FAILED"<<endl;
	  --ie;
	  ri+=Rministep;
	  continue;
	}

      // if( !edrift.DriftElectron(xi,yi,zi,ti) ) continue;
      // if( !eaval.DriftElectron(xi,yi,zi,ti) ) continue;
      // if( !eaval.AvalancheElectron(xi,yi,zi,ti) ) continue;

      double xf,yf,zf,tf,phif;
      int status;
      
      if( !tracking.compare("driftMC") || !tracking.compare("avalMC") )
	eaval.GetElectronEndpoint(0, 
				  xi, yi, zi, ti,
				  xf, yf, zf, tf,
				  status);
      else if( !tracking.compare("driftRKF") )
	edrift.GetEndPoint(xf,yf,zf,tf,status);
      else break;
	
      assert(TMath::Abs(TMath::Sqrt(xi*xi+yi*yi)-ri)<2.e-3);
      assert(zi==InitialZed);
      assert(ti==0.0);
      phif=TMath::ATan2(yf,xf);

      // find anode wire
      if( phif < 0. ) phif += TMath::TwoPi();
      double w = phif/AnodeWiresPitch-0.5;
      aw = (ceil(w)-w)<(w-floor(w))?int(ceil(w)):int(floor(w));
      //cout<<"hit: "<<w<<"\t"<<aw<<endl;
      cout<<"hit wire: "<<aw<<endl;

      double ne, ni, t_d=-1., gain, spread, loss;

      if( !tracking.compare("driftMC") || !tracking.compare("avalMC") )
	{
	  unsigned int une,uni;
	  eaval.GetAvalancheSize(une,uni);
	  ne=double(une);
	  ni=double(uni);
	}
      else if( !tracking.compare("driftRKF") )
	{
	  edrift.GetAvalancheSize(ne,ni);
	  // drift time
	  t_d=edrift.GetDriftTime();
	  // gain
	  gain=edrift.GetGain();
	  // arrival time distribution
	  spread = edrift.GetArrivalTimeSpread();
	  // attachment loss
	  loss = edrift.GetLoss();
	}
      else break;
      
      double lorentz_correction = (phif-InitialPhi);
      if( lorentz_correction < 0. ) lorentz_correction += TMath::TwoPi();
      else if( lorentz_correction > TMath::Pi() ) 
	lorentz_correction = TMath::TwoPi() - lorentz_correction;
      lorentz_correction *= TMath::RadToDeg();

      stringstream ss;
      ss<<ie<<")\t"<<status<<"\t"
	<<ri<<" cm\t"<<tf<<" ns\tw: "<<aw<<"\t"
	<<lorentz_correction<<" deg\tz: "<<zf
	<<" cm\tAval Param: "<<ne<<","<<ni;

      if( !tracking.compare("driftRKF") )
	{
	  ss <<"\tsigma_t: "<<spread
	     <<" ns\tloss: "<<loss<<"\tgain: "<<gain<<endl;
	  if( t_d != tf ) 
	    {
	      cerr<<"Drift Time "<<t_d<<" ERROR: "<<tf<<endl;
	      ss<<"* tf:"<<tf<<endl;
	    }
	}
      else
	ss << "\n";

      fout<<ss.str();
      cout<<ss.str();
      ri+=Rstep;
    }
  fout.close();
  cout<<"END"<<endl;
  cout<<"Number of Clusters: "<<ie<<endl;

  if( tracking.compare("avalMC") )
    {
      cout<<"Plot Driftlines"<<endl;
      viewdrift.Plot(true,true);
      cellView.Plot2d();
      TString cname = TString::Format("./PolarDrift_phi%1.4f_Z%2.1fcm.png",
				      InitialPhi,InitialZed);
      cDrift.SaveAs(cname.Data());
      cout<<cname<<" saved"<<endl;
      app.Run(true);
    }

  return 0;
}

