#include <cassert>
#include <sstream>
#include <iostream>

#include <TMath.h>
#include <TVector3.h>
#include <TH1D.h>

#include "Medium.hh"
#include "MediumMagboltz.hh"
#include "Sensor.hh"

using Garfield::Sensor;
using Garfield::Medium;
using Garfield::MediumMagboltz;

bool LoadGas(MediumMagboltz *gas, const char *gasfile){
    if(gas->LoadGasFile(gasfile)) return true;
    else {
        std::ostringstream oss;
        assert(getenv("AGRELEASE"));
        oss << getenv("AGRELEASE") << "/simulation/common/gas_files/" << gasfile;
        std::cout << "Loading gas file '" << oss.str() << "'" << std::endl;
        return gas->LoadGasFile(oss.str().c_str());
    }
}

// Inverse of the cumulative probability distribution for avalanche distances from the wire in Ar/CO2 90/10
// Finv(rndm.Uniform()) returns a random r-position according to the probability distribution from
// https://daq.triumf.ca/elog-alphag/alphag/24
double Finv(double x){
    double A = 2.03458e-11;
    A *= -7./2.;
    return TMath::Power(A/(x-1),2./7.);
}
const double mean_av_rad = 0.001766;

// pad signal convolution function ("AFTER-chip")
const double tauPads = 50.; // ns
const double Spads = 1.; // mV/fC
double Hpads(double t)
{
  double tau = tauPads, S = Spads;
  return S * TMath::Power( t / (4.* tau), 4.) * TMath::Exp( 4. - t / tau );
}

// anode wire signal convolution function (Leonid Kurchaninov)
const double tauAnds = 20.;
const double Sands = 1.;
double Hands(double t)
{
  double tau = tauAnds, S = Sands;
  return S * (t / tau) * TMath::Exp( 1.0 - t / tau );
}

// Read-out signal extraction
TH1D GetROSignal(Sensor* s, TString* electrode, bool conv=false)
{
  double Tstart, BinWidth;
  unsigned int nBins;
  s->GetTimeWindow(Tstart, BinWidth, nBins);
  double Tstop = Tstart + BinWidth * double(nBins) ;

  TString hname  = TString::Format("h%s", electrode->Data());
  TString htitle = TString::Format("Signal %s;t [ns];", electrode->Data());
  if(conv)
    htitle+="V [mV]";
  else
   {
      hname+="_raw";
      htitle+="i [#muA]";
    }

  TH1D hs(hname.Data(),htitle.Data(),nBins,Tstart,Tstop);

  for(unsigned int b=1; b<=nBins; ++b)
    hs.SetBinContent(b,s->GetSignal(electrode->Data(),b));

  return hs;
}

// Extract angle between electron velocity and electric field at given point
double LorentzAngle(double x, double y, double z, Sensor* s)
{
  // Electric and Magnetic Fields
  double Ex,Ey,Ez,VV,Bx,By,Bz;
  int stat;
  // Drift Velocity
  Medium* m;
  double Vx,Vy,Vz;
  s->ElectricField(x,y,z,Ex,Ey,Ez,VV,m,stat);
  if( stat ) return -99999.;
  s->MagneticField(x,y,z,Bx,By,Bz,stat);
  if( stat ) return -99999.;
  m->ElectronVelocity(Ex,Ey,Ez,Bx,By,Bz,Vx,Vy,Vz);
  // Lorentz Angle
  TVector3 V(Vx,Vy,Vz);
  TVector3 E(Ex,Ey,Ez);
#if DEBUG > 1
  double phi = TMath::ATan2(y,x);
  cout<<phi*TMath::RadToDeg()<<" deg\tV = ("<<Vx<<","<<Vy<<","<<Vz<<") cm/ns\t E = ("<<Ex<<","<<Ey<<","<<Ez<<") V/cm\t"<<V.Angle(E)*TMath::RadToDeg()<<" deg"<<endl;
#endif
  return TMath::Pi()-V.Angle(E);
}

double CalculateGain(double AWdiam, Sensor* s)
{
  //  Electric and Magnetic Fields
  double Ex,Ey,Ez,VV,Bx,By,Bz;
  int status;
  double AWrad = AWdiam*0.5;
  double x = 18.1986, y = 0.223342,
    z=0.,
    phi=0.0,
    dr = AWrad*1.e-3, // cm
    integral=0.;
  double r2 = TMath::Sqrt(x*x+y*y);
  double r1 = r2 - 13.*AWrad;
  for(double radius = r1; radius<=r2; radius+=dr)
    {
      x = radius*TMath::Cos(phi);
      //      y = radius*TMath::Sin(phi);
      Medium* m;
      s->ElectricField(x,y,z,Ex,Ey,Ez,VV,m,status);
      if( status ) continue;
      //      cout<<"V = "<<VV<<" V @ ("<<x<<","<<y<<","<<z<<") cm\t";
      s->MagneticField(x,y,z,Bx,By,Bz,status);
      if( status ) continue;
      double alpha;
      m->ElectronTownsend(Ex,Ey,Ez,Bx,By,Bz,alpha);
      // double pt = ((MediumGas*)m)->GetPressureTabled(), pp = m->GetPressure();
      // double scale = pt/pp;
      // alpha*=scale;
      // cout<<"alpha = "<<alpha<<" cm^-1\t scale: "<<scale<<endl;
      double part = alpha * dr;
      integral += part;
    }
  return TMath::Exp(integral);
}

// Transformation between cartesian and polar coordinates
void Cartesian2Polar(const double x0, const double y0, 
		     double& r, double& theta) 
{
  if( x0 == 0. && y0 == 0. ) 
    {
      r = theta = 0.;
      return;
    }
  r = sqrt(x0 * x0 + y0 * y0);
  theta = 180. * atan2(y0, x0) / M_PI;
}

void Polar2Cartesian(const double r, const double theta, 
		     double& x0, double& y0) 
{
  x0 = r * cos(M_PI * theta / 180.);
  y0 = r * sin(M_PI * theta / 180.);
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
