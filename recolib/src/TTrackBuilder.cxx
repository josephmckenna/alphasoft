#include "TTrackBuilder.hh"

TTrackBuilder::TTrackBuilder(double B, std::string location, bool trace): fMagneticField(B), fLocation(location), fTrace(trace)
{
   if( fMagneticField < 0. ) // garfield++ sim with field map
   {
      fSTR = new LookUpTable(ALPHAg::_co2frac); // field map version (simulation)
      fMagneticField = 1.;
   }
   else
   {
      fSTR = new LookUpTable(ALPHAg::_co2frac, fMagneticField, fLocation); // uniform field version (simulation)
   }
}



void TTrackBuilder::BuildTracks( const std::vector<track_t> track_vector, const std::vector<TSpacePoint*> PointsArray, std::vector<TTrack>& TTrackArray )
{
   TTrackArray.reserve( TTrackArray.size() + track_vector.size());
   int n=0;
   for (const track_t& t: track_vector)
   //for( auto it=track_vector.begin(); it!=track_vector.end(); ++it)
      {
         TTrackArray.emplace_back(fMagneticField);
         TTrack& thetrack = TTrackArray.back();
         //std::cout<<"Reco::AddTracks Check Track # "<<n<<" "<<std::endl;
         for ( const int& pointNumber: t)
            {
               thetrack.AddPoint( *PointsArray[pointNumber] );
            }
         //            std::cout<<"\n";
         ++n;
      }
   //fTracksArray.Compress();
   //std::cout<<"Reco::AddTracks "<<n<<"\t"<<track_vector.size()<<"\t"<<TTrackArray.size()<<std::endl;   assert(n==int(track_vector->size()));
   assert(fTracksArray.size()==track_vector->size());
   if( fTrace )
      std::cout<<"Reco::AddTracks # entries: "<<TTrackArray.size()<<std::endl;
}
