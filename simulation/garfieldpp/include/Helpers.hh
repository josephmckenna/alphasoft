#include <cassert>
#include <sstream>
#include <iostream>

#include <TMath.h>
#include <TVector3.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>

#include "Garfield/Medium.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Sensor.hh"

bool LoadGas(Garfield::MediumMagboltz *gas, const char *gasfile)
{
    if(gas->LoadGasFile(gasfile))
       {
          std::cerr << "Loaded "<< gasfile << " from pwd." << std::endl;
          return true;
       }
    else 
       {
          std::ostringstream oss;
          assert(getenv("AGRELEASE"));
          oss << getenv("AGRELEASE") << "/simulation/common/gas_files/" << gasfile;
          std::cerr << "Loading gas file '" << oss.str() << "'" << std::endl;
          return gas->LoadGasFile(oss.str().c_str());
       }
    return false;
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
TH1D GetROSignal(Garfield::Sensor* sensor, TString* electrode, bool conv=false)
{
  double Tstart, BinWidth;
  unsigned int nBins;
  sensor->GetTimeWindow(Tstart, BinWidth, nBins);
  double Tstop = Tstart + BinWidth * double(nBins) ;
  std::cout<<"Plotting: "<<electrode->Data()<<" from: "<<Tstart<<" to: "<<Tstop<<" ns in "<<nBins<<" bins"<<std::endl;
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
    hs.SetBinContent(b,sensor->GetSignal(electrode->Data(),b));

  int mbin=hs.GetMinimumBin();
  double bc = hs.GetBinContent(mbin);
  if( bc == 0. ){
     mbin=hs.GetMaximumBin();
     bc = hs.GetBinContent(mbin);
  }
  std::cout<<hs.GetName()<<" peak "<<bc<<" @ "<<mbin*BinWidth<<" s"<<std::endl;
  
  return hs;
}

// Extract angle between electron velocity and electric field at given point
double LorentzAngle(double x, double y, double z, Garfield::Sensor* sensor)
{
  // Electric and Magnetic Fields
  double Ex,Ey,Ez,VV,Bx,By,Bz;
  int stat;
  // Drift Velocity
  Garfield::Medium* medium;
  double Vx,Vy,Vz;
  sensor->ElectricField(x,y,z,Ex,Ey,Ez,VV,medium,stat);
  if( stat ) return -99999.;
  sensor->MagneticField(x,y,z,Bx,By,Bz,stat);
  if( stat ) return -99999.;
  medium->ElectronVelocity(Ex,Ey,Ez,Bx,By,Bz,Vx,Vy,Vz);
  // Lorentz Angle
  TVector3 V(Vx,Vy,Vz);
  TVector3 E(Ex,Ey,Ez);
#if DEBUG > 1
  double phi = TMath::ATan2(y,x);
  cout<<phi*TMath::RadToDeg()<<" deg\tV = ("<<Vx<<","<<Vy<<","<<Vz<<") cm/ns\t E = ("<<Ex<<","<<Ey<<","<<Ez<<") V/cm\t"<<V.Angle(E)*TMath::RadToDeg()<<" deg"<<endl;
#endif
  return TMath::Pi()-V.Angle(E);
}

double CalculateGain(double AWdiam, Garfield::Sensor* sensor)
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
      Garfield::Medium* medium;
      sensor->ElectricField(x,y,z,Ex,Ey,Ez,VV,medium,status);
      if( status ) continue;
      //      cout<<"V = "<<VV<<" V @ ("<<x<<","<<y<<","<<z<<") cm\t";
      sensor->MagneticField(x,y,z,Bx,By,Bz,status);
      if( status ) continue;
      double alpha;
      medium->ElectronTownsend(Ex,Ey,Ez,Bx,By,Bz,alpha);
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


TH2D _hEv;
TH2D _hrE;
TH2D _hrv;
double ElectricFieldHisto(double x, double y, double z, Garfield::Sensor* sensor)
{
  // Electric and Magnetic Fields
  double Ex,Ey,Ez,VV,Bx,By,Bz;
  int dummy;
  // Drift Velocity
  Garfield::Medium* medium;
  double Vx,Vy,Vz;
  sensor->ElectricField(x,y,z,Ex,Ey,Ez,VV,medium,dummy);
  sensor->MagneticField(x,y,z,Bx,By,Bz,dummy);
  medium->ElectronVelocity(Ex,Ey,Ez,Bx,By,Bz,Vx,Vy,Vz);
  // Lorentz Angle
  TVector3 V(Vx,Vy,Vz);
  TVector3 E(Ex,Ey,Ez);

  double Emag=E.Mag(),Vmag=V.Mag(),r=TMath::Sqrt(x*x+y*y);
  _hEv.Fill(Emag,Vmag);
  _hrE.Fill(r,Emag);
  _hrv.Fill(r,Vmag);
  return TMath::Pi()-V.Angle(E);
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
