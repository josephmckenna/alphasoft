#ifndef __RECOLINEFITTER__
#define __RECOLINEFITTER__

#include "AnaSettings.hh"
#include "TTrack.hh"
#include "TFitLine.hh"

class TRecoLineFitter

{
   private:
      double fLineChi2Cut;
      double fLineChi2Min;
      double fNspacepointsCut;
   const bool fTrace;

   public:
   TRecoLineFitter(AnaSettings* ana_settings, bool trace): fTrace(trace)
   {
      fLineChi2Cut = ana_settings->GetDouble("RecoModule","LineChi2Cut");
      fLineChi2Min = ana_settings->GetDouble("RecoModule","LineChi2Min");
      fNspacepointsCut = ana_settings->GetInt("RecoModule","NpointsCut");
   }
   int FitLine(const std::vector<TTrack> TracksArray, std::vector<TFitLine*>& LineArray, const int thread_no = 1, const int total_threads = 1) const;


};


#endif