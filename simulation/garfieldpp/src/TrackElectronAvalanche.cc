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

#include "Garfield/MediumMagboltz.hh"
#include "Garfield/GeometrySimple.hh"
#include "Garfield/SolidTube.hh"

#include "TPC.hh"
#include "Garfield/Sensor.hh"

#include "Garfield/TrackHeed.hh"
#include "Garfield/DriftLineRKF.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/AvalancheMicroscopic.hh"

#include "Garfield/ViewCell.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewDrift.hh"

#include <TRandom2.h>
#include "Garfield/Random.hh"

#include "Helpers.hh"

#define DEBUG 0


int main(int argc, char * argv[])
{
  double r0=10.9251, phi0=TMath::PiOver2(), z0=0., B=1.,
    theta0 = TMath::PiOver2(),
    phi00=phi0;
  if( argc == 3 )
    {
      phi0 = atof(argv[1]); // rad
      z0   = atof(argv[2]); // cm
      phi00=phi0;
    }
  else if( argc == 4 )
    {
      phi0 = atof(argv[1]); // rad
      z0   = atof(argv[2]); // cm
      B    = atof(argv[3]); // T
      phi00=phi0;
    }
    else if( argc == 5 )
    {
      phi0 = atof(argv[1]); // rad
      z0   = atof(argv[2]); // cm
      B    = atof(argv[3]); // T
      theta0 = atof(argv[4]); // rad
      phi00=phi0;
    }
    else if( argc == 6 )
    {
      phi0 = atof(argv[1]); // rad
      z0   = atof(argv[2]); // cm
      B    = atof(argv[3]); // T
      theta0 = atof(argv[4]); // rad
      phi00 = atof(argv[5]); // rad
    }

  double CathodeVoltage = -4000.,// V
    AnodeVoltage = 3200.,
     FieldVoltage = -110.;

  double MagneticField=abs(B); // T

  unsigned int the_seed = 919850618;
  Garfield::randomEngine.Seed(the_seed);

  TString fname;
  if( B >= 0. )
    fname = TString::Format("%s/TrackAvalanche_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT_initR%1.2fcm_initPhi%1.3frad_initTheta%1.3frad_initZ%1.2fcm.root",
                            getenv("GARFIELDPP"),
			    CathodeVoltage,AnodeVoltage,FieldVoltage,
			    MagneticField,
			    r0,phi0,theta0,z0);
  else
     fname = TString::Format("%s/TrackAvalanche_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT_initR%1.2fcm_initPhi%1.3frad_initTheta%1.3frad_initZ%1.2fcm_Bmap.root",
                             getenv("GARFIELDPP"),
			    CathodeVoltage,AnodeVoltage,FieldVoltage,
			    MagneticField,
                             r0,phi0,theta0,z0);
  TFile* fout = TFile::Open(fname.Data(),"RECREATE");

  if(!fout->IsOpen())
    {
      std::cerr << "Something went wrong opening the output file. Does the directory exist?" << std::endl;
      return -1;
    }

  TNtuple ntefin("ntefin","e- endpoint","x:y:z:t:n");
  TNtuple ntionpos("ntionpos","ion position","x:y:z:t");
  TNtuple ntfionpos("ntfionpos","ion final position","x:y:z:t");
  TNtuple ntpion("ntpion","MCpion","x:y:z:n");

  // Create the medium
  Garfield::MediumMagboltz *gas = new Garfield::MediumMagboltz;
  // Gas file created with other software
  TString gasfile = TString::Format("%s/simulation/common/gas_files/ar_70_co2_30_725Torr_20E200000_4B1.10.gas",
				    getenv("AGRELEASE"));
  if( LoadGas(gas,gasfile.Data()) )
    std::cerr<<gasfile<<std::endl;
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
      std::cerr<<"Magnetic Field set to "<<MagneticField<<" T"<<std::endl;
    }
  else
    {
      if( !drift_cell.SetSymmetries("rz") )
	{
	  std::cerr<<"set symm failed"<<std::endl;
	  return -46;
	}
      // set scale parameter to convert Gauss to Tesla (or to go to 0.65T later)
      if( !drift_cell.ReadMagneticFieldMap(BfieldMap.Data(), MagneticField*1.e-4) )
	{
	  std::cerr<<"READ B-map failed"<<std::endl;
	  return -49;
	}
      std::cerr<<"Magnetic Field Map set (scaled to "<<MagneticField*1.e-4<<")"<<std::endl;
    }
  drift_cell.SetGas(gas);
  drift_cell.init();

  // Finally assembling a Sensor object
  Garfield::Sensor sensor;
  //  sensor.DisableDebugging();
  // Calculate the electric field
  sensor.AddComponent(&drift_cell);

  // Request signal calculation for the electrode named with labels above,
  // using the weighting field provided by the Component object cmp.
  std::vector<std::string> anodes = drift_cell.GetAnodeReadouts();
  for(unsigned int a=0; a < anodes.size(); ++a)
    sensor.AddElectrode(&drift_cell,anodes[a]);

  sensor.AddElectrode(&drift_cell, "ro");

  // Set Time window for signal integration, units in [ns]
  const double tMin = 0.;
  const double tMax = 7000.; // ns = 10 us
  const double tStep = 10.0;
  const int nTimeBins = int((tMax - tMin) / tStep);
  sensor.SetTimeWindow(0., tStep, nTimeBins);


  //----------------------------------------------------
  // Visualizations
  // area
  double areaX1=-drift_cell.GetROradius(),
    areaX2=drift_cell.GetROradius(),
    areaY1=-drift_cell.GetROradius(),
    areaY2=drift_cell.GetROradius(),
    areaZ1=-0.4,areaZ2=0.4;

  // My Cell
  // This canvas will be used to display the drift lines
  TString ccname = TString::Format("avalanchetrack_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_initR%1.2fcm_initPhi%1.2frad_initZ%1.2fcm",
				   CathodeVoltage,AnodeVoltage,FieldVoltage,r0,phi0,z0);

  TCanvas cc(ccname.Data(),ccname.Data(), 1400, 1400);
  Garfield::ViewCell viewCell;
  viewCell.SetComponent(&drift_cell);
  viewCell.SetCanvas(&cc);
  viewCell.SetArea(areaX1,areaY1,areaZ1,areaX2,areaY2,areaZ2);

  // Construct object to visualise drift lines
  Garfield::ViewDrift viewdrift;
  viewdrift.SetCanvas(&cc);
  viewdrift.SetArea(areaX1,areaY1,areaZ1,areaX2,areaY2,areaZ2);
  //----------------------------------------------------


  //----------------------------------------------------
  //----------------------------------------------------
  // TRACK GENERATION
  // charged particle: 250 MeV pi-
  double momentum = 250.e6; // eV/c
  Garfield::TrackHeed track;
  track.SetSensor(&sensor);
  track.SetParticle("pi");
  track.SetMomentum(momentum);
#ifdef OLD_AV_MC
  if( MagneticField > 0. )
     track.EnableMagneticField();
#endif
  //  track.EnableElectricField();
  track.EnablePlotting(&viewdrift);
  //----------------------------------------------------

  //----------------------------------------------------
  //----------------------------------------------------
  // Transport Class for Electrons
  // MC Avalanche
  Garfield::AvalancheMC eaval;
  eaval.SetSensor(&sensor);
#ifdef OLD_AV_MC
  if( MagneticField > 0. )
     eaval.EnableMagneticField();
#endif
  // eaval.EnableSignalCalculation();
  const double step = 1.e-4;// cm = 1 um
  //  const double step = 0.03;// cm = 300 um
  eaval.SetDistanceSteps(step);
  eaval.EnablePlotting(&viewdrift);
  // //----------------------------------------------------
  // // Microscopic Avalanche
  // Garfield::AvalancheMicroscopic eaval;
  // eaval.SetSensor(&sensor);
  // eaval.EnableMagneticField();
  // //  eaval.DisableSignalCalculation();
  // eaval.EnablePlotting(&viewdrift);
  //----------------------------------------------------

  //----------------------------------------------------
  //----------------------------------------------------
  // Transport Class for Ions
  // MC Avalanche
  Garfield::AvalancheMC iaval;
  iaval.SetSensor(&sensor);
#ifdef OLD_AV_MC
  iaval.EnableMagneticField();
#endif
  iaval.EnableSignalCalculation();
  const double dist_step = 2.e-4;// cm = 2 um
  iaval.SetDistanceSteps(dist_step);
  //  iaval.EnablePlotting(&viewdrift);
  //----------------------------------------------------


  //----------------------------------------------------
  // I.C. set-up
  TRandom2 rndm;
  rndm.SetSeed(the_seed);

  double t0=0.; //ns
  std::cerr<<" r0 = "<<r0<<" cm; phi0 = "<<phi0<<" rad = "<<phi0*TMath::RadToDeg()<<" deg"<<std::endl;
  double x0 = r0*TMath::Cos(phi0), y0 = r0*TMath::Sin(phi0); //cm
  std::cerr<<" x0 = "<<x0<<"; y0 = "<<y0<<"; z0 = "<<z0<<"  cm"<<std::endl;
  std::cerr<<" dir phi = "<<phi00<<" rad = "<<phi00*TMath::RadToDeg()<<" deg"
      <<" dir theta = "<<theta0<<" rad = "<<theta0*TMath::RadToDeg()<<" deg"<<std::endl;
  double dx = TMath::Cos(phi00)*TMath::Sin(theta0),
    dy = TMath::Sin(phi00)*TMath::Sin(theta0),
    dz = TMath::Cos(theta0);
  std::cerr<<" dir(x,y,z) = ( "<<dx<<" , "<<dy<<" , "<<dz<<")"<<std::endl;
  //----------------------------------------------------


  //----------------------------------------------------
  // Start simulation
  std::cerr<<"\nBEGIN"<<std::endl;
  sensor.ClearSignal();
  sensor.NewSignal();

  std::cout<<"\n=========== TrackHeed ==========="<<std::endl;
  // CREATE pi- track
  track.NewTrack(x0, y0, z0, t0, dx, dy, dz);
  std::cout<<"Cluster Density: "<<track.GetClusterDensity()<<" cm^-1"<<std::endl;
  std::cout<<"Stopping Power: "<<track.GetStoppingPower()<<" eV/cm"<<std::endl;
  std::cout<<"Asymptotic W Value: "<<track.GetW()<<" eV"<<std::endl;
  std::cout<<"Fano Factor: "<<track.GetFanoFactor()<<std::endl;
  std::cout<<"=================================\n"<<std::endl;

  // position and time of the cluster and energy deposit
  double xcl,ycl,zcl,tcl,ecl,extra;
  // number of electrons in the cluster
  int ncl,
    // number of clusters
    Nclusters=0,
    // total number of electrons
    Ne=0;// ,
    // // total number of electrons reaching Anode Wires
    // NeAW=0;

  double Nions = 100.;
  // LOOP over clusters produced by ionizing radiation
  while(track.GetCluster(xcl, ycl, zcl, tcl, ncl, ecl, extra))
    {
      std::cerr<<"========================================================"<<std::endl;
      std::cerr<<"Cluster # "<<Nclusters<<" @ ("<<xcl<<", "<<ycl<<", "<<zcl
	  <<") cm\ttime = "<<tcl<<" ns"<<std::endl;

      ntpion.Fill(xcl,ycl,zcl,ncl);
      ++Nclusters;

      std::cerr<<"Number of e- in the cluster: "<<ncl<<std::endl;
      Ne+=ncl;

      // Electron initial point
      double xe,ye,ze,te,re/*,phie*/;
      // Electron energy and initial direction
      double ee,dxe,dye,dze;
      for(int ie = 0; ie < ncl; ++ie)
	{
	  track.GetElectron(ie,xe,ye,ze,te,ee,dxe,dye,dze);

          re = TMath::Sqrt(xe*xe+ye*ye);
          //phie = TMath::ATan2(ye,xe);
          std::cerr<<"\te- "<<ie<<") pos: ("<<xe<<","<<ye<<","<<ze<<") r="<<re<<" at: "<<te<<std::endl;
	  //eaval.AvalancheElectron(xe,ye,ze,te,ee);
          eaval.DriftElectron(xe,ye,ze,te);

          //          int ne,ni;
	  unsigned int ne,ni;
	  ne=ni=0;
	  eaval.GetAvalancheSize(ne,ni);
	  std::cerr<<"\tAvalanche Size: #ions = "<<ni<<"; #e- "<<ne<<std::endl;

	  double exf,eyf,ezf,etf,eef;
          exf=eyf=ezf=etf=eef=-1.;

	  iaval.SetIonSignalScalingFactor(double(ni));
	  // std::cerr<<"\tTracking "<<Nions
	  //     <<" ions out of "<<eaval.GetNumberOfElectronEndpoints()<<std::endl;
          //std::cerr<<"\tTracking "<<eaval.GetNumberOfElectronEndpoints()<<" e-"<<std::endl;
          for(uint i=0; i<eaval.GetNumberOfElectronEndpoints(); ++i)
	     {
                double xi,yi,zi,ti,ei,
                   xf,yf,zf,tf,ef;
	      int status;
	      eaval.GetElectronEndpoint(i,
	        			xi, yi, zi, ti,// ei,
	        			xf, yf, zf, tf,// ef,
	        			status);

              double ri = TMath::Sqrt(xi*xi+yi*yi);
              if( TMath::Abs(ri-re) < 1.e-3 )
	        {
	          exf=xf;
	          eyf=yf;
	          ezf=zf;
	          etf=tf;
	          eef=ef;
                }
	      ntionpos.Fill(xi,yi,zi,ti);

	      std::cerr<<"\t init: ("<<xi<<","<<yi<<","<<zi<<")\tr = "<<ri
                       <<" cm; phi = "<<TMath::ATan2(yi,xi)*TMath::RadToDeg()<<" deg\tt ="<<ti<<" ns"<<std::endl;
              std::cerr<<"\tfinal: ("<<xf<<","<<yf<<","<<zf<<")\tr = "<<TMath::Sqrt(xf*xf+yf*yf)
                       <<" cm; phi = "<<TMath::ATan2(yf,xf)*TMath::RadToDeg()<<" deg\tt ="<<tf<<" ns"<<std::endl;

              if( i >= Nions ) continue;
              iaval.DriftIon(xi,yi,zi,ti);

	      unsigned int NN = iaval.GetNumberOfDriftLinePoints();
	      double xf_ion,yf_ion,zf_ion,tf_ion;
	      iaval.GetDriftLinePoint(NN-1, xf_ion, yf_ion, zf_ion, tf_ion);
              std::cerr<<"\tion endpoint: "<<xf_ion<<"\t"<<yf_ion<<"\t"<<zf_ion<<"\t"<<tf_ion<<"\trad: "<<TMath::Sqrt(xf_ion*xf_ion+yf_ion*yf_ion)<<" cm"<<std::endl;
              ntfionpos.Fill(xf_ion, yf_ion, zf_ion, tf_ion);
             }

	  ntefin.Fill(exf,eyf,ezf,etf,double(ni));
	  std::cerr<<"\te- endpoint: "<<exf<<"\t"<<eyf<<"\t"<<ezf<<"\t"<<etf<<"\t"<<eef<<"\n"<<std::endl;
	} // loop over e- in cluster
      std::cerr<<"========================================================\n"<<std::endl;
    }// loop over clusters
  std::cerr<<"END"<<std::endl;

  fout->cd();
  ntefin.Write();
  ntionpos.Write();
  ntfionpos.Write();
  ntpion.Write();

  std::cerr<<"Plot Driftlines"<<std::endl;
  viewdrift.Plot(true,true);
  viewCell.Plot2d();
  //cc.SaveAs(".pdf");
  TString sname(getenv("GARFIELDPP"));
  sname+="/";  sname+=ccname; sname+=".pdf";
  cc.SaveAs(sname);
  fout->cd();
  cc.Write();

  std::cerr<<"READOUT!"<<std::endl;
  //  Sensor ROsens(sensor);
  // AFTER Response Function
  //  ROsens.SetTransferFunction(Hpads);

  // Pre-Amp Response Function
  sensor.SetTransferFunction(Hands);

  bool convolute = true;
  if( convolute )
    {
      std::cerr<<"Apply the transfer function"<<std::endl;
      // Apply the transfer function
      sensor.ConvoluteSignals();
      //      ROsens.ConvoluteSignals();
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
  //  TH1D hro = GetROSignal(&ROsens, &roname, convolute);
  TH1D hro = GetROSignal(&sensor, &roname, convolute);
  hro.Write();
  std::cerr<<"READOUT complete"<<std::endl;

  fout->Close();

  return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
