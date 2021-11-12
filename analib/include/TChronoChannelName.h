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
#include "TChronoChannel.h"



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
  std::string fChronoBoardName;
  std::vector<std::string> fName;
  public:
  
   TChronoChannelName();
   TChronoChannelName(MVOdb* Odb, const std::string& board);
   
   TChronoChannelName(TString json, const std::string& b);

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
