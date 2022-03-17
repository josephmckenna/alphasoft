#ifndef __TRECOHELIXFITTER__
#define __TRECOHELIXFITTER__

#include "AnaSettings.hh"
#include "TTrack.hh"
#include "TFitHelix.hh"
#include <iostream>

class TRecoHelixFitter

{
   private:
      double fHelChi2RCut;
      double fHelChi2ZCut;
      double fHelChi2RMin;
      double fHelChi2ZMin;
      double fHelDcut;
   const bool fTrace;

   public:
   TRecoHelixFitter(AnaSettings* ana_settings, bool trace): fTrace(trace)
   {
      fHelChi2RCut = ana_settings->GetDouble("RecoModule","HelChi2RCut");
      fHelChi2ZCut = ana_settings->GetDouble("RecoModule","HelChi2ZCut");
      fHelChi2RMin = ana_settings->GetDouble("RecoModule","HelChi2RMin");
      fHelChi2ZMin = ana_settings->GetDouble("RecoModule","HelChi2ZMin");
      fHelDcut = ana_settings->GetDouble("RecoModule","HelDcut");
   }
   int FitHelix(const std::vector<TTrack> TracksArray, std::vector<TFitHelix>& HelixArray, const int thread_no = 1, const int total_threads = 1) const;


};

#endif