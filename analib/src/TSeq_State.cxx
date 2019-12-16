#ifndef _TSeq_State_
#include "TSeq_State.h"
#endif


ClassImp(TSeq_State)



TSeq_State::TSeq_State()
{
// ctor
  fSeqNum = 0;
  fID = 0;
  fState = 0;
  fTime = 0.;
  fDO=NULL;
  fAO=NULL;
  fTI=NULL;
}

TSeq_State::TSeq_State(TSeq_State* State)
{
//copy constructor
  fSeq=State->GetSeq();
  fSeqNum=State->GetSeqNum();
  fID=State->GetID();
  fState=State->GetState();
  fTime=State->GetDuration();
  fDO=State->GetDigitalOut();
  fAO=State->GetAnalogueOut();
  fTI=State->GetTriggerIn();
  fComment=State->GetComment();
}

void TSeq_State::Print()
{
  std::cout<<"Seq:\t"<<fSeq<<std::endl;
  std::cout<<"num:\t"<<fSeqNum<<std::endl;
  std::cout<<"ID:\t"<<fID<<std::endl;
  std::cout<<"Duration:\t"<<fTime<<std::endl;
  if (fDO)
  {
    std::cout<<"Digital Out:\t";
    for (size_t i=0; i<fDO->Channels.size(); i++)
      std::cout<<fDO->Channels[i]<<" ";
    std::cout<<std::endl;
  }
  if (fAO)
  {
    std::cout<<"Previous AO state:\t"<<fAO->PrevState<<std::endl;
    std::cout<<"Number of steps:\t"<<fAO->steps<<std::endl;
    std::cout<<"Analogue Out Initial:\t";
    for (size_t i=0; i<fAO->AOi.size(); i++)
      std::cout<<fAO->AOi[i]<<" ";
    std::cout<<std::endl;
    std::cout<<"Analogue Out Final:\t";
    for (size_t i=0; i<fAO->AOf.size(); i++)
      std::cout<<fAO->AOf[i]<<" ";
    std::cout<<std::endl;
  }
  if (fTI)
  {
    std::cout<<"Trigger In Wait t"<<fTI->waitTime<<std::endl;
    std::cout<<"Trigger In InfoWait"<<fTI->InfWait<<std::endl;
    std::cout<<"Trigger In:\t";
    for (size_t i=0; i<fTI->Channels.size(); i++)
      std::cout<<fTI->Channels[i]<<" ";
    std::cout<<std::endl;
  }
  std::cout<<"Comment:"<<GetComment()<<std::endl;
}

TSeq_State::~TSeq_State()
{
  if (fDO) delete fDO;
  if (fAO) delete fAO;
  if (fTI) delete fTI;
}

//


