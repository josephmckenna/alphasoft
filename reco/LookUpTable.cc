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
				  fMapBegin(0.),
				  fMinTime(-1.),fMaxTime(0.)
{     
  frad.clear();
  fdrift.clear();
  flor.clear();

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
							fMapBegin(0.),
							fMinTime(-1.),fMaxTime(0.)
{
  frad.clear();
  fdrift.clear();
  flor.clear();

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

LookUpTable::LookUpTable(double quencherFrac):finterpol_tdrad(0),finterpol_tdphi(0),
					      fMapBegin(700.),
					      fMinTime(-1.),fMaxTime(0.)
{  
  frad.clear();
  fdrift.clear();
  flor.clear();
  frad_zed.clear();
  fdrift_zed.clear();
  flor_zed.clear();

  unsigned idx=0;
  fZed.insert( std::pair<double,unsigned>(0.,idx) );
  for( double zz=fMapBegin; zz<1160.; zz+=5. )
    {
      fZed.insert( std::pair<double,unsigned>(zz,idx) );
      ++idx;
    }
  
  if( SetGas(quencherFrac) )
    {
      fMinTime = fdrift_zed[0].front();
      fMaxTime = fdrift_zed[0].back();
        
      for( auto it = fZed.begin(); it!= fZed.end(); ++it )
	{
	  finterpol_trad_zed.push_back( new ROOT::Math::Interpolator(fdrift_zed[it->second], 
								     frad_zed[it->second]) );
	  finterpol_tphi_zed.push_back( new ROOT::Math::Interpolator(fdrift_zed[it->second], 
								      flor_zed[it->second]) );
	}
      
      finterpol_trad = finterpol_trad_zed[0];
      finterpol_tphi = finterpol_tphi_zed[0];
    }
}

LookUpTable::~LookUpTable()
{
  if(finterpol_trad) delete finterpol_trad;
  if(finterpol_tphi) delete finterpol_tphi;
  if(finterpol_tdrad) delete finterpol_tdrad;
  if(finterpol_tdphi) delete finterpol_tdphi;
  if(finterpol_trad_zed.size()) //finterpol_trad_zed.clear();
     for (uint i=1; i<finterpol_trad_zed.size(); i++)
        delete finterpol_trad_zed[i];
  finterpol_trad_zed.clear();
  if(finterpol_tphi_zed.size())// finterpol_tphi_zed.clear();
    for (uint i=1; i<finterpol_tphi_zed.size(); i++)
        delete finterpol_tphi_zed[i];
  finterpol_tphi_zed.clear();
}

bool LookUpTable::SetRun( int run )
{
  TString lookup_name = TString::Format("%s/ana/LookUp_0.00T_STRR%d_fit.dat",getenv("AGRELEASE"),run);
  std::ifstream lookup(lookup_name.Data());
  if( !lookup.good() )
    {
      while( run > 1000 )
  	{
  	  --run;
  	  lookup_name = TString::Format("%s/ana/LookUp_0.00T_STRR%d_fit.dat",getenv("AGRELEASE"),run);
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
	   <<" mm  Max Time: "<<fdrift.back()
	   <<" ns  (cathode r = "<<_cathradius<<" mm)"<<std::endl;

  if(  minrad > _cathradius || minrad == 0. )
    {
      return SetDefault();
    }

  return true;
}

bool LookUpTable::SetDefault()
{
  TString lookup_name_new = TString::Format("%s/ana/strs/LookUp_0.00T_good.dat",getenv("AGRELEASE"));
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
  TString fgarfname = TString::Format("%s/ana/strs/garfppSTR_B%1.2fT_Ar%1.0fCO2%1.0f_CERN.dat",
				      getenv("AGRELEASE"),
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

bool LookUpTable::SetGas( double quencherFrac )
{
  std::cout << "LookUpTable::SetGas(" << quencherFrac << ")  with Babcock map" << std::endl;  
  TString fgarfname;
  std::ifstream fgarf;
  std::string head,col;
  double t,r,w;
  //unsigned idx=0;
  for( auto it = fZed.begin(); it!=fZed.end(); ++it )
    {
      std::cout<<"LookUpTable::SetGas @ z = "<<it->first<<" mm"<<std::endl;
      fgarfname = TString::Format("%s/ana/strs/garfppSTR_Bmap_z%1.0fmm_Ar%1.0fCO2%1.0f.dat",
				  getenv("AGRELEASE"),
				  it->first,(1.-quencherFrac)*1.e2,quencherFrac*1.e2);
      std::cout<<fgarfname<<"...";
      fgarf.open(fgarfname.Data());
      if( fgarf.is_open()) std::cout<<" OK"<<std::endl;
      else std::cout<<" FAIL"<<std::endl;

      std::getline(fgarf,head);
      std::getline(fgarf,col);
      std::cout<<"LookUpTable:: "<<head<<std::endl;
      //    idx=it->second;
      frad.clear();
      fdrift.clear();
      flor.clear();
      while(1)
	{
	  fgarf>>t>>r>>w;
	  if( !fgarf.good() ) break;
	  frad.push_back(r);
	  fdrift.push_back(t);
	  flor.push_back(w);
	}
      frad_zed.push_back(frad);
      fdrift_zed.push_back(fdrift);
      flor_zed.push_back(flor);
      fgarf.close();
    }
  return true;
}

double LookUpTable::GetRadius(const double t)
{
  // parameter in ns
  // returns mm
  if(t < fMinTime || t > fMaxTime) return 0.;
  else return finterpol_trad->Eval(t);
}

double LookUpTable::GetAzimuth(const double t)
{
  // parameter in ns
  // returns radians
  if(t < fMinTime || t > fMaxTime) return 0.;
  else return finterpol_tphi->Eval(t);
}

double LookUpTable::GetdRdt(const double t)
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

double LookUpTable::GetdPhidt(const double t)
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

double LookUpTable::GetRadius(const double t, const double z)
{
  // parameter in ns
  // returns mm
  if( fabs(z) < fMapBegin ) return GetRadius(t);
  unsigned pos = FindGridPoint(z);
  if( t < fdrift_zed[pos].front() || t > fdrift_zed[pos].back() )
    return 0.;
  else
    return finterpol_trad_zed[pos]->Eval(t);
}

double LookUpTable::GetAzimuth(const double t, const double z)
{
  // parameter in ns
  // returns radians
  if( fabs(z) < fMapBegin ) return GetAzimuth(t);
  unsigned pos = FindGridPoint(z);
  if( t < fdrift_zed[pos].front() || t > fdrift_zed[pos].back() )
    return 0.;
  else
    return finterpol_tphi_zed[pos]->Eval(t);
}

double LookUpTable::GetdRdt(const double t, const double z)
{
  // parameter in ns
  // returns mm
  if( fabs(z) < fMapBegin ) return GetdRdt(t);

  unsigned pos = FindGridPoint(z);

  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  double r_t = -1.;
  if( t < fdrift_zed[pos].front() || t > fdrift_zed[pos].back() )
    return 0.;
  else
    r_t = finterpol_tphi_zed[pos]->Deriv(t);
  gErrorIgnoreLevel = error_level_save;
  return r_t;
}

double LookUpTable::GetdPhidt(const double t, const double z)
{
  // parameter in ns
  // returns radians
  if( fabs(z) < fMapBegin ) return GetdPhidt(t);
  
  unsigned pos = FindGridPoint(z);

  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  double p_t=-1.;
  if( t < fdrift_zed[pos].front() || t > fdrift_zed[pos].back() )
    return 0.;
  else
    p_t = finterpol_tphi_zed[pos]->Deriv(t);
  gErrorIgnoreLevel = error_level_save;
  return p_t;
}

double LookUpTable::GetdR(const double t)
{
  // parameter in ns
  // returns mm
  if(finterpol_tdrad) return finterpol_tdrad->Eval(t);
  else return 0;
}

double LookUpTable::GetdPhi(const double t)  // FIXME: use real errors
{
  // parameter in ns
  // returns radians
  if(finterpol_tdphi) return finterpol_tdphi->Eval(t);
  else return 0;
}

unsigned LookUpTable::FindGridPoint(const double z)
{
  double zabs = fabs( z ), sgn = z>0.?1.:-1.,
    pos=0.;
  double z_ = floor( zabs );
  double dec = zabs - z_;

  // hard-coded grid spacing
  if( dec < 0.3333 ) pos = sgn*z_;
  else if( dec >= 0.3333 && dec < 0.6667 ) pos = sgn*(z_+0.5);
  else pos = sgn*ceil(zabs);

  return fZed[pos];
}
