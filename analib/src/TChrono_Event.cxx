#ifndef _Chrono_Event_
#include "TChrono_Event.h"
#endif


std::ostream& operator<<(std::ostream& o, const TChronoChannel& c)
{
   return o << "Board: " << c.GetBoard() << "\tChannel: " << c.GetChannel(); 
}
bool operator==(const TChronoChannel & lhs,const TChronoChannel & rhs) {
    return ((lhs.GetChannel() == rhs.GetChannel()) && (lhs.GetBoard() == rhs.GetBoard()));
}

ClassImp(TChrono_Event)

TChrono_Event::TChrono_Event(): TChronoChannel()
{
// ctor
   fID=-1.;
   fCounts = 0;
   ts=0.;
   local_ts=0.;
   runtime=-1.;
}

void TChrono_Event::Reset()
{
   SetBoard(-1.);
   SetChannel(-1.);
   fID=-1.;
   fCounts = 0;
   ts=0.;
   local_ts=0.;
   runtime=-1.;
}

void TChrono_Event::Print()
{
   std::cout<<"Board Index:\t"<<GetBoard()<<std::endl;
   std::cout<<"Event ID:\t"<<fID<<std::endl;
   std::cout<<"Channel:\t"<<GetChannel()<<std::endl;
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
