#include <iostream>
#include <fstream>
#include <cstdlib>

#include "TMath.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TFile.h"
#include "TRandom3.h"

#include "Random.hh"

#include "MediumMagboltz.hh"
#include "DriftLineRKF.hh"
#include "Sensor.hh"

#include "TPC.hh"
#include "Helpers.hh"

using namespace std;
using namespace Garfield;

#include "VAF.hh"

int main()
{
  double CathodeVoltage=-4000.,// V
    MagneticField=1.; // T

  // TFile* fout = TFile::Open("Gain_DriftRKF_Ar-CO2-CF4.root","RECREATE");
  TFile* fout = TFile::Open("Gain_DriftRKF_Ar-CO2.root","RECREATE");
  TCanvas cg("gain", "gain", 2100, 1600);
  TGraph* ggain[5];
  //  int col[] ={kCyan,kViolet,kRed,kGreen,kBlue};
  int col[] ={kViolet,kCyan,kRed,kGreen,kBlue};
  //  TLegend* leg = new TLegend(0.13,0.68,0.3,0.89);

  const unsigned int aw_number = 65+256;
  const double maxStepSize=0.03;// cm

  // Set Seed for Random Generator
  unsigned int the_seed = 1985810831;
  randomEngine.Seed(the_seed);
  int Ne=1000;
  TRandom3 rndm;
  rndm.SetSeed(the_seed);

  double FieldVoltage = -100.; // V

  int iq = 0;
  double AWd = 30.;  

  double QuenchFraction=0.3;
  double pressure=725.;
  double qfrac = QuenchFraction*100.;
  // for(double qfrac=10.; qfrac<=50.; qfrac+=10.)
  //   {
      double arfrac = 100.-qfrac;
      cerr<<"### "<<iq<<" Ar-CO2 "<<arfrac<<":"<<qfrac<<" ###"<<endl;
      bool first=true;

      // Create the medium
      MediumMagboltz* gas = new MediumMagboltz();
      // Gas file created with other software

      TString gasfile = TString::Format("%s/gasfiles/ar_%.0f_co2_%.0f_NTP_20E200000_6B1.25.gas",
       					getenv("AGTPC_TABLES"),arfrac,qfrac);
      if( pressure )
	gasfile = TString::Format("%s/gasfiles/ar_%2.0f_co2_%2.0f_%1.0fTorr_20E200000_4B1.10.gas",
				  getenv("AGTPC_TABLES"),
				  (1.-QuenchFraction)*1.e2,QuenchFraction*1.e2,pressure);

      if( LoadGas(gas,gasfile.Data()) )
       	cerr<<gasfile<<endl;
      else
	return 1;
       	//break;
      //LoadGas(gas,"ar_64_co2_26_cf4_10_NTP_20E200000_0B0.gas");
      //      LoadGas(gas,gasfile.Data());

      TString garfdata = TString::Format("%s/Data/IonMobility_Ar+_Ar.txt",getenv("GARFIELD_HOME"));
      gas->LoadIonMobility(garfdata.Data());

      ggain[iq] = new TGraph();
      //      TString gname = TString::Format("ggain_ar_%.0f_co2_%.0f_cf4_10",arfrac,qfrac);
      TString gname = TString::Format("ggain_ar_%.0f_co2_%.0f",arfrac,qfrac);
      ggain[iq]->SetName(gname.Data());

      //  TString fname = TString::Format("gain_DriftRKF_ar_%.0f_co2_%.0f_cf4_10.dat",arfrac,qfrac);
      TString fname = TString::Format("gain_DriftRKF_ar_%.0f_co2_%.0f.dat",arfrac,qfrac);
      ofstream ofs(fname.Data());

      int nv=0;
      for(int iv = 7; iv<18; iv+=2)
	{
	  TPC drift_cell(CathodeVoltage,AnodeVoltage[iv],FieldVoltage);
	  //FieldVoltage[iv]);
	  drift_cell.SetPrototype(false);
	  drift_cell.SetMagneticField(0.,0.,MagneticField); // T
	  drift_cell.SetGas(gas);
	  drift_cell.init();

	  if( first ){
	    cerr<<"FW diam: "<<drift_cell.GetDiameterFieldWires()
		<<"cm \tRO rad: "<<drift_cell.GetROradius()<<" cm"<<endl;
	    first=false;}

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

	  double xw,yw,AV,l,q;
	  string alab;
	  int ntrp;
	  drift_cell.GetWire(aw_number,xw,yw,AWd,AV,alab,l,q,ntrp);
	  //	  double rad = double(ntrp+1)*AWd*0.5;
	  //	  double rad = 0.5;
	  double rad = 0.7999;
	  if(nv==0)
	    cerr<<"Wire 0: "<<alab
		<<" @ (x,y) = ("<<xw<<","<<yw
		<<") cm\tL = "<<l<<" cm\t(charge = "<<q<<")"<<endl;
	  if( AV != AnodeVoltage[iv] )
	    cerr<<"Anode Wire "<<alab<<" Voltage: "<<AV
		<<" V\tset point: "<<AnodeVoltage[iv]<<" V"<<endl;

	  DriftLineRKF edrift;
	  edrift.SetSensor(&sensor);
	  edrift.SetMaximumStepSize(maxStepSize);
	  edrift.EnableStepSizeLimit();

	  double xi,yi,
	    zi=0.,ti=0.,
	    csi,gain=0.,Tote=0.;
	  for( int ie=0; ie<Ne; ++ie )
	    {
	      // generate e- uniformly around the anode
	      csi = rndm.Uniform(0.,TMath::TwoPi());
	      xi = rad * TMath::Cos(csi);
	      yi = rad * TMath::Sin(csi);
	      xi += xw;
	      yi += yw;
	      if( (ie%100) == 0 ) cerr<<ie<<" @ (x,y) = ("<<xi<<","<<yi<<") cm"<<endl;

	      if( !edrift.DriftElectron(xi,yi,zi,ti) ) continue;

	      gain+=edrift.GetGain();
	      ++Tote;
	    }// e- loop

	  if( Tote != 0.)
	    gain/=Tote;
	  else
	    gain=0.;

	  // cerr<<"------------------------------------------------"<<endl;
	  // cerr<<" ******** GAIN = "<<gain<<" @ "<<AnodeVoltage[iv]<<" V ********"<<endl;
	  // cerr<<"------------------------------------------------"<<endl;
	  cerr<<nv<<"\t"<<AnodeVoltage[iv]<<'\t'<<gain<<endl;
	  ofs << AnodeVoltage[iv] << '\t' << gain << endl;
	  ggain[iq]->SetPoint(nv,AnodeVoltage[iv],gain);
	  ++nv;
	}// voltage loop
      ofs.close();

      //TString gtitle = TString::Format("rTPC Gain for Ar-CO_{2}-CF_{4} @ NTP for %1.0f #mum Anodes;Anode Voltage [V];Gain",
      //				       AWd*1.e4);
      TString gtitle = TString::Format("rTPC Gain for Ar-CO_{2} @ NTP for %1.0f #mum Anodes;Anode Voltage [V];Gain",
       				       AWd*1.e4);
      ggain[iq]->SetTitle(gtitle.Data());
      ggain[iq]->SetMarkerStyle(8);
      ggain[iq]->SetMarkerSize(2);
      ggain[iq]->SetMarkerColor(col[iq]);
      ggain[iq]->SetLineColor(col[iq]);
      // TString lname = TString::Format("%.0f:%.0f",arfrac,qfrac);
      // leg->AddEntry(ggain[iq],lname.Data(),"LP");

      if(iq==0)
	{
	  ggain[iq]->Draw("ALP");
	  ggain[iq]->GetXaxis()->SetNdivisions(328);
	  ggain[iq]->GetXaxis()->SetLabelSize(0.015);
	  ggain[iq]->GetYaxis()->SetTitleOffset(1.4);
	}
      else
	ggain[iq]->Draw("LPsame");

      ggain[iq]->Write();
      delete gas;
      ++iq;
      cerr<<"##############################################################################"<<endl;
      //    }// quencher frac loop

      //  leg->Draw("same");
  cg.SetGrid();
  cg.SetLogy();
  //  TString sname = TString::Format("Gain_Ar-CO2_NTP_%1.0fumWire.pdf",AWd*1.e4);
  TString sname = TString::Format("Gain_Ar-CO2_NTP_%1.0fumWire_B%1.2fT.pdf",AWd*1.e4,MagneticField);
  cg.SaveAs(sname.Data());

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
