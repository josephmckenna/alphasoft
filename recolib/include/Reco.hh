#ifndef __RECO__
#define __RECO__ 1

#include <vector>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TClonesArray.h>

#include "SignalsType.hh"
#include "AnaSettings.hh"

#include "LookUpTable.hh"
#include "TracksFinder.hh"
#include "TFitVertex.hh"
#include "TFitLine.hh"

#include "TRecoHelixFitter.hh"
#include "TRecoLineFitter.hh"
#include "TRecoTrackFinder.hh"
#include "TRecoVertexFitter.hh"
#include "TTrackBuilder.hh"
#include "TSpacePointBuilder.hh"

class Reco: public TRecoHelixFitter, public TRecoLineFitter, public TRecoTrackFinder, public TRecoVertexFitter, public TTrackBuilder, public TSpacePointBuilder
{
private:

public:
   Reco(std::string, double);
   Reco(AnaSettings*, double, std::string ,bool);
   ~Reco();
   void UseSTRfromData(int runNumber);
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
