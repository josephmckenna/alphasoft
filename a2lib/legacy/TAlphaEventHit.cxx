//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventHit                                                       //
//                                                                      //
// contains p-clusters and n-clusters                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <TMath.h>

#include "TAlphaEventHit.h"

ClassImp(TAlphaEventHit);

//______________________________________________________________________________
TAlphaEventHit::TAlphaEventHit(const char* SilName)
  : TAlphaEventObject( SilName, 0 )
{
  fNn = 0;
  fNp = 0;
}

//______________________________________________________________________________
TAlphaEventHit::TAlphaEventHit(const Int_t SilNum)
  : TAlphaEventObject( SilNum, 0 )
{
  fNn = 0;
  fNp = 0;
}

TAlphaEventHit::TAlphaEventHit(TAlphaEventHit* hit)
  : TAlphaEventObject( hit->GetSilNum(), 0)
{
  SetY( hit->Y() );
  SetZ( hit->Z() );
  fNn=hit->GetNn();
  fNp=hit->GetNp();

  fPSigma2=hit->GetPSigma2();
  fNSigma2=hit->GetNSigma2();
  MRS();
  fHitSignificance=hit->GetHitSignifance();
}

//______________________________________________________________________________
TAlphaEventHit::TAlphaEventHit(const Int_t SilNum, 
			       TAlphaEventPCluster * &p, TAlphaEventNCluster * &n)
  : TAlphaEventObject( SilNum, 0 )
{
  SetY( p->Y() );
  SetZ( n->Z() );

  fNn = n->GetNStrips();
  fNp = p->GetNStrips();
  MRS();
  
  fPSigma2 = 0.0000429; // p^2/12
  fNSigma2 = 0.000638; // n^2/12
  
  fHitSignificance = TMath::Sqrt( n->GetSigma()* n->GetSigma() + p->GetSigma()* p->GetSigma());
}

//______________________________________________________________________________
TAlphaEventHit::TAlphaEventHit(const Char_t *SilName, 
			       TAlphaEventPCluster * &p, TAlphaEventNCluster * &n)
  : TAlphaEventObject( SilName, 0 )
{
  SetY( p->Y() );
  SetZ( n->Z() );
  fNn = n->GetNStrips();
  fNp = p->GetNStrips();
  MRS();

  fPSigma2 = 0.0000429; // p^2/12
  fNSigma2 = 0.000638; // n^2/12
}

//______________________________________________________________________________
TAlphaEventHit::~TAlphaEventHit()
{
}

//______________________________________________________________________________
void TAlphaEventHit::Print(Option_t* /*option*/) const
{
  printf("\n-------TAlphaEventHit------\n");
  printf("p: %lf n: %lf\n",Y(),Z());
  printf("XYZ %lf %lf %lf\n",XMRS(),YMRS(),ZMRS());
  printf("Layer %d\n",GetLayer());
  printf("Silnum: %d\n",GetSilNum());
  printf("Nn: %d Np: %d\n",fNn,fNp);
  printf("-------------------------------\n");
}
