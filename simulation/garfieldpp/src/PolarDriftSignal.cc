#include <iostream>
#include <string>

#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TMath.h>

#include "Garfield/SolidTube.hh"
#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/DriftLineRKF.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/ViewCell.hh"
#include "Garfield/FundamentalConstants.hh"

using namespace std;
using namespace Garfield;

int main(int argc, char * argv[]) 
{ 
  double InitialPhi = 0., // rad
    InitialZed = 0.; // cm
  string tracking="driftMC";
  bool plot=false;

  double pressure=725.; // Torr @ CERN

  if( argc == 3 )
    {
      InitialPhi     = atof(argv[1])*TMath::DegToRad();
      InitialZed     = atof(argv[2]);
    }
  else if( argc == 4 )
    {
      InitialPhi     = atof(argv[1])*TMath::DegToRad();
      InitialZed     = atof(argv[2]);
      tracking       = argv[3];
    }
 else if( argc == 5 )
    {
      InitialPhi     = atof(argv[1])*TMath::DegToRad();
      InitialZed     = atof(argv[2]);
      tracking       = argv[3];
      plot           = bool(atoi(argv[4]));
    }

  cerr<<"============================="<<endl;
  cerr<<"\t"<<argv[0]<<endl;
  cerr<<"============================="<<endl;


  TApplication* app;
  if(plot) app = new TApplication("PolarDriftSignal", &argc, argv);

  // Make a gas medium.
  MediumMagboltz gas;
  TString gasdata = TString::Format("%s/simulation/common/gas_files/ar_70_co2_30_725Torr_20E200000_4B1.10.gas",getenv("AGRELEASE"));
  gas.LoadGasFile(gasdata.Data());
  gas.SetPressure(pressure);
  TString iondata = TString::Format("%s/Data/IonMobility_Ar+_Ar.txt",
				     getenv("GARFIELD_HOME"));
  gas.LoadIonMobility(iondata.Data());


  // Define the cell layout.
  constexpr double lZ = 115.2;
  ComponentAnalyticField cmp;
  Sensor sensor;
  sensor.AddComponent(&cmp);

  cmp.SetMedium(&gas);
  cmp.SetMagneticField(0, 0, 1);
  cmp.SetPolarCoordinates();
  // Outer wall.
  constexpr double rRO = 19.0;
  cmp.AddPlaneR(rRO, 0, "ro");
  cmp.AddReadout("ro");
  // sensor.AddElectrode(&cmp, "ro");
  // Inner wall.
  constexpr double rCathode = 10.925;
  cmp.AddPlaneR(rCathode, -4000., "c");
  constexpr int nWires = 256;
  // Phi periodicity.
  const double sphi = 360. / double(nWires);
 
  // Field wires.
  // Radius [cm]
  constexpr double rFW = 17.4;
  // Diameter [cm]
  constexpr double dFW = 152.e-4;
  // Potential [V]
  constexpr double vFW = -110.;
  // Tension [g]
  constexpr double tFW = 120.;

  // Anode wires.
  constexpr double rAW = 18.2;
  constexpr double dAW = 30.e-4;
  constexpr double vAW = 3200.;
  constexpr double tAW = 40.;

  // Add the wires.
  for (int i = 0; i < nWires; ++i) 
    { 
      // Field wires.
      cmp.AddWire(rFW, i * sphi, dFW, vFW, "f", 2 * lZ);
      // Anode wires.
      auto ename = std::string(TString::Format("a%03d", i).Data());
      cmp.AddWire(rAW, (i + 0.5) * sphi, dAW, vAW, ename, 2 * lZ);
      cmp.AddReadout(ename);
      sensor.AddElectrode(&cmp, ename);
  }

  // // Pads.
  // constexpr double gap = rRO - rAW;
  // constexpr int nSecs = 32;
  // constexpr double pitchPhi = TwoPi / nSecs;
  // constexpr double pitchZ = 0.4;
  // constexpr int nRows = int(2 * lZ / pitchZ);
  // std::cout << "Number of pad rows: " << nRows << std::endl;
  // for (int j = 0; j < nRows; ++j) 
  //   {
  //     const double z0 = -lZ + j * pitchZ;
  //     const double z1 = z0 + pitchZ;
  //     std::string row = std::string(TString::Format("%03d", j).Data());
  //     for (int i = 0; i < nSecs; ++i) 
  // 	{
  // 	  std::string sec = std::string(TString::Format("%02d", i).Data());
  // 	  const double phi0 = i * pitchPhi * RadToDegree; 
  // 	  const double phi1 = phi0 + pitchPhi * RadToDegree;
  // 	  std::string ename = "pad" + row + "_" + sec;
  // 	  cmp.AddPixelOnPlaneR(rRO, phi0, phi1, z0, z1, ename, gap);
  // 	  cmp.AddReadout(ename);
  // 	  sensor.AddElectrode(&cmp, ename);
  // 	}
  //   }

  TCanvas cDrift;
  ViewCell cellView;
  ViewDrift viewdrift;

  if( plot ) {
    TString cname = TString::Format("Polar_%s_Signal",tracking.c_str());
    cDrift.SetName(cname);
   
    cellView.SetCanvas(&cDrift);
    cellView.SetComponent(&cmp);
    cellView.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
		     1.1 * rRO,  1.1 * rRO,  1.);
    cellView.EnableWireMarkers(false);
    
    
    // Construct object to visualise drift lines
    viewdrift.SetCanvas(&cDrift);
    viewdrift.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
		      1.1 * rRO,  1.1 * rRO,  1.);
  }

  //----------------------------------------------------
  // Transport Class for Electrons drift
  // Runge-Kutta
  DriftLineRKF edrift;
  edrift.SetSensor(&sensor);
  const double maxStepSize=0.03;// cm
  edrift.SetMaximumStepSize(maxStepSize);
  if(plot) edrift.EnablePlotting(&viewdrift);
  //  edrift.EnableDebugging();
  //----------------------------------------------------
  // Avalanche MC
  AvalancheMC eaval;
  eaval.SetSensor(&sensor);
  eaval.EnableMagneticField();
  // const double distanceStep = 2.e-3; // cm
  // eaval.SetDistanceSteps(distanceStep);
  if(plot) eaval.EnablePlotting(&viewdrift);

  // Electron initial point
  double ri=rCathode;
  double xi,yi,zi=InitialZed,ti=0.0,phii = InitialPhi;
  double Rstep = 0.05, Rministep = 0.01;
  int ie = 0;
  cout<<"\nBEGIN"<<endl;
  TString fname = TString::Format("%s/polar_driftsignal/drift_tables/PolarDriftLine_%s_phi%1.4f_Z%2.1fcm.dat",getenv("GARFIELDPP"),
				  tracking.c_str(),
				  InitialPhi,InitialZed);
  ofstream fout(fname.Data());
  cout<<"I'M SETUP to USE "<<tracking<<endl;
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
	  gain=ni;
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

      fout<<xi<<"\t"<<yi<<"\t"<<phif<<"\t"<<tf<<"\t"<<gain<<endl;
      cout<<ss.str();
      ri+=Rstep;
    }
  fout.close();
  cout<<"END"<<endl;
  cout<<"Number of Clusters: "<<ie<<endl;

  if( tracking.compare("avalMC") && plot )
    {
      cout<<"Plot Driftlines"<<endl;
      viewdrift.Plot(true,true);
      cellView.Plot2d();
      TString cname = TString::Format("%s/polar_driftsignal/drift_plots/PolarDrift_phi%1.4f_Z%2.1fcm.png",getenv("GARFIELDPP"),InitialPhi,InitialZed);
      cDrift.SaveAs(cname.Data());
      cout<<cname<<" saved"<<endl;
      app->Run(true);
    }

  return 0;
}
