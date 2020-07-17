#ifndef __MATCH__
#define __MATCH__ 1

#include <vector>
#include <set>
#include "TH1D.h"
#include "TH2D.h"

#include "SignalsType.hh"
#include "AnaSettings.h"
#include "TFile.h"
#include "manalyzer.h"


class Match
{
private:
   bool fTrace;
   bool fDebug;
   bool diagnostic;

   AnaSettings* ana_settings;
   double fCoincTime; // ns

   int maxPadGroups; // max. number of separate groups of pads coincident with single wire signal
   int padsNmin;     // minimum number of coincident pad hits to attempt reconstructing a point
   double padSigma; // width of single avalanche charge distribution = 2*(pad-aw)/2.34
   double padSigmaD; // max. rel. deviation of fitted sigma from padSigma
   double padFitErrThres; // max. accepted error on pad gaussian fit mean
   bool use_mean_on_spectrum;
   double spectrum_mean_multiplyer; //if use_mean_on_spectrum is true, this is used.
   double spectrum_cut;              //if use_mean_on_spectrum is false, this is used.
   double spectrum_width_min;

   double grassCut;       // don't consider peaks smaller than grassCut factor of a
   double goodDist;       // neighbouring peak, if that peak is closer than goodDist

   double charge_dist_scale;  // set to zero to not use, other value gets multiplied by padThr
   double padThr;               // needed for wire-dependent pad threshold

   double phi_err = ALPHAg::_anodepitch*ALPHAg::_sq12;
   double zed_err = ALPHAg::_padpitch*ALPHAg::_sq12;
   int CentreOfGravityFunction = -1;

   double relCharge[8] = {1., 1.33687717, 1.50890722, 1.56355571, 1.56355571, 1.50890722, 1.33687717, 1.};

   //bool diagnostic;

   std::vector<signal>* fCombinedPads;
   std::vector< std::pair<signal,signal> >* spacepoints;

   std::set<short> PartionBySector(std::vector<signal>* padsignals, std::vector< std::vector<signal> >& pad_bysec);
   std::vector< std::vector<signal> > PartitionByTime( std::vector<signal>& sig );
   std::vector<std::vector<signal>> CombPads(std::vector<signal>* padsignals);
   void CentreOfGravity( std::vector<signal> &vsig );
   int CentreOfGravity_blobs( std::vector<signal> &vsig );
   void CentreOfGravity_nohisto( std::vector<signal> &vsig );
   void CentreOfGravity_nofit( std::vector<signal> &vsig );
   void CentreOfGravity_single_peak( std::vector<signal> &vsig );
   void CentreOfGravity_multi_peak( std::vector<signal> &vsig );
   void CentreOfGravity_histoblobs( std::vector<signal> &vsig );

   std::vector<std::pair<double, double> > FindBlobs(TH1D *h);

   void SortPointsAW(  const std::pair<double,int>& pos,
                       std::vector<std::pair<signal,signal>*>& vec,
                       std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>>& spaw );
   void SortPointsAW(  std::vector<std::pair<signal,signal>*>& vec,
                       std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>>& spaw );

   //  void SortPointsAW(  const std::pair<double,int>& pos,
   //		      std::vector<std::pair<signal,signal>*>& vec,
   //		      std::map<int,std::vector<std::pair<signal,signal>*>>& spaw );
   void CombPointsAW(std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>>& spaw,
                     std::map<int,std::vector<std::pair<signal,signal>*>>& merger);
   void CombPointsAW(std::map<int,std::vector<std::pair<signal,signal>*>>& spaw,
                     std::map<int,std::vector<std::pair<signal,signal>*>>& merger);

   uint MergePoints(std::map<int,std::vector<std::pair<signal,signal>*>>& merger,
                    std::vector<std::pair<signal,signal>>& merged,
                    uint& number_of_merged);

   std::vector<std::pair<double, double> > FindBlobs(TH1D *h, const std::vector<int> &cumulBins);
   std::vector<std::pair<double, double> > FindBlobs(const std::vector<signal> &sigs,
                                                     int ifirst, int ilast);

   TH1D *hsigCoarse, *hsig;

   TH1D* hcognpeaks;
   TH2D* hcognpeaksrms;
   TH2D* hcognpeakswidth;
   TH1D* hcogsigma;
   TH1D* hcogerr;
   TH2D* hcogpadssigma;
   TH2D* hcogpadsamp;
   TH2D* hcogpadsint;
   TH2D* hcogpadsampamp;

   padmap pmap;

public:
   Match(AnaSettings* ana_settings);
   Match(std::string json): Match(new AnaSettings(json.c_str()))  {}
   ~Match();

   void Init();
   void Setup(TFile* OutputFile);
   void CombinePads(std::vector<signal>* padsignals);
   void MatchElectrodes(std::vector<signal>* awsignals);
   void MatchElectrodes(std::vector<signal>* awsignals,std::vector<signal>* padsignals);
   void CombPoints();
   void FakePads(std::vector<signal>* awsignals);

   std::vector<signal>* GetCombinedPads() { return fCombinedPads; }
   std::vector< std::pair<signal,signal> >* GetSpacePoints() { return spacepoints; }

   void SetTrace(bool t) { fTrace=t; }
   void SetDebug(bool d) { fDebug=d; }
   void SetDiagnostic(bool d) { diagnostic=d; }
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
