// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "NeuralFinder.hh"

#include <iostream>

NeuralFinder::NeuralFinder(TClonesArray* points):
   TracksFinder(points)
{
   // // No inherent reason why these parameters should be the same as in base class
   // fSeedRadCut = 150.;
   // fPointsDistCut = 8.1;
   // fSmallRad = _cathradius;
   // fNpointsCut = 7;
   // //  std::cout<<"NeuralFinder::NeuralFinder"<<std::endl;
}

//==============================================================================================
int NeuralFinder::RecTracks()
{
   int Npoints = fPointsArray.size();
   return fNtracks;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
