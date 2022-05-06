#include "TRecoHelixFitter.hh"

int TRecoHelixFitter::FitHelix(const std::vector<TTrack> TracksArray, std::vector<TFitHelix> &HelixArray, const int thread_no, const int total_threads) const
{

   if (thread_no == 1)
   {
      HelixArray.clear();
      HelixArray.reserve(TracksArray.size());
   }

   int n = HelixArray.size();

   const float slice_size = TracksArray.size() / (float)total_threads;
   const int start = floor(slice_size*(thread_no - 1));
   int stop = floor( slice_size * thread_no );
   //I am the last thread
   if (thread_no == total_threads)
      stop = TracksArray.size();

   for (int it = start; it < stop; ++it)
   {
      HelixArray.emplace_back(TracksArray[it]);// Copies TTrack
      TFitHelix& helix = HelixArray.back();
      helix.SetChi2ZCut(fHelChi2ZCut);
      helix.SetChi2RCut(fHelChi2RCut);
      helix.SetChi2RMin(fHelChi2RMin);
      helix.SetChi2ZMin(fHelChi2ZMin);
      helix.SetDCut(fHelDcut);
      //helix->Print();
#ifdef __MINUIT2FIT__
      helix.FitM2();
#else
      helix.Fit();
#endif
      if (helix.GetStatR() > 0 && helix.GetStatZ() > 0) helix.CalculateResiduals();

      if (helix.IsGood()) {
         // calculate momumentum
         double pt = helix.Momentum();
         if (fTrace) {
            helix.Print();
            std::cout << "Reco::FitHelix()  hel # " << n << " p_T = " << pt
                      << " MeV/c in B = " << helix.GetMagneticField() << " T" << std::endl;
         }
         ++n;
      } else {
         if (fTrace) helix.Reason();
         helix.Clear();
         HelixArray.pop_back();
      }
   }
   // fHelixArray.Compress();
   if (n != (int)HelixArray.size())
      std::cerr << "Reco::FitHelix() ERROR number of lines " << n << " differs from array size " << HelixArray.size()
                << std::endl;
   return n;
}
