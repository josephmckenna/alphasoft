// Store event class definition
// for ALPHA-g TPC AGTPCanalysis
// Stores essential information from TEvent
// Authors: A. Capra, M. Mathers
// Date: April 2017

#ifndef __TSTOREEVENT__
#define __TSTOREEVENT__ 1

#include <TObject.h>
#include <TObjArray.h>
#include "TFitLine.hh"
#include "TFitHelix.hh"
#include "TStoreHelix.hh"
#include "TStoreLine.hh"
#include "TSpacePoint.hh"
#include <TClonesArray.h>
#include <TVector3.h>
#include "TBarEvent.hh"
#include <iomanip>
class TFitLine;
class TFitHelix;
class TStoreEvent: public TObject
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

  std::vector<BarHit> fBarHit; //Barrel hits
  
public:
  TStoreEvent();
  TStoreEvent(const TStoreEvent&);
  virtual ~TStoreEvent();  // destructor

  TStoreEvent& operator=(const TStoreEvent&);
  void SetEvent(const std::vector<TSpacePoint*>* points, const std::vector<TFitLine*>* lines, 
                const std::vector<TFitHelix*>* helices);

  inline int GetEventNumber() const {return fID;}
  inline void SetEventNumber(int n) {fID = n;}

  inline double GetTimeOfEvent() const {return fEventTime;}
  inline void SetTimeOfEvent(double t) {fEventTime = t;}

  inline int GetNumberOfPoints() const {return fNpoints;}
  inline void SetNumberOfPoints(int Npoints) {fNpoints = Npoints;}
  inline int GetNumberOfTracks() const {return fNtracks;}
  inline void SetNumberOfTracks(int Ntrk) {fNtracks = Ntrk;}
  
  double GetMeanZSigma()
  {
    double mean=0;
    int nhelix=0;
    for (int i=0; i<fNtracks; i++)
    {
      TStoreHelix* h=(TStoreHelix*)fStoreHelixArray.At(i);
      if (h)
      {
        if (h->GetStatus() >0 )
        {
          mean+=h->GetZchi2();
          nhelix++;
        }
      }
    }
    if (nhelix) mean=mean/(double)nhelix;
    return mean;
  }
  double GetMeanRSigma()
  {
    double mean=0;
    int nhelix=0;
    for (int i=0; i<fNtracks; i++)
    {
      TStoreHelix* h=(TStoreHelix*)fStoreHelixArray.At(i);
      if (h)
      {

        if (h->GetStatus() >0 )
        {
          mean+=h->GetRchi2();
          nhelix++;
        }
      }
    }
    if (nhelix) mean=mean/(double)nhelix;
    return mean;
  }

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
  
  void AddBarrelHits(TBarEvent* b) { fBarHit=b->GetBars();}
  Int_t GetBarMultiplicity() { return fBarHit.size(); }
  virtual void Print(Option_t *option="") const;
  virtual void Reset();

  ClassDef(TStoreEvent,6)
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
