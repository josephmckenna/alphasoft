// Store line class implementation
// for ALPHA-g TPC AGTPCanalysis
// Authors: A. Capra
// Date: June 2017

#include "TStoreLine.hh"
#include <iostream>
#include "TPCconstants.hh"

TStoreLine::TStoreLine():fDirection(kUnknown,kUnknown,kUnknown),
			 fPoint(kUnknown,kUnknown,kUnknown),
			 fDirectionError(kUnknown,kUnknown,kUnknown),
			 fPointError(kUnknown,kUnknown,kUnknown),
			 fSpacePoints(0),fNpoints(-1),
			 fchi2(-1.),fStatus(-2),
			 fResidual(kUnknown,kUnknown,kUnknown),
			 fResiduals2(kUnknown)
{}

TStoreLine::TStoreLine(TFitLine* line, 
		       const TObjArray* points):fDirection( line->GetU() ),
						fPoint( line->Get0() ),
						fDirectionError( line->GetUxErr2(), line->GetUyErr2(), line->GetUzErr2() ),
						fPointError( line->GetX0Err2(), line->GetY0Err2(), line->GetZ0Err2() ),
						fSpacePoints( points ), fNpoints(fSpacePoints->GetEntries()), 
						fchi2( line->GetChi2()/double(line->GetDoF()) ), fStatus(line->GetStatus()),
						fResidual( line->GetResidual() ), fResiduals( line->GetResidualsVector() ),
  fResiduals2( line->GetResidualsSquared() )
						
{}

TStoreLine::TStoreLine(TFitLine* line):fDirection( line->GetU() ),
				       fPoint( line->Get0() ),
				       fSpacePoints( 0 ),
				       fNpoints( line->GetNumberOfPoints() ),
				       fResidual( line->GetResidual() ), fResiduals( line->GetResidualsVector() ),
				       fResiduals2( line->GetResidualsSquared() )
{
  fDirectionError.SetXYZ( line->GetUxErr2(), line->GetUyErr2(), line->GetUzErr2() );
  fPointError.SetXYZ( line->GetX0Err2(), line->GetY0Err2(), line->GetZ0Err2() );

  fchi2 = line->GetChi2()/double(line->GetDoF());
 
  fStatus = line->GetStatus();
}

TStoreLine::~TStoreLine()
{}

void TStoreLine::Print(Option_t*) const
{
  std::cout<<" *** TStoreLine ***"<<std::endl;
  std::cout<<"# of points: "<<fNpoints<<std::endl;
  std::cout<<" q = ("<<std::setw(5)<<std::left<<fPoint.X()
	   <<", "<<std::setw(5)<<std::left<<fPoint.Y()
	   <<", "<<std::setw(5)<<std::left<<fPoint.Z()<<")\n"
	   <<" u = ("<<std::setw(5)<<std::left<<fDirection.X()
	   <<", "<<std::setw(5)<<std::left<<fDirection.Y()
	   <<", "<<std::setw(5)<<std::left<<fDirection.Z()<<")\n"
	   <<"phi = "<<fDirection.Phi()<<" rad\t"
	   <<"theta = "<<fDirection.Theta()<<" rad\n";
  std::cout<<"chi^2 = "<<fchi2<<std::endl;
  std::cout<<"Status: "<<fStatus<<std::endl;
  std::cout<<"--------------------------------------------------------------------------"<<std::endl;
}
ClassImp(TStoreLine)
