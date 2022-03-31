// Tracks finder class implementation 
// for ALPHA-g TPC analysis
// Author: J. T. K. MCKENNA
// Date: March 2022
// ...Not working yet...

#include "TPCconstants.hh"
#include "KDTreeFinder.hh"
#include <iostream>

KDTreeFinder::KDTreeFinder(const std::vector<TSpacePoint>* points, const double maxIncrease, const double LastPointRadCut):
   TracksFinder(points),
   fLastPointRadCut(LastPointRadCut),
   fPointsRadCut(4.),
   fPointsPhiCut( ALPHAg::_anodepitch*2. ),
   fPointsZedCut( ALPHAg::_padpitch*1.1 ),
   fMaxIncreseAdapt(maxIncrease)
{
   // No inherent reason why these parameters should be the same as in base class
   fPoints = new TKDTreeID(points->size(), 3, 1);
   const int npoints = points->size();
   fX.reserve(npoints);
   fY.reserve(npoints);
   fZ.reserve(npoints);
   fCluster.reserve(npoints);
   fClusterID = 0;
   for (const TSpacePoint* p: fPointsArray)
   {
       fCluster.emplace_back(0);
       if (p)
       {
          fX.emplace_back(p->GetX());
          fY.emplace_back(p->GetY());
          fZ.emplace_back(p->GetZ());
       }
       else
       {
          fX.emplace_back(0.);
          fY.emplace_back(0.);
          fZ.emplace_back(0.);
       }
   }
   fPoints->SetData(0, fX.data());
   fPoints->SetData(1, fY.data());
   fPoints->SetData(2, fZ.data());
   fPoints->Build();

   fSeedRadCut = 150.;
   fPointsDistCut = 8.1;
   fSmallRad = ALPHAg::_cathradius;
   fNpointsCut = 7;
   if( debug )
      std::cout<<"KDTreeFinder::KDTreeFinder ctor!"<<std::endl;
}

//==============================================================================================
int KDTreeFinder::RecTracks(std::vector<track_t>& TrackVector)
{
   int Npoints = fPointsArray.size();
   if( Npoints<=0 )
      return -1;
   if( debug )
      std::cout<<"KDTreeFinder::KDTreeFinder() # of points: "<<Npoints<<std::endl;

   // Pattern Recognition algorithm
   for(int i=0; i<Npoints; ++i)
      {
         TSpacePoint* point=fPointsArray[i];
         //  This point is already in this cluster
         if (!point) continue;
         /*
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
            }*/

         // do not start a track far from the anode
         if( point->GetR() < fSeedRadCut && TrackVector.size() > 0 ) break;

         track_t vector_points;
         vector_points.clear();

         PointCluster(i, fPointsDistCut,vector_points);
         if (!vector_points.empty())
         {
            fClusterID++;
            std::cout <<"\t" << fClusterID <<": " << vector_points.size() <<"\n";
         }
         if( int(vector_points.size()) < fNpointsCut )
            {
               ++points_cut;
               continue;
            }
         else
            {
               vector_points.push_front(i);

               TrackVector.push_back( vector_points );
               ++fNtracks;
            }
      }//i loop

   if( fNtracks != int(TrackVector.size()) )
      std::cerr<<"KDTreeFinder::KDTreeFinder(): Number of found tracks "<<fNtracks
               <<" does not match the number of entries "<<TrackVector.size()<<std::endl;
   else if( debug )
      {
         std::cout<<"KDTreeFinder::KDTreeFinder(): Number of found tracks "<<fNtracks<<std::endl;
         std::cout<<"KDTreeFinder::KDTreeFinder() -- Reasons: Track Not Advancing "<<track_not_advancing
                  <<" Points Cut: ("<<fNpointsCut<<"): "<<points_cut
                  <<" Radius Cut ("<<fLastPointRadCut<<" mm): "<<rad_cut<<std::endl;
      }

   return fNtracks;
}
#include <array>
void KDTreeFinder::PointCluster(const int index, double distcut, track_t& atrack) 
{
   if (fPointsArray.size() == (size_t) index + 1 )
      return;
   TSpacePoint* thisPoint = fPointsArray[index];
   if (!thisPoint)
      return;
   if (fCluster[index][fClusterID]) return;
   
   fCluster[index].set(fClusterID);
   atrack.push_back(index);
   std::array<double,3> xyz = { thisPoint->GetX(),thisPoint->GetY(),thisPoint->GetZ()};
   std::cout <<thisPoint->GetX() << "\t"<< thisPoint->GetY() << "\t"<< thisPoint->GetZ() <<"\n";
   std::vector<int> index_in_range;
   fPoints->FindInRange (xyz.begin(), distcut, index_in_range);
   std::cout <<"Points in range: "<< index_in_range.size() <<"\n";
   for (const int& i: index_in_range)
   {
       //std::cout << "Getting entry " << i <<std::endl;
       const TSpacePoint* NearPoint = fPointsArray[i];
       if (!NearPoint)
          continue;
       //if (NearPoint->GetR() > fLastPointRadCut)
       //   continue;

       // Force ordering of hits?
       //if (NearPoint->GetR() < NextPoint->GetR())
       //   continue;

       if (NearPoint->GetR() <= fSmallRad )
          continue;

       /*if (fCluster[i].count() > 4 )
       {
           
           atrack.clear();
           for (int j = 0; j < fCluster.size() ; j++)
              fCluster[i].reset(fClusterID);
           return;
       }*/
       //fCluster[i].set(fClusterID);
       //atrack.push_back(i);
       PointCluster( i, distcut, atrack);
   }

}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
