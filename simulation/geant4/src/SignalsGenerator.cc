#include "SignalsGenerator.hh"
#include <string>
#include <algorithm>
#include <math.h>
#include<iostream>

static const double _sq2 = 1.0/sqrt(2.0);

SignalsGenerator::SignalsGenerator(double awnl, double padnl):fAnodeNoiseLevel(awnl),
							      fPadNoiseLevel(padnl),
							      fBinWidth(16.),
							      fNbins(511),fPedLen(100),
							      fChargeSpread(10),
							      mV2ADC(8.2),
							      gen(201609031130)

{
  Initialize();
}

SignalsGenerator::~SignalsGenerator()
{
  delete[] fInductionAnodes;
}

std::vector<double> SignalsGenerator::Response(std::ifstream& fin)
{
  double v;
  std::vector<double> response;
  while(1)
    {
      fin>>v;
      if( !fin.good() ) break;
      response.push_back( v );
    }
  return response;
}

void SignalsGenerator::Initialize()
{
  std::string fname = getenv("AGRELEASE");
  // AWB transfer function
  std::ifstream fawsig(fname+"/simulation/common/response/anodeResponseADC.dat");
  fAWaval = Response(fawsig);
  fawsig.close();
  // PWB transfer function
  std::ifstream fpadsig(fname+"/simulation/common/response/padResponseADC.dat");
  fPADaval = Response(fpadsig);
  fpadsig.close();

  // AW induction
  fInductionAnodes = new double[7];
  fInductionAnodes[0] = -0.01;
  fInductionAnodes[1] = -0.04;
  fInductionAnodes[2] = -0.13;
  fInductionAnodes[3] = 1.;
  fInductionAnodes[4] = -0.13;
  fInductionAnodes[5] = -0.04;
  fInductionAnodes[6] = -0.01;

  // parameter that characterizes
  // the charge induced on the pads (taken from Sauli)
  fPadsChargeSigma = 2. * (190.-182.) / 2.34;

  // parameters that characterize
  // the noise on the electrodes
  fAnodeNoisePkPk = fAnodeNoiseLevel*sqrt(12.)*mV2ADC*0.5;
  fPadNoisePkPk = fPadNoiseLevel*sqrt(12.)*mV2ADC*0.5;

  // init the wf containters
  for(uint aw=0; aw<256; ++aw)
    {
      fAnodeSignals[aw] = new std::vector<double>;
      fAnodeSignals[aw]->reserve(fNbins);
      for(int b=0; b<fNbins; ++b)
      	{
      	  fAnodeSignals[aw]->push_back( MakeNoise(fAnodeNoisePkPk) );
      	}
      fAnodeReadout[aw] = new std::vector<int>;
    }

  for(int s=0; s<32; ++s)
    for(int c=0; c<576; ++c)
      {
	std::pair<int,int> pad(s,c);
	fPadSignals[pad] = new std::vector<double>;
	fPadSignals[pad]->reserve(fNbins);
	for(int b=0; b<fNbins; ++b)
	  {
	    fPadSignals[pad]->push_back( MakeNoise(fPadNoisePkPk) );
	  }
	fPadReadout[pad] = new std::vector<int>;
      }
}

void SignalsGenerator::Reset()
{
  for( auto it = fAnodeSignals.begin(); it != fAnodeSignals.end(); ++it )
    {
      for( auto jt = it->second->begin(); jt != it->second->end(); ++jt )
	*jt = MakeNoise(fAnodeNoisePkPk);
      fAnodeReadout[it->first]->clear();
    }

  for( auto it = fPadHit.begin(); it != fPadHit.end(); ++it )
    {
      for( auto jt = fPadSignals[*it]->begin(); jt != fPadSignals[*it]->end(); ++jt )
	*jt = MakeNoise(fPadNoisePkPk);

      fPadSignals_zerosuppression[*it]->clear();
      fPadReadout[*it]->clear();
    }
  fPadHit.clear();
}

void SignalsGenerator::AddAnodeSignal(uint& aw, double& t, double& gain)
{
  int w = aw-3;
  if( w<0 ) w+=256;
  int bin = GetBin(t);
  for(int a=0; a<7; ++a)
    {
      double scale = gain*fInductionAnodes[a];
      AddSignal(bin,scale,&fAWaval,fAnodeSignals[w]);
      ++w;
      if(w>=256) w-=256;
    }
}

void SignalsGenerator::AddPadSignal(std::pair<int,int>& pad, double& t, double& gain, double& z)
{
  int bin = GetBin(t);
  // std::cout<<"SignalsGenerator::AddPadSignal @ bin: "<<bin<<" (time = "<<t<<" ns) for pad: ("
  // 	   <<pad.first<<","<<pad.second<<")"<<std::endl;
  for(int c=pad.second-fChargeSpread; c<=pad.second+fChargeSpread; ++c)
    {
      if( c < 0 ) continue;
      if( c >= 576 ) break;

      std::pair<int,int> id(pad.first,c);
      // std::cout<<"SignalsGenerator::AddPadSignal pad("<<id.first<<","<<id.second<<") size: ";
      // std::cout<<fPadSignals[id].size();

      double z1 = double(c) * 4. - 1152.,
	z2 = ( double(c) + 1.0 ) * 4. - 1152.;
      // std::cout<<c<<"\tz: "<<z<<"  z1: "<<z1<<"  z2: "<<z2<<std::endl;

      double scale = -gain*fPadsChargeProfile(z1,z2,z);
      // std::cout<<c<<"\tscale: "<<scale<<" = "<<gain<<" * "<<fPadsChargeProfile(z1,z2,z)<<std::endl;

      AddSignal(bin,scale,&fPADaval,fPadSignals[id]);

      fPadHit.insert(id);
    }
}

void SignalsGenerator::AddSignal(int& bin, double& scale, std::vector<double>* aval, std::vector<double>* sig)
{
  std::vector<double> wf(aval->begin(),aval->end());
  std::for_each( wf.begin(), wf.end(), [scale](double& d) { d*=scale; } );
  std::transform(sig->begin()+bin, sig->end(),
		 wf.begin(), sig->begin()+bin, std::plus<double>());
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
  int b = (ceil(bd)-bd)<(bd-floor(bd))?int(ceil(bd)):int(floor(bd));
  return b+fPedLen;
}

std::vector<int>* SignalsGenerator::GetAnodeSignal(uint aw)
{
  for( auto it = fAnodeSignals[aw]->begin(); it != fAnodeSignals[aw]->end(); ++it )
    {
      double v = *it;
      int adc = (ceil(v)-v)<(v-floor(v))?int(ceil(v)):int(floor(v));
      fAnodeReadout[aw]->push_back(adc);
    }
  return fAnodeReadout[aw];
}

std::map<uint,std::vector<int>*>* SignalsGenerator::GetAnodeSignal()
{
   for(uint aw=0; aw<256; ++aw)
    {
      GetAnodeSignal(aw);
    }
   return &fAnodeReadout;
}

std::vector<int>* SignalsGenerator::GetPadSignal(std::pair<int,int> pad)
{
  for( auto it = fPadSignals[pad]->begin(); it != fPadSignals[pad]->end(); ++it )
    {
      double v = *it;
      int adc = (ceil(v)-v)<(v-floor(v))?int(ceil(v)):int(floor(v));
      fPadReadout[pad]->push_back(adc);
    }
  return fPadReadout[pad];
}

std::map<std::pair<int,int>,std::vector<int>*>* SignalsGenerator::GetPadSignal()
{
  for( auto it = fPadHit.begin(); it != fPadHit.end(); ++it )
    {
      GetPadSignal( *it );
    }
  return &fPadReadout;
}

std::map<std::pair<int,int>,std::vector<int>*>* SignalsGenerator::GetZsPadSignal()
{
  //  std::map<std::pair<int,int>,std::vector<int>>* zspads;
  std::cout<<"SignalsGenerator::GetZsPadSignal() size: "<< fPadHit.size() <<std::endl;
  for( auto it = fPadHit.begin(); it != fPadHit.end(); ++it )
    {
      //zspads[*it] = fPadSignals[*it];
      //zspads->emplace( *it, fPadSignals[*it] );
      //fPadSignals_zerosuppression.emplace( *it, fPadReadout[*it] );
      fPadSignals_zerosuppression.emplace( *it, GetPadSignal( *it ) );
      //      std::cout << it->first << "\t" << it->second << std::endl;
    }
  //   return zspads;
  return &fPadSignals_zerosuppression;
}

double SignalsGenerator::MakeNoise( double& pkpk )
{
  return fNoise(gen,std::uniform_real_distribution<double>::param_type(-pkpk,pkpk));
}

void SignalsGenerator::PrintNoiseLevels()
{
  std::cout<<"SignalsGenerator::SignalsGenerator AW noise level: +/-"<<int(fAnodeNoisePkPk)
   	   <<" ADC = "<<fAnodeNoiseLevel<<" mV"<<std::endl;
  std::cout<<"SignalsGenerator::SignalsGenerator PAD noise level: +/-"<<int(fPadNoisePkPk)
	  <<" ADC = "<<fPadNoiseLevel<<" mV"<<std::endl;
}
