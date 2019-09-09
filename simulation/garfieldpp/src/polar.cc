#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>

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

using namespace Garfield;

int main(int argc, char * argv[]) { 

  TApplication app("app", &argc, argv);
  plottingEngine.SetDefaultStyle();
 
  // Make a gas medium.
  MediumMagboltz gas;

  // Build the geometry.
  constexpr double lZ = 115.2;
  constexpr double rRO = 19.03;
  GeometrySimple geo;
  SolidTube tube(0, 0, 0, 0, rRO, lZ);
  geo.AddSolid(&tube, &gas);

  ComponentAnalyticField cmp;
  cmp.SetGeometry(&geo);
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
  cmp.SetPeriodicityPhi(sphi);
  cmp.PrintCell();

  TCanvas cPotential;
  ViewField fieldView;
  fieldView.SetCanvas(&cPotential);
  fieldView.SetComponent(&cmp);
  fieldView.SetArea(-1.1 * rRO, -1.1 * rRO,
                     1.1 * rRO,  1.1 * rRO);
  fieldView.PlotContour();
  //  fieldView.PlotContour("e");
  ViewCell cellView;
  cellView.SetCanvas(&cPotential);
  cellView.SetComponent(&cmp);
  cellView.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
                    1.1 * rRO,  1.1 * rRO,  1.);
  cellView.EnableWireMarkers(false);
  cellView.Plot2d();

  app.Run(true);
}

