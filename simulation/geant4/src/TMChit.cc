#include "TMChit.hh"
#include <TMath.h>

TMChit::TMChit(int id, int pdg,
	       double x, double y, double z):fID(id),fPDG(pdg),
					     fx(x),fy(y),fz(z),
					     ft(0.),fEdep(0.)
{
  fr = TMath::Sqrt(fx*fx+fy*fy);
  fp = TMath::ATan2(fy,fx);
  // if( fp < 0. )
  //   fp+=TMath::TwoPi();
}
	       
TMChit::TMChit(int id, int pdg,
	       double x, double y, double z,
	       double r, double phi):fID(id),fPDG(pdg),
				     fx(x),fy(y),fz(z),
				     fr(r),fp(phi),
				     ft(0.),fEdep(0.)
{}

TMChit::TMChit(int id, int pdg,
	       double r, double phi, double z, 
	       double t):fID(id),fPDG(pdg),
			 fz(z),			 
			 fr(r),fp(phi),
			 ft(t),fEdep(0.)
{
  fx = fr*TMath::Cos(fp);
  fy = fr*TMath::Sin(fp);
}

TMChit::TMChit(int id, int pdg,
	       double x, double y, double z,
	       double r, double phi,
	       double t, double E):fID(id),fPDG(pdg),
				   fx(x),fy(y),fz(z),
				   fr(r),fp(phi),
				   ft(t),fEdep(E)
{}

ClassImp(TMChit)

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
