#ifndef _TChrono_Channel_
#define _TChrono_Channel_


#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

enum { CBTRG, CB01, CB02, CB03, CB04 };

#include <map>

#ifndef ROOT_TString
#include "TString.h"
#endif

#include "ChronoUtil.h"



class TChronoChannel {
   private:
      std::string fBoardName;
      int fChannel;
   public:
      static const std::map<std::string,int> CBMAP;

   TChronoChannel();
   TChronoChannel(const std::string& Board, const int Channel);
   TChronoChannel(const TChronoChannel& c);
   const std::string GetBoard() const
   {
      return fBoardName;
   }

   int GetBoardNumber() const
   {
      return CBMAP.at(fBoardName);
   }

   int GetChannel() const
   {
      return fChannel;
   }
   void SetBoard(const std::string board) { fBoardName = board; }
   void SetChannel(int channel) { fChannel = channel; }
   //void SetChannel (int channel, int board )   { fChannel = channel; fBoard = board; }
   void SetChannel( const TChronoChannel& c)
   {
      fChannel = c.GetChannel();
      fBoardName =c.GetBoard();
   }
   int GetIndex() const
   {
      return CBMAP.at(fBoardName) * CHRONO_N_CHANNELS + fChannel;
   }
   bool IsValidChannel() const
   {
      return (!fBoardName.empty() && fChannel >= 0);
   }
   const std::string GetBranchName() const
   {
      return fBoardName;
   }
   TChronoChannel& operator=(const TChronoChannel& other)
   {
      fBoardName = other.GetBoard();
      fChannel = other.GetChannel();
      return *this;
   }
};
std::ostream& operator<<(std::ostream& o,const TChronoChannel& c);
bool operator==( const TChronoChannel & lhs, const TChronoChannel & rhs);

#endif

