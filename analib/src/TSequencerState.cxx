#ifndef _TSequencerState_
#include "TSequencerState.h"
#endif


ClassImp(TSequencerState)



TSequencerState::TSequencerState()
{
// ctor
  fID = 0;
  fSeqNum = 0;
  fState = 0;
  fTime = 0.;
}

TSequencerState::TSequencerState(const TSequencerState& State):
   TObject(),
   fID(State.GetID()),
   fSeq(State.GetSeq()),
   fSeqNum(State.GetSeqNum()),
   fState(State.GetState()),
   fTime(State.GetDuration()),
   fDO(*State.GetDigitalOut()),
   fAO(*State.GetAnalogueOut()),
   fTI(*State.GetTriggerIn()),
   fComment(State.GetComment())
{
//copy constructor
  
}

void TSequencerState::Print()
{
  std::cout<<"Seq:\t"<<fSeq<<std::endl;
  std::cout<<"num:\t"<<fSeqNum<<std::endl;
  std::cout<<"ID:\t"<<fID<<std::endl;
  std::cout<<"state:\t"<<fState<<std::endl;
  std::cout<<"Duration:\t"<<fTime<<std::endl;
  std::cout<<"Digital Out:\t";
  if (fDO.Channels.size())
  {
     for (size_t i=0; i<fDO.Channels.size(); i++)
        std::cout<<fDO.Channels[i]<<" ";
     std::cout<<std::endl;
  }
  if (fAO.AOi.size() || fAO.AOf.size())
  {
    std::cout<<"Previous AO state:\t"<<fAO.PrevState<<std::endl;
    std::cout<<"Number of steps:\t"<<fAO.steps<<std::endl;
    std::cout<<"Analogue Out Initial:\t";
    for (size_t i=0; i<fAO.AOi.size(); i++)
      std::cout<<fAO.AOi[i]<<" ";
    std::cout<<std::endl;
    std::cout<<"Analogue Out Final:\t";
    for (size_t i=0; i<fAO.AOf.size(); i++)
      std::cout<<fAO.AOf[i]<<" ";
    std::cout<<std::endl;
  }
  if (fTI.Channels.size())
  {
    std::cout<<"Trigger In Wait t"<<fTI.waitTime<<std::endl;
    std::cout<<"Trigger In InfoWait"<<fTI.InfWait<<std::endl;
    std::cout<<"Trigger In:\t";
    for (size_t i=0; i<fTI.Channels.size(); i++)
      std::cout<<fTI.Channels[i]<<" ";
    std::cout<<std::endl;
  }
  std::cout<<"Comment:"<<GetComment()<<std::endl;
}

TSequencerState::~TSequencerState()
{

}

//


