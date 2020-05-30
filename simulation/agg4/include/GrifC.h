#ifndef GRIFCH
#define GRIFCH

#include <stdint.h>
#include <string.h>
#include <vector>

#ifndef NUM_PADS_CHAN
#define NUM_PADS_CHAN 18432
#endif
#ifndef MAX_ALPHA16
#define MAX_ALPHA16 32
#endif
#ifndef NUM_CHAN_ALPHA16
#define NUM_CHAN_ALPHA16 16
#endif

struct GrifCEvent_t
{
  int      eventNo; // event counter, starting from 1

  bool     udpPresent[NUM_PADS_CHAN];  // udp packet received

  //  std::vector<int16_t> waveform[NUM_PADS_CHAN];
  std::vector<int> waveform[NUM_PADS_CHAN];

  int  numChan;  // count of received channels

  bool error;    // event has an error
  bool complete; // event is complete
   
  GrifCEvent_t() // ctor
  {
    Reset();
  }
  ~GrifCEvent_t(){} // dtor

  void Reset()
  {
    eventNo=0;
    //    MEMZERO(udpPresent);
    memset((udpPresent),0,sizeof(udpPresent));
    numChan = 0;
    error    = false;
    complete = false;
  }
  void Print() const
  {
    printf("Fake GrifC event: %d, channels: %d, error %d, complete %d\n", 
	   eventNo, numChan, error, complete);
  }
};

#endif
