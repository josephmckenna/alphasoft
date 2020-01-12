#ifndef _TSeq_State_
#define _TSeq_State_

#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#include <vector>
#include <map>

class AnalogueOut;
class DigitalOut;

class TriggerIn: public TObject
{
  public:
   int InfWait;
   double waitTime;
   std::vector<int> Channels;
   TriggerIn(){}
   ~TriggerIn(){}
   TriggerIn(TriggerIn* t): InfWait(t->InfWait), waitTime(t->waitTime), Channels(t->Channels) {}
   ClassDef(TriggerIn, 1);
};

class DigitalOut: public TObject
{
  public:
    //I expect only bools, but lets get greedy and be ready for fuzzy logic
    std::vector<bool> Channels;
    DigitalOut(){}
    ~DigitalOut(){}
    DigitalOut(DigitalOut* d): Channels(d->Channels) {}
    
    ClassDef(DigitalOut, 1);

};
class AnalogueOut: public TObject
{
  public:
    int PrevState;
    int steps;
    std::vector<double> AOi;
    std::vector<double> AOf;
    AnalogueOut(){}
    ~AnalogueOut(){}
    AnalogueOut(AnalogueOut* a): PrevState(a->PrevState), steps(a->steps), AOi(a->AOi), AOf(a->AOf) {}
    ClassDef(AnalogueOut, 1);
};


class TSequencerState : public TObject
{
  private:
    Int_t fID;
    TString fSeq;
    Int_t fSeqNum;
    Int_t fState;
    double fTime;
    DigitalOut* fDO;
    AnalogueOut* fAO;
    TriggerIn* fTI;
    TString fComment;
 
  public:
    TSequencerState(TSequencerState* Event);
    TSequencerState();
    using TObject::Print;
    virtual void Print();
    virtual ~TSequencerState();
    TString Clean(TString a) { 
      TString b(a);
      b.ReplaceAll("\r","\n");//Fix windows' stupid miss use of return carriadge 
      return b;
    }
    TString GetSeq()		{ return fSeq; }
    Int_t GetSeqNum()		{ return fSeqNum; }
    Int_t GetID()		{ return fID; }
    TString GetComment()	{ 
      return Clean(fComment);
    }
    Int_t GetState()		{ return fState; }
    Double_t GetDuration() { return fTime; }
    DigitalOut* GetDigitalOut() { return fDO; }
    AnalogueOut* GetAnalogueOut() { return fAO; }
    TriggerIn* GetTriggerIn() { return fTI; }
    
    void SetSeq( TString Seq )		{ fSeq = Seq; }
    void SetSeqNum( Int_t SeqNum )	{ fSeqNum = SeqNum; }
    void SetID( Int_t ID )	{ fID = ID; }
    void SetTime( double Time ) { fTime = Time; }
    void SetState( Int_t state )		{ fState = state; }
    void SetComment( TString comment)     { fComment=comment; }
    void AddAO( AnalogueOut* AO ) { fAO = AO; }
    void AddDO( DigitalOut* DO ) { fDO = DO; }
  
    ClassDef(TSequencerState, 1);
};

#endif
