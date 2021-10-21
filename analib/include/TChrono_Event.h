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

struct ChronoChannel{
   int Channel = -1;
   int Board = -1;
   int GetIndex() const
   {
      return Board*CHRONO_N_CHANNELS+Channel;
   }
};
std::ostream& operator<<(std::ostream& o, ChronoChannel& c);
bool operator==( const ChronoChannel & lhs, const ChronoChannel & rhs);


class TChrono_Event : public TObject
{
   private:
      ChronoChannel fChannel;
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
      ChronoChannel GetChannel() const      { return fChannel; }
      uint32_t GetCounts() const    { return fCounts;  }
      uint32_t GetLocalTS() const   { return local_ts; }
      uint64_t GetTS() const        { return ts; }
      Double_t GetRunTime() const   { return runtime; }

      void SetID( Int_t _ID )             { fID=_ID; }
      void SetChannel( ChronoChannel _chan)       { fChannel=_chan; }
      void SetChannel (int channel, int board )   { fChannel.Channel = channel; fChannel.Board = board; }
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
