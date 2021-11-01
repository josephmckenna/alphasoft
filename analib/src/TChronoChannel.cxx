#include "TChronoChannel.h"

std::ostream& operator<<(std::ostream& o, const TChronoChannel& c)
{
   return o << "Board: " << c.GetBoard() << "\tChannel: " << c.GetChannel(); 
}

bool operator==(const TChronoChannel & lhs,const TChronoChannel & rhs) {
    return ((lhs.GetChannel() == rhs.GetChannel()) && (lhs.GetBoard() == rhs.GetBoard()));
}