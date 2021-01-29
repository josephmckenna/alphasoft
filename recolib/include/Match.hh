#ifndef __MATCH__
#define __MATCH__ 1

#include <vector>
#include <set>
#include "TH1D.h"
#include "TH2D.h"

#include "SignalsType.hh"
#include "AnaSettings.hh"
#include "TFile.h"

#include <mutex>          // std::mutex

class Match
{
private:
   bool fTrace;
   bool fDebug;
   bool diagnostic;

   std::mutex* manalzer_global_mtx;

   const AnaSettings* ana_settings;
   const double fCoincTime; // ns

   const int maxPadGroups; // max. number of separate groups of pads coincident with single wire signal
   const int padsNmin;     // minimum number of coincident pad hits to attempt reconstructing a point
   const double padSigma; // width of single avalanche charge distribution = 2*(pad-aw)/2.34
   const double padSigmaD; // max. rel. deviation of fitted sigma from padSigma
   const double padFitErrThres; // max. accepted error on pad gaussian fit mean
   const bool use_mean_on_spectrum;
   const double spectrum_mean_multiplyer; //if use_mean_on_spectrum is true, this is used.
   const double spectrum_cut;              //if use_mean_on_spectrum is false, this is used.
   const double spectrum_width_min;

   const double grassCut;       // don't consider peaks smaller than grassCut factor of a
   const double goodDist;       // neighbouring peak, if that peak is closer than goodDist

   const double charge_dist_scale;  // set to zero to not use, other value gets multiplied by padThr
   const double padThr;               // needed for wire-dependent pad threshold

   const double phi_err = ALPHAg::_anodepitch*ALPHAg::_sq12;
   const double zed_err = ALPHAg::_padpitch*ALPHAg::_sq12;
   int CentreOfGravityFunction = -1; //One day we can maybe make this const :)

   double relCharge[8] = {1., 1.33687717, 1.50890722, 1.56355571, 1.56355571, 1.50890722, 1.33687717, 1.};



   //bool diagnostic;

   //std::vector<signal>* fCombinedPads;
   //std::vector< std::pair<signal,signal> >* spacepoints;

   std::pair<std::set<short>,std::vector< std::vector<ALPHAg::signal> >> PartitionBySector(std::vector<ALPHAg::signal>* padsignals);
   std::vector< std::vector<ALPHAg::signal> > PartitionByTime( std::vector<ALPHAg::signal>& sig );

   void CentreOfGravity( std::vector<ALPHAg::signal> &vsig, std::vector<ALPHAg::signal>* combpads ); // #1
   void CentreOfGravity_blobs( std::vector<ALPHAg::signal> &vsig, std::vector<ALPHAg::signal>* combpads); // #2

   std::vector<std::pair<double, double> > FindBlobs(TH1D *h);

   void SortPointsAW(  const std::pair<double,int>& pos,
                       std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>& vec,
                       std::map<int,std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>,std::greater<int>>& spaw );
   void SortPointsAW(  std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>& vec,
                       std::map<int,std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>,std::greater<int>>& spaw );

   //  void SortPointsAW(  const std::pair<double,int>& pos,
   //		      std::vector<std::pair<signal,signal>*>& vec,
   //		      std::map<int,std::vector<std::pair<signal,signal>*>>& spaw );
   void CombPointsAW(std::map<int,std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>,std::greater<int>>& spaw,
                     std::map<int,std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>>& merger);
   void CombPointsAW(std::map<int,std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>>& spaw,
                     std::map<int,std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>>& merger);

   uint MergePoints(std::map<int,std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>*>>& merger,
                    std::vector<std::pair<ALPHAg::signal,ALPHAg::signal>>& merged,
                    uint& number_of_merged);

 
   std::vector<std::pair<double, double> > FindBlobs(const std::vector<ALPHAg::signal> &sigs,
                                                     int ifirst, int ilast);


   //Use static to share these histograms between all instances of Match
   // static TH1D *hsigCoarse;
   // static TH1D *hsig;
   static TH1D* hcognpeaks;
   // static TH2D* hcognpeaksrms;
   // static TH2D* hcognpeakswidth;
   static TH1D* hcogsigma;
   static TH1D* hcogerr;
   static TH2D* hcogpadssigma;
   static TH2D* hcogpadsamp;
   static TH2D* hcogpadsint;
   static TH2D* hcogpadsampamp;
   static TH1D* htimecog;
   static TH1D* htimeblobs;
   static TH1D* htimefit;

   ALPHAg::padmap pmap;

public:
   Match(const AnaSettings* ana_settings, bool mt=false);
   Match(std::string json): Match(new AnaSettings(json.c_str()))  {}
   ~Match();
   void SetGlobalLockVariable(std::mutex* _manalyzerLock)
   {
      manalzer_global_mtx=_manalyzerLock;
   }
   void Init();
   void Setup(TFile* OutputFile);

   std::vector<std::vector<ALPHAg::signal>> CombPads(std::vector<ALPHAg::signal>* padsignals);
   std::vector<ALPHAg::signal>*  CombinePads(std::vector<ALPHAg::signal>* padsignals);
   std::vector<ALPHAg::signal>* CombinePads(std::vector< std::vector<ALPHAg::signal> > *comb); // this is the used now  -- AC 27-08-2020
   std::vector<ALPHAg::signal>* CombineAPad(std::vector< std::vector<ALPHAg::signal> > *comb,std::vector<ALPHAg::signal>* CombinedPads, size_t PadNo); //this is the replacement for CombinePads -- Joe 12-10-2020

   void MatchElectrodes(std::vector<ALPHAg::signal>* awsignals);
   std::vector< std::pair<ALPHAg::signal,ALPHAg::signal> >* MatchElectrodes(std::vector<ALPHAg::signal>* awsignals,
                        std::vector<ALPHAg::signal>* padsignals);
   std::vector< std::pair<ALPHAg::signal,ALPHAg::signal> >*  FakePads(std::vector<ALPHAg::signal>* awsignals);

   std::vector< std::pair<ALPHAg::signal,ALPHAg::signal> >* CombPoints(std::vector< std::pair<ALPHAg::signal,ALPHAg::signal> >* spacepoints);

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
