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
#include <TH1D.h>
#include <TFile.h>

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
  double //InitialPhi = 0., // rad
    InitialPhi = 40.*TMath::DegToRad(),
    InitialZed = 0., // cm
    //InitialRad = 10.925;
    InitialRad = 17.401;

  //  string tracking="driftMC";
  string tracking="driftRKF";

   if( argc == 2 )
    {
      InitialRad     = atof(argv[1]);
    }
   else if( argc == 3 )
    {
      InitialRad     = atof(argv[1]);
      InitialPhi     = atof(argv[2])*TMath::DegToRad();
    }
  else if( argc == 4 )
    {
      InitialRad     = atof(argv[1]);
      InitialPhi     = atof(argv[2])*TMath::DegToRad();
      tracking       = argv[3];
    }
  else if( argc == 5 )
    {
      InitialRad     = atof(argv[1]);
      InitialPhi     = atof(argv[2])*TMath::DegToRad();
      InitialZed     = atof(argv[3]);
      tracking       = argv[4];
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
  //  constexpr double lZ = 115.2;
  constexpr double lZ = 115.2/16.;
  //  constexpr double lZ = 230.4/16.;
  //  constexpr double rRO = 19.03; <-- ???
  constexpr double rRO = 19.0;
  //  constexpr double rRO = 18.9;
  //  constexpr double rRO = 19.1;
  ComponentAnalyticField cmp;
  cmp.SetMedium(&gas);
  cmp.SetMagneticField(0.,0.,1.);
  cmp.SetPolarCoordinates(); // <-- the 'hat trick'
  // Outer wall.
  cmp.AddPlaneR(rRO, 0, "ro");
  // Inner wall.
  constexpr double rCathode = 10.925;
  cmp.AddPlaneR(rCathode, -4000., "c");
  int Nwires =  256;
  // Phi periodicity.
  const double sphi = 360. / double(Nwires);
  cout<<"Phi periodicity: "<<sphi<<endl;

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

  // Pads
  constexpr double gap = rRO - rAW;
  constexpr double pad_pitch_z = 0.4;
  //  constexpr int nrows = 576;
  constexpr int nrows = int(2.*lZ/pad_pitch_z);
  cout<<"Number of pad rows: "<<nrows<<endl;
  constexpr int nsecs= 32;
  constexpr double pad_pitch_phi = 2.*M_PI/double(nsecs);

  // cmp.AddWire(rFW, 0, dFW, vFW, "f", 2 * lZ, tFW);
  // cmp.AddWire(rAW, 0.5 * sphi, dAW, vAW, "a", 2 * lZ, tAW, 19.25); 
  // cmp.AddReadout("a");

  TString ename;
  // Field wires.
  for( int i = 0; i < Nwires; ++i )
    { 
      const double phi = i * sphi;
      ename = TString::Format("f%03d",i);
      //cout<<ename<<"\t"<<phi<<endl;
      cmp.AddWire(rFW, phi, dFW, 
 		  vFW, ename.Data(), 
 		  2 * lZ, tFW);
    }
  // Anode wires.
  for( int i = 0; i < Nwires; ++i )
    {
      const double phi = i * sphi + 0.5 * sphi;
      ename = TString::Format("a%03d",i);
      cmp.AddWire(rAW, phi , dAW, 
 		  vAW, ename.Data(), 
 		  2 * lZ, tAW, 
 		  19.25); 
      // cmp.AddReadout(ename.Data());
    }

  for( int i = 0; i < Nwires/4; ++i )
    {
      ename = TString::Format("a%03d",i);
      cmp.AddReadout(ename.Data());
    }

  // cathodic pads.
  int idx=-1;
  double PadPosZmin = -0.5 * nrows * pad_pitch_z;
  double PadPosZmax = PadPosZmin + pad_pitch_z;
  for(int row = 0; row<nrows; ++row)
    {
      double PadPosPhiMin = 0., PadPosPhiMax = pad_pitch_phi;
      for(int sec = 0; sec<nsecs; ++sec)
  	{
  	  idx = sec + nsecs * row;
  	  ename = TString::Format("pad%05dsec%02drow%03d",idx,sec,row);

  	  cmp.AddPixelOnPlaneR(rRO,
  			       PadPosPhiMin,PadPosPhiMax,
  			       PadPosZmin,PadPosZmax,
  			       ename.Data(),gap);
  	  PadPosPhiMin += pad_pitch_phi;
  	  PadPosPhiMax += pad_pitch_phi;
	  cmp.AddReadout(ename.Data());
  	}
      PadPosZmax += pad_pitch_z;
      PadPosZmin += pad_pitch_z;
    }

  //  cmp.SetPeriodicityPhi(sphi);
  //  cmp.PrintCell();
  if( cmp.IsPolar() ) cout<<"Hat trick"<<endl;

  const double tStart = 0.;
  // real = ADC
  const int nSteps = 411;
  const double tStep = 16.;
  // fake = zoom/debug
  // const int nSteps = 301;
  // const double tStep = 1.;

  // Finally assembling a Sensor object
  Sensor sensor;
  // Calculate the electric field
  sensor.AddComponent(&cmp);
  //  sensor.AddElectrode(&cmp,"a");

  for( int i = 0; i < Nwires/4; ++i )
    { 
      ename = TString::Format("a%03d",i);
      sensor.AddElectrode(&cmp,ename.Data());
    }

  for(int row = 0; row<nrows; ++row)
    for(int sec = 0; sec<nsecs; ++sec)
      {
  	idx = sec + nsecs * row;
  	ename = TString::Format("pad%05dsec%02drow%03d",idx,sec,row);
	sensor.AddElectrode(&cmp,ename.Data());
      }
  sensor.SetTimeWindow(tStart, tStep, nSteps);

  // Construct object to visualise cell
  TCanvas cDrift;
  ViewCell cellView;
  cellView.SetCanvas(&cDrift);
  cellView.SetComponent(&cmp);
  cellView.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
                    1.1 * rRO,  1.1 * rRO,  1.);
  cellView.EnableWireMarkers(false);

  // Construct object to visualise drift lines
  ViewDrift viewdrift;
  viewdrift.SetCanvas(&cDrift);
  viewdrift.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
		    1.1 * rRO,  1.1 * rRO,  1.);

  // Construct object to visualise signal
  TCanvas cSignal;
  // Plot the induced current.
  ViewSignal* signalView = new ViewSignal();
  signalView->SetCanvas(&cSignal);
  signalView->SetSensor(&sensor);
  //  cSignal->Divide(3,3);

  //----------------------------------------------------
  // Transport Class for Electrons drift
  // Runge-Kutta
  DriftLineRKF edrift;
  edrift.SetSensor(&sensor);
  // edrift.EnableSignalCalculation();
  // const double maxStepSize=0.03;// cm
  // edrift.SetMaximumStepSize(maxStepSize);
  edrift.EnablePlotting(&viewdrift);
  //----------------------------------------------------
  // Avalanche MC
  AvalancheMC eaval;
  eaval.SetSensor(&sensor);
  eaval.EnableMagneticField();
  // eaval.EnableSignalCalculation();
  //  const double distanceStep = 2.e-3; // cm
  //  eaval.SetDistanceSteps(distanceStep);
  eaval.EnablePlotting(&viewdrift);

  //----------------------------------------------------
  // Signal Generation
  AvalancheMC iaval;
  iaval.SetSensor(&sensor);
  iaval.EnableSignalCalculation();
  iaval.EnableMagneticField();  
  // const double distanceStep = 2.e-3; // cm
  // iaval.SetDistanceSteps(distanceStep);
  double np = 1.e5;
  iaval.SetIonSignalScalingFactor(np);
  //  iaval.EnableDebugging();
 //----------------------------------------------------

   // Electron initial point
  double xi,yi,zi=InitialZed,ti=0.0,phii = InitialPhi;
  double ri=InitialRad, Rstep = 0.05, Rministep=0.01;
  int ie = 0, emax=1;

  cout<<"\nBEGIN with "<<tracking<<endl;
  TString fname = TString::Format("./PolarSignal_%s_endpoints_phi%1.4f_Z%2.1fcm.dat",
				  tracking.c_str(),
				  InitialPhi,InitialZed);
  ofstream fout(fname.Data());
  fout<<"Using "<<tracking<<endl;

  // variables to identify the aw and pads
  int aw=-1, sec=-1, row=-1;
  double AnodeWiresPitch = sphi*TMath::DegToRad();

  // loop over all radii
  // or on a fixed number of e-
  while( ri<rRO && ie<emax )
    {
      //---------------- initial conditions -----------------------
      ++ie;
      xi=ri*TMath::Cos(phii);
      yi=ri*TMath::Sin(phii);
      cout<<ie<<")\tstart @ ("<<xi<<","<<yi<<","<<zi<<") cm\t";
      //-----------------------------------------------------------

      //------------------ionization drift ------------------------
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
	  //break;
	  return 1;
	}
      // if (!ok) return 0;
      if( !ok ) 
      	{
      	  cerr<<tracking<<" FAILED"<<endl;
      	  --ie;
      	  ri+=Rministep;
      	  continue;
      	  //return 1;
      	}
      //-----------------------------------------------------------

      //--------------- final state -------------------------------
      double xf,yf,zf,tf,phif;
      int status;
      
      if( !tracking.compare("driftMC") || !tracking.compare("avalMC") )
      eaval.GetElectronEndpoint(0, 
				xi, yi, zi, ti,
				xf, yf, zf, tf,
				status);
      else if( !tracking.compare("driftRKF") )
	edrift.GetEndPoint(xf,yf,zf,tf,status);
      else return 1; //break;
	
      assert(TMath::Abs(TMath::Sqrt(xi*xi+yi*yi)-ri)<2.e-3);
      assert(zi==InitialZed);
      assert(ti==0.0);
      phif=TMath::ATan2(yf,xf);

      // find anode wire
      if( phif < 0. ) phif += TMath::TwoPi();
      double w = phif/AnodeWiresPitch-0.5;
      aw = (ceil(w)-w)<(w-floor(w))?int(ceil(w)):int(floor(w));
      //cout<<"hit: "<<w<<"\t"<<aw<<endl;
      cout<<"hit wire: "<<aw;//<<<endl;
      // find pads
      sec = int( phif/(2.*M_PI)*nsecs );
      double z = zf + lZ;
      row = int( z/(2.*lZ)*nrows );
      idx = sec + nsecs * row;
      cout<<"\thit pad: "<<idx<<" ("<<sec<<","<<row<<")"<<endl;

      double ne, ni, t_d=-1., gain=-1., spread=-1., loss=-1.;

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
	  // // arrival time distribution
	  // spread = edrift.GetArrivalTimeSpread();
	  // // attachment loss
	  // loss = edrift.GetLoss();
	}
      else return 1;// break;
      
      double lorentz_correction = (phif-InitialPhi);
      if( lorentz_correction < 0. ) lorentz_correction += TMath::TwoPi();
      lorentz_correction *= TMath::RadToDeg();

      stringstream ss;
      ss<<ie<<")\t"<<status<<"\t"
	<<ri<<" cm\t"<<tf<<" ns\t"<<lorentz_correction
	<<" deg\tz: "<<zf<<" cm\tAval Param: "<<ne<<","<<ni;

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
      //-----------------------------------------------------------

      // status and save
      fout<<ss.str();
      cout<<ss.str();

      //------------- signal generation ---------------------------
      double xx=xf,yy=yf,rr=sqrt(xf*xf+yf*yf),zz=zf;
      ok=false; 
      //      int att=0;
      while(!ok)
	{
	  cout<<"sig gen at r="<<rr<<" cm on aw: "<<aw<<" at "<<tf<<" ns"<<endl;
	  ok = iaval.DriftIon(xx,yy,zz,tf);
	  cout<<"done"<<endl;
	  // ++att;
	  // if(att%2==0) xx-=0.0001;
	  // else if(att%3==0) yy+=0.0001;
	  // else if(att%4==0) xx+=0.0001;
	  // else yy-=0.0001;
	  // rr=sqrt(xx*xx+yy*yy);
	}
      //-----------------------------------------------------------

      ri+=Rstep;
    }
  
  cout<<"END"<<endl;
  cout<<"Number of Clusters: "<<ie<<endl;
  fout<<"Number of Clusters: "<<ie<<endl;
  
  TString cname;
  fname = TString::Format("Outsignals_R%1.3fcm_phi%1.4f_Z%2.1fcm.root",
			  InitialRad,InitialPhi,InitialZed);
  TFile* froot = TFile::Open(fname,"RECREATE");
  cout<<"Plot Signal"<<endl;

  // pads have different readout
  Sensor ROsens(sensor);
 
  bool doconv=false;
  if( doconv )
    {
      // AFTER Response Function
      ROsens.SetTransferFunction(Hpads);
      if( ROsens.ConvoluteSignals() ) cout<<"Pad readout succcess!!"<<endl;
      else cout<<"Pad signal convolution unsuccesfull"<<endl;
      
      // AWB Response Function
      sensor.SetTransferFunction(Hands);
      if( sensor.ConvoluteSignals() ) cout<<"AW readout succcess!!"<<endl;
      else cout<<"AW signal convolution unsuccesfull"<<endl;
    }
  
  TString plot_signal_name;
  if( aw >= 0 )
    {
      plot_signal_name = TString::Format("a%03d",aw);
      cout<<"Plotting "<<plot_signal_name<<endl;
      signalView->PlotSignal(plot_signal_name.Data());
      //   signalView->PlotSignal("a");

      cname = TString::Format("./PolarSignal_signal%d_R%1.3fcm_phi%1.4f.pdf",
			      aw,InitialRad,InitialPhi);
      cSignal.SaveAs(cname.Data());
      cout<<cname<<" saved"<<endl;

      cname = TString::Format("AWsignals_R%1.3fcm_phi%1.4f_Z%2.1fcm",
			      InitialRad,InitialPhi,InitialZed);
      TCanvas* cawsigs= new TCanvas(cname,cname,1950,500);
      cawsigs->Divide(5,1);

      for(int aws=-2; aws<=2; ++aws)
	{
	  int aw_temp=aw+aws;
	  plot_signal_name = TString::Format("a%03d",aw_temp);
	  cout<<"Plotting "<<plot_signal_name<<endl;
	  TH1D* h = new TH1D( GetROSignal(&sensor,&plot_signal_name,doconv) );
	  cawsigs->cd(aws+3);
	  h->Draw();
	  //((TH1D*)h->Clone())->Write();
	  if( froot->IsOpen() ) h->Write();
	}
      cawsigs->SaveAs(TString::Format("./%s.pdf",cawsigs->GetName()));
   }
  else
    cout<<"I don't know which aw signal to plot"<<endl;

  if( sec >=0 && row >=0 )
    {
      cname = TString::Format("PadSignals_R%1.3fcm_phi%1.4f_Z%2.1fcm",
			      InitialRad,InitialPhi,InitialZed);
      TCanvas* cpadsigs= new TCanvas(cname,cname,1700,1950);
      cpadsigs->Divide(3,5);
      for(int sc=-1; sc<=1; ++sc)
	{
	  int sec_temp = sec+sc;
	  for(int rw=-2; rw<=2; ++rw)
	    {
	      int row_temp = row+rw;
	      idx = sec_temp + nsecs * row_temp;
	      plot_signal_name = TString::Format("pad%05dsec%02drow%03d",idx,sec_temp,row_temp);
	      int pos = (sc+2)+3*(rw+2);
	      cout<<"Plotting "<<plot_signal_name<<" in "<<pos<<endl;
	      TH1D* h = new TH1D( GetROSignal(&sensor,&plot_signal_name,doconv) );
	      cpadsigs->cd(pos);
	      h->Draw();
	      //((TH1D*)h->Clone())->Write();
	      if( froot->IsOpen() ) h->Write();
	    }
	}
      cpadsigs->SaveAs(TString::Format("./%s.pdf",cpadsigs->GetName()));
    }
  else
    cout<<"I don't know which pad signal to plot"<<endl;

  cout<<"Plot Driftlines"<<endl;
  viewdrift.Plot(true,true);
  cellView.Plot2d();
  cname = TString::Format("./PolarSignal_driftlines_R%1.3fcm_phi%1.4f_Z%2.1fcm.pdf",
			  InitialRad,InitialPhi,InitialZed);
  cDrift.SaveAs(cname.Data());
  cout<<cname<<" saved"<<endl;

  app.Run(true);

  return 0;
}
