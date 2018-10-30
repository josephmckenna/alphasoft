#include <cstddef>
#include "VirtualOdb.h"
#ifndef _TChronoChannelName_
#define _TChronoChannelName_

#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif


#include <TBufferJSON.h>
#include <fstream>
#include "chrono_module.h"

class TChronoChannelName : public TObject
{
  private:
  Int_t fChronoBoxIndex; //Box index isn't used yet...
  Int_t fChronoBoardIndex;
  TString Name[CHRONO_N_CHANNELS];
  public:
   TChronoChannelName();
   TChronoChannelName(VirtualOdb* Odb, Int_t b, Int_t BoxIndex=-1);
   TChronoChannelName(TString json, Int_t b);

   void DumpToJson(int runno);

   using TObject::Print;
   virtual void Print();
   virtual ~TChronoChannelName();
   Int_t GetBoxIndex()                   { return fChronoBoxIndex; }
   Int_t GetBoardIndex()                 { return fChronoBoardIndex; }
   TString GetChannelName(Int_t Channel) { return Name[Channel]; }
   Int_t GetChannel(TString ChannelName, Bool_t exact_match=kTRUE);

   void SetBoxIndex(Int_t _index)        { fChronoBoxIndex = _index; }
   void SetBoardIndex(Int_t _index)      { fChronoBoardIndex = _index; }
   void SetChannelName(TString _name, Int_t i) { Name[i]=_name; }
   
  ClassDef( TChronoChannelName, 1 )
};
#endif
