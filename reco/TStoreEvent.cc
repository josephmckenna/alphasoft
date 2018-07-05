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
  //  std::cout<<"TStoreEvent::SetEvent N points: "<<points->GetEntries()<<std::endl;
  for(int i=0; i<points->GetEntries(); ++i)
    {
      fSpacePoints.AddLast( points->At(i) );
    }
  //  fSpacePoints.SetOwner(kTRUE);

  //  std::cout<<"TStoreEvent::SetEvent N lines: "<<tracks->GetEntries()<<std::endl;
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


  //  std::cout<<"TStoreEvent::SetEvent N helices: "<<helices->GetEntries()<<std::endl;
  for(int i=0; i<helices->GetEntries(); ++i)
    {
      TFitHelix* anHelix = (TFitHelix*) helices->At(i);
      if( anHelix->GetStatus() > 0 )
	{
	  fStoreHelixArray.AddLast( new TStoreHelix( anHelix, anHelix->GetPointsArray() ) );
	  fNpoints += double(anHelix->GetNumberOfPoints());
	}
      //      std::cout<<"TStoreEvent::SetEvent "<<i<<" N points: "<<fNpoints<<std::endl;
    }

  fNtracks = helices->GetEntries()>tracks->GetEntries()?double(helices->GetEntries()):double(tracks->GetEntries());
  //  std::cout<<"TStoreEvent::SetEvent N tracks: "<<fNtracks<<std::endl;

  if( fNtracks > 0. )
    fPattRecEff = fNpoints/fNtracks;
  else
    fPattRecEff = 0.;
  //  std::cout<<"TStoreEvent::SetEvent return"<<std::endl;
}

TStoreEvent::~TStoreEvent()
{
  Reset();
}
void TStoreEvent::Print(Option_t*) const
{
  std::cout<<"=============== TStoreEvent "<<std::setw(5)
	   <<fID<<" ==============="<<std::endl;
  std::cout<<"Number of Points: "<<fNpoints
	   <<"\tNumber Of Tracks: "<<fNtracks<<std::endl;
  fVertex.Print();
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

  fStoreLineArray.Delete();
  fStoreHelixArray.Delete();

  fSpacePoints.Clear();

  fVertex.SetXYZ(-99999.,-99999.,-99999.);

  fPattRecEff = -1.;
  // fCosmicCosineAngle = -99.;
}

ClassImp(TStoreEvent)
