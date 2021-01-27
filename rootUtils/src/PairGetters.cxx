#include "PairGetters.h"

std::pair<Int_t,Int_t> GetChronoBoardChannel(Int_t runNumber, const char* ChannelName)
{
   Int_t chan=-1;
   Int_t board=-1;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
   return {board,chan};
}
