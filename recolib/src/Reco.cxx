#include "Reco.hh"


#ifdef BUILD_AG_SIM
#include "TMChit.hh"
#endif

#include <TDirectory.h>


Reco::Reco(std::string json, double B): 
   Reco(
      new AnaSettings(json.c_str()),
      B,
      "CERN",
      false)
{

}

Reco::Reco(AnaSettings* ana_set, double B, std::string loc, bool trace):
   TRecoHelixFitter(ana_set,trace),
   TRecoLineFitter(ana_set,trace),
   TRecoTrackFinder(ana_set,trace),
   TRecoVertexFitter(ana_set->GetDouble("RecoModule","VtxChi2Cut"), trace),
   TTrackBuilder(B,loc,trace),
   TSpacePointBuilder(ana_set,B,loc,trace)
{

}

Reco::~Reco()
{

}

void Reco::UseSTRfromData(int runNumber)
{
   TSpacePointBuilder::UseSTRfromData(runNumber);
   TTrackBuilder::UseSTRfromData(runNumber);
}



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
