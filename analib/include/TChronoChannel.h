#ifndef _TChrono_Channel_
#define _TChrono_Channel_


#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#include "ChronoUtil.h"

class TChronoChannel {
   private:
      int fBoard;
      int fChannel;
   public:   
   TChronoChannel()
   {
      int fBoard = -1;
      int fChannel = -1;
   }
   TChronoChannel(const int Board, const int Channel): fBoard(Board), fChannel(Channel)
   {
   }
   TChronoChannel(const TChronoChannel& c): fBoard(c.fBoard), fChannel(c.fChannel)
   {
   }
   int GetBoard() const
   {
      return fBoard;
   }
   int GetChannel() const
   {
      return fChannel;
   }
   void SetBoard(int board) { fBoard = board; }
   void SetChannel(int channel) { fChannel = channel; }
   void SetChannel (int channel, int board )   { fChannel = channel; fBoard = board; }
   void SetChannel( const TChronoChannel& c)  { fChannel = c.GetChannel(); fBoard = c.GetBoard(); }
   TChronoChannel GetTChronoChannel() { return *this; }
   bool IsValidChannel() const
   {
      return (fBoard >= 0 && fChannel >= 0);
   }
   int GetIndex() const
   {
      return fBoard * CHRONO_N_CHANNELS + fChannel;
   }
   std::string GetBranchName() const
   {
      return std::string("cb0") + std::to_string(fBoard + 1);
   }
   TChronoChannel& operator=(const TChronoChannel& other)
   {
      fBoard = other.GetBoard();
      fChannel = other.GetChannel();
      return *this;
   }
};
std::ostream& operator<<(std::ostream& o,const TChronoChannel& c);
bool operator==( const TChronoChannel & lhs, const TChronoChannel & rhs);

#endif

