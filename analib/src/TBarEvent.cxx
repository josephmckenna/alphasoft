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
  for(BarHit* barhit: barEvt.GetBars())  {
    if(barhit)
    {
      BarHit* h = new BarHit(*(barhit));
      fBarHit.push_back(h);
    }
  }
  for(EndHit* endhit: barEvt.GetEndHits()) {
    if(endhit)
    {
      EndHit* h = new EndHit(*(endhit));
      fEndHit.push_back(h);
    }
  }
  for(SimpleTdcHit* tdchit: barEvt.GetTdcHits()) {
    if(tdchit)
    {
      SimpleTdcHit* h = new SimpleTdcHit(*(tdchit));
      fTdcHit.push_back(h);
    }
  }
}


TBarEvent::~TBarEvent()
{
   // dtor
   //   Reset();
   fEventID=-1;
   fEventTime=-1.;
   //Revisit this, potential source of leaks. Try this: for(int i) { if(fBarHit.at(i)) {only now delete it} }
   //The at(i) insures we don't go out of scope, and the if(barhit) ensures its not empty. I have tried one or the other and they both seg fault. So either both, or even the changes to constructors might have changed something.
  /*for(BarHit* barhit: fBarHit)  {
    if(barhit) {
      delete barhit;
    }
  }
  for(EndHit* endhit: fEndHit) {
    if(endhit) {
      delete endhit;
    }
  }
  for(SimpleTdcHit* tdchit: fTdcHit) {
    if(tdchit) {
      delete tdchit;
    }
  }*/

   fEndHit.clear();
   fBarHit.clear();
   fTdcHit.clear();
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
}

ClassImp(EndHit)

EndHit::EndHit()
{
  fBarID=-1;
  fTDCTime=-1;
  fADCTime=-1;
  fAmp=-1;
  fAmpRaw=-1;
  fTDCMatched=false;
}

EndHit::EndHit(const EndHit &h)
{
// copy ctor
  fBarID=h.GetBar();
  fTDCTime=h.GetTDCTime();
  fADCTime=h.GetADCTime();
  fAmp=h.GetAmp();
  fAmpRaw=h.GetAmpRaw();
  fTDCMatched=h.IsTDCMatched();
}

void EndHit::Print()
{
  std::cout<<"EndHit::Print() -- Bar ID:"<<fBarID<<std::endl;
  std::cout<<"ADC time: "<<fADCTime<<" Amplitude: "<<fAmp<<std::endl;
  std::cout<<"TDC Matched? "<<fTDCMatched;
  if (fTDCMatched) std::cout<<" TDC time: "<<fTDCTime;
  std::cout<<std::endl;
}

EndHit::~EndHit()
{
// dtor
}

ClassImp(SimpleTdcHit)

SimpleTdcHit::SimpleTdcHit()
{
// ctor
}

SimpleTdcHit::SimpleTdcHit(const SimpleTdcHit &h)
{
// copy ctor

  fBarID=h.GetBar();
  fTime=h.GetTime();
  fFineTimeCount=h.GetFineTimeCount();
  fFineTime=h.GetFineTime();
}

void SimpleTdcHit::Print()
{
  std::cout<<"SimpleTdcHit::Print() -- Bar ID:"<<fBarID<<std::endl;
  std::cout<<" TDC time: "<<fTime<<std::endl;
  std::cout<<" Fine time counter: "<<fFineTimeCount<<" Fine time: "<<fFineTime;
  std::cout<<std::endl;
}

SimpleTdcHit::~SimpleTdcHit()
{
// dtor
}

ClassImp(BarHit)

BarHit::BarHit()
{
  fBarID=-1;
  fTopTDCTime=-1; 
  fTopADCTime=-1;
  fTopAmp=-1;
  fBotTDCTime=-1;
  fBotADCTime=-1;
  fBotAmp=-1;
  fTPCMatched=false;
  fTPCMatched=false;
  fTPC = TVector3(); //Empty vector
  fZed=-9999;
// ctor

}

BarHit::BarHit(const BarHit &h)
{
// copy ctor
  fBarID=h.GetBar();
  fTopTDCTime=h.GetTDCTop();
  fTopADCTime=h.GetADCTop();
  fTopAmp=h.GetAmpTop();
  fBotTDCTime=h.GetTDCBot();
  fBotADCTime=h.GetADCBot();
  fBotAmp=h.GetAmpBot();
  fTPCMatched=h.IsTPCMatched();
  fTPC=h.GetTPC();
  fZed=h.GetTDCZed();
}

void BarHit::Print()
{
   std::cout<<"BarHit::Print() -- Bar ID:"<<fBarID<<std::endl;
   std::cout<<"Top TDC time: "<<fTopTDCTime<<std::endl;
   std::cout<<"Bottom TDC time: "<<fBotTDCTime<<std::endl;
   std::cout<<"Top Amplitude: "<<fTopAmp<<std::endl;
   std::cout<<"Bottom Amplitude: "<<fBotAmp<<std::endl;
   std::cout<<"Time Diff: "<<fTopTDCTime-fBotTDCTime<<std::endl;
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
