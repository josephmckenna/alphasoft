#ifndef __RECO__
#define __RECO__ 1

#include <vector>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TClonesArray.h>

#include "SignalsType.hh"
#include "AnaSettings.h"

#include "LookUpTable.hh"
#include "TracksFinder.hh"
#include "TFitVertex.hh"
#include "TFitLine.hh"

enum finderChoice { base, adaptive, neural };

class TracksFinder;
class TFitVertex;
class Reco
{
private:
   bool fTrace;
   double fMagneticField;

   AnaSettings* ana_settings;

   double f_rfudge;
   double f_pfudge;

   TracksFinder *pattrec;

   std::vector<TSpacePoint*> fPointsArray;
   std::vector<TTrack*> fTracksArray;
   std::vector<TFitLine*> fLinesArray;
   std::vector<TFitHelix*> fHelixArray;
   LookUpTable* fSTR;

   // general TracksFinder parameters, also used by other finders
   // unsigned fNhitsCut; No used...
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

   // useful histos
   TH2D* hsprp; // spacepoints in found tracks
   TH2D* hspxy;
   TH2D* hspzp;
   TH1D* hspaw;

   TH1D* hchi2; // chi^2 of line fit
   TH2D* hchi2sp; // chi^2 of line fit Vs # of spacepoints

public:
   Reco(std::string, double);
   Reco(AnaSettings*, double);
   ~Reco();

   void Setup(TFile*);

   void AddMChits( const TClonesArray* mchits );

   void AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints );
   void AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints, double zcut );
   void AddSpacePoint( const TObjArray* points );
   void AddSpacePoint( std::vector<signal> *spacepoints );
   int FindTracks(finderChoice finder=adaptive);
   void AddTracks( const std::vector<track_t>* track_vector );
   int FitLines();
   int FitHelix();
   int RecVertex(TFitVertex* Vertex);

   inline void SetFudgeFactors(double fdgr, double fdgp) {f_rfudge=fdgr; f_pfudge=fdgp;}
   inline void GetFudgeFactors(double& fdgr, double& fdgp) const {fdgr=f_rfudge; fdgp=f_pfudge;}

   void Reset();

   inline std::vector<TSpacePoint*>* GetPoints()  { return &fPointsArray; }
   inline std::vector<TTrack*>*  GetTracks()  { return &fTracksArray; }
   inline std::vector<TFitLine*>* GetLines()   { return &fLinesArray; }
   inline std::vector<TFitHelix*>* GetHelices() { return &fHelixArray; }

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

   inline int GetNumberOfPoints() const { return fPointsArray.size(); }
   inline int GetNumberOfTracks() const { return fTracksArray.size(); }

   inline const TracksFinder* GetTracksFinder() const { return pattrec; }

   void UseSTRfromData(int runNumber);

   void PrintPattRec();
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
