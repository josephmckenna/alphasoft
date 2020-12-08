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
#include "SolidBox.hh"
#include "GeometrySimple.hh"
#include "Sensor.hh"
#include "FundamentalConstants.hh"

#include "TPC.hh"
#include "Helpers.hh"

using namespace Garfield;

int main(int argc, char * argv[]) 
{

  double CathodeVoltage=-4000.,// V
    AnodeVoltage = 3100.,
    FieldVoltage = -99;
  double Bfield = 1.;//T

  TApplication app("app", &argc, argv);
  plottingEngine.SetDefaultStyle();
 
  // // Make a gas medium.
  // MediumMagboltz gas;

  // // Build the geometry.
  // GeometrySimple geo;
  // SolidBox box(0, 0, 0, 4, 4, 4);
  // geo.AddSolid(&box, &gas);

  // // Setup the cell.
  // ComponentAnalyticField cmp;
  // cmp.SetGeometry(&geo);
  // cmp.AddPlaneY( 0.375, -2000, "t");
  // cmp.AddPlaneY(-0.125,     0, "b");
  // cmp.AddWire(0, 0, 30.e-4, 3000., "s", 20., 60.);
  // cmp.SetPeriodicityX(0.15);

  // Create the medium
  MediumMagboltz* gas = new MediumMagboltz();
  // Gas file created with other software
  TString gasfile = TString::Format("%s/simulation/common/gas_files/ar_70_co2_30_725Torr_20E200000_4B1.10.gas",
				    getenv("AGRELEASE"));
  if( !LoadGas(gas,gasfile.Data()) )
    return -47;

  TString garfdata = TString::Format("%s/Data/IonMobility_Ar+_Ar.txt",getenv("GARFIELD_HOME"));
  gas->LoadIonMobility(garfdata.Data());

  TPC drift_cell(CathodeVoltage,AnodeVoltage,FieldVoltage);
  drift_cell.SetPrototype(false);
  drift_cell.SetMagneticField(0.,0.,Bfield); // T
  drift_cell.SetGas(gas);
  drift_cell.init();

  unsigned int aw_number=0;
  double xx,yy; // mm
  drift_cell.GetAnodePosition(aw_number,xx,yy,false,false);
  std::cout<<"location of aw "<<aw_number<<" : ("<<xx<<","<<yy<<") cm"<<std::endl;

  // double area=0.01; // mm
  // double xmin=xx-area, xmax=xx+area, ymin=yy-area, ymax=yy+area;

  drift_cell.SetScanningAreaLargest();
  //  drift_cell.SetScanningArea(-0.01, 0.01, -0.01, 0.01);
  // drift_cell.SetScanningArea(xmin, xmax, ymin, ymax);
  //  drift_cell.SetGravity(0, 1, 0);
  drift_cell.SetGravity(0., 0., -1.);

  std::vector<double> csag;
  std::vector<double> xsag;
  std::vector<double> ysag;
  double stretch = 0.;
  drift_cell.WireDisplacement(aw_number+257, true, csag, xsag, ysag, stretch); 
  const unsigned int nPoints = csag.size();
  TGraph gSagY(nPoints);
  for (unsigned int i = 0; i < nPoints; ++i) 
    {
      gSagY.SetPoint(i, csag[i], 1.e4 * ysag[i]); 
    } 
  TCanvas cSag("cSag", "", 600, 600);
  gSagY.SetLineWidth(2);
  gSagY.SetLineColor(kGreen + 2);
  gSagY.Draw("al");
  gSagY.GetXaxis()->SetTitle("#it{z} [cm]");
  gSagY.GetYaxis()->SetTitle("Sag [#mum]");
  cSag.Update();
  app.Run(true);
}
