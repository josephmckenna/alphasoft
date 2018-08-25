#ifndef _TChrono_Event_
#define _TChrono_Event_

#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TChrono_Event : public TObject
{
  private:
    Int_t fChronoBoxIndex;
    Int_t fChronoBoardIndex;
    Int_t fID;
    uint32_t ts;
    Double_t runtime;
  
  public:
    TChrono_Event();
    using TObject::Print;
    virtual void Print();
    virtual ~TChrono_Event();
    Int_t GetBoxIndex()    { return fChronoBoxIndex; }
    Int_t GetBoardIndex()  { return fChronoBoardIndex; }
    Int_t GetID()          { return fID; }
    uint32_t Getts()       { return ts; }
    Double_t GetRunTime()  { return runtime; }
    
    
    void SetBoxIndex( Int_t _index )    { fChronoBoxIndex=_index; }
    void SetBoardIndex( Int_t _index )  { fChronoBoardIndex=_index; }
    void SetID( Int_t _ID )          { fID=_ID; }
    void Setts( uint32_t _ts )       { ts=_ts; }
    void SetRunTime( Double_t _RunTime)  { runtime = _RunTime; }
    
    
    
    void Reset();
  
    ClassDef(TChrono_Event, 1);
};

#endif
