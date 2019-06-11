// Store event class definition
// for ALPHA-g TPC AGTPCanalysis
// Stores essential information from TEvent
// Authors: A. Capra, M. Mathers
// Date: April 2017

#ifndef __TSTOREA2EVENT__
#define __TSTOREA2EVENT__ 1

#include <TObject.h>
#include <TObjArray.h>
#include <TClonesArray.h>
#include <TVector3.h>

class TFitLine;
class TFitHelix;
class TStoreA2Event: public TObject
{
private:
  int fID;
  double fEventTime;

  int fNpoints;
  int fNtracks;

  TObjArray fStoreHelixArray;
  TObjArray fStoreLineArray;
  TObjArray fSpacePoints;

  TObjArray fUsedHelices;

  TVector3 fVertex;
  int fVertexStatus;

  double fPattRecEff;

  
public:
  TStoreA2Event();
  TStoreA2Event(const TStoreA2Event&);
  virtual ~TStoreA2Event();  // destructor

  TStoreA2Event& operator=(const TStoreA2Event&);

  void SetEvent(const TClonesArray* points, 
		const TClonesArray* lines, const TClonesArray* helices);

  inline int GetEventNumber() const {return fID;}
  inline void SetEventNumber(int n) {fID = n;}

  inline double GetTimeOfEvent() const {return fEventTime;}
  inline void SetTimeOfEvent(double t) {fEventTime = t;}

  inline int GetNumberOfPoints() const {return fNpoints;}
  inline void SetNumberOfPoints(int Npoints) {fNpoints = Npoints;}
  inline int GetNumberOfTracks() const {return fNtracks;}
  inline void SetNumberOfTracks(int Ntrk) {fNtracks = Ntrk;}

  inline const TObjArray* GetHelixArray() const {return &fStoreHelixArray;}
  inline const TObjArray* GetLineArray() const {return &fStoreLineArray;}
  //  inline const TObjArray* GetTracksArray() const {return &fStoredTracks;}

  inline const TObjArray* GetUsedHelices()       const {return &fUsedHelices;}
  inline void SetUsedHelices(const TObjArray* a)       {fUsedHelices = *a;}

  int AddLine(TFitLine* l);
  int AddHelix(TFitHelix* h);

  inline const TObjArray* GetSpacePoints() const { return &fSpacePoints; }

  inline void SetVertex(TVector3 vtx)     {fVertex = vtx; }
  inline const TVector3 GetVertex()       {return fVertex;}

  inline void SetVertexStatus(int status)  {fVertexStatus = status; }
  inline int GetVertexStatus() const       {return fVertexStatus;}

  inline double GetNumberOfPointsPerTrack() const {return fPattRecEff;}
  
  virtual void Print(Option_t *option="") const;
  virtual void Reset();

  ClassDef(TStoreA2Event,1)
};

#endif