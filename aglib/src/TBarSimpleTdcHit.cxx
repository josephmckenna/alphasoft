#include "TBarSimpleTdcHit.h"

ClassImp(TBarSimpleTdcHit)

TBarSimpleTdcHit::TBarSimpleTdcHit()
{
// ctor
}

void TBarSimpleTdcHit::Print()
{
  std::cout<<"SimpleTdcHit::Print() -- Bar ID:"<<fBarID<<std::endl;
  std::cout<<" TDC time: "<<fTime;
  std::cout<<std::endl;
}

TBarSimpleTdcHit::~TBarSimpleTdcHit()
{
// dtor
}
