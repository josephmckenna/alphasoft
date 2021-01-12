#include "BoolGetters.h"


#ifdef BUILD_AG
Bool_t ChronoboxesHaveChannel(Int_t runNumber, const char* Name)
{
   for (int boards=0; boards<CHRONO_N_BOARDS; boards++)
   {
      for (int chans=0; chans<CHRONO_N_CHANNELS; chans++)
      {
         if (Get_Chrono_Channel(runNumber,boards,Name)>-1)
         {
            return kTRUE;
         }
      }
   }
   return kFALSE;
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
