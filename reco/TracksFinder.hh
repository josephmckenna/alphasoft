// Tracks finder class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: Sep. 2016

#ifndef __TFINDER__
#define __TFINDER__ 1

#include <vector>
#include <list>
typedef std::list<int> track_t;
//typedef std::list<int>::iterator point_t;

#include "TObjArray.h"

#include "TTrack.hh"

class TSpacePoint;
class TracksFinder
{
private:
  TObjArray* fPointsArray;
  int fNpoints;

  TObjArray* fTracksArray;
  //  std::vector<TObjArray*> fTracksArray;
  int fNtracks;  

  double fSeedRadCut;
  double fPointsRadCut_1, fPointsRadCut_2, fPointsRadCut_3;

  double fPointsDistCut;

  double fPointsPhiCut;
  double fPointsZedCut;

  double fSmallRad;

  int fNpointsCut;

  std::vector<int> fExclusionList;

public:  
  TracksFinder(TObjArray*);

  inline const TObjArray* GetPointsArray() const {return fPointsArray;}
  inline int GetNumberOfPoints()           const {return fNpoints;}

  inline void SetSeedRadCut(double cut)    { fSeedRadCut=cut; }
  inline double GetSeedRadCut() const      { return fSeedRadCut; }
  inline void SetPointsDistCut(double cut) { fPointsDistCut=cut; }
  inline double GetPointsDistCut() const   { return fPointsDistCut; }
  inline void SetNpointsCut(int cut)       { fNpointsCut=cut; }
  inline int GetNpointsCut() const         { return fNpointsCut; }
  
  
  //  inline std::vector<TObjArray*> GetTracks() {return fTracksArray;}
  inline const TObjArray* GetTracksArray() const {return fTracksArray;}
  //  inline TObjArray* GetTrack(int i) {return fTracksArray.at(i);}
  inline TTrack* GetTrack(int i)       { return (TTrack*) fTracksArray->At(i);}
  inline int GetNumberOfTracks() const {return fNtracks;}

  bool Skip(int);

  int RecTracks(TObjArray*);
  int FitLines();

  int AdaptiveFinder(TObjArray*);
  int NextPoint( int, double, track_t&);
  int NextPoint( int, double, double, double, track_t&);

  void JasonAnodeMethod();
  track_t FirstPass(int);
  int SecondPass( int, track_t& );
  int ThirdPass( track_t&, int );
  
};


#endif
