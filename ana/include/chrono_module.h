// 
// chronobox 
// 
// A. Capra
// JTK McKenna
#include "TChronoChannelName.h"
#define CHRONO_CLOCK_FREQ 100000000
#define CHRONO_CLOCK_CHANNEL 59
#define CHRONO_SYNC_CHANNEL 1
#define CHRONO_N_BOARDS 2
#define CHRONO_N_BOXES 1
#define CHRONO_N_CHANNELS 60
#define CHRONO_N_TS_CHANNELS 4

#ifndef _CHRONOMODULE_
#define _CHRONOMODULE_
#include <algorithm>    // std::sort

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
      Channel=m%CHRONO_N_CHANNELS;
      ChronoBoard=floor(m/CHRONO_N_CHANNELS);
   }
   double GetRunTime()
   {
      return RunTime;
   }
   //operator+=
//   TSISEvent* TSISEvent::operator+=( TSISEvent* b)
   ChronoEvent* operator+=( ChronoEvent* b)
   {
      //Events from differnt SIS modules cannot be added!
      //this->Print();
      //std::cout <<"Adding module "<<b->GetSISModule() << " to "<<this->GetSISModule()<<std::endl;
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
