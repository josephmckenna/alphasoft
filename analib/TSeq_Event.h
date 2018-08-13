#ifndef _TSeq_Event_
#define _TSeq_Event_

#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TSeq_Event : public TObject
{
  private:
    TString fSeq;
    Int_t fSeqNum;
    Int_t fID;
    TString fEventName;
    TString fDescription;
    TString fSeqHeader;
    Int_t fonCount;
    Int_t fonState;
  
  public:
    TSeq_Event(TSeq_Event* Event);
    TSeq_Event();
    using TObject::Print;
    virtual void Print();
    virtual ~TSeq_Event();
    TString Clean(TString a) { 
      TString b(a);
      b.ReplaceAll("\r","\n");//Fix windows' stupid miss use of return carriadge 
      return b;
    }
    TString GetSeq()		{ return fSeq; }
    Int_t GetSeqNum()		{ return fSeqNum; }
    Int_t GetID()		{ return fID; }
    TString GetEventName()	{ return fEventName; }
    TString GetDescription()	{ 
      return Clean(fDescription);
    }
    TString GetSeqHeader()		{ return Clean(fSeqHeader); }
    Int_t GetonCount()		{ return fonCount; }
    Int_t getonState()		{ return fonState; }
    
    void SetSeq( TString Seq )		{ fSeq = Seq; }
    void SetSeqNum( Int_t SeqNum )	{ fSeqNum = SeqNum; }
    void SetID( Int_t ID )	{ fID = ID; }
    void SetEventName( TString EventName )	{ fEventName = EventName; }
    void SetDescription( TString Description )	{ fDescription = Description; }
    void SetSeqHeader( TString SeqHeader )	{ fSeqHeader = SeqHeader; } //New... I result in duplicated data for each dump... root files are compressed, do we want save this data elsewhere?
    void SetonCount( Int_t count )		{ fonCount = count; }
    void SetonState( Int_t state )		{ fonState = state; }

    void Reset();
  
    ClassDef(TSeq_Event, 1);
};

#endif
