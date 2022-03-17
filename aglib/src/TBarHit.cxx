
#include "TBarHit.h"
ClassImp(TBarHit)

TBarHit::TBarHit()
{
// ctor

}

void TBarHit::Print()
{
   std::cout<<"BarHit::Print() -- Bar ID:"<<fBarID<<std::endl;
   if (fBotHit.GetBar())
      {
         std::cout<<"Bot hit: vvv"<<std::endl;
         fBotHit.Print();
      }
   if (fTopHit.GetBar())
      {
         std::cout<<"Top hit: vvv"<<std::endl;
         fTopHit.Print();
      }
   std::cout<<"Time Diff:"<<fTopHit.GetTDCTime() - fBotHit.GetTDCTime()<<std::endl;
  std::cout<<"TPC Matched? "<<fTPCMatched;
  if (fTPCMatched) {std::cout<<" TPC hit: "<<std::endl; fTPC.Print();}
  else std::cout<<std::endl;
}


TBarHit::~TBarHit()
{


}