// Track class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: June 2016

#include <iostream>
#include <vector>

#include "TTrack.hh"
#include "TSpacePoint.hh"

#include "TPCconstants.hh"

TTrack::TTrack():fNpoints(0),
		 fB(0.),
		 fStatus(-1),fParticle(0),
		 fPointsCut(28),
		 fResidual(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown),
		 fResiduals2(ALPHAg::kUnknown),
		 fPoint(0)
{}

TTrack::TTrack(TObjArray* array, double B):fNpoints(0),
					   fB(B),
					   fStatus(-1),fParticle(0),
					   fPointsCut(28),
					   fResidual(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown),fResiduals2(ALPHAg::kUnknown),
					   fPoint(0)
{ 
  fNpoints=array->GetEntriesFast();
  fPoints.reserve(fNpoints);
  
  for(int ip=0; ip<fNpoints; ++ip)
    fPoints.emplace_back(*(TSpacePoint*)array->At(ip));
}

TTrack::TTrack(const TObjArray* array):fB(0.),
				       fStatus(-1),fParticle(0),
				       fPointsCut(28),
				       fResidual(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown),
				       fResiduals2(ALPHAg::kUnknown),
				       fPoint(0)
{ 
  fNpoints=array->GetEntriesFast();
  fPoints.reserve(fNpoints);
  for(int ip=0; ip<fNpoints; ++ip)
        fPoints.emplace_back(*(TSpacePoint*)array->At(ip));
}

TTrack::TTrack(double B):fNpoints(0),
			 fB(B),
			 fStatus(-1),fParticle(0),
			 fPointsCut(28),
			 fResidual(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown),fResiduals2(0.),
			 fPoint(0)
{ }


void TTrack::Clear(Option_t*)
{
  fPoints.clear();
  if (fPoint) delete fPoint;
  fPoint=NULL;
  fResiduals.clear();
  fNpoints=0;
  fB=0.;
  fStatus=-1;
  fParticle=0;
  fPointsCut=28;
  fResidual={ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown};
  fResiduals2=0.;
}

TTrack::~TTrack()
{
  fPoints.clear();
  //  if (fPoint) delete fPoint;
  fResiduals.clear();
}

TTrack::TTrack( const TTrack& right ):TObject(right),
				      fPoints(right.fPoints),
				      fNpoints(right.fNpoints),
				      fB(right.fB),
				      fStatus(right.fStatus),
				      fParticle(right.fParticle),
				      fResiduals2(right.fResiduals2),
				      fPoint(right.fPoint)
				      
{ 
  fResidual = right.fResidual;
  fResiduals = right.fResiduals;
  #if USE_MAPS
  fResidualsRadii = right.fResidualsRadii;
  fResidualsXY = right.fResidualsXY;
  #endif
}

TTrack& TTrack::operator=( const TTrack& right )
{
  fPoints     = right.fPoints;
  fNpoints    = right.fNpoints;
  fStatus     = right.fStatus;
  fParticle   = right.fParticle;
  fResiduals2 = right.fResiduals2;
  fResidual   = right.fResidual;
  fResiduals  = right.fResiduals;
  #if USE_MAPS
  fResidualsRadii = right.fResidualsRadii;
  fResidualsXY = right.fResidualsXY;
  #endif
  fPoint      = right.fPoint;
  return *this;
}

int TTrack::AddPoint(const TSpacePoint& aPoint)
{
  if( aPoint.IsGood(ALPHAg::_cathradius, ALPHAg::_fwradius) )
    {
      fPoints.emplace_back(aPoint);
      ++fNpoints;
    }
  return fNpoints;
}

void TTrack::Fit()
{
  std::cerr<<"TTrack::Fit() is NOT IMPLEMENTED"<<std::endl;
}

bool TTrack::IsGood()
{
  if(fStatus>0)
    return true;
  else
    return false;
}

void TTrack::Reason()
{
  std::cout<<" TTrack::Reason() Status: "<<GetStatus()<<std::endl;
}

double TTrack::GetApproxPathLength()
{
  TVector3 r1( Evaluate(ALPHAg::_cathradius*ALPHAg::_cathradius) );
  TVector3 r2( Evaluate(ALPHAg::_trapradius*ALPHAg::_trapradius) );
  return TMath::Abs(r1.Mag()-r2.Mag());
}

double TTrack::CalculateResiduals()
{
  fResiduals2=0.;
  fResidual.SetXYZ(0.,0.,0.);
  fResiduals.clear();
  #if USE_MAPS
  fResidualsRadii.clear();
  fResidualsPhi.clear();
  fResidualsXY.clear();
  #endif
  int npoints=fPoints.size();
  for(int i=0; i<npoints; ++i)
    {
      const TSpacePoint& aPoint = fPoints.at(i);
      TVector3 p(aPoint.GetX(),
		 aPoint.GetY(),
		 aPoint.GetZ());
      double r = aPoint.GetR();

      TVector3 res( p-Evaluate(r*r) );
      fResidual += res; 

      double resmag = res.Mag();
      fResiduals.push_back( resmag );
      #if USE_MAPS
      fResidualsRadii.insert( std::pair<double,double>( r, resmag ) );
      fResidualsPhi.insert( std::pair<double,double>( aPoint->GetPhi(), resmag ) );
      fResidualsXY.insert( std::pair<std::pair<double,double>,double>
			   (std::pair<double,double>( aPoint->GetX(),
						      aPoint->GetY()),
			    resmag ) ); 
      #endif
      fResiduals2 += res.Mag2();

    }
  return fResiduals2;
}

double TTrack::MinDistPoint(TVector3&)
{
  std::cerr<<"TTrack::MinDistPoint(TVector3&) is NOT IMPLEMENTED"<<std::endl;
  if(!fPoint)
    {
      std::cerr<<"Call TTrack::SetPoint(TVector3* aPoint) first"<<std::endl;
      return -9999999.;
    }
  else
    return (*fPoint-Evaluate(fPoint->Perp2())).Mag();
}

void TTrack::Print(Option_t*) const
{
  std::cout<<" *** TTrack ***"<<std::endl;
  std::cout<<"# of points: "<<fNpoints<<std::endl;
  std::cout<<"Magnetic Field: "<<fB<<" T"<<std::endl;
  if(fResidual.Mag()!=0.0)
    std::cout<<"  Residual = ("
	     <<std::setw(5)<<std::left<<fResidual.X()
	     <<", "<<std::setw(5)<<std::left<<fResidual.Y()
	     <<", "<<std::setw(5)<<std::left<<fResidual.Z()<<") mm"<<std::endl;
  if(fResiduals2!=0.0) 
    std::cout<<"  Residuals Squared = "<<fResiduals2<<" mm^2"<<std::endl;
  if(fParticle!=0)
    std::cout<<"PDG code "<<fParticle<<std::endl;
  std::cout<<"Status: "<<fStatus<<std::endl;
  std::cout<<"--------------------------------------------------------------------------"<<std::endl;
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
