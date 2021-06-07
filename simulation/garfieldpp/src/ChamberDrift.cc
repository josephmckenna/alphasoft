#include <iostream>
#include <fstream>
#include <cstdlib>

#include "TPC.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Sensor.hh"

#include "Garfield/DriftLineRKF.hh"

#include "Garfield/ViewCell.hh"
#include "Garfield/ViewDrift.hh"

#include "Garfield/Random.hh"

#include "Helpers.hh"

#include "TFile.h"
#include "TApplication.h"
#include "TNtupleD.h"

using namespace std;
using namespace Garfield;

#define DEBUG 1

int main(int argc, char * argv[])
{
  double InitialPhi = 0., // rad
    InitialZed = 0., // cm
    CathodeVoltage = -4000.,// V
    AnodeVoltage = 3200.,
    FieldVoltage = -99.;

  double MagneticField=1.; // T

  double QuenchFraction=0.1;

  double pressure=725.; // Torr @ CERN, it may be 735 Torr = 980 hPa, altitude 432 m

  bool plot=false;


  if( argc == 3 )
    {
      InitialPhi     = atof(argv[1]);
      InitialZed     = atof(argv[2]);
    }
  else if( argc == 5 )
    {
      InitialPhi     = atof(argv[1]);
      InitialZed     = atof(argv[2]);
      CathodeVoltage = atof(argv[3]);
      AnodeVoltage   = atof(argv[4]);
    }
  else if( argc == 6 )
    {
      InitialPhi     = atof(argv[1]);
      InitialZed     = atof(argv[2]);
      CathodeVoltage = atof(argv[3]);
      AnodeVoltage   = atof(argv[4]);
      FieldVoltage   = atof(argv[5]);
    }  
  else if( argc == 7 )
    {
      InitialPhi     = atof(argv[1]);
      InitialZed     = atof(argv[2]);
      CathodeVoltage = atof(argv[3]);
      AnodeVoltage   = atof(argv[4]);
      FieldVoltage   = atof(argv[5]);
      QuenchFraction = atof(argv[6]);
    }  
  else if( argc == 8 )
    {
      InitialPhi     = atof(argv[1]);
      InitialZed     = atof(argv[2]);
      CathodeVoltage = atof(argv[3]);
      AnodeVoltage   = atof(argv[4]);
      FieldVoltage   = atof(argv[5]);
      QuenchFraction = atof(argv[6]);
      MagneticField  = atof(argv[7]);
    }
  else if( argc == 9 )
    {
      InitialPhi     = atof(argv[1]);
      InitialZed     = atof(argv[2]);
      CathodeVoltage = atof(argv[3]);
      AnodeVoltage   = atof(argv[4]);
      FieldVoltage   = atof(argv[5]);
      QuenchFraction = atof(argv[6]);
      MagneticField  = atof(argv[7]);
      plot           = bool(atoi(argv[8]));
    }

  cerr<<"============================="<<endl;
  cerr<<"\t"<<argv[0]<<endl;
  cerr<<"============================="<<endl;

  TApplication app("ChamberDrift", &argc, argv);

  // Set Seed for Random Generator
  unsigned int the_seed = 1985810831;
  randomEngine.Seed(the_seed);

  // Create the medium
  MediumMagboltz *gas = new MediumMagboltz;
  
  // Gas file created with other software
  TString gasfile = TString::Format("%s/simulation/common/gas_files/ar_%2.0f_co2_%2.0f_NTP_25E1000000_10B2.00.gas",
                                    getenv("AGRELEASE"),
                                    (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2);
  cerr<<gasfile<<endl;
  if(gas->LoadGasFile(gasfile.Data()))
     std::cerr << "Gas file: "<< gasfile << " loaded" << std::endl;
  else return -47;
  if( pressure )
    gas->SetPressure(pressure);

  TString garfdata = TString::Format("%s/Data/IonMobility_Ar+_Ar.txt",
				     getenv("GARFIELD_HOME"));
  if(!gas->LoadIonMobility(garfdata.Data())) return -48;

  TPC drift_cell(CathodeVoltage,AnodeVoltage,FieldVoltage);
  drift_cell.SetPrototype(false);
  if( MagneticField >=0. )
    {
      drift_cell.SetMagneticField(0.,0.,MagneticField); // T
      cerr<<"Magnetic Field set to "<<MagneticField<<" T"<<endl;
    }
  else
    {
      // // Oxford field Map - 13 Feb 2016
      // TString BfieldMap("./fieldmaps/TPC_Field_Map.csv");
      // Babcock field Map - 20 Sep 2017 
      TString BfieldMap= TString::Format("%s/simulation/common/fieldmaps/Babcock_Field_Map.csv",
                                         getenv("AGRELEASE"));
      cerr<<"B map: "<<BfieldMap<<endl;
      if( !drift_cell.SetSymmetries("rz") ) 
	{
	  cerr<<"set symm failed"<<endl;
	  return -46;
	}
      // set scale parameter to convert Gauss to Tesla (or to go to 0.65T later)
      if( !drift_cell.ReadMagneticFieldMap(BfieldMap.Data(), TMath::Abs(MagneticField)*1.e-4) ) 
	{
	  cerr<<"READ B-map failed"<<endl;
	  return -49;
	} 
      cerr<<"Magnetic Field Map set (scaled to "<<MagneticField*1.e-4<<")"<<endl;
    }
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

  // area
  double  areaX1 = -drift_cell.GetROradius(), areaX2 = drift_cell.GetROradius(),
    areaY1 = areaX1, areaY2 = areaX2,
    areaZ1 = InitialZed-0.2, areaZ2 = InitialZed+0.2;

  // My Cell
  ViewCell viewCell;
  // Construct object to visualise drift lines
  ViewDrift viewdrift;

  TCanvas* cc;

  if( plot ) {
  // This canvas will be used to display the drift lines
  TString ccname = TString::Format("cell_drift_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT",
  				   InitialPhi,InitialZed,
  				   (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,
  				   drift_cell.CathodeVoltage,
  				   drift_cell.AnodeVoltage,
  				   drift_cell.FieldVoltage,
  				   MagneticField);
  cc = new TCanvas(ccname.Data(),ccname.Data(), 1800, 1800);

  viewCell.SetComponent(&drift_cell);
  viewCell.SetCanvas(cc);
  viewCell.SetArea(areaX1,areaY1,areaZ1,areaX2,areaY2,areaZ2);

  viewdrift.SetCanvas(cc);
  viewdrift.SetArea(areaX1,areaY1,areaZ1,areaX2,areaY2,areaZ2);
  }

  //----------------------------------------------------
  // Transport Class for Electrons drift
  // Runge-Kutta
  DriftLineRKF edrift;
  edrift.SetSensor(&sensor);
  const double maxStepSize=0.03;// cm
  edrift.SetMaximumStepSize(maxStepSize);
  if(plot) edrift.EnablePlotting(&viewdrift);
  //----------------------------------------------------

  TString fname = TString::Format("%s/chamber_drift/drift_tables/Drift_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT.dat",
                                  getenv("GARFIELDPP"),
				  InitialPhi,InitialZed,
				  (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,
				  CathodeVoltage,AnodeVoltage,FieldVoltage,
				  MagneticField);
  cerr<<"Saving data to "<<fname<<endl;
  ofstream ftd(fname.Data());
  if( !ftd.good() ) return -50;

  fname = TString::Format("%s/chamber_drift/drift_root/ChamberDrift_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT.root",
                                  getenv("GARFIELDPP"),
				  InitialPhi,InitialZed,
				  (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,
				  CathodeVoltage,AnodeVoltage,FieldVoltage,
				  MagneticField);
  TFile* fout = TFile::Open(fname.Data(),"RECREATE");
  if(!fout->IsOpen())
    {
      std::cerr << "Something went wrong opening the output file. Does the directory exist?" << std::endl;
      return -51;
    }
  TNtupleD ntefin("ntefin","e- endpoint","x:y:z:t:n");
  TH2D* hEv=new TH2D("hEv","hEv;electric field [V/cm];velocity [cm/ns]",
                     100,100.,1.e4,
                     100,0.,1.e-2);
  TH2D* hrE=new TH2D("hrE","hrE;radius [cm];electric field [V/cm]",
                     100,drift_cell.GetCathodeRadius(),drift_cell.GetROradius(),
                     1000,100.,1.e4);
  TH2D* hrv=new TH2D("hrv","hrv;radius [cm];velocity [cm/ns]",
                     100,drift_cell.GetCathodeRadius(),drift_cell.GetROradius(),
                     100,0.,1.e-2);
  _hEv=*hEv;
  _hrE=*hrE;
  _hrv=*hrv;

  cerr<<"\nBEGIN"<<endl;
 
  // Electron initial point
  double xi,yi,zi=InitialZed,
    ti=0.0;
  double phii = InitialPhi;
  double Rstep = 0.05; // cm
 
  // Electron counter
  int ie = 0;
  for(double ri=drift_cell.GetCathodeRadius(); ri<drift_cell.GetROradius(); ri+=Rstep)
    {
      ++ie;
      xi=ri*TMath::Cos(phii);
      yi=ri*TMath::Sin(phii);
      if( ri == drift_cell.GetFieldWiresRadius() || ri == drift_cell.GetAnodeWiresRadius() )
	continue;
      ElectricFieldHisto(xi,yi,zi,&sensor);
      if( !edrift.DriftElectron(xi,yi,zi,ti) ) continue;

      // drift time
      double t_d=edrift.GetDriftTime();
      // gain
      double gain=edrift.GetGain();
     
      double Xion,Yion,Zion,Tion;
      unsigned int NP = edrift.GetNumberOfDriftLinePoints();
      edrift.GetDriftLinePoint(NP-1,Xion,Yion,Zion,Tion);
      assert(TMath::Abs(TMath::Sqrt(Xion*Xion+Yion*Yion)-18.2)<=Rstep);

      // wire hit
      double phiIon = TMath::ATan2(Yion,Xion);
      if( phiIon < 0. ) phiIon+=TMath::TwoPi();

      ftd<<xi<<"\t"<<yi<<"\t"<<phiIon<<"\t"<<t_d<<"\t"<<gain<<endl;
    
#if DEBUG>0
      cerr<<"\t"<<ie<<"\t"<<ri
	  <<"\t"<<t_d<<"\t"<<phiIon*TMath::RadToDeg()<<"\t"<<gain<<endl;
#endif
      ntefin.Fill(Xion,Yion,Zion,t_d,gain);
    }
  cerr<<"END"<<endl;
  ftd.close();
  fout->cd();
  ntefin.Write();
  _hEv.Write();
  _hrE.Write();
  _hrv.Write();

  cerr<<"Number of Clusters: "<<ie<<endl;

  if(plot)
     {
        cerr<<"Plot Driftlines"<<endl;
        viewdrift.Plot(true,true);
        viewCell.Plot2d();

        TString sname = TString::Format("%s/chamber_drift/drift_plots/%s.pdf",getenv("GARFIELDPP"),cc->GetName());
        cc->SaveAs(sname);
        
        app.Run(true);
     }
  return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
