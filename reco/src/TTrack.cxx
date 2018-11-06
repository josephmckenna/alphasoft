// Track class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: June 2016

#include <iostream>
#include <vector>

#include "TTrack.hh"
#include "TSpacePoint.hh"

#include "TPCconstants.hh"

TTrack::TTrack():fPoints(0),fNpoints(0),
		 fB(0.),
		 fStatus(-1),fParticle(0),
		 fPointsCut(28),
		 fResidual(kUnknown,kUnknown,kUnknown),
		 fResiduals2(kUnknown),
		 //fGraph(0),
		 fPoint(0)
{}

TTrack::TTrack(TObjArray* array, double B):fPoints(0),fNpoints(0),
					   fB(B),
					   fStatus(-1),fParticle(0),
					   fPointsCut(28),
					   fResidual(kUnknown,kUnknown,kUnknown),fResiduals2(kUnknown),
					   //fGraph(0),
					   fPoint(0)
{ 
  for(int ip=0; ip<array->GetEntriesFast(); ++ip)
    fPoints.AddLast(array->At(ip));
  fNpoints=fPoints.GetEntriesFast();

  fPoints.Sort();
}

TTrack::TTrack(TObjArray* array):fB(0.),
				 fStatus(-1),fParticle(0),
				 fPointsCut(28),
				 fResidual(kUnknown,kUnknown,kUnknown),fResiduals2(kUnknown),
				 //fGraph(0),
				 fPoint(0)
{ 
  for(int ip=0; ip<array->GetEntriesFast(); ++ip)
    fPoints.AddLast(array->At(ip));
  fNpoints=fPoints.GetEntriesFast();

  fPoints.Sort();
}

TTrack::TTrack(double B):fPoints(0),fNpoints(0),
			 fB(B),
			 fStatus(-1),fParticle(0),
			 fPointsCut(28),
			 fResidual(kUnknown,kUnknown,kUnknown),fResiduals2(0.),
			 //fGraph(0),
			 fPoint(0)
{ }

TTrack::~TTrack()
{
  // fPoints.SetOwner(kTRUE);
  //fPoints.Delete();
  fPoints.Clear();
  //  if(fGraph) delete fGraph;
  if(fPoint) delete fPoint;
}

TTrack::TTrack( const TTrack& right ):TObject(right),
				      fPoints(right.fPoints),
				      fNpoints(right.fNpoints),
				      fB(right.fB),
				      fStatus(right.fStatus),
				      fParticle(right.fParticle),
				      fResiduals2(right.fResiduals2),
				      //fGraph(right.fGraph),
				      fPoint(right.fPoint)
				      
{ 
  fResidual = right.fResidual;
  fResiduals = right.fResiduals;
  fResidualsRadii = right.fResidualsRadii;
  fResidualsXY = right.fResidualsXY;
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
  fResidualsRadii = right.fResidualsRadii;
  fResidualsXY = right.fResidualsXY;
  //fGraph      = right.fGraph;
  fPoint      = right.fPoint;
  return *this;
}

int TTrack::AddPoint(TSpacePoint* aPoint)
{
  if( aPoint->IsGood(_cathradius, _fwradius) )
    {
      fPoints.AddLast(new TSpacePoint(*aPoint));
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
  TVector3 r1( Evaluate(_cathradius*_cathradius) );
  TVector3 r2( Evaluate(_trapradius*_trapradius) );
  return TMath::Abs(r1.Mag()-r2.Mag());
}

double TTrack::CalculateResiduals()
{
  TSpacePoint* aPoint=0;
  fResiduals.clear();
  fResidualsRadii.clear();
  fResidualsPhi.clear();
  fResidualsXY.clear();
  for(int i=0; i<fPoints.GetEntriesFast(); ++i)
    {
      aPoint = (TSpacePoint*) fPoints.At(i);
      TVector3 p(aPoint->GetX(),
		 aPoint->GetY(),
		 aPoint->GetZ());
      double r = aPoint->GetR();

      TVector3 res( p-Evaluate(r*r) );
      fResidual += res; 

      double resmag = res.Mag();
      fResiduals.push_back( resmag );

      fResidualsRadii.insert( std::pair<double,double>( r, resmag ) );
      fResidualsPhi.insert( std::pair<double,double>( aPoint->GetPhi(), resmag ) );
      fResidualsXY.insert( std::pair<std::pair<double,double>,double>
			   (std::pair<double,double>( aPoint->GetX(),
						      aPoint->GetY()),
			    resmag ) ); 
      fResiduals2 += res.Mag2();

    }
  aPoint=0;
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

// void TTrack::Draw(Option_t*)
// {
//   if(fStatus<1) return;

//   double rho2i =0.,
//     rho2f = (_padradius+1.)*(_padradius+1.),
//     Npoints = 50.,
//     rs = TMath::Abs(rho2f-rho2i)/Npoints;

//   fGraph = new TPolyLine3D();
//   for(double r2 = rho2i; r2 <= rho2f; r2 += rs)
//     {
//       TVector3 p = Evaluate(r2);
//       fGraph->SetNextPoint(p.X(),p.Y(),p.Z());
//     }
//   fGraph->SetLineColor(kGreen);
//   fGraph->SetLineWidth(2);
// }

// TPolyLine* TTrack::GetGraph2D() const
// {
//   if(!fGraph) return 0;
//   float* p = fGraph->GetP();
//   int n = fGraph->GetN();
//   TPolyLine* line  = new TPolyLine(n);
//   for(int i=0; i<n; ++i)
//     {
//       line->SetPoint(i, p[3*i], p[3*i+1]);
//     }
//   return line;
// }

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
