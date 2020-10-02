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

#include <mutex>          // std::mutex

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

   std::vector<asignal>* fCombinedPads;
   std::vector< std::pair<asignal,asignal> >* spacepoints;

   std::set<short> PartionBySector(std::vector<asignal>* padsignals, std::vector< std::vector<asignal> >& pad_bysec);
   std::vector< std::vector<asignal> > PartitionByTime( std::vector<asignal>& sig );

   void CentreOfGravity( std::vector<asignal> &vsig ); // #0
   //  void CentreOfGravity_blobs( std::vector<asignal> &vsig,  std::vector<asignal> &padcog ); // #6
   void CentreOfGravity_blobs( std::vector<asignal> &vsig); // #6
   void CentreOfGravity_nohisto( std::vector<asignal> &vsig ); // #2
   void CentreOfGravity_nofit( std::vector<asignal> &vsig ); // #1
   void CentreOfGravity_single_peak( std::vector<asignal> &vsig ); // #3
   void CentreOfGravity_multi_peak( std::vector<asignal> &vsig ); // #4
   void CentreOfGravity_histoblobs( std::vector<asignal> &vsig ); // #6

   std::vector<std::pair<double, double> > FindBlobs(TH1D *h);

   void SortPointsAW(  const std::pair<double,int>& pos,
                       std::vector<std::pair<asignal,asignal>*>& vec,
                       std::map<int,std::vector<std::pair<asignal,asignal>*>,std::greater<int>>& spaw );
   void SortPointsAW(  std::vector<std::pair<asignal,asignal>*>& vec,
                       std::map<int,std::vector<std::pair<asignal,asignal>*>,std::greater<int>>& spaw );

   //  void SortPointsAW(  const std::pair<double,int>& pos,
   //		      std::vector<std::pair<asignal,asignal>*>& vec,
   //		      std::map<int,std::vector<std::pair<asignal,asignal>*>>& spaw );
   void CombPointsAW(std::map<int,std::vector<std::pair<asignal,asignal>*>,std::greater<int>>& spaw,
                     std::map<int,std::vector<std::pair<asignal,asignal>*>>& merger);
   void CombPointsAW(std::map<int,std::vector<std::pair<asignal,asignal>*>>& spaw,
                     std::map<int,std::vector<std::pair<asignal,asignal>*>>& merger);

   uint MergePoints(std::map<int,std::vector<std::pair<asignal,asignal>*>>& merger,
                    std::vector<std::pair<asignal,asignal>>& merged,
                    uint& number_of_merged);

   std::vector<std::pair<double, double> > FindBlobs(TH1D *h, const std::vector<int> &cumulBins);
   std::vector<std::pair<double, double> > FindBlobs(const std::vector<asignal> &sigs,
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

   TH1D* htimecog;
   TH1D* htimeblobs;
   TH1D* htimefit;

   padmap pmap;

   std::mutex mtx;

public:
   Match(AnaSettings* ana_settings);
   Match(std::string json): Match(new AnaSettings(json.c_str()))  {}
   ~Match();

   void Init();
   void Setup(TFile* OutputFile);

   std::vector<std::vector<asignal>> CombPads(std::vector<asignal>* padsignals);
   void CombinePads(std::vector<asignal>* padsignals);
   void CombinePads(std::vector< std::vector<asignal> > *comb); // this is the used now  -- AC 27-08-2020

   void MatchElectrodes(std::vector<asignal>* awsignals);
   void MatchElectrodes(std::vector<asignal>* awsignals,
                        std::vector<asignal>* padsignals);
   void CombPoints();
   void FakePads(std::vector<asignal>* awsignals);

   std::vector<asignal>* GetCombinedPads() { return fCombinedPads; }
   std::vector< std::pair<asignal,asignal> >* GetSpacePoints() { return spacepoints; }

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
