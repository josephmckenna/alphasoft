#include "TRecoHelixFitter.hh"

int TRecoHelixFitter::FitHelix(const std::vector<TTrack> TracksArray, std::vector<TFitHelix *> &HelixArray) const
{
   int n = 0;
   HelixArray.reserve(HelixArray.size() + TracksArray.size());

   for (const TTrack &at : TracksArray) {
      //at.Print();
      TFitHelix *helix = new TFitHelix(at); // Copy constructor
      helix->SetChi2ZCut(fHelChi2ZCut);
      helix->SetChi2RCut(fHelChi2RCut);
      helix->SetChi2RMin(fHelChi2RMin);
      helix->SetChi2ZMin(fHelChi2ZMin);
      helix->SetDCut(fHelDcut);
      //helix->Print();
#ifdef __MINUIT2FIT__
      helix.FitM2();
#else
      helix->Fit();
#endif
      if (helix->GetStatR() > 0 && helix->GetStatZ() > 0) helix->CalculateResiduals();

      if (helix->IsGood()) {
         // calculate momumentum
         double pt = helix->Momentum();
         if (fTrace) {
            helix->Print();
            std::cout << "Reco::FitHelix()  hel # " << n << " p_T = " << pt
                      << " MeV/c in B = " << helix->GetMagneticField() << " T" << std::endl;
         }
         HelixArray.emplace_back(helix);
         ++n;
      } else {
         if (fTrace) helix->Reason();
         helix->Clear();
      }
   }
   // fHelixArray.Compress();
   if (n != (int)HelixArray.size())
      std::cerr << "Reco::FitHelix() ERROR number of lines " << n << " differs from array size " << HelixArray.size()
                << std::endl;
   return n;
}
