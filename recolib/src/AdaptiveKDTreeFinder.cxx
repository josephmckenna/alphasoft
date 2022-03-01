// Tracks finder class implementation based on AdaptiveKDTreeFinder
// for ALPHA-g TPC analysis
// Author: J. T. K. MCKENNA
// Date: March 2022

#include "TPCconstants.hh"
#include "AdaptiveKDTreeFinder.hh"
#include <iostream>

AdaptiveKDTreeFinder::AdaptiveKDTreeFinder(std::vector<TSpacePoint*>* points, const double maxIncrease, const double LastPointRadCut):
   TracksFinder(points),
   fLastPointRadCut(LastPointRadCut),
   fPointsRadCut(4.),
   fPointsPhiCut( ALPHAg::_anodepitch*2. ),
   fPointsZedCut( ALPHAg::_padpitch*1.1 ),
   fMaxIncreseAdapt(maxIncrease),
   fPoints(points.size(), 3, 1)

{
   // No inherent reason why these parameters should be the same as in base class
   const npoints = points.size();
   fX.reserve(npoints);
   fY.reserve(npoints);
   fZ.reserve(npoints);
   for (const TSpacePoint* p: points)
   {
       fX.emplace_back(p->GetX());
       fY.emplace_back(p->GetY());
       fZ.emplace_back(p->GetZ());
   }
   fPoints.SetData(0, fX.data());
   fPoints.SetData(1, fY.data());
   fPoints.SetData(1, fZ.data());
   fPoints.Build();

   fSeedRadCut = 150.;
   fPointsDistCut = 8.1;
   fSmallRad = ALPHAg::_cathradius;
   fNpointsCut = 7;
   if( debug )
      std::cout<<"AdaptiveKDTreeFinder::AdaptiveKDTreeFinder ctor!"<<std::endl;
}

//==============================================================================================
int AdaptiveKDTreeFinder::RecTracks()
{
   int Npoints = fPointsArray.size();
   if( Npoints<=0 )
      return -1;
   if( debug )
      std::cout<<"AdaptiveKDTreeFinder::AdaptiveKDTreeFinder() # of points: "<<Npoints<<std::endl;

   // Pattern Recognition algorithm
   for(int i=0; i<Npoints; ++i)
      {
         TSpacePoint* point=fPointsArray[i];
         if (!point) continue;
         // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
         // thus I include spacepoints up to r=173mm
         // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
         if( !point->IsGood(ALPHAg::_cathradius, ALPHAg::_fwradius-1.) )
            {
#if BUILD_EXCLUSION_LIST
               fExclusionList.push_back(fPointsArray[i]);
#endif
               fPointsArray[i]=NULL;
               continue;
            }

         // do not start a track far from the anode
         if( point->GetR() < fSeedRadCut && fTrackVector.size() > 0 ) break;

         track_t vector_points;
         vector_points.clear();

         int gapidx = NextPoint( point, i , Npoints, fPointsDistCut, vector_points );
         TSpacePoint* LastPoint =  fPointsArray.at( gapidx );

         double AdaptDistCut = fPointsDistCut*1.1;
         while( LastPoint->GetR() > fSmallRad )
            {
               // LastPoint->Print("rphi");
               // std::cout<<"AdaptDistCut: "<<AdaptDistCut<<" mm"<<std::endl;
               if( AdaptDistCut > fMaxIncreseAdapt ) break;
               gapidx = NextPoint( LastPoint, gapidx ,Npoints, AdaptDistCut, vector_points );
               LastPoint = fPointsArray.at( gapidx );
               AdaptDistCut*=1.1;
            }

         if( int(vector_points.size()) < fNpointsCut )
            {
               ++points_cut;
               continue;
            }
         else if( LastPoint->GetR() > fLastPointRadCut )
            {
               ++rad_cut;
               continue;
            }
         else
            {
               vector_points.push_front(i);

               fTrackVector.push_back( vector_points );
               for(auto& it: vector_points)
                  {
#if BUILD_EXCLUSION_LIST
                     fExclusionList.push_back(fPointsArray[it]);
#endif
                     fPointsArray[it]=NULL;
                  }
               ++fNtracks;
            }
      }//i loop

   if( fNtracks != int(fTrackVector.size()) )
      std::cerr<<"AdaptiveKDTreeFinder::AdaptiveKDTreeFinder(): Number of found tracks "<<fNtracks
               <<" does not match the number of entries "<<fTrackVector.size()<<std::endl;
   else if( debug )
      {
         std::cout<<"AdaptiveKDTreeFinder::AdaptiveKDTreeFinder(): Number of found tracks "<<fNtracks<<std::endl;
         std::cout<<"AdaptiveKDTreeFinder::AdaptiveKDTreeFinder() -- Reasons: Track Not Advancing "<<track_not_advancing
                  <<" Points Cut: ("<<fNpointsCut<<"): "<<points_cut
                  <<" Radius Cut ("<<fLastPointRadCut<<" mm): "<<rad_cut<<std::endl;
      }

   return fNtracks;
}

int AdaptiveKDTreeFinder::NextPoint(TSpacePoint* SeedPoint, const int index, const int Npoints, double distcut, track_t& atrack) const
{
   TSpacePoint* NextPoint = 0;

   int LastIndex = index;
   TSpacePoint* NextPoint = index +1;
   std::array<double,3> xyz = {NextPoint->GetX(),NextPoint->GetY(),NextPoint->GetZ()};
   std::vector<int> index_in_range;
   fPoints.FindInRange (xyz.begin(), distcut, index_in_range);
   for (int i: index_in_range)
   {
       std::vector<int>::iterator f = std::find(atrack.begin(), atrack.end(),i);
       // If this point isn't in list
       if (f == atrack.end())
       {
          atrack.push_back(i);
          NextPoint(NextPoint, i, Npoints, fPointsDistCut, atrack);
       }
   }

   for(int j = index+1; j < Npoints; ++j)
      {
         NextPoint = fPointsArray[j];
         if (!NextPoint) continue;
         if( SeedPoint->Distance( NextPoint ) <= distcut )
            {
               SeedPoint = NextPoint;
               atrack.push_back(j);
               LastIndex = j;
               distcut = fPointsDistCut;
               //Just an idea: but right now I change the results
               /*if( int(atrack.size()) > fNpointsCut )
                 {
                 //Track already has more points than the points cut... abort
                 return LastIndex;
                 }*/
            }
      }// j loop
   return LastIndex;
}

int AdaptiveKDTreeFinder::NextPoint(const int index,
                              double radcut, double phicut, double zedcut,
                              track_t& atrack) const
{
   TSpacePoint* SeedPoint = fPointsArray.at( index );
   TSpacePoint* NextPoint = 0;

   int LastIndex = index;
   int Npoints = fPointsArray.size();
   for(int j = index+1; j < Npoints; ++j)
      {
         NextPoint = fPointsArray[j];
         if (!NextPoint) continue;
         if( SeedPoint->MeasureRad( NextPoint ) <= radcut &&
             SeedPoint->MeasurePhi( NextPoint ) <= phicut &&
             SeedPoint->MeasureZed( NextPoint ) <= zedcut )
            {
               SeedPoint = NextPoint;
               atrack.push_back(j);
               LastIndex = j;
               radcut = fPointsRadCut;
               phicut = fPointsPhiCut;
               zedcut = fPointsZedCut;
            }
      }// j loop

   return LastIndex;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
