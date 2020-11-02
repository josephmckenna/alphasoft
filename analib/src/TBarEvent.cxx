#ifdef BUILD_AG
#include "TBarEvent.hh"




ClassImp(TBarEvent)

TBarEvent::TBarEvent()
{
// ctor
}



TBarEvent::~TBarEvent()
{
   // dtor
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

ClassImp(EndHit)

EndHit::EndHit()
{
// ctor
}

void EndHit::Print()
{
  std::cout<<"EndHit::Print() -- Bar ID:"<<fBarID<<std::endl;
  std::cout<<"ADC time: "<<fADCTime<<" Amplitude: "<<fAmp<<" Integral: "<<fIntegral<<std::endl;
  std::cout<<"TDC Matched? "<<fTDCMatched;
  if (fTDCMatched) std::cout<<" TDC time: "<<fTDCTime;
  std::cout<<std::endl;
}

EndHit::~EndHit()
{
// dtor
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
   double zed = ((speed/cFactor) * double(timeDiff))*0.5; //in meter
   return zed;
}

void BarHit::Print()
{
   std::cout<<"BarHit::Print() -- Bar ID:"<<fBarID<<std::endl;
   if (fBotHit)
      {
         std::cout<<"Bot hit: vvv"<<std::endl;
         fBotHit->Print();
      }
   if (fTopHit)
      {
         std::cout<<"Top hit: vvv"<<std::endl;
         fTopHit->Print();
      }
   std::cout<<"Time Diff:"<<fTopHit->GetTDCTime()-fBotHit->GetTDCTime()<<"\tZedTDC:"<<fZedTDC<<"\tZedADC:"<<fZedADC<<std::endl;
  std::cout<<"TPC Matched? "<<fTPCMatched;
  if (fTPCMatched) {std::cout<<" TPC hit: "<<std::endl; fTPC.Print();}
  else std::cout<<std::endl;
}


BarHit::~BarHit()
{


}
#endif
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
