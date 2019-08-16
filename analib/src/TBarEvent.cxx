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
   for (size_t i=0; i<fBarHit.size(); i++)
      fBarHit.at(i).Print();
}

ClassImp(BarHit)

BarHit::BarHit()
{
// ctor

}

double BarHit::CalculateZed( double _TimeTop, double _TimeBot )
{
   double timeDiff=_TimeBot-_TimeTop;
   double speed=TMath::C();
   double cFactor=1.58;
   return ((speed/cFactor) * double(timeDiff))*0.5; //in meter
}

void BarHit::Print()
{
   std::cout<<"Bar ID:"<<fBarID<<std::endl;
   std::cout<<"t top: "<<fTimeTop<<"\tt bot: "<<fTimeBot<<"\tDiff:"<<fTimeTop-fTimeBot<<"\tZedTDC:"<<fZedTDC<<std::endl;
   std::cout<<"ADCtop:"<<fAmpTop<<"\tADCbot:"<<fAmpBot<<"\t\tZedADC:"<<fZedADC<<std::endl;
}


BarHit::~BarHit()
{


}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
