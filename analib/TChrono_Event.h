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
      Int_t fChannel;
      uint32_t fCounts;
      uint32_t local_ts; //raw 32bit TS
      uint64_t ts;       //Calculated 64 TS
      Double_t runtime;

   public:
      TChrono_Event();
      using TObject::Print;
      virtual void Print();
      virtual ~TChrono_Event();
      Int_t GetBoxIndex()      { return fChronoBoxIndex; }
      Int_t GetBoardIndex()    { return fChronoBoardIndex; }
      Int_t GetID()            { return fID; }
      Int_t GetChannel()       { return fChannel; }
      uint32_t GetCounts()     { return fCounts;  }
      uint32_t GetLocalTS()    { return local_ts; }
      uint64_t GetTS()         { return ts; }
      Double_t GetRunTime()    { return runtime; }

      void SetBoxIndex( Int_t _index )    { fChronoBoxIndex=_index; }
      void SetBoardIndex( Int_t _index )  { fChronoBoardIndex=_index; }
      void SetID( Int_t _ID )             { fID=_ID; }
      void SetChannel( Int_t _chan)       { fChannel=_chan; }
      void SetCounts( uint32_t _counts )  { fCounts = _counts; }
      void SetTS( uint64_t _ts )          { ts=_ts; }
      void SetRunTime( Double_t _RunTime) { runtime = _RunTime; }

      void Reset();

      ClassDef(TChrono_Event, 1);
};

#endif
