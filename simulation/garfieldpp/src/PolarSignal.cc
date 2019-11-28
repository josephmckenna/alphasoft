#include <iostream>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <vector>

using namespace std;

#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TGraph.h>
#include <TAxis.h>

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
#include "ViewSignal.hh"

#include "Helpers.hh"

using namespace Garfield;

int main(int argc, char * argv[]) 
{ 
  double InitialPhi = 0., // rad
    InitialZed = 0.; // cm

  if( argc == 3 )
    {
      InitialPhi     = atof(argv[1])*TMath::DegToRad();
      InitialZed     = atof(argv[2]);
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
  constexpr double rRO = 19.03;
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

  /*
  for (unsigned int i = 0; i < 256; ++i) {
    const double phi = i * sphi;
    cmp.AddWire(rFW, phi, dFW, vFW, "f", 2 * lZ, tFW);
    cmp.AddWire(rAW, phi + 0.5 * sphi, dAW, vAW, "a", 2 * lZ, tAW, 19.25); 
  }
  */
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

  const double tStart = 0.;
  const int nSteps = 411;
  const double tStep = 16.;

  // Finally assembling a Sensor object
  Sensor sensor;
  // Calculate the electric field
  sensor.AddComponent(&cmp);
  sensor.AddElectrode(&cmp,"a");
  sensor.SetTimeWindow(tStart, tStep, nSteps);
  sensor.SetTransferFunction(Hands);

  // Construct object to visualise drift lines
  ViewDrift viewdrift;
  viewdrift.SetCanvas(&cDrift);
  viewdrift.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
                    1.1 * rRO,  1.1 * rRO,  1.);

  TCanvas cSignal;
  // Plot the induced current.
  ViewSignal* signalView = new ViewSignal();
  signalView->SetCanvas(&cSignal);
  signalView->SetSensor(&sensor);

  //----------------------------------------------------
  // Transport Class for Electrons drift
  // // Runge-Kutta
  // DriftLineRKF edrift;
  // edrift.SetSensor(&sensor);
  // const double maxStepSize=0.03;// cm
  // edrift.SetMaximumStepSize(maxStepSize);
  // //  edrift.EnableStepSizeLimit();
  // edrift.EnablePlotting(&viewdrift);
  //----------------------------------------------------
  // Avalanche MC
  AvalancheMC eaval;
  eaval.SetSensor(&sensor);
  eaval.EnableMagneticField();
  eaval.EnableSignalCalculation();
  eaval.SetDistanceSteps(2.e-3);
  eaval.EnablePlotting(&viewdrift);

   // Electron initial point
  double xi,yi,zi=InitialZed,ti=0.0,phii = InitialPhi;
  double Rstep = 0.05;
  int ie = 0;
  cout<<"\nBEGIN"<<endl;
  TString fname = TString::Format("./PolarDriftLine_phi%1.4f_Z%2.1fcm.dat",
				  InitialPhi,InitialZed);
  ofstream fout(fname.Data());
  for(double ri=rCathode; ri<rRO; ri+=Rstep)
    {
      ++ie;
      xi=ri*TMath::Cos(phii);
      yi=ri*TMath::Sin(phii);
      cout<<ie<<")\tstart @ ("<<xi<<","<<yi<<","<<zi<<") cm"<<endl;
      //      if( !edrift.DriftElectron(xi,yi,zi,ti) ) continue;
      //if( !eaval.DriftElectron(xi,yi,zi,ti) ) continue;
      if( !eaval.AvalancheElectron(xi,yi,zi,ti) ) continue;

      double xf,yf,zf,tf,phif;
      int status;
      // edrift.GetEndPoint(xf,yf,zf,tf,status);
      eaval.GetElectronEndpoint(0, 
				xi, yi, zi, ti,
				xf, yf, zf, tf,
				status);
      assert(TMath::Abs(TMath::Sqrt(xi*xi+yi*yi)-ri)<2.e-3);
      assert(zi==InitialZed);
      assert(ti==0.0);
      phif=TMath::ATan2(yf,xf);

      // double ne,ni;
      // edrift.GetAvalancheSize(ne,ni);

      unsigned int ne,ni;
      eaval.GetAvalancheSize(ne,ni);

      // // drift time
      // double t_d=edrift.GetDriftTime();
      // // gain
      // double gain=edrift.GetGain();
      // // arrival time distribution
      // double spread = edrift.GetArrivalTimeSpread();
      // // attachment loss
      // double loss = edrift.GetLoss();
      
      double lorentz_correction = (phif-InitialPhi);
      if( lorentz_correction < 0. ) lorentz_correction += TMath::TwoPi();
      lorentz_correction *= TMath::RadToDeg();

      stringstream ss;
      ss<<ie<<")\t"<<status<<"\t"
	  <<ri<<" cm\t"<<tf<<" ns\t"<<lorentz_correction
	<<" deg\tz: "<<zf<<" cm\tAval Param: "<<ne<<","<<ni<<endl;
      //      <<"\tsigma_t: "<<spread
      // <<" ns\tloss: "<<loss<<"\tgain: "<<gain<<endl;

      // if( t_d != tf ) 
      // 	ss<<"* tf:"<<tf<<endl;

      fout<<ss.str();
      cout<<ss.str();
    }
  fout.close();
  cout<<"END"<<endl;
  cout<<"Number of Clusters: "<<ie<<endl;
  
  cout<<"Plot Signal"<<endl;
  sensor.ConvoluteSignal();
  signalView->PlotSignal("a");
  TString cname = TString::Format("./PolarSignal_signal_phi%1.4f_Z%2.1fcm.pdf",
				  InitialPhi,InitialZed);
  cSignal.SaveAs(cname.Data());
  cout<<cname<<" saved"<<endl;

  cout<<"Plot Driftlines"<<endl;
  viewdrift.Plot(true,true);
  cellView.Plot2d();
  cname = TString::Format("./PolarSignal_driftlines_phi%1.4f_Z%2.1fcm.pdf",
			  InitialPhi,InitialZed);
  cDrift.SaveAs(cname.Data());
  cout<<cname<<" saved"<<endl;

  app.Run(true);

  return 0;
}

