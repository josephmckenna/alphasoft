#ifndef _Chrono_Event_
#include "TChrono_Event.h"
#endif

ClassImp(TChrono_Event)

TChrono_Event::TChrono_Event()
{
// ctor
  fChronoBoxIndex=-1.;
  fChronoBoardIndex=-1.;
  fID=-1.;
  fChannel = -1;
  fCounts = 0;
  ts=0.;
  local_ts=0.;
  runtime=-1.;
}

void TChrono_Event::Reset()
{
  fChronoBoxIndex=-1.;
  fChronoBoardIndex=-1.;
  fID=-1.;
  fChannel = -1;
  fCounts = 0;
  ts=0.;
  local_ts=0.;
  runtime=-1.;
}

void TChrono_Event::Print()
{

  std::cout<<"Box Index:\t"<<fChronoBoxIndex<<std::endl;;
  std::cout<<"Board Index:\t"<<fChronoBoardIndex<<std::endl;
  std::cout<<"Event ID:\t"<<fID<<std::endl;
  std::cout<<"Channel:\t"<<fChannel<<std::endl;
  std::cout<<"Counts:\t" << fCounts <<std::endl;
  std::cout<<"TimeStamp:\t"<<ts<<std::endl;;
  std::cout<<"RunTime:\t"<<runtime<<std::endl;
}

TChrono_Event::~TChrono_Event()
{


}

//


