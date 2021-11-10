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

#include "Sequencer2.h"

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
   TriggerIn(const TriggerIn& t): TObject(), InfWait(t.InfWait), waitTime(t.waitTime), Channels(t.Channels) {}
   void Reset()
   {
      InfWait = 0;
      waitTime = 0.0;
      Channels.clear();
   }
   ClassDef(TriggerIn, 1);
};

class DigitalOut: public TObject
{
  public:
    //I expect only bools, but lets get greedy and be ready for fuzzy logic
    std::vector<bool> Channels;
    DigitalOut(){}
    ~DigitalOut(){}
    DigitalOut(const DigitalOut& d): TObject(), Channels(d.Channels) {}
    void Reset()
    {
       Channels.clear();
    }
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
    AnalogueOut(const AnalogueOut& a): TObject(), PrevState(a.PrevState), steps(a.steps), AOi(a.AOi), AOf(a.AOf) {}
    void Reset()
    {
       PrevState = 0;
       steps = 0;
       AOi.clear();
       AOf.clear();
    }
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
    DigitalOut fDO;
    AnalogueOut fAO;
    TriggerIn fTI;
    TString fComment;
 
  public:
    TSequencerState(const TSequencerState& Event);
    TSequencerState();
    using TObject::Print;
    virtual void Print();
    int NsyncsSet(std::map<TString,int>);
    void PrintNsyncsSet();    
    std::map<TString,int> syncs_Nsyncsset;
    virtual ~TSequencerState();
    TString Clean(TString a) const { 
      TString b(a);
      b.ReplaceAll("\r","\n");//Fix windows' stupid miss use of return carriadge 
      return b;
    }
    TString GetSeq() const                    { return fSeq; }
    Int_t GetSeqNum() const                   { return fSeqNum; }
    Int_t GetID() const                       { return fID; }
    TString GetComment() const                { return Clean(fComment); }
    Int_t GetState() const                    { return fState; }
    Double_t GetDuration() const              { return fTime; }
    DigitalOut* GetDigitalOut()               { return &fDO; }
    AnalogueOut* GetAnalogueOut()             { return &fAO; }
    TriggerIn* GetTriggerIn()                 { return &fTI; }
    const DigitalOut* GetDigitalOut() const   { return &fDO; }
    const AnalogueOut* GetAnalogueOut() const { return &fAO; }
    const TriggerIn* GetTriggerIn() const     { return &fTI; }
    
    void SetSeq( TString Seq )		{ fSeq = Seq; }
    void SetSeqNum( Int_t SeqNum )	{ fSeqNum = SeqNum; }
    void SetID( Int_t ID )	{ fID = ID; }
    void SetTime( double Time ) { fTime = Time; }
    void SetState( Int_t state )		{ fState = state; }
    void SetComment( TString comment)     { fComment=comment; }
    void Set(SeqXML_State* s)
    {
       this->SetState( s->getID() );
       this->SetTime( s->getTime() );      
       
       //AnalogueOut
       fAO.steps = s->getLoopCnt();
       for (const double& i: *s->GetAOi())
          fAO.AOi.push_back(i);
       //fAO.AOi(*s->GetAOi());
       for (const double& f: *s->GetAOf())
          fAO.AOf.push_back(f);
       
       //fAO.AOf(*s->GetAOf());
       fAO.PrevState=-999;
       
       //DigitalOut
       for (const bool& d: *s->GetDO())
          fDO.Channels.push_back(d);
//       fDO.Channels(*s->GetDO());
    }

    void Reset()
    {
       fID = -1;
       fSeq = "";
       fSeqNum = -1;
       fState = -1;
       fTime = 0.0;
       fDO.Reset();
       fAO.Reset();
       fTI.Reset();
       fComment = "";
    }
  
    ClassDef(TSequencerState, 1);
};

#endif
