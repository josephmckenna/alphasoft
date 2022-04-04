#ifndef _TSEQUENCERTRIGGERIN_
#define _TSEQUENCERTRIGGERIN_
#include "TObject.h"
#include <vector>

class TSequencerTriggerIn: public TObject
{
  public:
   int InfWait;
   double waitTime;
   std::vector<int> Channels;
   TSequencerTriggerIn(){}
   ~TSequencerTriggerIn(){}
   TSequencerTriggerIn(const TSequencerTriggerIn& t): TObject(), InfWait(t.InfWait), waitTime(t.waitTime), Channels(t.Channels) {}
   void Reset()
   {
      InfWait = 0;
      waitTime = 0.0;
      Channels.clear();
   }
   ClassDef(TSequencerTriggerIn, 1);
};

#endif