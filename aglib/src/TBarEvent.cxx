#ifdef BUILD_AG
#include "TBarEvent.hh"




ClassImp(TBarEvent)

TBarEvent::TBarEvent()
{
// ctor
}

TBarEvent::TBarEvent(TBarEvent &barEvt)
{
// copy ctor
  fEventID = barEvt.GetID();
  fEventTime = barEvt.GetRunTime();
  for(TBarHit* barhit: barEvt.GetBars())  {
    if(barhit)
    {
      TBarHit* h = new TBarHit(*(barhit));
      fBarHit.push_back(h);
    }
  }
  for(TBarEndHit* endhit: barEvt.GetEndHits()) {
    if(endhit)
    {
      TBarEndHit* h = new TBarEndHit(*(endhit));
      fEndHit.push_back(h);
    }
  }
  for(TBarSimpleTdcHit* tdchit: barEvt.GetTdcHits()) {
    if(tdchit)
    {
      TBarSimpleTdcHit* h = new TBarSimpleTdcHit(*(tdchit));
      fTdcHit.push_back(h);
    }
  }
}


TBarEvent::~TBarEvent()
{
   Reset();
}

void TBarEvent::Print()
{
   std::cout << "TBarEvent::Print() -------------"<<std::endl;
   std::cout <<"EventID:\t"<< fEventID      <<std::endl;
   std::cout <<"RunTime:\t"  << fEventTime    <<std::endl;
   std::cout <<"End Hits:\t"     << fEndHit.size()<<std::endl;
   std::cout <<"Bar Hits:\t"     << fBarHit.size()<<std::endl;
   for (size_t i=0; i<fEndHit.size(); i++)
      fEndHit.at(i)->Print();
   for (size_t i=0; i<fBarHit.size(); i++)
      fBarHit.at(i)->Print();
   std::cout << "End of TBarEvent::Print() ------"<<std::endl;
}

#endif
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */