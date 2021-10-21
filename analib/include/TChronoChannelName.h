// 
// chronobox 
// 
// A. Capra
// JTK McKenna



#ifndef _TChronoChannelName_
#define _TChronoChannelName_

#include "mvodb.h"

#include <fstream>
#include <iostream>

#include "ChronoUtil.h"


#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#include "TBufferJSON.h"


class TChronoChannelName : public TObject
{
  private:
  Int_t fChronoBoardIndex;
  std::vector<std::string> fName;
  public:
  
   TChronoChannelName();
   TChronoChannelName(MVOdb* Odb, Int_t board);
   
   TChronoChannelName(TString json, Int_t b);

   void DumpToJson(int runno);

   using TObject::Print;
   virtual void Print();
   virtual ~TChronoChannelName();
   Int_t GetBoardIndex() const                 { return fChronoBoardIndex; }
   std::string GetChannelName(Int_t Channel) const { return fName.at(Channel); }
   Int_t GetChannel(std::string ChannelName, const bool exact_match=kTRUE) const;

   void SetBoardIndex(Int_t index)      { fChronoBoardIndex = index; }
   void SetChannelName(std::string name, size_t i) { fName.at(i)= name; }
   
  ClassDef( TChronoChannelName, 1 )
};
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
