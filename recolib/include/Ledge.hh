//
// Analyze waveforms and extract
// Leading Edge information
// 
// Author: A. Capra
// Date: Nov. 2019
//

#ifndef __LEDGE__
#define __LEDGE__ 1

#include <string>
#include <vector>
#include <iterator>
#include <numeric>
#include <algorithm>
#include <cassert>
#include <cmath>

#include "Alpha16.h"
#include "Feam.h"

#include "TClonesArray.h"

#include "SignalsType.hh"

class Ledge
{
public:
   Ledge():fBaseline(100),fBinSize(ALPHAg::_timebin),
	  fTimeOffset(0.),
	  fGain(1.),fOffset(0.),
	  fCutBaselineRMS(0.),
	  fPulseHeightThreshold(0.),
	  fCFDfrac(1.),fMaxTime(4500.),
	  fDebug(false){};

  ~Ledge(){};

  int FindAnodeTimes(const Alpha16Event*);
  int FindPadTimes(const FeamEvent*);
  int FindAnodeTimes(TClonesArray*);
  int FindPadTimes(TClonesArray*);
  
  std::vector<asignal>* Analyze(std::vector<Alpha16Channel*> );
  std::vector<asignal>* Analyze(std::vector<FeamChannel*> );
  int Analyze(const std::vector<int>*, double& time, double& amp, double& err);

  inline std::vector<asignal>* GetSignal() { return fSignals; }
  
  inline void SetPedestalLength(int l)          { fBaseline = l; }
  inline void SetGain(double g)                 { fGain = g; }
  inline void SetRMSBaselineCut(double c)       { fCutBaselineRMS = c; }
  inline void SetPulseHeightThreshold(double t) { fPulseHeightThreshold = t; }
  inline void SetCFDfraction(double f)          { fCFDfrac = f; }
  inline void SetTimeOffset(double o)           { fTimeOffset = o; }
  inline void SetDebug(bool d=true)             { fDebug = d; }
    
private:
  int fBaseline;
  double fBinSize; // = 1000.0/62.5;// 62.5 MHz ADC
  double fTimeOffset;

  double fGain;
  double fOffset;

  double fCutBaselineRMS;
  double fPulseHeightThreshold;
  double fCFDfrac;

  double fMaxTime;

  std::vector<asignal>* fSignals;

  bool fDebug;

  inline void ComputeMeanRMS(std::vector<int>::const_iterator first,
			     std::vector<int>::const_iterator last,
			     double& mean, double& rms)
  {
    if(fBaseline==0.) return;
    int length = std::distance( first, last );
    assert(length==fBaseline);

    mean = (double) std::accumulate(first,last,0);
    mean/=double(length);
    
    std::vector<double> temp(length);
    // calculate the difference from the mean: xi-m
    std::transform(first, last, temp.begin(),bind2nd(std::minus<double>(), mean));
    // square it: (xi-m)*(xi-m)
    std::transform(temp.begin(),temp.end(),temp.begin(),temp.begin(),std::multiplies<double>());
    // norm = (N-1)/N 
    double norm = double(length-1)/double(length);
    // rms = sqrt( (sum_i yi*(xi-m)*(xi-m))/ norm )
    rms = sqrt( std::accumulate(temp.begin(), temp.end(), 0.) / norm); // rms
  }

  inline double FindLeadingEdge(std::vector<int>::const_iterator first,
				std::vector<int>::const_iterator last,
				double& baseline, double& threshold)
  {
    auto it = std::find_if( first, last, [&](int v)
			    { return ((double(v)-baseline)*-1.) > threshold; });
    if( it != last && ((double(*std::prev( it ))-baseline)*-1.) <= threshold ) 
      return double(std::distance(first, it));
    return 0.;
  }
};

#endif


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
