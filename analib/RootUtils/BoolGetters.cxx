#include "BoolGetters.h"



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
