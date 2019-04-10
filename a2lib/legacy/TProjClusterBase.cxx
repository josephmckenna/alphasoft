// TProjClusterBase
#include <TMath.h>
#include <TVector3.h>
#include "TProjClusterBase.h"
#include "TAlphaEventHelix.h"

ClassImp( TProjClusterBase )

//_____________________________________________________________________
TProjClusterBase::TProjClusterBase()
{
  // ctor
  Clear();
}

//_____________________________________________________________________
TProjClusterBase::TProjClusterBase(TProjClusterBase *pcb)
{
  // ctor
  fRPhi = pcb->RPhi();
  fZ = pcb->Z();
  fAngleFromNormal = pcb->GetAngleFromNormal();
  fTrack = pcb->GetTrack();
  fRPhiSigma = pcb->GetRPhiSigma();
  fZSigma = pcb->GetZSigma();
  fLambda = pcb->GetLambda();
  fd0 = pcb->Getd0();
  fMCClosest = pcb->GetMCClosest();
  fN = pcb->GetN();
}

//_____________________________________________________________________
TProjClusterBase::TProjClusterBase(Double_t RPhi, Double_t Z)
{
  // ctor
  fRPhi = RPhi;
  fZ = Z;
  fAngleFromNormal = -999.;
  fTrack = -1;
  fRPhiSigma = 0.;
  fZSigma = 0.;
  fLambda = -999.;
  fd0 = -999.;
  fMCClosest = kFALSE;
  fN = 0;
}

//_____________________________________________________________________
TProjClusterBase::TProjClusterBase(Double_t RPhi, Double_t Z, 
				   Double_t AngleFromNormal, Int_t Track)
{
  // ctor
  fRPhi = RPhi;
  fZ = Z;
  fAngleFromNormal = AngleFromNormal;
  fTrack = Track;
  SetSigmas(kTRUE);
//  fRPhiSigma = 0.;
//  fZSigma = 0;
  fLambda = -999.;
  fd0 = -999.;
  fMCClosest = kFALSE;
  fN = 0;
}

//_____________________________________________________________________
TProjClusterBase::~TProjClusterBase()
{
  // dtor
  Clear();
}
void TProjClusterBase::Clear(Option_t * /*option*/)
{
  fRPhi = 0.0;
  fZ = 0.0;
  fAngleFromNormal = 0.0;
  fRPhiSigma = 0.0;
  fZSigma = 0.0;
  fLambda = -999.;
  fd0 = -999.;
  fMCClosest = kFALSE;
  fN = 0;
}

//_____________________________________________________________________
void TProjClusterBase::SetAngle(TVector3 *vl, TVector3 *vr)
{
  Double_t xm = vr->X();
  Double_t ym = vr->Y();
  
  Double_t xt = vl->X() - vr->X();
  Double_t yt = vl->Y() - vr->Y();
  
  Double_t tmag2 = xt*xt + yt*yt;
  Double_t mmag2 = xm*xm + ym*ym;
  
  Double_t ptot2 = tmag2*mmag2;
  if(ptot2 <= 0.) 
    {
      fAngleFromNormal = -999.;
    } 
  else 
    {
      Double_t arg = (xt*xm + yt*ym)/TMath::Sqrt(ptot2);
      if(arg >  1.0) arg =  1.0;
      if(arg < -1.0) arg = -1.0;
      fAngleFromNormal = TMath::ACos(arg);
    }
}

//_____________________________________________________________________
void TProjClusterBase::SetSigmas()
{
  fZSigma = 0.4;

  Double_t aDeg = fAngleFromNormal*180./TMath::Pi();
  if( aDeg > 0 && aDeg < 25 )
    fRPhiSigma = 0.3;
  else if( aDeg >= 25 && aDeg < 65 )
    fRPhiSigma = 0.0914 + aDeg*0.00819;
  else if( aDeg >= 65 && aDeg <= 90 )
    fRPhiSigma = 0.6;
  else if( aDeg > 90. )
    {
      fRPhiSigma = 0.6;
      //printf("RPhi sigma out of range!\n");
    }

  if( fAngleFromNormal == -999. )
    {
      fZSigma = 0.57;
      fRPhiSigma = 1.16;
    }
  else
    {
      fZSigma = 0.21 + 0.30 * fabs(fLambda);
    }

  printf("angle %1.2lf(%3.0lf) zsig: %lf rphisig: %lf\n", fAngleFromNormal,aDeg,fZSigma,fRPhiSigma);
}

//_____________________________________________________________________
void TProjClusterBase::SetSigmas(Bool_t kAngle)
{
if(kAngle)
{
	fZSigma=0.;
	fRPhiSigma=0.;
	
	//Double_t Zpar[]={ 0.28029     0.0246954, 0.0668954 };
	// errors	{ 0.00694431, 0.0539557, 0.0876015}
	Double_t Zpar[]={0.280, 0.03, 0.07 };
	
	//Double_t RPhipar[]=	{0.370915,  0.101425, 0.21145};
	// errors 		{0.0203306, 0.117043, 0.154117}
	Double_t RPhipar[]={0.37,0.1,0.2};
	
	Double_t AngleFromNormal=0.;
	if(fAngleFromNormal == -999.) AngleFromNormal=TMath::PiOver2();
	else AngleFromNormal=fAngleFromNormal;
	
	for(Int_t n=0;n<3;++n)
	{
		fZSigma+=Zpar[n]*TMath::Power(AngleFromNormal,(Double_t) n );
		fRPhiSigma+=RPhipar[n]*TMath::Power(AngleFromNormal,(Double_t) n );
	}
}

else SetSigmas();
	
}

//_____________________________________________________________________
void TProjClusterBase::Print(Option_t *) const
{
  printf("\n-------- TProjClusterBase --------\n");
  printf("RPhi: %lf +/- %lf\n", fRPhi, fRPhiSigma);
  printf("Z: %lf +/- %lf\n", fZ, fZSigma);
  printf("Track: %d\n", fTrack);
  printf("AngleFromNormal: %lf\n", fAngleFromNormal);
  printf("Lambda: %lf d0: %lf\n", fLambda, fd0);
  printf("N: %d\n", fN);
  printf("MCClosest: %d\n", fMCClosest);
  printf("-----------------------------------\n\n");
  
}
//end
