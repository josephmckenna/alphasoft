#include "TBarEndHit.h"
ClassImp(TBarEndHit)

TBarEndHit::TBarEndHit()
{
// ctor
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