// Store helix class implementation
// for ALPHA-g TPC AGTPCanalysis
// Authors: A. Capra, M. Mathers
// Date: April 2017

#include "TStoreHelix.hh"
#include <iostream>
#include <iomanip>

TStoreHelix::TStoreHelix():fc(kUnknown), 
			   fphi0(kUnknown), fD(kUnknown),
			   flambda(kUnknown), fz0(kUnknown),
			   fx0(kUnknown), fy0(kUnknown),
			   ferr2c(kUnknown),
			   ferr2phi0(kUnknown),ferr2D(kUnknown),
			   ferr2lambda(kUnknown),
			   ferr2z0(kUnknown),
			   fBranch(0), fBeta(0),
			   fSpacePoints(0),fNpoints(-1),
			   fchi2R(-1), fchi2Z(-1),
			   fStatus(-2),
			   fMomentum(kUnknown,kUnknown,kUnknown),
			   fMomentumError(kUnknown,kUnknown,kUnknown)
{}

TStoreHelix::TStoreHelix(TFitHelix* helix, const TObjArray* points):fc(helix->GetC()), fRc(helix->GetRc()), 
								    fphi0(helix->GetPhi0()), fD(helix->GetD()),
								    flambda(helix->GetLambda()), fz0(helix->GetZ0()),
								    fx0( helix->GetX0() ), fy0( helix->GetY0() ),
								    ferr2c(helix->GetErrC()), ferr2Rc(helix->GetErrRc()), 
								    ferr2phi0(helix->GetErrPhi0()), ferr2D(helix->GetErrD()),
								    ferr2lambda(helix->GetErrLambda()), ferr2z0(helix->GetErrZ0()),
								    fBranch( helix->GetBranch() ), fBeta( helix->GetFBeta() ),
								    fSpacePoints(points), fNpoints(helix->GetNumberOfPoints()),
  fchi2R(helix->GetRchi2()/double(helix->GetRDoF())),
  fchi2Z(helix->GetZchi2()/double(helix->GetZDoF())),
  fStatus( helix->GetStatus() ),  
  fMomentum(helix->GetMomentumV()), fMomentumError(helix->GetMomentumVerror())
{}

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
  fStatus( helix->GetStatus() ),  
  fMomentum(helix->GetMomentumV()), fMomentumError(helix->GetMomentumVerror())
{}

TStoreHelix::~TStoreHelix()
{}


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
