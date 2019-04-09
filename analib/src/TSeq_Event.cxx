#ifndef _TSeq_Event_
#include "TSeq_Event.h"
#endif

ClassImp(TSeq_Event)

TSeq_Event::TSeq_Event()
{
// ctor
  fSeqNum = 0;
  fID = 0;
  fonCount = 0;
  fonState = 0;

}

TSeq_Event::TSeq_Event(TSeq_Event* Event) // <-- this is not a copy constructor  -- AC 2018
{
//copy constructor
  fSeq=Event->GetSeq();
  fSeqNum=Event->GetSeqNum();
  fID=Event->GetID();
  fEventName=Event->GetEventName();
  fDescription=Event->GetDescription();
  fSeqHeader=Event->GetSeqHeader();
  fonCount=Event->GetonCount();
  fonState=Event->getonState();  
}

void TSeq_Event::Reset()
{
  fSeq="";
  fSeqNum=0;
  fID=0;
  fEventName="";
  fDescription="";
  fSeqHeader="";
  fonCount=0;
  fonState=0; 
}

void TSeq_Event::Print()
{
  std::cout<<"Seq:\t"<<fSeq<<std::endl;
  std::cout<<"num:\t"<<fSeqNum<<std::endl;
  std::cout<<"ID:\t"<<fID<<std::endl;
  std::cout<<"Name:\t"<<fEventName<<std::endl;
  std::cout<<"Description:\t"<<fDescription<<std::endl;
  //std::cout<<"Header:\t"<<fSeqHeader<<std::endl;
  std::cout<<"onCounts:\t"<<fonCount<<std::endl;
  std::cout<<"onState:\t"<<fonState<<std::endl;
}

TSeq_Event::~TSeq_Event()
{


}

//



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
