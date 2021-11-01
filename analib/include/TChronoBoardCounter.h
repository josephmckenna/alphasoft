#ifndef _TCHRONO_BOARD_COUNTER_
#define _TCHRONO_BOARD_COUNTER_


#include "TObject.h"
#include <array>
#include <assert.h>
#include "ChronoUtil.h"
#include "store_cb.h"

class TChronoBoardCounter: public TObject
{
   public:
      double   fStartTime;
      double   fStopTime;
      int      fBoard;
      std::array<int,CHRONO_N_CHANNELS> fCounts;
   
   public:
   //default constr.
   TChronoBoardCounter();

   //Copy constr.
   TChronoBoardCounter(const TChronoBoardCounter& counter);

   //Constr. for the board and dump times (can populate counts later)
   TChronoBoardCounter(double startTime, double stopTime, int board);

   //Constr. for just the board (can populate times later)
   TChronoBoardCounter(int board);

   //=== Getters ===
   double GetStartTime() const                             { return fStartTime; };
   double GetStopTime() const                              { return fStopTime; };
   double GetBoard() const                                 { return fBoard; };
   //Range safe
   int GetCount(int channel) const                   { return fCounts.at(channel); };
   //Unsafe but faster
   int GetCountFast(int channel) const                   { return fCounts[channel]; };


   //=== Setters ===
   void SetStartTime(double startTime)                     { fStartTime = startTime; };
   void SetStopTime(double stopTime)                       { fStopTime = stopTime; };
   void SetBoard(double board)                             { fBoard = board; };
   void AddCountsToChannel(int channel, int counts)        { fCounts.at(channel) += counts; };

   TChronoBoardCounter& operator+=(const TChronoBoardCounter& rhs);
   TChronoBoardCounter& operator+=(const TCbFIFOEvent& cbFIFO);

   virtual ~TChronoBoardCounter();

   ClassDef(TChronoBoardCounter,1)
};

TChronoBoardCounter operator+(const TChronoBoardCounter& lhs, const TChronoBoardCounter& rhs);



#endif
