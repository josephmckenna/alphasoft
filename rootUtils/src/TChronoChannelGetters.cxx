#include "TChronoChannelGetters.h"
#include "IntGetters.h"

#ifdef BUILD_AG

TChronoChannel Get_Chrono_Channel(Int_t runNumber, const char* ChannelName, Bool_t ExactMatch)
{
   for (int board=0; board<CHRONO_N_BOARDS; board++)
   {
      int chan=Get_Chrono_Channel_In_Board(runNumber, board, ChannelName, ExactMatch);
      if (chan>0)
      {
         TChronoChannel c(board,chan);
         return c;
      }
   }
   return {-1, -1};
}

#endif 