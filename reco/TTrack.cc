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
		 fResiduals2(0.),
		 fGraph(0),fPoint(0)
{
  fResidual.SetXYZ(0.0,0.0,0.0);
}

TTrack::TTrack(TObjArray* array, double B):fB(B),
					   fStatus(-1),fParticle(0),
					   fPointsCut(28),fResiduals2(0.),
					   fGraph(0),fPoint(0)
{ 
  for(int ip=0; ip<array->GetEntries(); ++ip)
    fPoints.AddLast(array->At(ip));
  fNpoints=fPoints.GetEntries();

  fPoints.Sort();

  fResidual.SetXYZ(0.0,0.0,0.0);
}

TTrack::TTrack(TObjArray* array):fB(0.),
				 fStatus(-1),fParticle(0),
				 fPointsCut(28),fResiduals2(0.),
				 fGraph(0),fPoint(0)
{ 
  for(int ip=0; ip<array->GetEntries(); ++ip)
    fPoints.AddLast(array->At(ip));
  fNpoints=fPoints.GetEntries();

  fPoints.Sort();

  fResidual.SetXYZ(0.0,0.0,0.0);
}

TTrack::TTrack(double B):fPoints(0),fNpoints(0),
			 fB(B),
			 fStatus(-1),fParticle(0),
			 fPointsCut(28),fResiduals2(0.),
			 fGraph(0),fPoint(0)
{ 
  fResidual.SetXYZ(0.0,0.0,0.0);
}

TTrack::~TTrack()
{
  // fPoints.SetOwner(kTRUE);
  // fPoints.Delete();
  fPoints.Clear();
  if(fGraph) delete fGraph;
  if(fPoint) delete fPoint;
}

TTrack::TTrack( const TTrack& right ):TObject(right),
				      fPoints(right.fPoints),
				      fNpoints(right.fNpoints),
				      fStatus(right.fStatus),
				      fParticle(right.fParticle),
				      fResiduals2(right.fResiduals2),
				      fGraph(right.fGraph),
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
  fGraph      = right.fGraph;
  fPoint      = right.fPoint;
  return *this;
}

int TTrack::AddPoint(TSpacePoint* aPoint)
{
  if( aPoint->IsGood(_cathradius, _fwradius) )
    {
      fPoints.AddLast(aPoint);
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
  for(int i=0; i<fPoints.GetEntries(); ++i)
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

void TTrack::Draw(Option_t*)
{
  if(fStatus<1) return;

  double rho2i =0.,
    rho2f = (_padradius+1.)*(_padradius+1.),
    Npoints = 50.,
    rs = TMath::Abs(rho2f-rho2i)/Npoints;

  fGraph = new TPolyLine3D();
  for(double r2 = rho2i; r2 <= rho2f; r2 += rs)
    {
      TVector3 p = Evaluate(r2);
      fGraph->SetNextPoint(p.X(),p.Y(),p.Z());
    }
  fGraph->SetLineColor(kGreen);
  fGraph->SetLineWidth(2);
}

TPolyLine* TTrack::GetGraph2D() const
{
  if(!fGraph) return 0;
  float* p = fGraph->GetP();
  int n = fGraph->GetN();
  TPolyLine* line  = new TPolyLine(n);
  for(int i=0; i<n; ++i)
    {
      line->SetPoint(i, p[3*i], p[3*i+1]);
    }
  return line;
}

void TTrack::Print(Option_t*) const
{
  std::cout<<" *** TTrack ***"<<std::endl;
  std::cout<<"# of points: "<<fNpoints<<std::endl;
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

// int TTrack::TrackFinding()
// {
//   int Nentries = fPoints->GetEntries();
//   if( Nentries == 0 ) return 0;
//   else if( Nentries < 3 ) return -1;

//   if(gVerb)
//     std::cout<<"TTrack::TrackFinding()"<<std::endl;

//   std::vector<int> removed;
//   for(int ipnt=0; ipnt<fPoints->GetEntries();++ipnt)
//     {

//       bool skip = false;
//       for(int ir = 0; ir<removed.size(); ++ir){
// 	if( ipnt == removed.at(ir) ) skip = true; }
//       if(skip) continue;

//       TSpacePoint* SeedPoint=(TSpacePoint*) fPoints->At(ipnt);
//       if( SeedPoint->GetR() < gMinRad ) break;
//       if(gVerb > 2)
// 	{
// 	  std::cout<<"Seed Point: "<<ipnt
// 		   <<"\t"<<SeedPoint->GetX()
// 		   <<"\t"<<SeedPoint->GetY()
// 		   <<"\t"<<SeedPoint->GetZ()
// 		   <<"\t\t"<<SeedPoint->GetHeight()<<std::endl;
// 	}
//       removed.push_back(ipnt);

//       if(fB)
// 	{
// 	  TFitHelix* anHelix = new TFitHelix(); // initialize a new helix
// 	  fTrackArray->AddLast(anHelix);
// 	  ((TFitHelix*) fTrackArray->Last())->AddPoint(SeedPoint);
// 	}
//       else
// 	{
// 	  TFitLine* aLine = new TFitLine(); // initialize a new line
// 	  fTrackArray->AddLast(aLine);
// 	  ((TFitLine*) fTrackArray->Last())->AddPoint(SeedPoint);
// 	}

//       ++fNtracks;

//       for(int jpnt=0; jpnt<fPoints->GetEntries();++jpnt)
// 	{

// 	  skip = false;
// 	  for(int ir = 0; ir<removed.size(); ++ir){
// 	    if( jpnt == removed.at(ir) ) skip = true; }
// 	  if(skip) continue;

// 	  TSpacePoint* NextPoint=(TSpacePoint*) fPoints->At(jpnt);
// 	  // if( SeedPoint->Distance(NextPoint) <= fPointsDistCut )
// 	  //   //	      && NextPoint->GetR() <= SeedPoint->GetR() )
// 	  if( SeedPoint->MeasureRad(NextPoint) <= fPointsRadCut &&
// 	      SeedPoint->MeasurePhi(NextPoint) <= fPointsPhiCut &&
// 	      SeedPoint->MeasureZed(NextPoint) <= fPointsZedCut )
// 	    {
// 	      // std::cout<<"Seed R = " << SeedPoint->GetR()<<"\t";
// 	      // std::cout<<"Next R = "<<NextPoint->GetR()<<"\n"; 
// 	      if(gVerb > 2)
// 		{
// 		  std::cout<<"Next Point: "<<jpnt
// 			   <<"\t"<<NextPoint->GetX()
// 			   <<"\t"<<NextPoint->GetY()
// 			   <<"\t"<<NextPoint->GetZ()
// 			   <<"\t\t"<<NextPoint->GetHeight()<<std::endl;
// 		}

// 	      if(fB)
// 		((TFitHelix*) fTrackArray->Last())->AddPoint(NextPoint);
// 	      else
// 		((TFitLine*) fTrackArray->Last())->AddPoint(NextPoint);

// 	      SeedPoint=NextPoint;
// 	      removed.push_back(jpnt);
// 	    }
// 	}
      
//       int Npoints=-1;
//       if(fB)
// 	Npoints = ((TFitHelix*) fTrackArray->Last())->GetNumberOfPoints();
//       else
// 	Npoints = ((TFitLine*) fTrackArray->Last())->GetNumberOfPoints();
//       if( (Npoints < 6 && fB ) || Npoints < 3 )
// 	{
// 	  --fNtracks;
// 	  fTrackArray->RemoveAt(fNtracks);
// 	}

//       SeedPoint=0;      
//     }

//   return fNtracks;
// }

// int TTrack::FitAll()
// //int TTrack::Fit()
// {
//   int status=-1;
//   if(fB)
//     {
//       if(gVerb)
// 	std::cout<<"TTrack::Fit() -- helix"<<std::endl;
//       for(int n=0; n<fTrackArray->GetEntries(); ++n)
// 	{
// 	  if(gVerb>2)
// 	    {
// 	      std::cout<<"Helix::Number of points: "<<((TFitHelix*) fTrackArray->At(n))->GetNumberOfPoints()<<std::endl;
// 	      std::cout << "Helix:  Rfirst = " << ((TSpacePoint*)((TFitHelix*) fTrackArray->At(n))->GetPointsArray()->First())->GetR() 
// 			<< ", Rlast = " << ((TSpacePoint*)((TFitHelix*) fTrackArray->At(n))->GetPointsArray()->Last())->GetR() << std::endl;
// 	    }

// 	  // fit helix canonical form to TPC hits
// 	  ((TFitHelix*) fTrackArray->At(n))->Fit();
// 	  // calculate momumentum
// 	  ((TFitHelix*) fTrackArray->At(n))->Momentum();

// 	  // dummy values for now
// 	  ((TFitHelix*) fTrackArray->At(n))->SetChi2RCut(99999.);
// 	  ((TFitHelix*) fTrackArray->At(n))->SetChi2ZCut(99999.);
// 	  ((TFitHelix*) fTrackArray->At(n))->SetcCut(99999.);
// 	  ((TFitHelix*) fTrackArray->At(n))->SetDCut(99999.);
// 	  ((TFitHelix*) fTrackArray->At(n))->SetMomentumCut(0.);

// 	  //      ((TFitHelix*) fTrackArray->At(n))->CalculateResiduals();

// 	  ((TFitHelix*) fTrackArray->At(n))->IsGood();
// 	  status=((TFitHelix*) fTrackArray->At(n))->GetStatus();
// 	  ((TFitHelix*) fTrackArray->At(n))->Draw();
// 	  //      ((TFitHelix*) fTrackArray->At(n))->Print();
// 	}
//     }
//   else
//     {
//       if(gVerb)
// 	std::cout<<"TTrack::Fit() -- line"<<std::endl;
//       for(int n=0; n<fTrackArray->GetEntries(); ++n)
// 	{
// 	  ((TFitLine*) fTrackArray->At(n))->Fit();
// 	  ((TFitLine*) fTrackArray->At(n))->IsGood();
// 	  // if( ((TFitLine*) fTrackArray->At(n))->IsGood() && gVerb >= 1 )
// 	  //   ((TFitLine*) fTrackArray->At(n))->Print();	
// 	  status=((TFitLine*) fTrackArray->At(n))->GetStatus();
// 	  //      ((TFitLine*) fTrackArray->At(n))->Draw();
// 	}
//     }

//   return status;
// }


// // int TTrack::Fit()
// // {
// //   int status=-1;
// //   if(fB)
// //     {
// //       fHelix = new TFitHelix();
// //       for(int ip=0;ip<fPoints->GetEntries();++ip)
// // 	fHelix->AddPoint((TSpacePoint*)fPoints->At(ip));
// //       if(gVerb>2)
// // 	{
// // 	  std::cout<<"Helix::Number of points: "<<fHelix->GetNumberOfPoints()<<std::endl;
// // 	  std::cout << "Helix:  Rfirst = " << ((TSpacePoint*)fHelix->GetPointsArray()->First())->GetR() 
// // 		    << ", Rlast = " << ((TSpacePoint*)fHelix->GetPointsArray()->Last())->GetR() << std::endl;
// // 	}

// //       // fit helix canonical form to TPC hits
// //       fHelix->Fit();
// //       // calculate momumentum
// //       fHelix->Momentum();

// //       // dumb values for now
// //       fHelix->SetChi2RCut(99999.);
// //       fHelix->SetChi2ZCut(99999.);
// //       fHelix->SetcCut(99999.);
// //       fHelix->SetDCut(99999.);
// //       fHelix->SetMomentumCut(0.);

// //       //      fHelix->CalculateResiduals();

// //       fHelix->IsGood();
// //       status=fHelix->GetStatus();
// //       fHelix->Draw();
// //       //      fHelix->Print();
// //     }
// //   else
// //     {
// //       fLine = new TFitLine();
// //       for(int ip=0;ip<fPoints->GetEntries();++ip)
// // 	{
// // 	  fLine->AddPoint((TSpacePoint*)fPoints->At(ip));
// // 	  if(gVerb>2)
// // 	    {
// // 	      std::cout<<ip<<")\t"
// // 		       <<((TSpacePoint*)fPoints->At(ip))->GetX()<<" "
// // 		       <<((TSpacePoint*)fPoints->At(ip))->GetY()<<" "
// // 		       <<((TSpacePoint*)fPoints->At(ip))->GetZ()<<"\n";
// // 	    }
// // 	}
// //       fLine->Fit();
// //       if( fLine->IsGood() && gVerb >= 1 )
// // 	fLine->Print();	
// //       status=fLine->GetStatus();
// //       //      fLine->Draw();
// //     }

// //   return status;
// // }
