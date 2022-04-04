#include "TBarSimpleTdcHit.h"

ClassImp(TBarSimpleTdcHit)

TBarSimpleTdcHit::TBarSimpleTdcHit()
{
// ctor
}

TBarSimpleTdcHit::TBarSimpleTdcHit(const TBarSimpleTdcHit &h)
{
// copy ctor
  fBarID=h.GetBar();
  fTime=h.GetTime();
  fFineTimeCount=h.GetFineTimeCount();
  fFineTime=h.GetFineTime();
}

void TBarSimpleTdcHit::Print()
{
  std::cout<<"SimpleTdcHit::Print() -- Bar ID:"<<fBarID<<std::endl;
  std::cout<<" TDC time: "<<fTime<<std::endl;
  std::cout<<" Fine time counter: "<<fFineTimeCount<<" Fine time: "<<fFineTime;
  std::cout<<std::endl;
}

TBarSimpleTdcHit::~TBarSimpleTdcHit()
{
// dtor
}
