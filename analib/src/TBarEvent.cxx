#include "TBarEvent.hh"




ClassImp(TBarEvent)

TBarEvent::TBarEvent()
{
// ctor
}



TBarEvent::~TBarEvent()
{
// dtor
}

void TBarEvent::Print()
{
   std::cout <<"TBarEvent:\t"<< fEventID      <<std::endl;
   std::cout <<"RunTime:\t"  << fEventTime    <<std::endl;
   std::cout <<"Hits:\t"     << fBarHit.size()<<std::endl;
}

ClassImp(BarHit)

BarHit::BarHit()
{
// ctor
}



BarHit::~BarHit()
{


}
