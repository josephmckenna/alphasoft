// TPC Electron Drift
// uses tabulated data from Garfield calculation 
// to determine for each primary ionization the 
// drift time and the Lorentz angle
//------------------------------------------------
// Author: A.Capra   Nov 2014
//------------------------------------------------

#include "TElectronDrift.hh"
#include <TMath.h>
#include <TString.h>
#include <vector>
#include <fstream>
#include <iostream>

extern double gMagneticField;
extern double gQuencherFraction;

TElectronDrift* TElectronDrift::fElectronDrift=0;

TElectronDrift::TElectronDrift()
{
  //#include "./ElectronDrift.dat"
  
  // std::vector<double> radpos(rad, rad + sizeof(rad) / sizeof(double) ); // cm
  // std::vector<double> drift(td, td + sizeof(td) / sizeof(double) ); // ns
  // std::vector<double> wire(phi, phi + sizeof(phi) / sizeof(double) ); // rad

  TString electrondrift_name = TString::Format("%s/simulation/common/driftTables/QF%2.0f/ElectronDrift_B%1.2fT.dat",
					       getenv("AGRELEASE"),
					       gQuencherFraction*1.e2,
					       gMagneticField);
  std::cout<<"TElectronDrift::TElectronDrift() "<<electrondrift_name<<" ... ";
  std::ifstream electrondrift(electrondrift_name.Data());
  if( electrondrift.is_open() )
    std::cout<<"OK"<<std::endl;
  else
    std::cout<<"FAIL"<<std::endl;
    
  double r,t,w;
  std::vector<double> radpos; // cm
  std::vector<double> drift; // ns
  std::vector<double> wire; // rad
  while(1)
    {
      electrondrift>>r>>t>>w;
      if( !electrondrift.good() ) break;
      radpos.push_back(r);
      drift.push_back(t);
      wire.push_back(w);
    }
  electrondrift.close();

  // finterpol_rtime = new ROOT::Math::Interpolator( radpos, drift, 
  // 						  ROOT::Math::Interpolation::kLINEAR );
  // finterpol_rphi  = new ROOT::Math::Interpolator( radpos, wire, 
  // 						  ROOT::Math::Interpolation::kLINEAR );
  finterpol_rtime = new ROOT::Math::Interpolator( radpos, drift, 
						  ROOT::Math::Interpolation::kCSPLINE );
  finterpol_rphi  = new ROOT::Math::Interpolator( radpos, wire, 
						  ROOT::Math::Interpolation::kCSPLINE );

  fAnodeSignal = new double[fTimeBins];
  TString awresponse_name = TString::Format("%s/simulation/common/response/anodeResponse.dat",
					    getenv("AGRELEASE"));
  std::ifstream awresponse(awresponse_name.Data());
  int bin; double V;
  while(1)
    {
      awresponse>>bin>>V;
      if( !awresponse.good() ) break;
      fAnodeSignal[bin] = V;
    }
  awresponse.close();

  fPadSignal = new double[fTimeBins];
  TString padresponse_name = TString::Format("%s/simulation/common/response/padResponse_pRO.dat",
					     getenv("AGRELEASE"));
  std::ifstream padresponse(padresponse_name.Data());
  while(1)
    {
      padresponse>>bin>>V;
      if( !padresponse.good() ) break;
      fPadSignal[bin] = V;
    }
  padresponse.close();

  //  double indp[] = {0.891626,0.631976,0.356072,0.159466,0.0567621,0.016057};
  // double indp[] = {0.89,0.63,0.36,0.16,0.06,0.02};
  // double inda[] = {-0.13,-0.04,-0.01};

  fInductionPads = new double[6];
  // for(int i=0; i<6; ++i)
  //   fInductionPads[i] = indp[i];
  fInductionPads[0] = 0.89; fInductionPads[1] = 0.63; 
  fInductionPads[2] = 0.36; fInductionPads[3] = 0.16; 
  fInductionPads[4] = 0.06; fInductionPads[5] = 0.02;

  fInductionAnodes = new double[3];
  // for(int i=0; i<6; ++i)
  //   fInductionAnodes[i] = inda[i];
  fInductionAnodes[0] = -0.13; 
  fInductionAnodes[1] = -0.04;
  fInductionAnodes[2] = -0.01;
}

TElectronDrift::~TElectronDrift()
{
  if(finterpol_rtime) delete finterpol_rtime;
  if(finterpol_rphi) delete finterpol_rphi;
  delete[] fAnodeSignal;
  delete[] fPadSignal;
  delete[] fInductionPads;
  delete[] fInductionAnodes;
}

double TElectronDrift::GetTime(double r)
{
  // parameter in mm
  // returns ns
  return finterpol_rtime->Eval(r*0.1);
}

double TElectronDrift::GetAzimuth(double r)
{  
  // parameter in mm
  // returns rad
  return finterpol_rphi->Eval(r*0.1);
}

TElectronDrift* TElectronDrift::ElectronDriftInstance()
{
  if(!fElectronDrift) fElectronDrift=new TElectronDrift();
  return fElectronDrift;
}

double TElectronDrift::GetAnodeSignal(int t) const
{
  if(t<fTimeBins) 
    return fAnodeSignal[t];
  else
    return 0.;
}

double TElectronDrift::GetPadSignal(int t) const
{
  if(t<fTimeBins) 
    return fPadSignal[t];
  else
    return 0.;
}

// double TElectronDrift::GetPadInduction(const double z, const double pos)
// {
//   double G = TPCBase::TPCBaseInstance()->GetROradius()-TPCBase::TPCBaseInstance()->GetAnodeWiresRadius();
//   double sigma = 2. * G / 2.34;
//   TF1* f = new TF1("fPadsChargeProfile","TMath::Gaus(x,[0],[1],0)",z-6.*sigma,z+6.*sigma);
//   f->SetParameters(z,sigma); // true z position
//   double ind = f->Eval(pos); // at pad centre
//   delete f;
//   return ind;
// }
ClassImp(TElectronDrift)
