#ifndef _TChronoChannelName_
#define _TChronoChannelName_

#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#include "../ana/chrono_module.h"

class TChronoChannelName : public TObject
{
  private:
  Int_t fChronoBoxIndex;
  Int_t fChronoBoardIndex;
  TString Name[CHRONO_N_CHANNELS];
  public:
   TChronoChannelName();
   using TObject::Print;
   virtual void Print();
   virtual ~TChronoChannelName();
   Int_t GetBoxIndex()      { return fChronoBoxIndex; }
   Int_t GetBoardIndex()    { return fChronoBoardIndex; }
   TString GetChannelName(Int_t Channel) { return Name[Channel]; }
   Int_t GetChannel(TString ChannelName, Bool_t exact_match=kTRUE);
   void SetBoardIndex(Int_t _index) { fChronoBoardIndex = _index; }
   void SetChannelName(TString _name, Int_t i) { Name[i]=_name; }
   
  ClassDef( TChronoChannelName, 1 )
};
#endif
