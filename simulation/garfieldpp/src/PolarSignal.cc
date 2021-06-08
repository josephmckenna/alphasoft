#include <iostream>
#include <string>

#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>

#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/DriftLineRKF.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/ViewSignal.hh"
#include "Garfield/ViewCell.hh"
#include "Garfield/FundamentalConstants.hh"

using namespace Garfield;

#include "Helpers.hh"

int main(int argc, char * argv[]) 
{ 
  double //InitialPhi = 0., // rad
    InitialPhi = 40.*TMath::DegToRad(),
    InitialZed = 0., // cm
    //InitialRad = 10.925;
    InitialRad = 17.401;

  //  string tracking="driftMC";
  std::string tracking="driftRKF";

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

  // Make a gas medium.
  MediumMagboltz gas;
  TString gasdata = TString::Format("%s/simulation/common/gas_files/ar_70_co2_30_725Torr_20E200000_4B1.10.gas",getenv("AGRELEASE"));
  gas.LoadGasFile(gasdata.Data());
  TString iondata = TString::Format("%s/Data/IonMobility_Ar+_Ar.txt",
				     getenv("GARFIELD_HOME"));
  gas.LoadIonMobility(iondata.Data());

  // Define the cell layout.
  constexpr double lZ = 115.2 / 16.;
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
  std::cout << "Phi periodicity: " << sphi << std::endl;

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
      // if (i < nWires / 4) {
	cmp.AddReadout(ename);
	sensor.AddElectrode(&cmp, ename);
	// }
  }

  // Pads.
  constexpr double gap = rRO - rAW;
  constexpr int nSecs = 32;
  constexpr double pitchPhi = TwoPi / nSecs;
  constexpr double pitchZ = 0.4;
  constexpr int nRows = int(2 * lZ / pitchZ);
  std::cout << "Number of pad rows: " << nRows << std::endl;
  for (int j = 0; j < nRows; ++j) 
    {
      const double z0 = -lZ + j * pitchZ;
      const double z1 = z0 + pitchZ;
      std::string row = std::string(TString::Format("%03d", j).Data());
      for (int i = 0; i < nSecs; ++i) 
	{
	  std::string sec = std::string(TString::Format("%02d", i).Data());
	  const double phi0 = i * pitchPhi * RadToDegree; 
	  const double phi1 = phi0 + pitchPhi * RadToDegree;
	  std::string ename = "pad" + row + "_" + sec;
	  cmp.AddPixelOnPlaneR(rRO, phi0, phi1, z0, z1, ename, gap);
	  cmp.AddReadout(ename);
	  sensor.AddElectrode(&cmp, ename);
	}
    }

  const double tStart = 0.;
  const int nSteps = 411;
  const double tStep = 16.;

  sensor.SetTimeWindow(tStart, tStep, nSteps);

  // Construct object to visualise cell
  TCanvas cDrift;
  Garfield::ViewCell cellView;
  cellView.SetCanvas(&cDrift);
  cellView.SetComponent(&cmp);
  cellView.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
                    1.1 * rRO,  1.1 * rRO,  1.);
  cellView.EnableWireMarkers(false);

  // Construct object to visualise drift lines
  Garfield::ViewDrift viewdrift;
  viewdrift.SetCanvas(&cDrift);
  viewdrift.SetArea(-1.1 * rRO, -1.1 * rRO, -1.,
		    1.1 * rRO,  1.1 * rRO,  1.);

  // Construct object to visualise signal
  TCanvas cSignal;
  // Plot the induced current.
  Garfield::ViewSignal* signalView = new Garfield::ViewSignal();
  signalView->SetCanvas(&cSignal);
  signalView->SetSensor(&sensor);
  //  cSignal->Divide(3,3);

  //----------------------------------------------------
  // Transport Class for Electrons drift
  // Runge-Kutta
  Garfield::DriftLineRKF edrift;
  edrift.SetSensor(&sensor);
  const double maxStepSize=0.03;// cm
  edrift.SetMaximumStepSize(maxStepSize);
  edrift.EnablePlotting(&viewdrift);
  //----------------------------------------------------
  // Avalanche MC
  Garfield::AvalancheMC eaval;
  eaval.SetSensor(&sensor);
  eaval.EnableMagneticField();
  //  const double distanceStep = 2.e-3; // cm
  //  eaval.SetDistanceSteps(distanceStep);
  eaval.EnablePlotting(&viewdrift);

  //----------------------------------------------------
  // Signal Generation
  Garfield::AvalancheMC iaval;
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

  std::cout<<"\nBEGIN with "<<tracking<<std::endl;
  TString fname = TString::Format("./PolarSignal_%s_endpoints_phi%1.4f_Z%2.1fcm.dat",
				  tracking.c_str(),
				  InitialPhi,InitialZed);
  std::ofstream fout(fname.Data());
  fout<<"Using "<<tracking<<std::endl;

  // variables to identify the aw and pads
  int aw=-1, sec=-1, row=-1, idx=-1;
  double AnodeWiresPitch = sphi*TMath::DegToRad();

  // loop over all radii
  // or on a fixed number of e-
  while( ri<rRO && ie<emax )
    {
      //---------------- initial conditions -----------------------
      ++ie;
      xi=ri*TMath::Cos(phii);
      yi=ri*TMath::Sin(phii);
      std::cout<<ie<<")\tstart @ ("<<xi<<","<<yi<<","<<zi<<") cm\t";
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
	  std::cerr<<tracking<<" UNKNOWN"<<std::endl;
	  //break;
	  return 1;
	}
      // if (!ok) return 0;
      if( !ok ) 
      	{
      	  std::cerr<<tracking<<" FAILED"<<std::endl;
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
      std::cout<<"hit wire: "<<aw;//<<<endl;
      // find pads
      sec = int( phif/(2.*M_PI)*nSecs );
      double z = zf + lZ;
      row = int( z/(2.*lZ)*nRows );
      idx = sec + nSecs * row;
      std::cout<<"\thit pad: "<<idx<<" ("<<sec<<","<<row<<")"<<std::endl;

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

      std::stringstream ss;
      ss<<ie<<")\t"<<status<<"\t"
	<<ri<<" cm\t"<<tf<<" ns\t"<<lorentz_correction
	<<" deg\tz: "<<zf<<" cm\tAval Param: "<<ne<<","<<ni;

      if( !tracking.compare("driftRKF") )
	{
	  ss <<"\tsigma_t: "<<spread
	     <<" ns\tloss: "<<loss<<"\tgain: "<<gain<<std::endl;
	  if( t_d != tf ) 
	    {
	      std::cerr<<"Drift Time "<<t_d<<" ERROR: "<<tf<<std::endl;
	      ss<<"* tf:"<<tf<<std::endl;
	    }
	}
      else
	ss << "\n";
      //-----------------------------------------------------------

      // status and save
      fout<<ss.str();
      std::cout<<ss.str();

      //------------- signal generation ---------------------------
      double xx=xf,yy=yf,rr=sqrt(xf*xf+yf*yf),zz=zf;
      ok=false; 
      //      int att=0;
      while(!ok)
	{
	  std::cout<<"sig gen at r="<<rr<<" cm on aw: "<<aw<<" at "<<tf<<" ns"<<std::endl;
	  ok = iaval.DriftIon(xx,yy,zz,tf);
	  std::cout<<"done"<<std::endl;
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
  
  std::cout<<"END"<<std::endl;
  std::cout<<"Number of Clusters: "<<ie<<std::endl;
  fout<<"Number of Clusters: "<<ie<<std::endl;
  
  TString cname;
  fname = TString::Format("Outsignals_R%1.3fcm_phi%1.4f_Z%2.1fcm.root",
			  InitialRad,InitialPhi,InitialZed);
  TFile* froot = TFile::Open(fname,"RECREATE");
  std::cout<<"Plot Signal"<<std::endl;

  //  // pads have different readout
  //  Sensor ROsens(sensor);
 
  bool doconv=false;
  if( doconv )
    {
      // // AFTER Response Function
      // ROsens.SetTransferFunction(Hpads);
      // if( ROsens.ConvoluteSignals() ) cout<<"Pad readout succcess!!"<<endl;
      // else cout<<"Pad signal convolution unsuccesfull"<<endl;
      
      // AWB Response Function
      sensor.SetTransferFunction(Hands);
      if( sensor.ConvoluteSignals() ) std::cout<<"AW readout succcess!!"<<std::endl;
      else std::cout<<"AW signal convolution unsuccesfull"<<std::endl;
    }
  
  TString plot_signal_name;
  if( aw >= 0 )
    {
      plot_signal_name = TString::Format("a%03d",aw);
      std::cout<<"Plotting "<<plot_signal_name<<std::endl;
      signalView->PlotSignal(plot_signal_name.Data());
      //   signalView->PlotSignal("a");

      cname = TString::Format("./PolarSignal_signal%d_R%1.3fcm_phi%1.4f.pdf",
			      aw,InitialRad,InitialPhi);
      cSignal.SaveAs(cname.Data());
      std::cout<<cname<<" saved"<<std::endl;

      cname = TString::Format("AWsignals_R%1.3fcm_phi%1.4f_Z%2.1fcm",
			      InitialRad,InitialPhi,InitialZed);
      TCanvas* cawsigs= new TCanvas(cname,cname,1950,500);
      cawsigs->Divide(5,1);

      for(int aws=-2; aws<=2; ++aws)
	{
	  int aw_temp=aw+aws;
	  plot_signal_name = TString::Format("a%03d",aw_temp);
	  std::cout<<"Plotting "<<plot_signal_name<<std::endl;
	  TH1D* h = new TH1D( GetROSignal(&sensor,&plot_signal_name,doconv) );
	  cawsigs->cd(aws+3);
	  h->Draw();
	  //((TH1D*)h->Clone())->Write();
	  if( froot->IsOpen() ) h->Write();
	}
      cawsigs->SaveAs(TString::Format("./%s.pdf",cawsigs->GetName()));
   }
  else
    std::cout<<"I don't know which aw signal to plot"<<std::endl;

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
	      idx = sec_temp + nSecs * row_temp;
	      plot_signal_name = TString::Format("pad%05dsec%02drow%03d",idx,sec_temp,row_temp);
	      int pos = (sc+2)+3*(rw+2);
	      std::cout<<"Plotting "<<plot_signal_name<<" in "<<pos<<std::endl;
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
    std::cout<<"I don't know which pad signal to plot"<<std::endl;

  std::cout<<"Plot Driftlines"<<std::endl;
  viewdrift.Plot(true,true);
  cellView.Plot2d();
  cname = TString::Format("./PolarSignal_driftlines_R%1.3fcm_phi%1.4f_Z%2.1fcm.pdf",
			  InitialRad,InitialPhi,InitialZed);
  cDrift.SaveAs(cname.Data());
  std::cout<<cname<<" saved"<<std::endl;

  app.Run(true);

  return 0;
}
