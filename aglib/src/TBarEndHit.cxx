#include "TBarEndHit.h"
ClassImp(TBarEndHit)

TBarEndHit::TBarEndHit()
{
   fBarID=-1;
   fTDCTime=-1; 
   fADCTime=-1;
   fAmp=-1;
   fAmpRaw=-1; 
   fTDCMatched=false;
}

void TBarEndHit::Print()
{
  std::cout<<"EndHit::Print() -- Bar ID:"<<fBarID<<std::endl;
  std::cout<<"ADC time: "<<fADCTime<<" Amplitude: "<<fAmp<<std::endl;
  std::cout<<"TDC Matched? "<<fTDCMatched;
  if (fTDCMatched) std::cout<<" TDC time: "<<fTDCTime;
  std::cout<<std::endl;
}

TBarEndHit::~TBarEndHit()
{
// dtor
}