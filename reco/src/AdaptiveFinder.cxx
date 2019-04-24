// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "AdaptiveFinder.hh"
#include "TFitLine.hh"
// #include "TFitHelix.hh"
#include <iostream>

AdaptiveFinder::AdaptiveFinder(std::vector<TSpacePoint*>* points):
   TracksFinder(points),
   fLastPointRadCut(135.),
   fPointsRadCut(4.),
   fPointsPhiCut( _anodepitch*2. ),
   fPointsZedCut( _padpitch*1.1 ),
   fMaxIncreseAdapt(41.)
{
   // No inherent reason why these parameters should be the same as in base class
   fSeedRadCut = 150.;
   fPointsDistCut = 8.1;
   fSmallRad = _cathradius;
   fNpointsCut = 7;
   //  std::cout<<"AdaptiveFinder::AdaptiveFinder"<<std::endl;
}

//==============================================================================================
int AdaptiveFinder::RecTracks()
{
   int Npoints = fPointsArray.size();
   if( Npoints<=0 )
      return -1;
   //  std::cout<<"AdaptiveFinder::AdaptiveFinder() # of points: "<<Npoints<<std::endl;

   // Pattern Recognition algorithm
   for(int i=0; i<Npoints; ++i)
      {
         TSpacePoint* point=fPointsArray[i];
         if (!point) continue;
         // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
         // thus I include spacepoints up to r=173mm
         // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
         if( !point->IsGood(_cathradius, _fwradius-1.) )
            {
#if BUILD_EXCLUSION_LIST
               fExclusionList.push_back(fPointsArray[i]);
#endif
               fPointsArray[i]=NULL;
               continue;
            }

         // do not start a track far from the anode
         if( point->GetR() < fSeedRadCut ) break;

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

               //AddTrack( vector_points );
            }
      }//i loop

   if( fNtracks != int(fTrackVector.size()) )
      std::cerr<<"AdaptiveFinder::AdaptiveFinder(): Number of found tracks "<<fNtracks
               <<" does not match the number of entries "<<fTrackVector.size()<<std::endl;
   else
      {
         std::cout<<"AdaptiveFinder::AdaptiveFinder(): Number of found tracks "<<fNtracks<<std::endl;
         std::cout<<"AdaptiveFinder::AdaptiveFinder() -- Reasons: Track Not Advancing "<<track_not_advancing
                  <<" Points Cut: "<<points_cut
                  <<" Radius Cut: "<<rad_cut<<std::endl;
      }

   return fNtracks;
}

int AdaptiveFinder::NextPoint(TSpacePoint* SeedPoint, int index, int Npoints, double distcut, track_t& atrack)
{
   TSpacePoint* NextPoint = 0;

   int LastIndex = index;
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

int AdaptiveFinder::NextPoint(int index,
                              double radcut, double phicut, double zedcut,
                              track_t& atrack)
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
