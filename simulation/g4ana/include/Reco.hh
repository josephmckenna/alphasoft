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

   // general TracksFinder parameters, also used by other finders
   unsigned fNhitsCut;
   unsigned fNspacepointsCut;
   double fPointsDistCut;
   double fSmallRadCut;         // unused?
   double fSeedRadCut;

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

   // TFitLine
   double fLineChi2Cut;
   double fLineChi2Min;

   // TFitHelix
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

   void AddMChits( const TClonesArray* mchits );

   void AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints );
   void AddTracks( const std::vector<track_t>* track_vector );
   int FitLines();
   int FitHelix();
   int RecVertex(TFitVertex* Vertex);

   void Reset();

   inline TClonesArray* GetPoints()  { return &fPointsArray; }
   inline TClonesArray* GetTracks()  { return &fTracksArray; }
   inline TClonesArray* GetLines()   { return &fLinesArray; }
   inline TClonesArray* GetHelices() { return &fHelixArray; }

   inline void SetTrace(bool t) { fTrace = t; }

   inline unsigned GetNspacepointsCut() const { return fNspacepointsCut; }
   inline double GetPointsDistCut() const     { return fPointsDistCut; }
   inline double GetMaxIncreseAdapt() const   { return fMaxIncreseAdapt; }
   inline double GetSeedRadCut() const        { return fSeedRadCut; }
   inline double GetSmallRadCut() const       { return fSmallRadCut; }
   inline double GetLastPointRadCut() const   { return fLastPointRadCut; }
   inline double GetLambda() const            { return fLambda; }
   inline double GetAlpha() const             { return fAlpha; }
   inline double GetB() const                 { return fB; }
   inline double GetTemp() const              { return fTemp; }
   inline double GetC() const                 { return fC; }
   inline double GetMu() const                { return fMu; }
   inline double GetCosCut() const            { return fCosCut; }
   inline double GetVThres() const            { return fVThres; }
   inline double GetDNormXY() const           { return fDNormXY; }
   inline double GetDNormZ() const            { return fDNormZ; }
   inline double GetTscale() const            { return fTscale; }
   inline int GetMaxIt() const                { return fMaxIt; }
   inline double GetItThres() const           { return fItThres; }


   inline int GetNumberOfPoints() const { return fPointsArray.GetEntriesFast(); }
   inline int GetNumberOfTracks() const { return fTracksArray.GetEntriesFast(); }
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
