// 
// chronobox 
// 
// A. Capra
// JTK McKenna

#define CHRONO_CLOCK_FREQ 100000000
#define CHRONO_CLOCK_CHANNEL 59
#define CHRONO_N_BOARDS 2
#define CHRONO_N_BOXES 1
#define CHRONO_N_CHANNELS 60
#define CHRONO_N_TS_CHANNELS 4


#ifndef _CHRONOMODULE_
#define _CHRONOMODULE_


#include "TChronoChannelName.h"
struct ChronoEvent
{
    Double_t RunTime;
    Int_t Channel;
    uint32_t Counts;
    Int_t ChronoBoard;
};
#endif
