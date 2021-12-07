#define CHRONO_CLOCK_FREQ 100000000
#define CHRONO_CLOCK_CHANNEL 59
#define CHRONO_SYNC_CHANNEL 1
#define CHRONO_N_BOARDS 5
#define CHRONO_N_CHANNELS 60

#ifndef _CHRONOUTIL_
#define _CHRONOUTIL_

#include <cstdint>
#include <cassert>
#include <cmath>

#include "TObject.h"

struct ChronoEvent
{
   uint32_t MidasTime;
   Double_t RunTime;
   Int_t Channel;
   uint32_t Counts;
   Int_t ChronoBoard;
   int GetScalerModule()
   {
      return ChronoBoard*CHRONO_N_CHANNELS + Channel;
   }
   void SetScalerModuleNo(int m)
   {
      Channel = m % CHRONO_N_CHANNELS;
      ChronoBoard = floor( (double)m / (double)CHRONO_N_CHANNELS );
   }
   double GetRunTime()
   {
      return RunTime;
   }
   ChronoEvent* operator+=( ChronoEvent* b)
   {
      assert(this->GetScalerModule()==b->GetScalerModule());
      Counts     +=b->Counts;
      return this;
   }
   static bool SortByTimeThenByChannel (ChronoEvent* i,ChronoEvent* j) 
   {
      if (i->RunTime == j->RunTime)
         return (i->Channel < j->Channel);
      return (i->RunTime < j->RunTime); 
   }
};
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
