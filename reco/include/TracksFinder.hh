// Tracks finder class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: Sep. 2016

#ifndef __TFINDER__
#define __TFINDER__ 1
#include "TSpacePoint.hh"
#include <vector>
#include <list>
#include <deque>
#include <set>
typedef std::deque<int> track_t;

#include "TClonesArray.h"

class TracksFinder
{
private:
  TClonesArray* fPointsArray;
  
  int fNtracks;  
  double fSeedRadCut;
  double fPointsDistCut;
  double fSmallRad;
  double fLastPointRadCut;
  double fPointsRadCut;
  double fPointsPhiCut;
  double fPointsZedCut;
  int fNpointsCut;
  double fMaxIncreseAdapt;

  std::set<int> fExclusionList;
  std::vector<track_t> fTrackVector;

  // Reasons for failing:
  int track_not_advancing;
  int points_cut;
  int rad_cut;

public:  
  TracksFinder(TClonesArray*);
  ~TracksFinder();

  inline void SetSeedRadCut(double cut)    { fSeedRadCut=cut; }
  inline double GetSeedRadCut() const      { return fSeedRadCut; }
  inline void SetPointsDistCut(double cut) { fPointsDistCut=cut; }
  inline double GetPointsDistCut() const   { return fPointsDistCut; }
  inline void SetSmallRadCut(double cut)   { fSmallRad=cut; }
  inline double GetSmallRadCut() const     { return fSmallRad; }
  inline void SetNpointsCut(int cut)       { fNpointsCut=cut; }
  inline int GetNpointsCut() const         { return fNpointsCut; }
  inline void SetMaxIncreseAdapt(double m) {fMaxIncreseAdapt = m;}
  inline double GetMaxIncreseAdapt() const {return fMaxIncreseAdapt;}
  inline void SetLastPointRadCut(double c) { fLastPointRadCut=c; }
  inline double GetLastPointRadCut() const { return fLastPointRadCut; }
  
  inline int GetNumberOfTracks() const {return fNtracks;}

  inline const std::vector<track_t>* GetTrackVector() const { return &fTrackVector; }
  void AddTrack(track_t&);

  bool Skip(int);

  int RecTracks();
 
  int AdaptiveFinder();
  int NextPoint( TSpacePoint*, int, int, double, track_t&);
  int NextPoint( int, double, double, double, track_t&);

  inline void GetReasons(int& t, int& n, int& r) { t=track_not_advancing; n=points_cut; r=rad_cut;}
};


#endif
