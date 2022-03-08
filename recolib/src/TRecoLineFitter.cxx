#include "TRecoLineFitter.hh"


int TRecoLineFitter::FitLine(const std::vector<TTrack> TracksArray, std::vector<TFitLine*>& LinesArray) const
{
   int n=0;
   LinesArray.reserve(LinesArray.size() + TracksArray.size());
   int ntracks=TracksArray.size();
   for(int it=0; it<ntracks; ++it )
      {
         const TTrack& at = TracksArray.at(it);
         TFitLine* line=new TFitLine(at); //Copy constructor
         line->SetChi2Cut( fLineChi2Cut );
         line->SetChi2Min( fLineChi2Min );
         line->SetPointsCut( fNspacepointsCut );
#ifdef __MINUIT2FIT__
         line->FitM2();
#else
         line->Fit();
#endif
         if( line->GetStat() > 0 )
            {
               line->CalculateResiduals();
            }
         if( line->IsGood() )
            {
               if( fTrace )
                  line->Print();
               ++n;
               LinesArray.push_back(line);
            }
         else
            {
               if( fTrace )
                  line-> Reason();
               delete line;
            }
      }
   //fLinesArray.Compress();
   if( n != (int)LinesArray.size() )
      std::cerr<<"Reco::FitLines() ERROR number of lines "<<n
               <<" differs from array size "<<LinesArray.size()<<std::endl;
   return n;
}