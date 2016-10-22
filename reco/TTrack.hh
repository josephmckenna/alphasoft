// Track class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: June 2016

#ifndef __TTRACK__
#define __TTRACK__ 1

#include <TObject.h>

class TSpacePoint;
class TFitHelix;
class TFitLine;
class TTrack: public TObject
{
private:

  TObjArray* fPoints;
  int fNpoints;
  bool fB;

  TFitHelix* fHelix;
  TFitLine* fLine;

  TObjArray* fTrackArray;

  double fPointsDistCut;
  double fPointsRadCut;
  double fPointsPhiCut;
  double fPointsZedCut;

public:
  TTrack(){};
  TTrack(TObjArray*,bool);
  ~TTrack();

  int TrackFinding();
  //  int FitAll();
  int Fit();

  inline const TObjArray* GetPointsArray() const {return fPoints;}
  inline int GetNumberOfPoints()           const {return fNpoints;}

  inline bool GetFieldStatus() const {return fB;}

  TFitHelix* GetHelix() {return fHelix;}
  TFitLine* GetLine()   {return fLine;}

  TObjArray* GetTracks() {return fTrackArray;}

  inline void SetDistCut(double d) { fPointsDistCut = d; }
  inline int GetDistCut() const { return fPointsDistCut; }
  inline void SetDistCut(double r, double phi, double z) 
  { fPointsRadCut = r; fPointsPhiCut = phi; fPointsZedCut = z; }

  ClassDef(TTrack,1)
};

#endif
