#include <iostream>
#include <fstream>
#include <cstdlib>

#include <TMath.h>
#include <TVector3.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TNtupleD.h>
#include <TFile.h>

#include "MediumMagboltz.hh"
#include "TPC.hh"
#include "Sensor.hh"

#include "DriftLineRKF.hh"

#include "ViewCell.hh"
#include "ViewField.hh"
#include "ViewDrift.hh"

#include "Random.hh"

#include "Helpers.hh"

using namespace std;
using namespace Garfield;

#define DEBUG 1

TH2D hEv;
TH2D hrE;
TH2D hrv;
double ElectricFieldHisto(double x, double y, double z, Sensor* s)
{
  // Electric and Magnetic Fields
  double Ex,Ey,Ez,VV,Bx,By,Bz;
  int dummy;
  // Drift Velocity
  Medium* m;
  double Vx,Vy,Vz;
  s->ElectricField(x,y,z,Ex,Ey,Ez,VV,m,dummy);
  s->MagneticField(x,y,z,Bx,By,Bz,dummy);
  m->ElectronVelocity(Ex,Ey,Ez,Bx,By,Bz,Vx,Vy,Vz);
  // Lorentz Angle
  TVector3 V(Vx,Vy,Vz);
  TVector3 E(Ex,Ey,Ez);

  double Emag=E.Mag(),Vmag=V.Mag(),r=TMath::Sqrt(x*x+y*y);
  hEv.Fill(Emag,Vmag);
  hrE.Fill(r,Emag);
  hrv.Fill(r,Vmag);
  return TMath::Pi()-V.Angle(E);
}

int main(int argc, char * argv[])
{
  double InitialPhi = 0., // rad
    InitialZed = 0., // cm
    CathodeVoltage = -4000.,// V
    AnodeVoltage = 3200.,
    FieldVoltage = -403.52;

  // double CathodeVoltage = -5325.,// V
  // AnodeVoltage = 1765.,
  // FieldVoltage = -186.;
  //    AnodeVoltage = 2050.,
  //    FieldVoltage = -227.;

  // double MagneticField=0.65; // T
  //  double MagneticField=0.;
  double MagneticField=1.;

  double QuenchFraction=0.1;

  double pressure=725.;

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
  cerr<<"============================="<<endl;
  cerr<<"\t"<<argv[0]<<endl;
  cerr<<"============================="<<endl;

  // Set Seed for Random Generator
  unsigned int the_seed = 1985810831;
  randomEngine.Seed(the_seed);

  // TString fname = TString::Format("./chamber_drift/ChamberDriftHisto_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT.root",
  // 				  InitialPhi,InitialZed,
  // 				  (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,
  // 				  CathodeVoltage,AnodeVoltage,FieldVoltage,
  // 				  MagneticField);
  // TFile* fout = TFile::Open(fname.Data(),"RECREATE");

  // TH1D hgain("hgain_drift","Gain DriftRKF",1.e4,1.,5.e6);

  // TNtupleD ntdrift("ntdrift","drift","r:wire_angle:drift_time:Lorentz");
 
  // Create the medium
  MediumMagboltz *gas = new MediumMagboltz;
  
  // Gas file created with other software
  // TString gasfile = TString::Format("%s/gasfiles/ar_90_co2_10_NTP_20E200000_6B1.25.gas",
  // 				    getenv("AGTPC_TABLES"));
  TString gasfile = TString::Format("%s/gasfiles/ar_%2.0f_co2_%2.0f_NTP_20E200000_6B1.25.gas",
   				    getenv("AGTPC_TABLES"),
				    (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2);
  if( pressure )
    gasfile = TString::Format("%s/gasfiles/ar_%2.0f_co2_%2.0f_%1.0fTorr_20E200000_4B1.10.gas",
			      getenv("AGTPC_TABLES"),
			      (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,pressure);
  cerr<<gasfile<<endl;
  if( LoadGas(gas,gasfile.Data()) )
    cerr<<"gasfile OK"<<endl;
  else
    return -47;

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
      TString BfieldMap("./fieldmaps/Babcock_Field_Map.csv");
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

  hEv = TH2D("hEv","Electron Velocity;E [V/cm];v [cm/ns]",10000,0.,50.e3,10000,0.,1.e-2);
  hrE = TH2D("hrE","Electric Field;r [cm];E [V/cm]",10000,
	     drift_cell.GetCathodeRadius(), drift_cell.GetROradius(),
	     10000,0.,50.e3);;
  hrv = TH2D("hrv","Electron Velocity;r [cm];v [cm/ns]",10000,
	     drift_cell.GetCathodeRadius(), drift_cell.GetROradius(),
	     10000,0.,1.e-2);


  // Finally assembling a Sensor object
  Sensor sensor;
  //  sensor.DisableDebugging();
  // Calculate the electric field
  sensor.AddComponent(&drift_cell);

  // Request signal calculation for the electrode named with labels above,
  // using the weighting field provided by the Component object cmp.
  vector<string> anodes = drift_cell.GetAnodeReadouts();
  for(unsigned int a=0; a < anodes.size(); ++a)
      sensor.AddElectrode(&drift_cell,anodes[a]);

  sensor.AddElectrode(&drift_cell, "ro");

  // area
  // double areaX1 = drift_cell.GetCathodeRadius(), areaX2 = drift_cell.GetROradius(),
  //   areaY1 = -0.1, areaY2 = areaX2 - areaX1 + areaY1,
  //   areaZ1 = InitialZed-0.2, areaZ2 = InitialZed+0.2;
  // double  areaX1 = -drift_cell.GetROradius(), areaX2 = drift_cell.GetROradius(),
  //   areaY1 = areaX1, areaY2 = areaX2,
  //   areaZ1 = InitialZed-0.2, areaZ2 = InitialZed+0.2;

  // // My Cell
  // // This canvas will be used to display the drift lines
  // TString ccname = TString::Format("./chamber_drift/cell_drift_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT",
  // 				   InitialPhi,InitialZed,
  // 				   (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,
  // 				   drift_cell.CathodeVoltage,
  // 				   drift_cell.AnodeVoltage,
  // 				   drift_cell.FieldVoltage,
  // 				   MagneticField);
  // TCanvas cc(ccname.Data(),ccname.Data(), 1800, 1800);
  // ViewCell viewCell;
  // viewCell.SetComponent(&drift_cell);
  // viewCell.SetCanvas(&cc);
  // viewCell.SetArea(areaX1,areaY1,areaZ1,areaX2,areaY2,areaZ2);

  // // Plot isopotential contours
  // TString cfname = TString::Format("./chamber_drift/isopotential_chamber_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV",
  // 				   drift_cell.CathodeVoltage,
  // 				   drift_cell.AnodeVoltage,
  // 				   drift_cell.FieldVoltage);
  // TCanvas cf(cfname.Data(),cfname.Data(), 800, 800);
  // ViewField viewF;
  // viewF.SetSensor(&sensor);
  // viewF.SetArea(areaX1,areaY1,areaX2,areaY2);
  // viewF.SetCanvas(&cf);
  // viewF.PlotContour();
  // cf.SaveAs(".png");

  // // Plot isopotential contours
  // TString cename = TString::Format("./chamber_drift/electricfield_chamber_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV",
  // 				   drift_cell.CathodeVoltage,
  // 				   drift_cell.AnodeVoltage,
  // 				   drift_cell.FieldVoltage);
  // TCanvas ce(cename.Data(),cename.Data(), 800, 800);
  // ViewField viewE;
  // viewE.SetSensor(&sensor);
  // viewE.SetArea(areaX1,areaY1,areaX2,areaY2);
  // viewE.SetCanvas(&ce);
  // viewE.PlotContour("e");
  // ce.SaveAs(".png");

  // // Construct object to visualise drift lines
  // ViewDrift viewdrift;
  // viewdrift.SetCanvas(&cc);
  // viewdrift.SetArea(areaX1,areaY1,areaZ1,areaX2,areaY2,areaZ2);

  //----------------------------------------------------
  // Transport Class for Electrons drift
  // Runge-Kutta
  DriftLineRKF edrift;
  edrift.SetSensor(&sensor);
  const double maxStepSize=0.03;// cm
  edrift.SetMaximumStepSize(maxStepSize);
  edrift.EnableStepSizeLimit();
  //  edrift.EnablePlotting(&viewdrift);
  //----------------------------------------------------

  //  fname = TString::Format("./chamber_drift/MasterTableDrift_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT.dat",
  TString fname = TString::Format("./chamber_drift/drifttables/Drift_phi%1.4f_Z%2.1fcm_Ar%2.0f-CO2%2.0f_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT.dat",
				  InitialPhi,InitialZed,
				  (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,
				  CathodeVoltage,AnodeVoltage,FieldVoltage,
				  MagneticField);
  cerr<<"\nBEGIN"<<endl;
  ofstream ftd(fname.Data());

  // Electron initial point
  double xi,yi,zi=InitialZed,
    ti=0.0;
  double phii = InitialPhi;
  //  double ri = drift_cell.GetCathodeRadius();
  //  double ri = 10.93;

  //  double Rstep = 0.02; // cm
  //  double Rstep = 0.003; // cm
  double Rstep = 0.05;
  //double Rstep = 0.01;
  // Electron counter
  int ie = 0;
  for(double ri=drift_cell.GetCathodeRadius(); ri<drift_cell.GetROradius(); ri+=Rstep)
      //  for(double phii = InitialPhi - drift_cell.GetAnodePitch()*0.5; phii<InitialPhi + drift_cell.GetAnodePitch()*0.5; phii+=drift_cell.GetAnodePitch()/1001.)
    {
      ++ie;
      xi=ri*TMath::Cos(phii);
      yi=ri*TMath::Sin(phii);
      if( ri == drift_cell.GetFieldWiresRadius() || ri == drift_cell.GetAnodeWiresRadius() )
	continue;
      if( !edrift.DriftElectron(xi,yi,zi,ti) ) continue;

      // drift time
      double t_d=edrift.GetDriftTime();
      // gain
      double gain=edrift.GetGain();
      //hgain.Fill(gain);
      // Lorentz Angle
      //      double alpha=LorentzAngle(xi,yi,zi,&sensor);
      double alpha=ElectricFieldHisto(xi,yi,zi,&sensor);
      double Xion,Yion,Zion,Tion;
      unsigned int NP = edrift.GetNumberOfDriftLinePoints();
      edrift.GetDriftLinePoint(NP-1,Xion,Yion,Zion,Tion);
      assert(TMath::Abs(TMath::Sqrt(Xion*Xion+Yion*Yion)-18.2)<=Rstep);
      // wire hit
      double phiIon = TMath::ATan2(Yion,Xion);
      if( phiIon < 0. ) phiIon+=TMath::TwoPi();

      //      ftd<<ri<<"\t"<<phiIon<<"\t"<<t_d<<"\t"<<alpha<<endl;
      ftd<<xi<<"\t"<<yi<<"\t"<<phiIon<<"\t"<<t_d<<"\t"<<alpha<<"\t"<<gain<<endl;
      // ntdrift.Fill(ri,phiIon,t_d,alpha);

#if DEBUG>0
      cerr<<"\t"<<ie<<"\t"<<ri
	  <<"\t"<<t_d<<"\t"<<phiIon*TMath::RadToDeg()<<"\t"
	  <<alpha*TMath::RadToDeg()<<"\t"<<gain<<endl;
#endif
	  //      ri+=Rstep;
    }
  ftd.close();
  cerr<<"END"<<endl;
  cerr<<"Number of Clusters: "<<ie<<endl;

  // fout->cd();
  // hgain.Write();
  // ntdrift.Write();
  // hEv.Write();
  // hrE.Write();
  // hrv.Write();
  // // cerr<<"Plot Driftlines"<<endl;
  // // viewdrift.Plot(true,true);
  // // viewCell.Plot2d();
  // // cc.SaveAs(".png");
  // // cc.Write();
  // // cf.Write();
  // // ce.Write();
  // fout->Close();

  return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
