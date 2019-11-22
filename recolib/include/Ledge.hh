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

#include "SignalsType.hh"

class Ledge
{
public:
  Ledge();
  ~Ledge();

  int FindAnodeTimes(const Alpha16Event*);
  int FindPadTimes(const FeamEvent*);
  
  // void Analyze(const Alpha16Channel*);
  // void Analyze(const FeamChannel*);
  std::vector<signal>* Analyze(std::vector<Alpha16Channel*> );
  std::vector<signal>* Analyze(std::vector<FeamChannel*> );
  int Analyze(const std::vector<int>*, double& time, double& amp, double& err);

  inline std::vector<signal>* GetSignal() { return fSignals; }
  
  inline void SetGain(double g)                 { fGain = g; }
  inline void SetRMSBaselineCut(double c)       { fCutBaselineRMS = c; }
  inline void SetPulseHeightThreshold(double t) { fPulseHeightThreshold = t; }
  inline void SetCFDfraction(double f)          { fCFDfrac = f; }
    
private:
  int fBaseline;
  double fBinSize;
  double fTimeOffset;

  double fGain;
  double fOffset;

  double fCutBaselineRMS;
  double fPulseHeightThreshold;
  double fCFDfrac;

  double fMaxTime;

  std::vector<signal>* fSignals;

  inline void ComputeMeanRMS(std::vector<int>::const_iterator first,
			     std::vector<int>::const_iterator last,
			     double& mean, double& rms)
  {
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
 
  inline double find_pulse_time(const int* adc, int nbins, 
				double baseline, double gain, double threshold)
  { 
    for (int i=1; i<nbins; i++) 
     {
       double v1 = (adc[i]-baseline)*gain;
       if( v1 > threshold )
	 {
	   double v0 = (adc[i-1]-baseline)*gain;
	   if( !(v0 <= threshold) ) return 0.; // <-- safeguard from what?
	   double ii = i-1+(v0-threshold)/(v0-v1);
	   //printf("find_pulse_time: %f %f %f, bins %d %f %d\n", v0, threshold, v1, i-1, ii, i);
	   return ii;
	 }
     }
    return 0.;
  }

  inline double FindLeadingEdge(std::vector<int>::const_iterator first,
				std::vector<int>::const_iterator last, 
				double threshold)
  {
    auto it = std::find_if( first, last, [threshold](int v){return double(v) > threshold;});
    if( it != last && *std::prev( it ) <= threshold ) return double(std::distance(first, it));
    return 0.;
  }
};

#endif
