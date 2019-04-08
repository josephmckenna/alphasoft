#include <iostream>
#include <fstream>
#include <cstdlib>

#include <TMath.h>
#include <TCanvas.h>
#include <TApplication.h>
#include <TFile.h>

#include "MediumMagboltz.hh"
#include "GeometrySimple.hh"
#include "SolidTube.hh"
#include "Sensor.hh"

#include "ViewGeometry.hh"
#include "ViewCell.hh"
#include "ViewField.hh"

#include "TPC.hh"
#include "Helpers.hh"

using namespace std;
using namespace Garfield;

int main(int argc, char * argv[])
{
   double CathodeVoltage=-4000.,// V
    AnodeVoltage = 3100.,
    FieldVoltage = -99;
  double Bfield = 0.;//T

  if( argc == 3 )
    {
      CathodeVoltage = atof(argv[1]);
      AnodeVoltage   = atof(argv[2]);
    }
  else if( argc == 4 )
    {
      CathodeVoltage = atof(argv[1]);
      AnodeVoltage   = atof(argv[2]);
      FieldVoltage   = atof(argv[3]);
    }
  cout<<"============================="<<endl;
  cout<<"\t"<<argv[0]<<endl;
  cout<<"============================="<<endl;

  TApplication app("app", &argc, argv);

  TString fname = TString::Format("chamberfield_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV.root",
				  CathodeVoltage,AnodeVoltage,FieldVoltage);
  TFile* fout = TFile::Open(fname.Data(),"RECREATE");

  // Create the medium
  MediumMagboltz* gas = new MediumMagboltz();
  // Gas file created with other software
  TString gasfile = TString::Format("%s/simulation/common/gas_files/ar_70_co2_30_725Torr_20E200000_4B1.10.gas",
				    getenv("AGRELEASE"));
  if( LoadGas(gas,gasfile.Data()) )
    cout<<gasfile<<endl;
  else
    return -47;

  TString garfdata = TString::Format("%s/Data/IonMobility_Ar+_Ar.txt",getenv("GARFIELD_HOME"));
  gas->LoadIonMobility(garfdata.Data());

  TPC drift_cell(CathodeVoltage,AnodeVoltage,FieldVoltage);
  drift_cell.SetPrototype(false);
  drift_cell.SetMagneticField(0.,0.,Bfield); // T
  // if(!drift_cell.SetSymmetries("rz")) return -46;
  // if(!drift_cell.ReadMagneticFieldMap("./fieldmaps/TPC_Field_Map.csv", 1.e-4)) return -49;  // set scale parameter to convert Gauss to Tesla (or to go to 0.65T later)
  drift_cell.SetGas(gas);
  drift_cell.init();

  // Finally assembling a Sensor object
  Sensor sensor;
  // Calculate the electric field
  sensor.AddComponent(&drift_cell);

  // Request signal calculation for the electrode named with labels above,
  // using the weighting field provided by the Component object cmp.
  vector<string> anodes = drift_cell.GetAnodeReadouts();
  for(unsigned int a=0; a < anodes.size(); ++a)
      sensor.AddElectrode(&drift_cell,anodes[a]);

  sensor.AddElectrode(&drift_cell, "ro");

  double gain = CalculateGain(drift_cell.GetDiameterAnodeWires(),&sensor);
  cout<<"------------------------------------------"<<endl;
  cout<<" ******** GAIN = "<<gain<<" ********"<<endl;
  cout<<"------------------------------------------"<<endl;

  cout<<"\n------------ Start Plotting ------------"<<endl;

  // area
  double areaX1=drift_cell.GetCathodeRadius(), areaX2=drift_cell.GetROradius(),
    areaY1=-1.,areaY2=7.5,
    areaZ1=0,areaZ2=120;

  // cout<<"*** Plot Geometry ***"<<endl;
  // // Plot Geometry
  // TCanvas *c1 = new TCanvas("geo", "geo", 800, 800);
  // ViewGeometry* viewGeo = new ViewGeometry;
  // viewGeo->SetGeometry(geo);
  // viewGeo->SetCanvas(c1);
  // viewGeo->Plot();

  cout<<"*** My Cell ***"<<endl;
  // My Cell
  TCanvas c2("cell", "cell", 800, 800);
  ViewCell viewCell;
  viewCell.SetComponent(&drift_cell);
  viewCell.SetCanvas(&c2);
  viewCell.Plot2d();
  c2.SetGrid();

  cout<<"*** Plot isopotential contours ***"<<endl;

  // Plot isopotential contours
  TString c3name = TString::Format("isopotential_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV",
				  CathodeVoltage,AnodeVoltage,FieldVoltage);
  TCanvas c3(c3name.Data(),c3name.Data() , 800, 800);
  ViewField viewV;
  viewV.SetSensor(&sensor);
  viewV.SetCanvas(&c3);
  viewV.PlotContour();
  // TF2* thePotential = (TF2*)c3.GetPrimitive("fPotential_0");
  // thePotential->SetName("IsoPotential");

  cout<<"*** Plot isopotential contours YZ***"<<endl;

  // Plot isopotential contours y/z
  TString c3yzname = TString::Format("isopotential_yz_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV",
				  CathodeVoltage,AnodeVoltage,FieldVoltage);
  TCanvas c3yz(c3yzname.Data(),c3yzname.Data() , 800, 800);
  ViewField viewVyz;
  viewVyz.SetSensor(&sensor);
  viewVyz.SetCanvas(&c3yz);
  viewVyz.SetPlane(1,0,0,0,0,0);
  viewVyz.SetArea(areaZ1,areaX1,areaZ2,areaX2);
  viewVyz.PlotContour();
  // TF2* thePotentialyz = (TF2*)c3.GetPrimitive("fPotential_0");
  // if(thePotentialyz) thePotentialyz->SetName("IsoPotential_yz");

  cout<<"*** Plot isopotential contours @ wires ***"<<endl;
  //  Plot isopotential contours @ wires
  TString c4name = TString::Format("isopotential_wires_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV",
				   CathodeVoltage,AnodeVoltage,FieldVoltage);
  TCanvas c4(c4name.Data(),c4name.Data() , 800, 800);
  ViewField viewVw;
  viewVw.SetSensor(&sensor);
  viewVw.SetArea(areaX1,areaY1,areaX2,areaY2);
  //  viewVw.SetVoltageRange(2000.,6.1e3);
  viewVw.SetCanvas(&c4);
  viewVw.PlotContour();

  cout<<"*** Plot Electric Field Magnitude contours @ wires ***"<<endl;
  // Plot Electric Field Magnitude contours @ wires
  TString c5name = TString::Format("Efield_wires_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV",
				  CathodeVoltage,AnodeVoltage,FieldVoltage);
  TCanvas c5(c5name.Data(),c5name.Data() , 800, 800);
  ViewField viewEw;
  viewEw.SetSensor(&sensor);
  viewEw.SetArea(areaX1,areaY1,areaX2,areaY2);
  viewEw.SetCanvas(&c5);
  viewEw.PlotContour("e");

  cout<<"*** Plot Electric Field Magnitude contours ***"<<endl;
  // Plot Electric Field Magnitude contours
  TString c6name = TString::Format("Efield_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV",
				   CathodeVoltage,AnodeVoltage,FieldVoltage);
  TCanvas c6(c6name.Data(),c6name.Data() , 800, 800);
  ViewField viewE;
  viewE.SetSensor(&sensor);
  viewE.SetCanvas(&c6);
  viewE.PlotContour("e");
  // TF2* theEField = (TF2*)c6.GetPrimitive("fPotential_0");
  // theEField->SetName("ElectricField");

  cout<<"Save Plots"<<endl;
  //  c1.SaveAs(".png");
  c2.SaveAs(".png");
  c3.SaveAs(".png");
  c3yz.SaveAs(".png");
  c4.SaveAs(".png");
  c5.SaveAs(".png");
  c6.SaveAs(".png");

  app.Run(kTRUE);

  cout<<"Write Plots to File: "<<fout->GetName()<<endl;
  fout->cd();
  //  c1.Write();
  c2.Write();
  c3.Write();
  c3yz.Write();
  c4.Write();
  c5.Write();
  c6.Write();
  //  thePotential->Write();
  //  theEField->Write();
  fout->Close();

  cout<<"Bye"<<endl;
  return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
