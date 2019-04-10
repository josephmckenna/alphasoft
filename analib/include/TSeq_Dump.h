#ifndef _TSeq_Dump_
#define _TSeq_Dump_

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TSeq_Dump : public TObject
{
private:
  
  const static int fNDet = 10;

  TString fSeq;
  Int_t fSeqNum;
  Int_t fID;
  TString fEventName;
  TString fDescription;

  Int_t fStartonCount;
  Int_t fStoponCount;
  Int_t fStartIntCount;
  Int_t fStopIntCount;

  Double_t fStartonTime;
  Double_t fStoponTime;

  Int_t fdet_integral[fNDet];

  //  Int_t fdeg_integral;
  Int_t ftrap_integral;
  Int_t fdown_integral;
  Int_t fsi_integral;
  Int_t ft5_integral;

  Bool_t fIsDone;
  Bool_t fHasStarted;
  Int_t fStartOfSeq;

public:
  
  TSeq_Dump();
  virtual ~TSeq_Dump();
  
  TString GetSeq()		{ return fSeq; }
  Int_t GetSeqNum()		{ return fSeqNum; }
  Int_t GetID()		{ return fID; }
  TString GetEventName()	{ return fEventName; }
  TString GetDescription()	{ return fDescription; }

  Int_t GetNDet() {return fNDet;}
  Int_t GetDetIntegral(Int_t index);   

  Int_t GetStartonCount()    { return fStartonCount; }
  Int_t GetStoponCount()     { return fStoponCount; }
  Int_t GetStartIntCount()    { return fStartIntCount; }
  Int_t GetStopIntCount()     { return fStopIntCount; }
  Double_t GetStartonTime()    { return fStartonTime; }
  Double_t GetStoponTime()     { return fStoponTime; }
  
  Bool_t IsDone() { return fIsDone; }
  Bool_t HasStarted() { return fHasStarted; }
  void SetDone( Bool_t done ) { fIsDone = done; }
  void SetStarted( Bool_t started ) { fHasStarted = started; }

  Int_t GetStartOfSeq() { return  fStartOfSeq; }
  void SetStartOfSeq( Int_t start ) { fStartOfSeq = start; }

  void SetSeq( TString Seq )		{ fSeq = Seq; }
  void SetSeqNum( Int_t SeqNum )	{ fSeqNum = SeqNum; }
  void SetID( Int_t ID )	{ fID = ID; }
  void SetEventName( TString EventName )	{ fEventName = EventName; }
  void SetDescription( TString Description )	{ fDescription = Description; }

  void SetStartIntCount( Int_t count )		{ fStartIntCount = count; }
  void SetStopIntCount( Int_t count )		{ fStopIntCount = count; }
  void SetStartonCount( Int_t count )		{ fStartonCount = count; }
  void SetStoponCount( Int_t count )		{ fStoponCount = count; }
  void SetStartonTime( Double_t count )		{ fStartonTime = count; }
  void SetStoponTime( Double_t count )		{ fStoponTime = count; }

  void SetDetIntegral(Int_t index, Int_t integral);   

  ClassDef( TSeq_Dump, 1 )
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
