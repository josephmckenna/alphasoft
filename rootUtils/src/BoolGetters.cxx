#include "BoolGetters.h"


#ifdef BUILD_AG
Bool_t ChronoboxesHaveChannel(Int_t runNumber, const char* Name)
{
   TChronoChannel c = Get_Chrono_Channel(runNumber,Name);
   return c.IsValidChannel();
}
#endif

Bool_t IsPathExist(const TString &s)
{
  struct stat buffer;
  return stat(s.Data(), &buffer)==0;
}



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
