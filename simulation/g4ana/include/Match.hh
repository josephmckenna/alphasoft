#ifndef __MATCH__
#define __MATCH__ 1

#include <vector>
#include <set>

#include "SignalsType.h"
#include "AnaSettings.h"

class Match
{
private:
  bool fTrace;

  AnaSettings* ana_settings;
  double fCoincTime; // ns

  int maxPadGroups; // max. number of separate groups of pads coincident with single wire signal
  double padSigma; // width of single avalanche charge distribution = 2*(pad-aw)/2.34
  double padSigmaD; // max. rel. deviation of fitted sigma from padSigma
  double padFitErrThres; // max. accepted error on pad gaussian fit mean
  bool use_mean_on_spectrum;
  double spectrum_mean_multiplyer; //if use_mean_on_spectrum is true, this is used.
  double spectrum_cut;              //if use_mean_on_spectrum is false, this is used.
  double spectrum_width_min;

  std::vector<signal> fCombinedPads;
  std::vector< std::pair<signal,signal> > spacepoints;

  std::set<short> PartionBySector(std::vector<signal>* padsignals, std::vector< std::vector<signal> >& pad_bysec);
  std::vector< std::vector<signal> > PartitionByTime( std::vector<signal>& sig );
  std::vector<std::vector<signal>> CombPads(std::vector<signal>* padsignals);
  void CentreOfGravity_nohisto( std::vector<signal> &vsig );
  void CentreOfGravity_nofit( std::vector<signal> &vsig );
  void CentreOfGravity( std::vector<signal> &vsig );

  void SortPointsAW(  const std::pair<double,int>& pos,
		      std::vector<std::pair<signal,signal>*>& vec, 
		      std::map<int,std::vector<std::pair<signal,signal>*>>& spaw );
  void CombPointsAW(std::map<int,std::vector<std::pair<signal,signal>*>>& spaw, 
		    std::map<int,std::vector<std::pair<signal,signal>*>>& merger);

  uint MergePoints(std::map<int,std::vector<std::pair<signal,signal>*>>& merger,
		   std::vector<std::pair<signal,signal>>& merged,
		   uint& number_of_merged);

  double phi_err;
  double zed_err;

public:
  Match(std::string);
  ~Match();

  void Init();
  void CombinePads(std::vector<signal>* padsignals);
  void MatchElectrodes(std::vector<signal>* awsignals);
  void CombPoints();
  void FakePads(std::vector<signal>* awsignals);

  std::vector<signal>* GetCombinedPads() { return &fCombinedPads; }
  std::vector< std::pair<signal,signal> >* GetSpacePoints() { return &spacepoints; }

  void SetTrace(bool t) { fTrace=t; }
};

#endif
