// Store helix class implementation
// for ALPHA-g TPC AGTPCanalysis
// Authors: A. Capra, M. Mathers
// Date: April 2017

#include "TStoreHelix.hh"
#include "TSpacePoint.hh"
#include <iostream>
#include <iomanip>

TStoreHelix::TStoreHelix():fc(ALPHAg::kUnknown), 
			   fphi0(ALPHAg::kUnknown), fD(ALPHAg::kUnknown),
			   flambda(ALPHAg::kUnknown), fz0(ALPHAg::kUnknown),
			   fx0(ALPHAg::kUnknown), fy0(ALPHAg::kUnknown),
			   ferr2c(ALPHAg::kUnknown),
			   ferr2phi0(ALPHAg::kUnknown),ferr2D(ALPHAg::kUnknown),
			   ferr2lambda(ALPHAg::kUnknown),
			   ferr2z0(ALPHAg::kUnknown),
			   fBranch(0), fBeta(0),
			   fSpacePoints(0),fNpoints(-1),
			   fchi2R(-1), fchi2Z(-1),
			   fStatus(-2),
			   fMomentum(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown),
			   fMomentumError(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown),
			   fResidual(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown),
  fResiduals2(ALPHAg::kUnknown)
{}

TStoreHelix::TStoreHelix(TFitHelix* helix, 
			 const std::vector<TSpacePoint*>* points):fc(helix->GetC()), fRc(helix->GetRc()), 
						  fphi0(helix->GetPhi0()), fD(helix->GetD()),
						  flambda(helix->GetLambda()), fz0(helix->GetZ0()),
						  fx0( helix->GetX0() ), fy0( helix->GetY0() ),
						  ferr2c(helix->GetErrC()), ferr2Rc(helix->GetErrRc()), 
						  ferr2phi0(helix->GetErrPhi0()), ferr2D(helix->GetErrD()),
						  ferr2lambda(helix->GetErrLambda()), ferr2z0(helix->GetErrZ0()),
						  fBranch( helix->GetBranch() ), fBeta( helix->GetFBeta() ),
  fSpacePoints(helix->GetNumberOfPoints()),
  fchi2R(helix->GetRchi2()/double(helix->GetRDoF())),
  fchi2Z(helix->GetZchi2()/double(helix->GetZDoF())),
  fStatus( helix->GetStatus() ),  
  fMomentum(helix->GetMomentumV()), fMomentumError(helix->GetMomentumVerror()),
  fResidual(helix->GetResidual()),fResiduals(helix->GetResidualsVector()),
  fResiduals2(helix->GetResidualsSquared())
{
  for( uint i=0; i<points->size(); ++i )
    {
      TSpacePoint* p = (TSpacePoint*) points->at(i);
      if( p->IsGood(ALPHAg::_cathradius, ALPHAg::_fwradius) ) 
	fSpacePoints.AddLast( new TSpacePoint( *p ) );
    }
  //  fSpacePoints.Compress();
  fNpoints = fSpacePoints.GetEntries();
}

TStoreHelix::TStoreHelix(TFitHelix* helix):fc(helix->GetC()), fRc(helix->GetRc()), 
					   fphi0(helix->GetPhi0()), fD(helix->GetD()),
					   flambda(helix->GetLambda()), fz0(helix->GetZ0()),
					   fx0( helix->GetX0() ), fy0( helix->GetY0() ),
					   ferr2c(helix->GetErrC()), ferr2Rc(helix->GetErrRc()), 
					   ferr2phi0(helix->GetErrPhi0()), ferr2D(helix->GetErrD()),
					   ferr2lambda(helix->GetErrLambda()), ferr2z0(helix->GetErrZ0()),
					   fBranch( helix->GetBranch() ), fBeta( helix->GetFBeta() ),
					   fSpacePoints(0), fNpoints(helix->GetNumberOfPoints()),
  fchi2R(helix->GetRchi2()/double(helix->GetRDoF())),
  fchi2Z(helix->GetZchi2()/double(helix->GetZDoF())),
  fStatus(helix->GetStatus()),  
  fMomentum(helix->GetMomentumV()), fMomentumError(helix->GetMomentumVerror()),
  fResidual(helix->GetResidual()),fResiduals(helix->GetResidualsVector()),
  fResiduals2(helix->GetResidualsSquared())
{}

TStoreHelix::TStoreHelix(const TStoreHelix& right):TObject(right),
						   fc(right.fc), fRc(right.fRc), 
						   fphi0(right.fphi0), fD(right.fD),
						   flambda(right.flambda), fz0(right.fz0),
						   fx0(right.fx0), fy0(right.fy0),
						   ferr2c(right.ferr2c), ferr2Rc(right.ferr2Rc), 
						   ferr2phi0(right.ferr2phi0), ferr2D(right.ferr2D),
						   ferr2lambda(right.ferr2lambda), ferr2z0(right.ferr2z0),
						   fBranch(right.fBranch),fBeta(right.fBeta),
						   fSpacePoints(right.fSpacePoints),fNpoints(right.fNpoints),
  fchi2R(right.fchi2R),fchi2Z(right.fchi2Z), fStatus(right.fStatus),  
  fMomentum(right.fMomentum), fMomentumError(right.fMomentumError),
  fResidual(right.fResidual),fResiduals(right.fResiduals),
  fResiduals2(right.fResiduals2)
{}


TStoreHelix& TStoreHelix::operator=(const TStoreHelix& right)
{
  fc = right.fc; 
  fRc = right.fRc; 
  fphi0 = right.fphi0;
  fD = right.fD;
  flambda = right.flambda;
  fz0 = right.fz0;
  fx0 = right.fx0;
  fy0 = right.fy0;
  ferr2c = right.ferr2c;
  ferr2Rc = right.ferr2Rc; 
  ferr2phi0 = right.ferr2phi0;
  ferr2D = right.ferr2D;
  ferr2lambda = right.ferr2lambda;
  ferr2z0 = right.ferr2z0;
  fBranch = right.fBranch;
  fBeta = right.fBeta;
  fSpacePoints = right.fSpacePoints;
  fNpoints = right.fNpoints;
  fchi2R = right.fchi2R;
  fchi2Z = right.fchi2Z; 
  fStatus = right.fStatus;  
  fMomentum = right.fMomentum;
  fMomentumError = right.fMomentumError;
  fResidual = right.fResidual;
  fResiduals = right.fResiduals;
  fResiduals2 = right.fResiduals2;
  return *this;
}

TStoreHelix::~TStoreHelix()
{
  fSpacePoints.Delete();
  fResiduals.clear();
}


void TStoreHelix::Print(Option_t*) const
{
  std::cout<<" *** TStoreHelix ***"<<std::endl;
  std::cout<<"# of points: "<<fNpoints<<std::endl;
  std::cout<<" ("<<std::setw(5)<<std::left<<fx0
	   <<", "<<std::setw(5)<<std::left<<fy0
	   <<", "<<std::setw(5)<<std::left<<fz0<<")\n"
	   <<" c = "<<std::setw(5)<<std::left<<fc
	   <<" Phi0 = "<<std::setw(5)<<std::left<<fphi0
	   <<"    D = "<<std::setw(5)<<std::left<<fD
	   <<"    L = "<<std::setw(5)<<std::left<<flambda
	   <<std::endl;
  std::cout<<"Radial Chi2 = "<<fchi2R
	   <<"\tAxial Chi2 = "<<fchi2Z
	   <<std::endl;
  if(fMomentum.Mag()!=0.0)
    {
      std::cout<<" Momentum = ("
	       <<std::setw(5)<<std::left<<fMomentum.X()<<", "
	       <<std::setw(5)<<std::left<<fMomentum.Y()<<", "
	       <<std::setw(5)<<std::left<<fMomentum.Z()<<") MeV/c"<<std::endl;
      std::cout<<" |p| = "<<fMomentum.Mag()
	       <<" MeV/c\t pT = "<<fMomentum.Perp()<<" MeV/c"<<std::endl;
    }
  std::cout<<"Status: "<<GetStatus()<<std::endl;
  std::cout<<"--------------------------------------------------------------------------"<<std::endl;

}

ClassImp(TStoreHelix)

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
