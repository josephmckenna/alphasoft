// Tracks finder class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#ifndef __NFINDER__
#define __NFINDER__ 1
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include <vector>
#include <list>
#include <set>

#include "TClonesArray.h"

class NeuralFinder: public TracksFinder
{
private:

public:
   NeuralFinder(TClonesArray*);
   ~NeuralFinder(){};

   virtual int RecTracks();
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
