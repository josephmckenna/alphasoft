// Track class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: June 2016

#include <iostream>

#include "TTrack.hh"
#include "TFitHelix.hh"
#include "TFitLine.hh"
#include "TDigi.hh"
#include "TSpacePoint.hh"

#include <vector>

TTrack::TTrack(TObjArray* array, bool B, double ghitdistcut):fPoints(array),fB(B),
					 fHelix(0),fLine(0),
					 fPointsDistCut(ghitdistcut),
					 fPointsRadCut(8.),
					 fPointsPhiCut(0.05),
					 fPointsZedCut(4.)
{
  fNpoints=fPoints->GetEntries();
  // if(gVerb>1)
  //   {
  //     std::cout<<"Number of points: "<<fNpoints<<std::endl;
  //     std::cout << "Before sort: Rfirst = " << ((TSpacePoint*)fPoints->First())->GetR() << ", Rlast = " << ((TSpacePoint*)fPoints->Last())->GetR() << std::endl;
  //   }
  fPoints->Sort();
  // if(gVerb>1)
  //   {
  //     std::cout << "After sort:  Rfirst = " << ((TSpacePoint*)fPoints->First())->GetR() << ", Rlast = " << ((TSpacePoint*)fPoints->Last())->GetR() << std::endl;
  //   }

  fTrackArray = new TObjArray();
}

TTrack::~TTrack()
{
  if( fHelix ) delete fHelix;
  if( fLine ) delete fLine;

  //  fTrackArray->Delete();
  delete fTrackArray;
}

int TTrack::TrackFinding(int gVerb, double gMinRad)
{
  int Ntracks=0,
    Nentries = fPoints->GetEntries();
  if( Nentries == 0 ) return 0;
  else if( Nentries < 3 ) return -1;

  if(gVerb)
    std::cout<<"TTrack::TrackFinding()"<<std::endl;

  std::vector<int> removed;
  for(int ipnt=0; ipnt<fPoints->GetEntries();++ipnt)
    {

      bool skip = false;
      for(unsigned int ir = 0; ir<removed.size(); ++ir){
	if( ipnt == removed.at(ir) ) skip = true; }
      if(skip) continue;

      TSpacePoint* SeedPoint=(TSpacePoint*) fPoints->At(ipnt);
      if( SeedPoint->GetR() < gMinRad ) break;
      if(gVerb > 2)
	{
	  std::cout<<"Seed Point: "<<ipnt
		   <<"\t"<<SeedPoint->GetX()
		   <<"\t"<<SeedPoint->GetY()
		   <<"\t"<<SeedPoint->GetZ()
		   <<"\t\t"<<SeedPoint->GetHeight()<<std::endl;
	}
      removed.push_back(ipnt);

      if(fB)
	{
	  TFitHelix* anHelix = new TFitHelix(); // initialize a new helix
	  fTrackArray->AddLast(anHelix);
	  ((TFitHelix*) fTrackArray->Last())->AddPoint(SeedPoint);
	}
      else
	{
	  TFitLine* aLine = new TFitLine(); // initialize a new line
	  fTrackArray->AddLast(aLine);
	  ((TFitLine*) fTrackArray->Last())->AddPoint(SeedPoint);
	}

      ++Ntracks;

      for(int jpnt=0; jpnt<fPoints->GetEntries();++jpnt)
	{

	  skip = false;
	  for(unsigned int ir = 0; ir<removed.size(); ++ir){
	    if( jpnt == removed.at(ir) ) skip = true; }
	  if(skip) continue;

	  TSpacePoint* NextPoint=(TSpacePoint*) fPoints->At(jpnt);
	  // if( SeedPoint->Distance(NextPoint) <= fPointsDistCut )
	  //   //	      && NextPoint->GetR() <= SeedPoint->GetR() )
	  if( SeedPoint->MeasureRad(NextPoint) <= fPointsRadCut &&
	      SeedPoint->MeasurePhi(NextPoint) <= fPointsPhiCut &&
	      SeedPoint->MeasureZed(NextPoint) <= fPointsZedCut )
	    {
	      // std::cout<<"Seed R = " << SeedPoint->GetR()<<"\t";
	      // std::cout<<"Next R = "<<NextPoint->GetR()<<"\n";
	      if(gVerb > 2)
		{
		  std::cout<<"Next Point: "<<jpnt
			   <<"\t"<<NextPoint->GetX()
			   <<"\t"<<NextPoint->GetY()
			   <<"\t"<<NextPoint->GetZ()
			   <<"\t\t"<<NextPoint->GetHeight()<<std::endl;
		}

	      if(fB)
		((TFitHelix*) fTrackArray->Last())->AddPoint(NextPoint);
	      else
		((TFitLine*) fTrackArray->Last())->AddPoint(NextPoint);

	      SeedPoint=NextPoint;
	      removed.push_back(jpnt);
	    }
	}

      int Npoints=-1;
      if(fB)
	Npoints = ((TFitHelix*) fTrackArray->Last())->GetNumberOfPoints();
      else
	Npoints = ((TFitLine*) fTrackArray->Last())->GetNumberOfPoints();
      if( (Npoints < 6 && fB ) || Npoints < 3 )
	{
	  --Ntracks;
	  fTrackArray->RemoveAt(Ntracks);
	}

      SeedPoint=0;
    }

  return Ntracks;
}

//int TTrack::FitAll()
int TTrack::Fit(int gVerb)
{
  int status=-1;
  if(fB)
    {
      if(gVerb)
	std::cout<<"TTrack::Fit() -- helix"<<std::endl;
      for(int n=0; n<fTrackArray->GetEntries(); ++n)
	{
	  if(gVerb>2)
	    {
	      std::cout<<"Helix::Number of points: "<<((TFitHelix*) fTrackArray->At(n))->GetNumberOfPoints()<<std::endl;
	      std::cout << "Helix:  Rfirst = " << ((TSpacePoint*)((TFitHelix*) fTrackArray->At(n))->GetPointsArray()->First())->GetR()
			<< ", Rlast = " << ((TSpacePoint*)((TFitHelix*) fTrackArray->At(n))->GetPointsArray()->Last())->GetR() << std::endl;
	    }

	  // fit helix canonical form to TPC hits
	  ((TFitHelix*) fTrackArray->At(n))->Fit();
	  // calculate momumentum
	  ((TFitHelix*) fTrackArray->At(n))->Momentum();

	  // dummy values for now
	  ((TFitHelix*) fTrackArray->At(n))->SetChi2RCut(99999.);
	  ((TFitHelix*) fTrackArray->At(n))->SetChi2ZCut(99999.);
	  ((TFitHelix*) fTrackArray->At(n))->SetcCut(99999.);
	  ((TFitHelix*) fTrackArray->At(n))->SetDCut(99999.);
	  ((TFitHelix*) fTrackArray->At(n))->SetMomentumCut(0.);

	  //      ((TFitHelix*) fTrackArray->At(n))->CalculateResiduals();

	  ((TFitHelix*) fTrackArray->At(n))->IsGood();
	  status=((TFitHelix*) fTrackArray->At(n))->GetStatus();
	  ((TFitHelix*) fTrackArray->At(n))->Draw();
	  //      ((TFitHelix*) fTrackArray->At(n))->Print();
	}
    }
  else
    {
      if(gVerb)
	std::cout<<"TTrack::Fit() -- line"<<std::endl;
      for(int n=0; n<fTrackArray->GetEntries(); ++n)
	{
	  ((TFitLine*) fTrackArray->At(n))->Fit();
	  ((TFitLine*) fTrackArray->At(n))->IsGood();
	  // if( ((TFitLine*) fTrackArray->At(n))->IsGood() && gVerb >= 1 )
	  //   ((TFitLine*) fTrackArray->At(n))->Print();
	  status=((TFitLine*) fTrackArray->At(n))->GetStatus();
	  //      ((TFitLine*) fTrackArray->At(n))->Draw();
	}
    }

  return status;
}


// int TTrack::Fit()
// {
//   int status=-1;
//   if(fB)
//     {
//       fHelix = new TFitHelix();
//       for(int ip=0;ip<fPoints->GetEntries();++ip)
// 	fHelix->AddPoint((TSpacePoint*)fPoints->At(ip));
//       if(gVerb>2)
// 	{
// 	  std::cout<<"Helix::Number of points: "<<fHelix->GetNumberOfPoints()<<std::endl;
// 	  std::cout << "Helix:  Rfirst = " << ((TSpacePoint*)fHelix->GetPointsArray()->First())->GetR()
// 		    << ", Rlast = " << ((TSpacePoint*)fHelix->GetPointsArray()->Last())->GetR() << std::endl;
// 	}

//       // fit helix canonical form to TPC hits
//       fHelix->Fit();
//       // calculate momumentum
//       fHelix->Momentum();

//       // dumb values for now
//       fHelix->SetChi2RCut(99999.);
//       fHelix->SetChi2ZCut(99999.);
//       fHelix->SetcCut(99999.);
//       fHelix->SetDCut(99999.);
//       fHelix->SetMomentumCut(0.);

//       //      fHelix->CalculateResiduals();

//       fHelix->IsGood();
//       status=fHelix->GetStatus();
//       fHelix->Draw();
//       //      fHelix->Print();
//     }
//   else
//     {
//       fLine = new TFitLine();
//       for(int ip=0;ip<fPoints->GetEntries();++ip)
// 	{
// 	  fLine->AddPoint((TSpacePoint*)fPoints->At(ip));
// 	  if(gVerb>2)
// 	    {
// 	      std::cout<<ip<<")\t"
// 		       <<((TSpacePoint*)fPoints->At(ip))->GetX()<<" "
// 		       <<((TSpacePoint*)fPoints->At(ip))->GetY()<<" "
// 		       <<((TSpacePoint*)fPoints->At(ip))->GetZ()<<"\n";
// 	    }
// 	}
//       fLine->Fit();
//       if( fLine->IsGood() && gVerb >= 1 )
// 	fLine->Print();
//       status=fLine->GetStatus();
//       //      fLine->Draw();
//     }

//   return status;
// }
