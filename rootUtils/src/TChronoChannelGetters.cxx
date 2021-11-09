#include "TChronoChannelGetters.h"
#include "IntGetters.h"

#ifdef BUILD_AG

TChronoChannel Get_Chrono_Channel(Int_t runNumber, const char* ChannelName, Bool_t ExactMatch)
{
   for (const std::pair<std::string,int>& board : TChronoChannel::CBMAP)
   {
      int chan = Get_Chrono_Channel_In_Board(runNumber, board.first, ChannelName, ExactMatch);
      if (chan >= 0)
      {
         TChronoChannel c(board.first,chan);
         return c;
      }
   }
   return {"", -1};
}

#endif 
