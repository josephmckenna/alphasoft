// Look-up Table class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#include "LookUpTable.hh"
#include <TMath.h>
#include <TString.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "TPCconstants.hh"

LookUpTable::LookUpTable(int run):finterpol_tdrad(0),finterpol_tdphi(0),
				  fMinTime(-1.),fMaxTime(0.)
{
  if( SetRun( run ) )
    {
      fMinTime = fdrift.front();
      fMaxTime = fdrift.back();
      
      // finterpol_trad = new ROOT::Math::Interpolator(drift, radpos, ROOT::Math::Interpolation::kLINEAR);
      // finterpol_tphi = new ROOT::Math::Interpolator(drift, wire, ROOT::Math::Interpolation::kLINEAR);
      finterpol_trad = new ROOT::Math::Interpolator(fdrift, frad, ROOT::Math::Interpolation::kCSPLINE);
      finterpol_tphi = new ROOT::Math::Interpolator(fdrift, flor, ROOT::Math::Interpolation::kCSPLINE);
      std::cout<<"LookUpTable::LookUpTable range: ["<<fMinTime<<","<<fMaxTime<<"]ns"<<std::endl;
    }
}

LookUpTable::LookUpTable(double quencherFrac, double B):finterpol_tdrad(0),finterpol_tdphi(0),
							fMinTime(-1.),fMaxTime(0.)
{
  if( SetGas(quencherFrac, B) )
    {
      fMinTime = fdrift.front();
      fMaxTime = fdrift.back();
      
      // finterpol_trad = new ROOT::Math::Interpolator(drift, radpos, ROOT::Math::Interpolation::kLINEAR);
      // finterpol_tphi = new ROOT::Math::Interpolator(drift, wire, ROOT::Math::Interpolation::kLINEAR);
      finterpol_trad = new ROOT::Math::Interpolator(fdrift, frad, ROOT::Math::Interpolation::kCSPLINE);
      finterpol_tphi = new ROOT::Math::Interpolator(fdrift, flor, ROOT::Math::Interpolation::kCSPLINE);
    }
}

LookUpTable::~LookUpTable()
{
  if(finterpol_trad) delete finterpol_trad;
  if(finterpol_tphi) delete finterpol_tphi;
  if(finterpol_tdrad) delete finterpol_tdrad;
  if(finterpol_tdphi) delete finterpol_tdphi;
}

bool LookUpTable::SetRun( int run )
{
  TString lookup_name = TString::Format("../ana/LookUp_0.00T_STRR%d_fit.dat",run);
  std::ifstream lookup(lookup_name.Data());
  if( !lookup.good() )
    {
      while( run > 1000 )
  	{
  	  --run;
  	  lookup_name = TString::Format("../ana/LookUp_0.00T_STRR%d_fit.dat",run);
  	  //std::cout<<"LookUpTable::SetRun "<<run<<"\t"<<lookup_name<<std::endl;
  	  lookup.open(lookup_name.Data());
  	  if( lookup.good() ) break;
  	}
    }

  std::cout<<"LookUpTable::SetRun() "<<lookup_name<<" ... ";
  if( lookup.is_open() )
    std::cout<<"OK"<<std::endl;
  else
    {
      std::cout<<"FAIL"<<std::endl;
      return SetDefault();
    }

  std::string head;
  std::getline(lookup,head);
  std::string col;
  std::getline(lookup,col);
 
  std::cout<<"LookUpTable:: "<<head<<std::endl;
  
  double r,t,w;
  while(1)
    {
      lookup>>t>>r>>w;
      if( !lookup.good() ) break;
      frad.push_back(r);
      fdrift.push_back(t);
      flor.push_back(w);
    }
  lookup.close();

  double minrad=frad.back();

  std::cout<<"LookUpTable:: Min Rad: "<<minrad
	   <<" Max Time: "<<fdrift.back()
	   <<" (cathode r = "<<_cathradius<<" mm)"<<std::endl;

  if(  minrad > _cathradius || minrad == 0. )
    {
      return SetDefault();
    }

  return true;
}

bool LookUpTable::SetDefault()
{
  TString lookup_name_new = "../ana/LookUp_0.00T_good.dat";
  std::ifstream lookup_new(lookup_name_new.Data());
  if( lookup_new.is_open() )
    std::cout<<"LookUpTable:: ...trying new: "<<lookup_name_new<<std::endl;

  std::string head;
  std::string col;
  std::getline(lookup_new,head);
  std::getline(lookup_new,col);
  std::cout<<"LookUpTable:: "<<head<<std::endl;
  frad.clear();
  fdrift.clear();
  flor.clear();

  double t,r,w;
  while(1)
    {
      lookup_new>>t>>r>>w;
      if( !lookup_new.good() ) break;
      frad.push_back(r);
      fdrift.push_back(t);
      flor.push_back(w);
    }
  lookup_new.close();
  return true;
}

bool LookUpTable::SetGas(double quencherFrac, double B )
{
  std::cout << "LookUpTable::SetGas(" << quencherFrac << ", " << B << ')' << std::endl;
  TString fgarfname = TString::Format("garfppSTR_B%1.2fT_Ar%1.0fCO2%1.0f.dat",
				      B,(1.-quencherFrac)*1.e2,quencherFrac*1.e2);
  std::ifstream fgarf(fgarfname.Data());  
  std::string head;
  std::getline(fgarf,head);
  std::string col;
  std::getline(fgarf,col);
  std::cout<<"LookUpTable:: "<<head<<std::endl;
  double t,r,w;
  while(1)
    {
      fgarf>>t>>r>>w;
      if( !fgarf.good() ) break;
      frad.push_back(r);
      fdrift.push_back(t);
      flor.push_back(w);
    }
  fgarf.close();
  return true;
}

double LookUpTable::GetRadius(double t)
{
  // parameter in ns
  // returns mm
  if(t < fMinTime || t > fMaxTime) return 0.;
  else return finterpol_trad->Eval(t);
}

double LookUpTable::GetAzimuth(double t)
{
  // parameter in ns
  // returns radians
  if(t < fMinTime || t > fMaxTime) return 0.;
  else return finterpol_tphi->Eval(t);
}

double LookUpTable::GetdRdt(double t)
{
  // parameter in ns
  // returns mm
  //  return finterpol_trad->Deriv(t);
  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  double r_t = finterpol_trad->Deriv(t);
  gErrorIgnoreLevel = error_level_save;
  return r_t;
}

double LookUpTable::GetdPhidt(double t)
{
  // parameter in ns
  // returns radians
  //  return finterpol_tphi->Deriv(t);
  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  double p_t = finterpol_tphi->Deriv(t);
  gErrorIgnoreLevel = error_level_save;
  return p_t;
}

double LookUpTable::GetdR(double t)
{
  // parameter in ns
  // returns mm
  if(finterpol_tdrad) return finterpol_tdrad->Eval(t);
  else return 0;
}

double LookUpTable::GetdPhi(double t)  // FIXME: use real errors
{
  // parameter in ns
  // returns radians
  if(finterpol_tdphi) return finterpol_tdphi->Eval(t);
  else return 0;
}
