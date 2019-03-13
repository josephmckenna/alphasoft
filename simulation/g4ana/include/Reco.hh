#ifndef __RECO__
#define __RECO__ 1

#include <vector>

#include "TClonesArray.h"

#include "SignalsType.h"
#include "AnaSettings.h"

#include "LookUpTable.hh"
#include "TracksFinder.hh"
#include "TFitVertex.hh"

class Reco
{
private:
  bool fTrace;
  double fMagneticField;

  AnaSettings* ana_settings;

  TClonesArray fPointsArray;
  TClonesArray fTracksArray;
  TClonesArray fLinesArray;
  TClonesArray fHelixArray;

  LookUpTable* fSTR;

  unsigned fNhitsCut;
  unsigned fNspacepointsCut;
  double fPointsDistCut;
  double fMaxIncreseAdapt;
  double fSeedRadCut;
  double fSmallRadCut;
  double fLastPointRadCut;
  double fLineChi2Cut;
  double fLineChi2Min;
  double fHelChi2RCut;
  double fHelChi2ZCut;
  double fHelChi2RMin;
  double fHelChi2ZMin;
  double fHelDcut;
  double fVtxChi2Cut;

  // Reasons for pattrec to fail:
  int track_not_advancing;
  int points_cut;
  int rad_cut;

public:
  Reco(std::string, double);
  ~Reco();

  void AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints );
  void AddTracks( const std::vector<track_t>* track_vector );
  int FitLines();
  int FitHelix();
  int RecVertex(TFitVertex* Vertex);

  void Reset();

  inline TClonesArray* GetPoints() { return &fPointsArray; }
  inline TClonesArray* GetTracks() { return &fTracksArray; }
  inline void SetTrace(bool t) { fTrace = t; }

  unsigned GetNspacepointsCut() const { return fNspacepointsCut; }
  double GetPointsDistCut() const     { return fPointsDistCut; }
  double GetMaxIncreseAdapt() const   { return fMaxIncreseAdapt; }
  double GetSeedRadCut() const        { return fSeedRadCut; }
  double GetSmallRadCut() const       { return fSmallRadCut; }
  double GetLastPointRadCut() const   { return fLastPointRadCut; }

  int GetNumberOfTracks() const { return fTracksArray.GetEntriesFast(); }
};

#endif
