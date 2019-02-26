#include "SignalsGenerator.hh"
#include <string>
#include <algorithm>
#include <math.h> 
#include<iostream>

static const double _sq2 = 1.0/sqrt(2.0);

SignalsGenerator::SignalsGenerator(double nl):fNoiseLevel(nl),
					      fBinWidth(16.),
					      fNbins(511),fPedLen(100),
					      mV2ADC(8.),
					      gen(201609031130)
{
  std::string fname = getenv("AGRELEASE");

  std::ifstream fawsig(fname+"/ana/anodeResponse.dat");
  fAWaval = Response(fawsig);
  fawsig.close();

  std::ifstream fpadsig(fname+"/ana/padResponse_deconv.dat");
  mV2ADC=160.;
  fPADaval = Response(fpadsig);
  fpadsig.close();
  mV2ADC=8.;

  fInductionAnodes = new double[7];
  fInductionAnodes[0] = -0.01;
  fInductionAnodes[1] = -0.04;
  fInductionAnodes[2] = -0.13; 
  fInductionAnodes[3] = 1.;
  fInductionAnodes[4] = -0.13; 
  fInductionAnodes[5] = -0.04;
  fInductionAnodes[6] = -0.01;

  fPadsChargeSigma = 2. * (190.-182.) / 2.34;

  int max_val = int(fNoiseLevel*sqrt(12.)*mV2ADC*0.5);
  uuint.param(std::uniform_int_distribution<int>::param_type(-max_val,max_val));

  Initialize();
}

SignalsGenerator::~SignalsGenerator()
{
  delete[] fInductionAnodes;
}

std::vector<int> SignalsGenerator::Response(std::ifstream& fin)
{
  int n=1,s=0,b=int(fBinWidth),
    max_size = fNbins-fPedLen;
  double t,v,vvv=0.;
  std::vector<int> response;
  response.reserve(max_size);
  while(1)
    {
      fin>>t>>v;
      if( !fin.good() ) break;
      if( s >= max_size ) break;

      vvv+=v;

      if( n%b==0 )
	{
	  response.push_back( int(vvv*mV2ADC/fBinWidth) );
	  vvv=0.;
	  ++s;
	}

      ++n;
    }
  return response;
}

void SignalsGenerator::Initialize()
{
  for(uint aw=0; aw<256; ++aw)
    { 
      fAnodeSignals[aw].reserve(fNbins);
      for(int b=0; b<fNbins; ++b)
      	{
      	  int v = uuint(gen);
      	  fAnodeSignals[aw].push_back(v);
      	}
    }
  
  for(int s=0; s<32; ++s)
    for(int c=0; c<576; ++c)
      { 
	std::pair<int,int> pad(s,c);
	fPadSignals[pad].reserve(fNbins);
	for(int b=0; b<fNbins; ++b)
	  {
	    int v = uuint(gen);
	    fPadSignals[pad].push_back(v);
	  }
      }
}

void SignalsGenerator::Reset()
{
  for( auto it = fAnodeSignals.begin(); it != fAnodeSignals.end(); ++it )
    {
      for( auto jt = it->second.begin(); jt != it->second.end(); ++jt )
	*jt = uuint(gen);
    }

  for( auto it = fPadHit.begin(); it != fPadHit.end(); ++it )
    {
      for( auto jt = fPadSignals[*it].begin(); jt != fPadSignals[*it].end(); ++jt )
	*jt = uuint(gen);
    }
  
  fPadHit.clear();
  fPadSignals_zerosuppression.clear();
}

void SignalsGenerator::AddAnodeSignal(uint& aw, double& t, double& gain)
{
  int w = aw-3;
  if( w<0 ) w+=256;
  int bin = GetBin(t);
  for(int a=0; a<7; ++a)
    {
      std::vector<int> wf(fAWaval.begin(),fAWaval.end());
      int scale = int(gain*fInductionAnodes[a]);
      //      std::cout<<a<<"\tscale: "<<scale<<std::endl;
      std::for_each( wf.begin(), wf.end(), [scale](int& d) { d*=scale; } );

      std::vector<int> sig = fAnodeSignals[w];
      std::transform(sig.begin()+bin, sig.end(), wf.begin(), sig.begin()+bin, std::plus<int>());
      fAnodeSignals[w] = sig;

      ++w;
      if(w>=256) w-=256;
    }
}

void SignalsGenerator::AddPadSignal(std::pair<int,int>& pad, double& t, double& gain, double& z)
{
  int bin = GetBin(t);
  // std::cout<<"SignalsGenerator::AddPadSignal @ bin: "<<bin<<" (time = "<<t<<" ns) for pad: ("
  // 	   <<pad.first<<","<<pad.second<<")"<<std::endl;
  for(int c=pad.second-6; c<=pad.second+6; ++c)
    {
      if( c < 0 ) continue;
      if( c >= 576 ) break;

      std::pair<int,int> id(pad.first,c);
      // std::cout<<"SignalsGenerator::AddPadSignal pad("<<id.first<<","<<id.second<<") size: ";
      // std::cout<<fPadSignals[id].size();

      std::vector<int> wf(fPADaval.begin(),fPADaval.end());
      double z1 = double(c) * 4. - 1152., 
	z2 = ( double(c) + 1.0 ) * 4. - 1152.;
      // std::cout<<c<<"\tz: "<<z<<"  z1: "<<z1<<"  z2: "<<z2<<std::endl;
      int scale = int(-gain*fPadsChargeProfile(z1,z2,z));
      // std::cout<<c<<"\tscale: "<<scale<<" = "<<gain<<" * "<<fPadsChargeProfile(z1,z2,z)<<std::endl;

      std::for_each( wf.begin(), wf.end(), [scale](int& d) { d*=scale; } );

      std::vector<int> sig = fPadSignals[id];
      //     std::cout<<"  value @ start: "<<*(sig.begin()+bin)<<std::endl;
      std::transform(sig.begin()+bin, sig.end(), wf.begin(), sig.begin()+bin, std::plus<int>());
      fPadSignals[id] = sig;
      fPadHit.insert(id);
    }
}

double SignalsGenerator::fPadsChargeProfile(double& z1, double& z2, double& mu)
{
  double x1 = -(z1-mu)*_sq2/fPadsChargeSigma;
  double x2 = -(z2-mu)*_sq2/fPadsChargeSigma;
  // std::cout<<"  ercf(x2) = "<<erfc(x2)<<"  ercf(x1) = "<<erfc(x1)<<std::endl;
  return 0.5*(erfc(x2)-erfc(x1));
}

int SignalsGenerator::GetBin(double& t)
{
  double bd = t/fBinWidth;
  int b = (ceil(bd)-bd)<(bd-floor(bd))?ceil(bd):floor(bd);
  return b+fPedLen;
}


const std::map<std::pair<int,int>,std::vector<int>>* SignalsGenerator::GetZsPadSignal()
{
  //  std::map<std::pair<int,int>,std::vector<int>>* zspads;
  std::cout<<"SignalsGenerator::GetZsPadSignal() size: "<< fPadHit.size() <<std::endl;
  for( auto it = fPadHit.begin(); it != fPadHit.end(); ++it )
    {
      //zspads[*it] = fPadSignals[*it];
      //zspads->emplace( *it, fPadSignals[*it] );
      fPadSignals_zerosuppression.emplace( *it, fPadSignals[*it] );
      //      std::cout << it->first << "\t" << it->second << std::endl;
    }
  //   return zspads;
  return &fPadSignals_zerosuppression;
}
