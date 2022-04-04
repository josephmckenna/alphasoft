#ifndef _TSEQUENCERDIGITALOUT_
#define _TSEQUENCERDIGITALOUT_

#include "TObject.h"
#include <vector>

class TSequencerDigitalOut: public TObject
{
  public:
    //I expect only bools, but lets get greedy and be ready for fuzzy logic
    std::vector<bool> Channels;
    TSequencerDigitalOut(){}
    ~TSequencerDigitalOut(){}
    TSequencerDigitalOut(const TSequencerDigitalOut& d): TObject(), Channels(d.Channels) {}
    void Reset()
    {
       Channels.clear();
    }
    ClassDef(TSequencerDigitalOut, 1);
};

#endif