// Store event class definition
// for ALPHA-g TPC AGTPCanalysis
// Stores essential information from TEvent
// Authors: A. Capra, M. Mathers
// Date: April 2017

#ifndef __TSTOREEVENT__
#define __TSTOREEVENT__ 1

#include <TObject.h>
#include <TObjArray.h>
#include <TClonesArray.h>
#include <TVector3.h>

class TStoreEvent: public TObject
{
private:
  int fID;

  double fNpoints;
  double fNtracks;

  TObjArray fStoreHelixArray;
  TObjArray fStoreLineArray;
  TObjArray fSpacePoints;

  TVector3 fVertex;

  double fPattRecEff;
  //  double fCosmicCosineAngle;

public:
  // TStoreEvent():fID(-1),
  // 		fNpoints(0),fNtracks(0),
  // 		//fStoreHelixArray(), 
  // 		fStoreLineArray(),
  // 		fSpacePoints()//,
  // 		//fPattRecEff(-1.),fCosmicCosineAngle(-99999.)
  // {}; // if nothing is passed, do nothing
  TStoreEvent();
  ~TStoreEvent();  // destructor

  void SetEvent(const TClonesArray* points, const TClonesArray* tracks, const TClonesArray* helices);

  inline int GetEventNumber() const {return fID;}
  inline void SetEventNumber(int n) {fID = n;}

  inline double GetNumberOfPoints() const {return fNpoints;}
  inline void SetNumberOfPoints(double Npoints) {fNpoints = Npoints;}
  inline double GetNumberOfTracks() const {return fNtracks;}
  inline void SetNumberOfTracks(double Ntrk) {fNtracks = Ntrk;}

  inline const TObjArray* GetHelixArray() const {return &fStoreHelixArray;}
  inline const TObjArray* GetLineArray() const {return &fStoreLineArray;}
  //  inline const TObjArray* GetTracksArray() const {return &fStoredTracks;}

  inline const TObjArray* GetSpacePoints() const { return &fSpacePoints; }

  inline const TVector3 GetVertex() const {return fVertex;}

  inline double GetNumberOfPointsPerTrack() const {return fPattRecEff;}
  //  inline double GetAngleBetweenTracks() const { return fCosmicCosineAngle; }

  virtual void Print(Option_t *option="") const;
  virtual void Reset();

  ClassDef(TStoreEvent,2)
};

#endif
