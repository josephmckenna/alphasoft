// Look-up Table class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#include "TLookUpTable.hh"
#include <TMath.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <sys/stat.h>

using std::cerr;
using std::cout;
using std::endl;

TLookUpTable* TLookUpTable::fLookUpTable=0;

TLookUpTable::TLookUpTable()
{
  #include "./LookUp.dat"

  std::vector<double> radpos (rad, rad + sizeof(rad) / sizeof(double) );// cm
  std::vector<double> drift (td, td + sizeof(td) / sizeof(double) );// ns
  std::vector<double> wire (phi, phi + sizeof(phi) / sizeof(double) );// rad

  fMinTime = drift.front();
  fMaxTime = drift.back();

  finterpol_trad  = new ROOT::Math::Interpolator(drift, radpos, ROOT::Math::Interpolation::kLINEAR);
  finterpol_tphi  = new ROOT::Math::Interpolator(drift, wire, ROOT::Math::Interpolation::kLINEAR);
}

TLookUpTable::TLookUpTable(const std::string &gas, double quencherFrac, double B): finterpol_trad(nullptr), finterpol_tphi(nullptr), finterpol_tdrad(nullptr), finterpol_tdphi(nullptr) {
    SetGas(gas, quencherFrac, B);
}

TLookUpTable::~TLookUpTable()
{
  if(finterpol_trad) delete finterpol_trad;
  if(finterpol_tphi) delete finterpol_tphi;
  if(finterpol_tdrad) delete finterpol_tdrad;
  if(finterpol_tdphi) delete finterpol_tdphi;
}

bool TLookUpTable::SetGas(const std::string &gas, double quencherFrac, double B){
    std::cout << "TLookUpTable::SetGas(" << gas << ", " << quencherFrac << ')' << std::endl;
    std::ostringstream oss;
    int percent = std::rint(100.*quencherFrac);
    oss << getenv("AGTPC_TABLES") << "/driftTables/" << gas << "/QF" << percent;
    gasdir = oss.str();
    struct stat sb;
    std::cout<<"Gas directory "<<gasdir;
    if (stat(gasdir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)){
        std::cout << " found." << std::endl;
        return true;
    } else {
        std::cout << " not found." << std::endl;
        return false;
    }
}

bool TLookUpTable::SetB(double B)
{
  std::vector<double> radpos;
  std::vector<double> dradpos;
  std::vector<double> tdrift;
  std::vector<double> phipos;
  std::vector<double> dphipos;

  char fn[1024];

  sprintf(fn, "%s/LookUp_%1.2fT_real.dat", gasdir.c_str(), B);
  std::ifstream f(fn);
  if(!f.is_open()){
      cerr << "File not found: " << fn << endl;
      sprintf(fn, "%s/LookUp_%1.2fT.dat", gasdir.c_str(), B);
      f.open(fn);
      if(!f.is_open()){
          cerr << "File not found: " << fn << endl;
          return false;
      }
  }
  while(f.peek() == '#'){
      char buf[1024];
      f.getline(buf,1023);
  }
  {
      std::string t, rmin, r, rmax, phimin, phi, phimax;
      f >> t >> rmin >> r >> rmax >> phimin >> phi >> phimax;   // check the header
      if(!(t == std::string("t") && rmin == std::string("rmin") && r == std::string("r") && rmax == std::string("rmax") && phimin == std::string("phimin") && phi == std::string("phi") && phimax == std::string("phimax"))){ // expected file format
          cerr << fn << " is formatted incorrectly." << endl;
          return false;
      }
  }
  bool write(true);
  while(f.good()){
      double t, rmin, r, rmax, phimin, phi, phimax;
      f >> t >> rmin >> r >> rmax >> phimin >> phi >> phimax;
      if(f.good()){
          tdrift.push_back(t);
          radpos.push_back(r*0.1);   // FIXME: remove translating back and forth once we've settled on one table type
          dradpos.push_back(0.1 * 0.5*(rmax-rmin));
          phipos.push_back(phi*TMath::Pi()/180.);
          dphipos.push_back(0.5*(phimax-phimin)*TMath::Pi()/180.); // For now we ignore assymmetric uncertainty
      }
      f.peek();
  }

  if(!(tdrift.size() == radpos.size() && radpos.size() == phipos.size())){
      cerr << "Something didn't work reading the file: " << fn << endl;
      return false;
  }

  cout << "Read LookUp table. Entries: " << tdrift.size() << endl;

  for(unsigned int i = 1; i < tdrift.size(); i++){
      if(!tdrift[i] > tdrift[i-1]){
          cerr << "Not increasing: " << i << ": " << tdrift[i] << " !> " << tdrift[i-1] << endl;
          return false;
      }
  }
  fMinTime = tdrift.front();
  fMaxTime = tdrift.back();

  if(finterpol_trad) delete finterpol_trad;
  if(finterpol_tphi) delete finterpol_tphi;
  if(finterpol_tdrad) delete finterpol_tdrad;
  if(finterpol_tdphi) delete finterpol_tdphi;
  finterpol_trad  = new ROOT::Math::Interpolator(tdrift, radpos, ROOT::Math::Interpolation::kLINEAR);
  finterpol_tphi  = new ROOT::Math::Interpolator(tdrift, phipos, ROOT::Math::Interpolation::kLINEAR);
  finterpol_tdrad  = new ROOT::Math::Interpolator(tdrift, dradpos, ROOT::Math::Interpolation::kLINEAR);
  finterpol_tdphi  = new ROOT::Math::Interpolator(tdrift, dphipos, ROOT::Math::Interpolation::kLINEAR);
  return true;
}

double TLookUpTable::GetRadius(double t)
{
  // parameter in ns
  // returns mm
  if(t < fMinTime || t > fMaxTime) return 0;
  else return (finterpol_trad->Eval(t)*10.);
}

double TLookUpTable::GetAzimuth(double t)
{
  // parameter in ns
  // returns radians
  if(t < fMinTime || t > fMaxTime) return 0;
  else return finterpol_tphi->Eval(t);
}

double TLookUpTable::GetdRdt(double t)  // FIXME: what is this for?
{
  // parameter in ns
  // returns mm
  return (finterpol_trad->Deriv(t)*10.);
}

double TLookUpTable::GetdPhidt(double t)  // FIXME: what is this for?
{
  // parameter in ns
  // returns radians
  return finterpol_tphi->Deriv(t);
}

double TLookUpTable::GetdR(double t)
{
  // parameter in ns
  // returns mm
  if(finterpol_tdrad) return (finterpol_tdrad->Eval(t)*10.);
  else return 0;
}

double TLookUpTable::GetdPhi(double t)  // FIXME: use real errors
{
  // parameter in ns
  // returns radians
  if(finterpol_tdphi) return finterpol_tdphi->Eval(t);
  else return 0;
}

std::vector<double> TLookUpTable::get_a_b_r0(double B){
  // FIXME: Currently all parameters only known for NTP Ar/Co2 90/10 and B = 1 or B = 0
  // r(t) parameters
    std::vector<double> params;
    if(B == 0){
        params.push_back(-2.0177e-6 ); // a
        params.push_back(-4.6233e-2);  // b
        params.push_back(181.92);      // r0
    } else if(B == 1){
        params.push_back(-3.1779e-6); // a
        params.push_back(-2.553e-2);  // b
        params.push_back(178.25);     // r0
    }
    return params;
}

double TLookUpTable::r_of_t(double t, double B){
    std::vector<double> params = get_a_b_r0(B);
    if(params.size() == 3)
        return params[0]*t*t + params[1]*t + params[2];
    else return -1;
}

double TLookUpTable::dr_of_t(double t, double B){
    std::vector<double> params = get_a_b_r0(B);
    if(params.size() == 3)
        return (2*params[0]*t + params[1])*deltat;
    else return -1;
}

double TLookUpTable::phi_of_r(double r, double B){
    if(B == 0) return(0);
    else if(B == 1){
        // phi(r) parameters
        double A =  - 0.098768;
        double B = 6.2998;
        double C = -54.476;
        r /= 10.;
        return A*r*r + B*r + C;
    }
}

TLookUpTable* TLookUpTable::LookUpTableInstance()
{
  if(!fLookUpTable) fLookUpTable=new TLookUpTable();
  return fLookUpTable;
}

ClassImp(TLookUpTable)
