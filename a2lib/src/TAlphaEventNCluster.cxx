//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventNCluster                                                  //
//                                                                      //
// n-side cluster (group of strips)
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "TAlphaEventNCluster.h"
#include "TVector3.h"
#include "TRotation.h"
#include "TMath.h"

ClassImp(TAlphaEventNCluster);

//______________________________________________________________________________
TAlphaEventNCluster::TAlphaEventNCluster(const char* SilName,TAlphaEventMap* m)
  : TAlphaEventObject(m, SilName, 0 )
{
  nStrips=0;
}

//______________________________________________________________________________
TAlphaEventNCluster::TAlphaEventNCluster(const Int_t SilNum,TAlphaEventMap* m)
  : TAlphaEventObject(m, SilNum, 0 )
{
  nStrips=0;
}

//______________________________________________________________________________
TAlphaEventNCluster::~TAlphaEventNCluster()
{

}

//______________________________________________________________________________
//void TAlphaEventNCluster::Calculate()
void TAlphaEventNCluster::Calculate(const int firstStrip,const int nstrips,const double* adc, const double* rms)
{
  nStrips=nstrips;
  //nside
  double weight = 0.;
  double norm = 0.;
  double RMSsum = 0.; //RMS of strips added in quadrature
  for(int i = 0; i < nStrips; i++)
    {
      MeanStrip+=firstStrip+i;
      double ADC=fabs(adc[i]);
      //TAlphaEventNStrip * strip = fStrips.at(istrip);
      weight += GetnPos(firstStrip+i) * ADC;
      norm   += ADC;
      RMSsum += rms[i] * rms[i];
      /*printf("n: %d x: %lf w: %lf\n",
	strip->GetnStrip(),
	GetnPos(strip->GetnStrip()),
	strip->GetnADC());*/
    }
  MeanStrip=MeanStrip/(double)nStrips;
  SetZ(weight/norm);
  fADC = norm;
  fSigma=( fADC / TMath::Sqrt(RMSsum));
  //printf("fZ: %lf, norm: %lf\n",fZ,norm);
}

//______________________________________________________________________________
void TAlphaEventNCluster::Print(Option_t* /*option*/) const
{
  printf("\n-------TAlphaEventNCluster------\n");
  printf("%lf %lf %lf\n",X(),Y(),Z());
  printf("ADC: %lf\n",fADC);
  printf("Sigma: %lf\n",fSigma);
  printf("Layer %d\n",GetLayer());
  printf("Silnum: %d\n",GetSilNum());
  printf("-------------------------------\n");
}
