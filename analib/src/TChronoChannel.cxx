#include "TChronoChannel.h"

TChronoChannel::TChronoChannel()
{
   fBoardName = "";
   fChannel   = -1;
}

TChronoChannel::TChronoChannel(const std::string &Board, const int Channel):
   fBoardName(Board), fChannel(Channel)
{

}
TChronoChannel::TChronoChannel(const TChronoChannel &c) : 
   fBoardName(c.fBoardName), fChannel(c.fChannel)
{
    
}

std::ostream &operator<<(std::ostream &o, const TChronoChannel &c)
{
   return o << "Board: " << c.GetBoard() << "\tChannel: " << c.GetChannel();
}

bool operator==(const TChronoChannel &lhs, const TChronoChannel &rhs)
{
   return ((lhs.GetChannel() == rhs.GetChannel()) && (lhs.GetBoard() == rhs.GetBoard()));
}