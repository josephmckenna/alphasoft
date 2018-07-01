// Store event class definition
// for ALPHA-g TPC AGTPCanalysis
// Stores essential information from TEvent
// Authors: A. Capra, M. Mathers
// Date: April 2017

#ifndef __TSTOREEVENT__
#define __TSTOREEVENT__ 1

#include <TObject.h>
#include <TObjArray.h>
#include <TVector3.h>

class TStoreEvent: public TObject
{
private:
  int fID;

  int fNhits;
  int fNtracks;

  TObjArray fStoreHelixArray;  // should this be fGoodHelixArray?
  TObjArray fStoreLineArray;
  //  TObjArray fStoredTracks;
  TObjArray fSpacePoints;

  TVector3 fVertex;
  TVector3 fMCVertex;

  double fPattRecEff;
  double fCosmicCosineAngle;
  int fUnmatchedAnodes;
  int fUnmatchedPads;

public:
  TStoreEvent():fID(-1),
		fNhits(0),fNtracks(0),
		fStoreHelixArray(), fStoreLineArray(),
		fSpacePoints(),
		fPattRecEff(-1.),fCosmicCosineAngle(-99999.),
		fUnmatchedAnodes(0),fUnmatchedPads(0)
  {}; // if nothing is passed, do nothing
  //  TStoreEvent();
  ~TStoreEvent();  // destructor

  //  void SetEvent();

  inline int GetEventNumber() const {return fID;}
  inline void SetEventNumber(int n) {fID = n;}

  inline int GetNumberOfHits() const {return fNhits;}
  inline void SetNumberOfHits(int Nhits) {fNhits = Nhits;}
  inline int GetNumberOfTracks() const {return fNtracks;}
  inline void SetNumberOfTracks(int Ntrk) {fNtracks = Ntrk;}

  inline const TObjArray* GetHelixArray() const {return &fStoreHelixArray;}
  inline const TObjArray* GetLineArray() const {return &fStoreLineArray;}
  //  inline const TObjArray* GetTracksArray() const {return &fStoredTracks;}

  inline const TObjArray* GetSpacePoints() const { return &fSpacePoints; }

  inline const TVector3 GetVertex() const {return fVertex;}

  inline void SetMCVertex(TVector3 vMC) {fMCVertex = vMC;}
  inline void SetMCVertex (double x,
                          double y,
                          double z)   {fMCVertex.SetXYZ(x,y,z);}
  inline const TVector3* GetMCVertex() const {return &fMCVertex;}

  inline double GetNumberOfHitsPerTrack() const {return fPattRecEff;}
  inline double GetAngleBetweenTracks() const { return fCosmicCosineAngle; }

  inline void SetNumberOfUnmatchedAnodes(int uaw) { fUnmatchedAnodes = uaw; }
  inline void SetNumberOfUnmatchedPads(int upd) { fUnmatchedPads = upd; }
  inline int GetNumberOfUnmatchedAnodes() const { return fUnmatchedAnodes; }
  inline int GetNumberOfUnmatchedPads() const { return fUnmatchedPads; }

  virtual void Print(Option_t *option="") const;
  virtual void Reset();

  ClassDef(TStoreEvent,2)
};

#endif
