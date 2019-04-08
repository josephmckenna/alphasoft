// Store event class implementation
// for ALPHA-g TPC AGTPCanalysis
// Authors: A. Capra, M. Mathers
// Date: April 2017

#include "TStoreEvent.hh"
#include "TStoreHelix.hh"
#include "TStoreLine.hh"
#include "TSpacePoint.hh"
//#include "TStoreTrack.hh"

#include "TFitLine.hh"
#include "TFitHelix.hh"

#include "TPCconstants.hh"

#include <iostream>

TStoreEvent::TStoreEvent():TObject(),fID(-1),
			   fNpoints(-1),fNtracks(-1),
			   fStoreHelixArray(20), fStoreLineArray(20),
			   fSpacePoints(5000),
			   fUsedHelices(20),
			   fVertex(kUnknown,kUnknown,kUnknown),
			   fVertexStatus(-3),
			   fPattRecEff(-1.)
{}

TStoreEvent::TStoreEvent(const TStoreEvent& right):TObject(right), fID(right.fID),
						   fEventTime(right.fEventTime),
						   fNpoints(right.fNpoints),
						   fNtracks(right.fNtracks),
						   fStoreHelixArray(right.fStoreHelixArray),
						   fStoreLineArray(right.fStoreLineArray),
						   fSpacePoints(right.fSpacePoints),
						   fUsedHelices(right.fUsedHelices),
						   fVertex(right.fVertex),fVertexStatus(right.fVertexStatus),
						   fPattRecEff(right.fPattRecEff),
						   fBarHit(right.fBarHit)
{}

TStoreEvent& TStoreEvent::operator=(const TStoreEvent& right)
{
  fID = right.fID;
  fEventTime = right.fEventTime;
  fNpoints = right.fNpoints;
  fNtracks = right.fNtracks;
  fStoreHelixArray = right.fStoreHelixArray;
  fStoreLineArray = right.fStoreLineArray;
  fSpacePoints = right.fSpacePoints;
  fUsedHelices = right.fUsedHelices;
  fVertex = right.fVertex;
  fVertexStatus = right.fVertexStatus;
  fPattRecEff = right.fPattRecEff;
  fBarHit = right.fBarHit;
  return *this;
}

void TStoreEvent::SetEvent(const TClonesArray* points, const TClonesArray* lines, 
			   const TClonesArray* helices)
{
  int npoints=points->GetEntriesFast();
  for(int i=0; i<npoints; ++i)
    {
      fSpacePoints.AddLast( new TSpacePoint(*(TSpacePoint*)points->At(i)) );
    }
  fSpacePoints.Compress();

  int nlines=lines->GetEntriesFast();
  for(int i=0; i<nlines; ++i)
    {
      TFitLine* aLine = (TFitLine*) lines->At(i);
      if( aLine->GetStatus() > 0 )
	{
	  fStoreLineArray.AddLast( new TStoreLine( aLine, aLine->GetPointsArray() ) );
	}
    }
  fStoreLineArray.Compress();

  fNtracks = helices->GetEntriesFast();
  for(int i=0; i<fNtracks; ++i)
    {
      TFitHelix* anHelix = (TFitHelix*) helices->At(i);
      if( anHelix->GetStatus() > 0 )
	{
	  fStoreHelixArray.AddLast( new TStoreHelix( anHelix, anHelix->GetPointsArray() ) );
	  fNpoints += anHelix->GetNumberOfPoints();
	}
    }
  fStoreHelixArray.Compress();

  if( fNtracks > 0 )
    fPattRecEff = (double)fNpoints/(double)fNtracks;
  else
    fPattRecEff = 0.;
}

TStoreEvent::~TStoreEvent()
{
  Reset();
}
void TStoreEvent::Print(Option_t* o) const
{
   if (strcmp(o,"title")==0)
      std::cout<<"EventNo\tTPCtime\tNpoints\tNtracks\tX\tY\tZ\t";
   else if (strcmp(o,"line")==0) 
      printf("%d\t%f\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t",fID,fEventTime,fNpoints,fNtracks,fVertexStatus,fVertex.X(),fVertex.Y(),fVertex.Z());
   else
   {
      std::cout<<"=============== TStoreEvent "<<std::setw(5)
         <<fID<<" ==============="<<std::endl;
      std::cout<<"Number of Points: "<<fNpoints
         <<"\tNumber Of Tracks: "<<fNtracks<<std::endl;
      std::cout<<"*** Vertex Position ***"<<std::endl;
      fVertex.Print();
      std::cout<<"***********************"<<std::endl;
      // for(int i=0; i<fStoreLineArray.GetEntries(); ++i)
      //   {
      //     ((TFitLine*)fStoreLineArray.At(i))->Print();
      //   }
      std::cout<<"======================================="<<std::endl;
   }
}

int TStoreEvent::AddLine(TFitLine* l) 
{ 
  fStoreLineArray.AddLast( new TStoreLine( l, l->GetPointsArray() ) ); 
  return fStoreLineArray.GetEntriesFast(); 
}
		 
int TStoreEvent::AddHelix(TFitHelix* h) 
{ 
  fStoreHelixArray.AddLast( new TStoreHelix( h, h->GetPointsArray() ) ); 
  return fStoreHelixArray.GetEntriesFast(); 
}

void TStoreEvent::Reset()
{
  fID = -1;
  fNpoints = -1;
  fNtracks = -1;

  fStoreLineArray.Delete();
  fStoreHelixArray.Delete();
  fUsedHelices.Clear();
  fSpacePoints.Delete();

  fVertex.SetXYZ(kUnknown,kUnknown,kUnknown);

  fPattRecEff = -1.;
  
  fBarHit.clear();
  // fCosmicCosineAngle = -99.;
}

ClassImp(TStoreEvent)

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
