#ifndef _Chrono_Event_
#include "TChrono_Event.h"
#endif


std::ostream& operator<<(std::ostream& o, ChronoChannel& c)
{
   return o << "Board: " << c.Board << "\tChannel: " << c.Channel; 
}
bool operator==(const ChronoChannel & lhs,const ChronoChannel & rhs) {
    return ((lhs.Channel == rhs.Channel) && (lhs.Board == rhs.Board));
}

ClassImp(TChrono_Event)

TChrono_Event::TChrono_Event()
{
// ctor
   fChannel.Board=-1.;
   fChannel.Channel=-1.;
   fID=-1.;
   fCounts = 0;
   ts=0.;
   local_ts=0.;
   runtime=-1.;
}

void TChrono_Event::Reset()
{
   fChannel.Board=-1.;
   fChannel.Channel=-1.;
   fID=-1.;
   fCounts = 0;
   ts=0.;
   local_ts=0.;
   runtime=-1.;
}

void TChrono_Event::Print()
{
   std::cout<<"Board Index:\t"<<fChannel.Board<<std::endl;
   std::cout<<"Event ID:\t"<<fID<<std::endl;
   std::cout<<"Channel:\t"<<fChannel.Channel<<std::endl;
   std::cout<<"Counts:\t" << fCounts <<std::endl;
   std::cout<<"TimeStamp:\t"<<ts<<std::endl;;
   std::cout<<"RunTime:\t"<<runtime<<std::endl;
}

TChrono_Event::~TChrono_Event()
{


}

//



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
