#ifndef _TSISChannel_
#define _TSISChannel_

//Simple container to for handing SIS channel IDs. Avoid users having to do manipulation of array indexes. JTKM
#include <iostream>
#include "TString.h"
#define NUM_SIS_MODULES 2
#define NUM_SIS_CHANNELS 32

class TSISChannel
{
   public:
      int fChannel;
      int fModule;

   TSISChannel();
   TSISChannel(const int CombinedChannelAndModule);
   TSISChannel(const int chan, const int mod);
   TSISChannel(const TSISChannel& c);
   TSISChannel& operator=(const TSISChannel& rhs);
   bool IsValid() const
   {
      if (fChannel >= 0 && fChannel < NUM_SIS_CHANNELS)
         if (fModule >= 0 && fModule < NUM_SIS_MODULES)
            return true;
      return false;
   }
   Int_t toInt() const {
       return fModule* NUM_SIS_CHANNELS + fChannel;
   }
};

bool operator==(const TSISChannel& lhs, const TSISChannel& rhs);
std::ostream& operator<< (std::ostream& os, const TSISChannel& ch);
TString& operator+=(TString& s, const TSISChannel& lhs);
#endif
