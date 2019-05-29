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
TAlphaEventPCluster::TAlphaEventPCluster(const char* SilName)
  : TAlphaEventObject( SilName, 0 )
{
  fStrips.SetOwner();
  fStrips.Clear();
}

//______________________________________________________________________________
TAlphaEventPCluster::TAlphaEventPCluster(const Int_t SilNum)
  : TAlphaEventObject( SilNum, 0 )
{
  fStrips.SetOwner();
  fStrips.Clear();
}

//______________________________________________________________________________
TAlphaEventPCluster::~TAlphaEventPCluster()
{
  fStrips.SetOwner();
  fStrips.Delete();
}


//______________________________________________________________________________
void TAlphaEventPCluster::Suppress()
{
  for(int istrip = 0; istrip < fStrips.GetEntriesFast(); istrip++)
    {
      TAlphaEventPStrip * strip = (TAlphaEventPStrip*)fStrips.At(istrip);
      strip->SetADC(0.);
  }
  return;
}

//______________________________________________________________________________
void TAlphaEventPCluster::Calculate()
{
  //nside
  double weight = 0.;
  double norm = 0.;
  double RMSsum = 0.; //RMS of strips added in quadrature  
  for(int istrip = 0; istrip < fStrips.GetEntriesFast(); istrip++)
    {
      TAlphaEventPStrip * strip = (TAlphaEventPStrip*)fStrips.At(istrip);
      weight += GetpPos(strip->GetStripNumber()) * strip->GetADC();
      norm   += strip->GetADC();
      RMSsum += strip->GetStripRMS() * strip->GetStripRMS();
      /*printf("n: %d x: %lf w: %lf\n",
	strip->GetnStrip(),
	GetnPos(strip->GetnStrip()), 
	strip->GetnADC());*/
    }
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
