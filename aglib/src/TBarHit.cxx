
#include "TBarHit.h"
ClassImp(TBarHit)

TBarHit::TBarHit():
   fTopHit(),
   fBotHit()
{
  fBarID=-1;
  fTPCMatched=false;
  fTPCMatched=false;
  fTPC = TVector3(); //Empty vector
  fZed=-9999;
// ctor

}

TBarHit::TBarHit(const TBarHit &h):
   fTopHit(h.fTopHit),
   fBotHit(h.fBotHit)
{
// copy ctor
  fBarID=h.GetBar();
  fTPCMatched=h.IsTPCMatched();
  fTPC=h.GetTPC();
  fZed=h.GetTDCZed();
}

void TBarHit::Print()
{
   std::cout<<"BarHit::Print() -- Bar ID:"<<fBarID<<std::endl;
   std::cout<<"Top TDC time: "<<fTopHit.GetTDCTime()<<std::endl;
   std::cout<<"Bottom TDC time: "<<fBotHit.GetTDCTime()<<std::endl;
   std::cout<<"Top Amplitude: "<<fTopHit.GetAmp()<<std::endl;
   std::cout<<"Bottom Amplitude: "<<fBotHit.GetAmp()<<std::endl;
   std::cout<<"Time Diff: "<<fTopHit.GetTDCTime() - fBotHit.GetTDCTime()<<std::endl;
  std::cout<<"TPC Matched? "<<fTPCMatched;
  if (fTPCMatched) {std::cout<<" TPC hit: "<<std::endl; fTPC.Print();}
  else std::cout<<std::endl;
}


TBarHit::~TBarHit()
{


}
