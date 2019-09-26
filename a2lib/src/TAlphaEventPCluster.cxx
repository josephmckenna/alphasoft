//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventNCluster                                                  //
//                                                                      //
// p-side cluster (group of strips)
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "TAlphaEventPCluster.h"
#include "TVector3.h"
#include "TRotation.h"
#include "TMath.h"

ClassImp(TAlphaEventPCluster);

//______________________________________________________________________________
TAlphaEventPCluster::TAlphaEventPCluster(const char* SilName,TAlphaEventMap* m)
  : TAlphaEventObject(m, SilName, 0 )
{
  nStrips=0;
}

//______________________________________________________________________________
TAlphaEventPCluster::TAlphaEventPCluster(const Int_t SilNum,TAlphaEventMap* m)
  : TAlphaEventObject(m, SilNum, 0 )
{
  nStrips=0;
}

//______________________________________________________________________________
TAlphaEventPCluster::~TAlphaEventPCluster()
{

}

//______________________________________________________________________________
//void TAlphaEventPCluster::Calculate()
void TAlphaEventPCluster::Calculate(const int firstStrip,const int nstrips,const double* adc, const double* rms)
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
      weight += GetpPos(firstStrip+i) * ADC;
      norm   += ADC;
      RMSsum += rms[i] * rms[i];
      /*printf("n: %d x: %lf w: %lf\n",
	strip->GetnStrip(),
	GetnPos(strip->GetnStrip()),
	strip->GetnADC());*/
    }
  MeanStrip=MeanStrip/(double)nStrips;
  if( norm )
    SetY(weight/norm);
  fADC = norm;
  fSigma=( fADC / TMath::Sqrt(RMSsum));
  //printf("fZ: %lf, norm: %lf\n",fZ,norm);
}

//______________________________________________________________________________
void TAlphaEventPCluster::Print(Option_t* /*option*/) const
{
  printf("\n-------TAlphaEventPCluster------\n");
  printf("%lf %lf %lf\n",X(),Y(),Z());
  printf("ADC: %lf\n",fADC);
  printf("Sigma: %lf\n",fSigma);
  printf("Layer %d\n",GetLayer());
  printf("Silnum: %d\n",GetSilNum());
  printf("-------------------------------\n");
}
