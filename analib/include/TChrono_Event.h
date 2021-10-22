#ifndef _TChrono_Event_
#define _TChrono_Event_

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
      return fBoard*CHRONO_N_CHANNELS+fChannel;
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


class TChrono_Event : public TChronoChannel, public TObject
{
   private:
      Int_t fID;
      uint32_t fCounts;
      uint32_t local_ts; //raw 32bit TS
      uint64_t ts;       //Calculated 64 TS
      Double_t runtime;

   public:
      TChrono_Event();
      using TObject::Print;
      virtual void Print();
      virtual ~TChrono_Event();
      Int_t GetID() const           { return fID; }
      
      uint32_t GetCounts() const    { return fCounts;  }
      uint32_t GetLocalTS() const   { return local_ts; }
      uint64_t GetTS() const        { return ts; }
      Double_t GetRunTime() const   { return runtime; }

      void SetID( Int_t _ID )             { fID=_ID; }

      void SetCounts( uint32_t _counts )  { fCounts = _counts; }
      void SetTS( uint64_t _ts )          { ts=_ts; }
      void SetRunTime( Double_t _RunTime) { runtime = _RunTime; }

      void Reset();

      ClassDef(TChrono_Event, 2);
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

