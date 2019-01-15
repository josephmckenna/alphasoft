#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

#include <TMath.h>
#include <TVector3.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TNtuple.h>
#include <TFile.h>

#include "MediumMagboltz.hh"
#include "GeometrySimple.hh"
#include "SolidTube.hh"

#include "TPC.hh"
#include "Sensor.hh"

#include "DriftLineRKF.hh"
#include "AvalancheMC.hh"
#include "AvalancheMicroscopic.hh"

#include "ViewCell.hh"
#include "ViewField.hh"
#include "ViewDrift.hh"

#include <TRandom2.h>
#include "Random.hh"

#include "Helpers.hh"

using namespace std;
using namespace Garfield;

#define DEBUG 0

int main(int argc, char * argv[])
{
  double r0=10.9251, phi0=1.583, z0=0., B=1.;

  if( argc == 4 )
    {
      r0   = atof(argv[1]); // cm
      phi0 = atof(argv[2]); // rad
      z0   = atof(argv[3]); // cm
    }
  else if( argc == 5 )
    {
      r0   = atof(argv[1]); // cm
      phi0 = atof(argv[2]); // rad
      z0   = atof(argv[3]); // cm
      B    = atof(argv[4]); // T
    }

  double CathodeVoltage = -4000.,// V
    AnodeVoltage = 3100.,
    FieldVoltage = -99.;

  double MagneticField=abs(B); // T

  unsigned int the_seed = 919850618;
  randomEngine.Seed(the_seed);

  TString fname = TString::Format("Avalanche_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT_initR%1.2fcm_initPhi%1.3frad_initZ%1.2fcm.root",
				  CathodeVoltage,AnodeVoltage,FieldVoltage,
				  MagneticField,
				  r0,phi0,z0);
  TFile* fout = TFile::Open(fname.Data(),"RECREATE");

  if(!fout->IsOpen())
    {
      cerr << "Something went wrong opening the output file. Does the directory exist?" << endl;
      return -1;
    }

  TNtuple ntefin("ntefin","e- endpoint","x:y:z:t");
  TNtuple ntionpos("ntionpos","ion position","x:y:z:t");
  TNtuple ntfionpos("ntfionpos","ion final position","x:y:z:t");

  // Create the medium
  MediumMagboltz *gas = new MediumMagboltz;
  // Gas file created with other software
  TString gasfile = TString::Format("%s/simulation/common/gas_files/ar_70_co2_30_725Torr_20E200000_4B1.10.gas",
				    getenv("AGRELEASE"));
  if( LoadGas(gas,gasfile.Data()) )
    cerr<<gasfile<<endl;
  else
    return -47;
  TString garfdata = TString::Format("%s/Data/IonMobility_Ar+_Ar.txt",
				     getenv("GARFIELD_HOME"));
  if(!gas->LoadIonMobility(garfdata.Data())) 
    return -48;

  TPC drift_cell(CathodeVoltage,AnodeVoltage,FieldVoltage);
  drift_cell.SetPrototype(false);

  // // Oxford field Map - 13 Feb 2016
  // TString BfieldMap("./fieldmaps/TPC_Field_Map.csv");
  // Babcock field Map - 20 Sep 2017
  TString BfieldMap = TString::Format("%s/simulation/common/fieldmaps/Babcock_Field_Map.csv",
				      getenv("AGRELEASE"));
  if( B >= 0. )
    {
      drift_cell.SetMagneticField(0.,0.,MagneticField);
      cerr<<"Magnetic Field set to "<<MagneticField<<" T"<<endl;
    }
  else
    {
      if( !drift_cell.SetSymmetries("rz") ) 
	{
	  cerr<<"set symm failed"<<endl;
	  return -46;
	}
      // set scale parameter to convert Gauss to Tesla (or to go to 0.65T later)
      if( !drift_cell.ReadMagneticFieldMap(BfieldMap.Data(), MagneticField*1.e-4) ) 
	{
	  cerr<<"READ B-map failed"<<endl;
	  return -49;
	} 
      cerr<<"Magnetic Field Map set (scaled to "<<MagneticField*1.e-4<<")"<<endl;
    }
  drift_cell.SetGas(gas);

  // Finally assembling a Sensor object
  Sensor sensor;
  sensor.DisableDebugging();
  // Calculate the electric field
  sensor.AddComponent(&drift_cell);

  // Request signal calculation for the electrode named with labels above,
  // using the weighting field provided by the Component object cmp.
  vector<string> anodes = drift_cell.GetAnodeReadouts();
  for(unsigned int a=0; a < anodes.size(); ++a)
    sensor.AddElectrode(&drift_cell,anodes[a]);

  sensor.AddElectrode(&drift_cell, "ro");

  // Set Time window for signal integration, units in [ns]
  const double tMin = 0.;
  const double tMax = 10000.; // ns = 10 us
  const double tStep = 1.0;
  const int nTimeBins = int((tMax - tMin) / tStep);
  sensor.SetTimeWindow(0., tStep, nTimeBins);

  // area
  double areaX1=-drift_cell.GetROradius(),
    areaX2=drift_cell.GetROradius(),
    areaY1=-drift_cell.GetROradius(),
    areaY2=drift_cell.GetROradius(),
    areaZ1=-0.4,areaZ2=0.4;

  // My Cell
  // This canvas will be used to display the drift lines
  TString ccname = TString::Format("avalancheline_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_initR%1.2fcm_initPhi%1.2frad_initZ%1.2fcm", 
				   CathodeVoltage,AnodeVoltage,FieldVoltage,r0,phi0,z0);

  TCanvas cc(ccname.Data(),ccname.Data(), 1400, 1400);
  ViewCell viewCell;
  viewCell.SetComponent(&drift_cell);
  viewCell.SetCanvas(&cc);
  viewCell.SetArea(areaX1,areaY1,areaZ1,areaX2,areaY2,areaZ2);

  // Construct object to visualise drift lines
  ViewDrift viewdrift;
  viewdrift.SetCanvas(&cc);
  viewdrift.SetArea(areaX1,areaY1,areaZ1,areaX2,areaY2,areaZ2);

  //----------------------------------------------------
  //----------------------------------------------------
  // Transport Class for Electrons
  // // MC Avalanche
  // AvalancheMC eaval;
  // eaval.SetSensor(&sensor);
  // eaval.EnableMagneticField();
  // eaval.EnableSignalCalculation();
  // const double step = 1.e-4;// cm = 1 um
  // //  const double step = 0.03;// cm = 300 um
  // eaval.SetDistanceSteps(step);
  // eaval.EnablePlotting(&viewdrift);
  // //----------------------------------------------------
  // Microscopic Avalanche
  AvalancheMicroscopic eaval;
  eaval.SetSensor(&sensor);
  eaval.EnableMagneticField();
  eaval.DisableSignalCalculation();
  eaval.EnablePlotting(&viewdrift);
  //----------------------------------------------------

  //----------------------------------------------------
  //----------------------------------------------------
  // Transport Class for Ions
  // MC Avalanche
  AvalancheMC iaval;
  iaval.SetSensor(&sensor);
  iaval.EnableMagneticField();
  iaval.EnableSignalCalculation();
  const double dist_step = 2.e-4;// cm = 2 um
  iaval.SetDistanceSteps(dist_step);
  //  iaval.EnablePlotting(&viewdrift);
  //----------------------------------------------------

  TRandom2 rndm;
  rndm.SetSeed(the_seed);

  // Start simulation
  cerr<<"\nBEGIN"<<endl;
  sensor.ClearSignal();
  sensor.NewSignal();
  
  cerr<<"AvalancheElectron:";
  //  cerr<<"Drift Electron RKF:";
  //  double z0=0., // cm
  double  t0=0.; //ns
  cerr<<" r0 = "<<r0<<" cm; phi0 = "<<phi0<<" rad = "<<phi0*TMath::RadToDeg()<<" deg"<<endl;
  double x0 = r0*TMath::Cos(phi0), y0 = r0*TMath::Sin(phi0); //cm
  cerr<<" x0 = "<<x0<<"; y0 = "<<y0<<"; z0 = "<<z0<<"  cm"<<endl;
  //  double e0 = 0.;// eV
  double e0 = 0.6; // eV

  double Ex,Ey,Ez,VV,Bx,By,Bz;
  int s=0;
  Medium* m;
  sensor.ElectricField(x0,y0,z0,Ex,Ey,Ez,VV,m,s);
  if( s ) cerr<<"Bad E field\n";
  else {
    cerr<<"In "<<m->GetName()<<" E = ("<<Ex<<","<<Ey<<","<<Ez<<") V/cm\t";
    cerr<<"V = "<<VV<<" V\t";
  }
  sensor.MagneticField(x0,y0,z0,Bx,By,Bz,s);
  if( s ) cerr<<"Bad B field\n";
  else {
    cerr<<" B = ("<<Bx<<","<<By<<","<<Bz<<") T\n";
  }
  cerr<<"---------------------------------------------------------\n"<<endl;

  double Nions = 100.;
  //  int Nelectrons = 1000;
  //  int Nelectrons = 100;
  int Nelectrons = 1;

  for(int den=0; den<Nelectrons; ++den)
    {
      int ne,ni;
      ne=ni=0;
      cerr<<den<<endl;
      while(ni == 0)
      	{
	  cerr<<"e- init: "<<x0<<"\t"<<y0<<"\t"<<z0<<"\t"<<t0<<"\t"<<e0<<"\n";
	  eaval.AvalancheElectron(x0, y0, z0, t0, e0);
	  //  eaval.DriftElectron(x0, y0, z0, t0, e0);

	  eaval.GetAvalancheSize(ne,ni);
	  cerr<<"Avalanche Size: #ions = "<<ni<<"; #e- "<<ne<<endl;
	}
      double exf,eyf,ezf,etf,eef;
      exf=eyf=ezf=etf=eef=-1.;
   
      iaval.SetIonSignalScalingFactor(double(ni)/Nions);

      cerr<<"Tracking "<<Nions<<" out of "<<eaval.GetNumberOfElectronEndpoints()<<" ions..."<<endl;
      for(unsigned i=0; i<eaval.GetNumberOfElectronEndpoints(); ++i)
	{
	  double xi,yi,zi,ti,
	    xf,yf,zf,tf,ef;
	  int status;
	  double ei;
	  eaval.GetElectronEndpoint(i, 
				    xi, yi, zi, ti, ei,
				    xf, yf, zf, tf, ef,
				    status);

	  double ri = TMath::Sqrt(xi*xi+yi*yi);
	  //	  if( ri == r0 )
	  if( TMath::Abs(ri-r0) < 1.e-3 )
	    {
	      exf=xf;
	      eyf=yf;
	      ezf=zf;
	      etf=tf;
	      eef=ef;
	      //	  estat=status;
	      cerr<<"EEP test: ri = "<<ri
		  <<" cm; phii = "<<TMath::ATan2(yi,xi)*TMath::RadToDeg()<<" deg"<<endl;
	    }
	  ntionpos.Fill(xi,yi,zi,ti);
	  //      ntionpos.Fill(xf,yf,zf,tf);
#if DEBUG>0
	  cerr<<i<<"\n\tinit: "
	      <<xi<<"\t"<<yi<<"\t"<<zi<<"\t"<<ti<<"\t"<<ei<<"\n\tfinal: "
	      <<xf<<"\t"<<yf<<"\t"<<zf<<"\t"<<tf<<"\t"<<ef<<endl;
#endif
	  if( i >= Nions ) continue;
	  iaval.DriftIon(xi,yi,zi,ti);
	  //    iaval.DriftIon(xf,yf,zf,tf);
	  unsigned int NN = iaval.GetNumberOfDriftLinePoints();
	  double xf_ion,yf_ion,zf_ion,tf_ion;
	  iaval.GetDriftLinePoint(NN-1, xf_ion, yf_ion, zf_ion, tf_ion);
#if DEBUG>0
	  cerr<<"\t\tion endpoint: "<<xf_ion<<"\t"<<yf_ion<<"\t"<<zf_ion<<"\t"<<tf_ion<<endl;
#endif

	  ntfionpos.Fill(xf_ion, yf_ion, zf_ion, tf_ion);
	}
      ntefin.Fill(exf,eyf,ezf,etf);
      cerr<<"\nEEP e- endpoint: "<<exf<<"\t"<<eyf<<"\t"<<ezf<<"\t"<<etf<<"\t"<<eef<<"\n"<<endl;
    }
  cerr<<"END"<<endl;

  fout->cd();
  ntefin.Write();
  ntionpos.Write();
  ntfionpos.Write();

  // cerr<<"Plot Driftlines"<<endl;
  // viewdrift.Plot(true,true);
  // viewCell.Plot2d();
  // cc.SaveAs(".pdf");
  // fout->cd();
  // cc.Write();

  cerr<<"READOUT!"<<endl;
  Sensor ROsens(sensor);
  // AFTER Response Function
  ROsens.SetTransferFunction(Hpads);

  // Pre-Amp Response Function
  sensor.SetTransferFunction(Hands);

  bool convolute = true;
  if( convolute )
    {
      cerr<<"Apply the transfer function"<<endl;
      // Apply the transfer function
      sensor.ConvoluteSignal();
      ROsens.ConvoluteSignal();
    }

  // WIRES SIGNALS
  TH1D hSigWire[drift_cell.GetNumberOfAnodeWires()];
  for(int w=0; w<drift_cell.GetNumberOfAnodeWires(); ++w)
    {
      TString wname = TString::Format("a%d",w);
      hSigWire[w] = GetROSignal(&sensor, &wname, convolute);
      hSigWire[w].Write();
    }

  TString roname("ro");
  TH1D hro = GetROSignal(&ROsens, &roname, convolute);
  hro.Write();
  cerr<<"READOUT complete"<<endl;

  return 0;
}
