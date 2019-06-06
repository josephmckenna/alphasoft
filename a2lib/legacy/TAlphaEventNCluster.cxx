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
  fStrips.clear();
}

//______________________________________________________________________________
TAlphaEventNCluster::TAlphaEventNCluster(const Int_t SilNum,TAlphaEventMap* m)
  : TAlphaEventObject(m, SilNum, 0 )
{
  fStrips.clear();
}

//______________________________________________________________________________
TAlphaEventNCluster::~TAlphaEventNCluster()
{
  int s=GetNStrips();
  for (int i=0; i<s; i++)
     delete fStrips[i];
  fStrips.clear();
}

//______________________________________________________________________________
void TAlphaEventNCluster::Suppress()
{
  int s=GetNStrips();
  for(int istrip = 0; istrip < s; istrip++)
    {
      TAlphaEventNStrip * strip = fStrips.at(istrip);
      strip->SetADC(0.);
    }
  return;
}

//______________________________________________________________________________
void TAlphaEventNCluster::Calculate()
{
  //nside
  double weight = 0.;
  double norm = 0.;
  double RMSsum = 0.; //RMS of strips added in quadrature
  int s=GetNStrips();
  for(int istrip = 0; istrip < s; istrip++)
    {
      TAlphaEventNStrip * strip = fStrips.at(istrip);
      weight += GetnPos(strip->GetStripNumber()) * strip->GetADC();
      norm   += strip->GetADC();
      RMSsum += strip->GetStripRMS() * strip->GetStripRMS();
      /*printf("n: %d x: %lf w: %lf\n",
	strip->GetnStrip(),
	GetnPos(strip->GetnStrip()),
	strip->GetnADC());*/
    }
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
