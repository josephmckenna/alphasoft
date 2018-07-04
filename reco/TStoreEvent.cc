// Store event class implementation
// for ALPHA-g TPC AGTPCanalysis
// Authors: A. Capra, M. Mathers
// Date: April 2017

#include "TStoreEvent.hh"
#include "TStoreHelix.hh"
#include "TStoreLine.hh"
//#include "TStoreTrack.hh"

#include <iostream>

TStoreEvent::TStoreEvent():fID(-1),
			   fNpoints(-1.),fNtracks(-1.),
			   fPattRecEff(-1.)
			   //,fCosmicCosineAngle(-99999.)
{}

void TStoreEvent::SetEvent(const TClonesArray* points, const TClonesArray* tracks, 
			   const TClonesArray* helices)
{
  //  fNpoints = double(points->GetEntries());
  std::cout<<"TStoreEvent::SetEvent N points: "<<points->GetEntries()<<std::endl;
  for(int i=0; i<points->GetEntries(); ++i)
    {
      fSpacePoints.AddLast( points->At(i) );
    }
  //  fSpacePoints.SetOwner(kTRUE);

  std::cout<<"TStoreEvent::SetEvent N lines: "<<tracks->GetEntries()<<std::endl;
  for(int i=0; i<fNtracks; ++i)
    {
      TFitLine* aLine = (TFitLine*) tracks->At(i);
      if( aLine->GetStatus() > 0 )
	{
	  fStoreLineArray.AddLast( new TStoreLine( aLine, aLine->GetPointsArray() ) );
	  fNpoints += double(aLine->GetNumberOfPoints());
	}
    }
  //  fStoreLineArray.SetOwner(kTRUE);


  std::cout<<"TStoreEvent::SetEvent N helices: "<<helices->GetEntries()<<std::endl;
  for(int i=0; i<helices->GetEntries(); ++i)
    {
      TFitHelix* anHelix = (TFitHelix*) helices->At(i);
      if( anHelix->GetStatus() > 0 )
	{
	  fStoreHelixArray.AddLast( new TStoreHelix( anHelix, anHelix->GetPointsArray() ) );
	  fNpoints += double(anHelix->GetNumberOfPoints());
	}
      std::cout<<"TStoreEvent::SetEvent "<<i<<" N points: "<<fNpoints<<std::endl;
    }

  fNtracks = helices->GetEntries()>tracks->GetEntries()?double(helices->GetEntries()):double(tracks->GetEntries());
  std::cout<<"TStoreEvent::SetEvent N tracks: "<<fNtracks<<std::endl;

  if( fNtracks > 0. )
    fPattRecEff = fNpoints/fNtracks;
  else
    fPattRecEff = 0.;
  std::cout<<"TStoreEvent::SetEvent return"<<std::endl;
}

// void TStoreEvent::SetEvent(TEvent* anEvent)
// {
//   // get essential parameters from TEvent
//   fID = anEvent->GetEventNumber();
//   if( gVerb > 1 )
//     std::cout<<"TStoreEvent: Event number stored as "<<fID<<std::endl;

//   fNpoints = anEvent->GetNumberOfPoints();
//   if( gVerb > 1 )
//     std::cout<<"TStoreEvent: Number of points stored as "<<fNpoints<<std::endl;

//   fMCVertex.SetXYZ(anEvent->GetMCVertex()->X(),
// 		   anEvent->GetMCVertex()->Y(),
// 		   anEvent->GetMCVertex()->Z());
//   if( gVerb > 1 )
//     std::cout<<"TStoreEvent: MCVertex stored as "<<fMCVertex.Perp()
// 	     <<", "<<fMCVertex.Phi()
// 	     <<", "<<fMCVertex.Z()<<std::endl;
  
//   const TObjArray* points = anEvent->GetPointsArray();
//   for(int i=0; i<points->GetEntries(); ++i)
//     {
//       fSpacePoints.AddLast( points->At(i) );
//     }

//   if( gMagneticField > 0. )
//     {
//       fNtracks = anEvent->GetNumberOfGoodHelices();
//       fVertex.SetXYZ(anEvent->GetVertexPosition()->X(),
// 		     anEvent->GetVertexPosition()->Y(),
// 		     anEvent->GetVertexPosition()->Z());
//       if( gVerb > 1 )
// 	std::cout<<"TStoreEvent: Vertex stored as "<<fVertex.Perp()
// 		 <<", "<<fVertex.Phi()
// 		 <<", "<<fVertex.Z()<<std::endl;
      
//       const TObjArray* temparray = anEvent->GetGoodHelixArray();
//       for(int i=0; i<temparray->GetEntries(); i++)
// 	{
// 	  TFitHelix* anHelix = (TFitHelix*) temparray->At(i);
// 	  if( anHelix->GetStatus() > 0 )
// 	    fStoreHelixArray.AddLast(new TStoreHelix(anHelix));
// 	  //	    fStoreHelixArray.AddLast(new TStoreHelix(anHelix, anHelix->GetPointsArray()));
// 	    // fStoredTracks.AddLast( new TStoreTrack( anHelix ) );
// 	}
//       if( gVerb > 1 )
//         std::cout<<"TStoreEvent: Number of reconstructed helices "<<fStoreHelixArray.GetEntries()<<std::endl;
//     }
//   else
//     {
//       fNtracks = anEvent->GetNumberOfLines();
//       fCosmicCosineAngle = anEvent->FindCosineAnglePair();
//       const TObjArray* temparray = anEvent->GetLineArray();
//       for(int i=0; i<temparray->GetEntries(); ++i)
// 	{
// 	  TFitLine* aLine = (TFitLine*) temparray->At(i);
// 	  if( aLine->GetStatus() > 0 )
// 	    fStoreLineArray.AddLast( new TStoreLine( aLine, aLine->GetPointsArray() ) );
// 	    //fStoreLineArray.AddLast( new TStoreLine(aLine) );
// 	  // fStoredTracks.AddLast( new TStoreTrack( aLine ) );
// 	  // std::cerr<<"fStoredTracks: "<<fStoredTracks.GetEntries()<<std::endl;
// 	}
//       if( gVerb > 1 )
//         std::cout<<"TStoreEvent: Number of reconstructed lines "<<fStoreLineArray.GetEntries()<<std::endl;
//     }
//   fPattRecEff = double(fNpoints) / double(fNtracks);
//   if( gVerb > 1 )
//     std::cout<<"TStoreEvent: Track Finding Efficiency: "<<fPattRecEff<<std::endl;

//   int totUnMatched = anEvent->GetNumberOfUnmatched(fUnmatchedAnodes,fUnmatchedPads);
//   if( fUnmatchedAnodes < 0 )
//     fUnmatchedAnodes=0;
//   if( gVerb > 1 )
//     std::cout<<"TStoreEvent: Total Number Of Unmatched: "<<totUnMatched<<"\tAnodes = "<<fUnmatchedAnodes<<"\tPads = "<<fUnmatchedPads<<std::endl;
//}

TStoreEvent::~TStoreEvent()
{
  Reset();
}
void TStoreEvent::Print(Option_t*) const
{
  //     for(int n=0; n<fHelixArray.GetEntries(); ++n)
  // {
  //   if( ((TStoreHelix*) fHelixArray.At(n))->GetStatus() > 1 || gVerb > 2 )
  //     ((TStoreHelix*) fHelixArray.At(n))->Print();
  // }
  //
  // for( int n = 0; n<fStoredTracks.GetEntries(); ++n )
  //   {
  //     fStoredTracks.At(n)->Print();
  //   }
  
  std::cout<<"=============== TStoreEvent "<<std::setw(5)<<fID<<" ==============="<<std::endl;
  std::cout<<"Number of Points: "<<fNpoints<<"\tNumber Of Tracks: "<<fNtracks<<std::endl;
  fVertex.Print();
  // if(fMCVertex.Z()!=-99999.)
  //   std::cout<<"MC vertex = (r,phi,z) = ("
  // 	     <<std::setw(5)<<std::left<<fMCVertex.Perp()<<", "
  // 	     <<std::setw(5)<<std::left<<fMCVertex.Phi()<<", "
  // 	     <<std::setw(5)<<std::left<<fMCVertex.Z()<<")"<<std::endl;
  for(int i=0; i<fStoreLineArray.GetEntries(); ++i)
    {
      ((TFitLine*)fStoreLineArray.At(i))->Print();
    }
  std::cout<<"======================================="<<std::endl;
}

void TStoreEvent::Reset()
{
  fID = -1;
  fNpoints = -1.;
  fNtracks = -1.;

  fSpacePoints.Clear();
  //fSpacePoints.Delete();

  //  fStoreLineArray.SetOwner(kTRUE);
  //fStoreLineArray.Delete();
  fStoreLineArray.Clear(); // I am giving 'new' entries, so I will delete them
  //  fStoredTracks.Clear();

  fStoreHelixArray.Clear();  // clear instead of delete
  //fStoreHelixArray.Delete();

  fVertex.SetXYZ(-99999.,-99999.,-99999.);

  fPattRecEff = -1.;
  // fCosmicCosineAngle = -99.;
}

ClassImp(TStoreEvent)
