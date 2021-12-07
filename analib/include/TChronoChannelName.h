// 
// chronobox 
// 
// A. Capra
// JTK McKenna



#ifndef _TChronoChannelName_
#define _TChronoChannelName_

#include <fstream>
#include <iostream>

#include "ChronoUtil.h"
#include "TChronoChannel.h"



#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#include "TBufferJSON.h"

class MVOdb;
class VirtualOdb;

class TChronoChannelName : public TObject
{
  private:
  std::string fChronoBoardName;
  std::vector<std::string> fName;
  public:
  
   TChronoChannelName();
   TChronoChannelName(VirtualOdb* Odb, Int_t b, Int_t BoxIndex=-1);
   TChronoChannelName(MVOdb* Odb, Int_t b, Int_t BoxIndex=-1);
   TChronoChannelName(TString json, Int_t b);

   void DumpToJson(int runno);

   using TObject::Print;
   virtual void Print();
   virtual ~TChronoChannelName();
   int GetBoardIndex() const                 { return TChronoChannel::CBMAP.at(fChronoBoardName); }
   const std::string GetBoardName() const { return fChronoBoardName; }
   int GetNumberOfChannels() const { return fName.size(); }
   const std::string GetChannelName(int Channel) const { return fName.at(Channel); }
   int GetChannel(std::string ChannelName, const bool exact_match=kTRUE) const;

   void SetBoard(const std::string& board)      { fChronoBoardName = board; }
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
