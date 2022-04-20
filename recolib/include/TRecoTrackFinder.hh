#ifndef __RECOTRACKFINDER__
#define __RECOTRACKFINDER__

#include <vector>
#include "TracksFinder.hh"
#include "AdaptiveFinder.hh"
#include "NeuralFinder.hh"
#include <iostream>
#include <string>
#include "AnaSettings.hh"
class TRecoTrackFinder
{
   private:
   TracksFinder *pattrec;

   // AdaptiveFinder
   double fMaxIncreseAdapt;
   double fLastPointRadCut;

   // NeuralFinder
   // V_kl = 0.5 * [1 + tanh(c/Temp \sum(T_kln*V_ln) - alpha/Temp{\sum(V_kn) + \sum(V_ml)} + B/Temp)]
   // NN parameters             // ALEPH values (see DOI 10.1016/0010-4655(91)90048-P)
   double fLambda;              // 5.
   double fAlpha;               // 5.
   double fB;                   // 0.2
   double fTemp;                // 1.
   double fC;                   // 10.
   double fMu;                  // 2.
   double fCosCut;              // 0.9     larger kinks between neurons set T value to zero
   double fVThres;              // 0.9     V value above which a neuron is considered active

   double fDNormXY;             // normalization for XY distance and
   double fDNormZ;              // Z distance, different to weight the influence of gaps differently
                                // no good reason for these values
   double fTscale;              // fudge factor to bring T values into range [0,1],
                                // probably has to be changed with other parameters...
   int fMaxIt;                  // number of iterations
   double fItThres;             // threshold defining convergence

   // general TracksFinder parameters, also used by other finders
   // unsigned fNhitsCut; No used...
   unsigned fNspacepointsCut;
   double fPointsDistCut;
   double fSeedRadCut;
   // Reasons for pattrec to fail:
   int track_not_advancing;
   int points_cut;
   int rad_cut;

   bool fTrace;
   public:
   
      //TRecoTrackFinder(std::string json, bool trace = false);
      TRecoTrackFinder(AnaSettings* ana_set,  bool trace = false);
      int FindTracks(const std::vector<TSpacePoint> PointsArray , std::vector<track_t>& TrackVector,  finderChoice finder);
      void PrintPattRec();


};





#endif