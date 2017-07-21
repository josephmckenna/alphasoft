//
// feevb.cxx
//
// Frontend/event builder for GRIF16 data
//

#include <stdio.h>
#include <netdb.h> // getnameinfo()
#include <stdlib.h> // malloc()
#include <string.h> // memcpy()
#include <errno.h> // errno
//#include <unistd.h>
//#include <time.h>
#include <assert.h> // assert
#include <math.h> // fabs()

#include <string>
#include <vector>
#include <deque>
#include <mutex>

#include "midas.h"

#include "TsSync.h"

const char *frontend_name = "feevb";                     /* fe MIDAS client name */
const char *frontend_file_name = __FILE__;               /* The frontend file name */

extern "C" {
   BOOL frontend_call_loop = TRUE;       /* frontend_loop called periodically TRUE */
   int display_period = 0;               /* status page displayed with this freq[ms] */
   int max_event_size = 1*1024*1024;     /* max event size produced by this frontend */
   int max_event_size_frag = 5 * 1024 * 1024;     /* max for fragmented events */
   int event_buffer_size = 8*1024*1024;           /* buffer size to hold events */
}

extern "C" {
  int interrupt_configure(INT cmd, INT source, PTYPE adr);
  INT poll_event(INT source, INT count, BOOL test);
  int frontend_init();
  int frontend_exit();
  int begin_of_run(int run, char *err);
  int end_of_run(int run, char *err);
  int pause_run(int run, char *err);
  int resume_run(int run, char *err);
  int frontend_loop();
  int read_event(char *pevent, INT off);
}

#ifndef EQ_NAME
#define EQ_NAME "EVB"
#endif

#ifndef EQ_EVID
#define EQ_EVID 1
#endif

EQUIPMENT equipment[] = {
   { EQ_NAME,                         /* equipment name */
      {EQ_EVID, 0, "SYSTEM",          /* event ID, trigger mask, Evbuf */
       EQ_MULTITHREAD, 0, "MIDAS",    /* equipment type, EventSource, format */
       TRUE, RO_ALWAYS,               /* enabled?, WhenRead? */
       50, 0, 0, 0,                   /* poll[ms], Evt Lim, SubEvtLim, LogHist */
       "", "", "",}, read_event,      /* readout routine */
   },
   {""}
};
////////////////////////////////////////////////////////////////////////////

static int verbose = 0;
static HNDLE hDB;
static HNDLE hKeySet; // equipment settings

static int gEventReadCount = 0;

////////////////////////////////////////////////////////////////////////////
//                     UNPACKING OF ALPHA16 DATA
////////////////////////////////////////////////////////////////////////////

static uint8_t getUint8(const void* ptr, int offset)
{
  return *(uint8_t*)(((char*)ptr)+offset);
}

static uint16_t getUint16(const void* ptr, int offset)
{
  uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
  return ((ptr8[0]<<8) | ptr8[1]);
}

static uint32_t getUint32(const void* ptr, int offset)
{
  uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
  return (ptr8[0]<<24) | (ptr8[1]<<16) | (ptr8[2]<<8) | ptr8[3];
}

// CRC16 from http://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum
static unsigned short crc16(const unsigned char* data_p, unsigned char length){
  unsigned char x;
  unsigned short crc = 0xFFFF;
  
  while (length--){
    x = crc >> 8 ^ *data_p++;
    x ^= x>>4;
    crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
  }
  return crc;
}

struct Alpha16info
{
   int packetType;
   int packetVersion;
   int acceptedTrigger;
   uint32_t hardwareId;
   uint32_t buildTimestamp;
   uint32_t eventTimestamp;
   uint32_t triggerOffset;
   int moduleId;
   int channelType;
   int channelId;
   int nsamples;
   int checksum;
   int xcrc16;

   void Print() const
   {
      printf("ALPHA16 data packet:\n");
      printf("  packet type:    0x%02x (%d)\n", packetType, packetType);
      printf("  packet version: 0x%02x (%d)\n", packetVersion, packetVersion);
      printf("  hwid:     0x%08x\n", hardwareId);
      printf("  buildts:  0x%08x\n", buildTimestamp);
      printf("  mod id:   0x%02x\n", moduleId);
      printf("  trig no:  0x%04x (%d)\n", acceptedTrigger, acceptedTrigger);
      printf("  event ts: 0x%08x (%d)\n", eventTimestamp, eventTimestamp);
      printf("  trig offset:   %d\n", triggerOffset);
      printf("  channel: type: %d, id: %d\n", channelType, channelId);
      printf("  nsamples: %d\n", nsamples);
      printf("  checksum: 0x%04x, computed checksum 0x%04x\n", checksum, xcrc16);
      //printf("length: %d, bank length %d\n", length, bankLength);
   };

   int Unpack(const void*ptr, int bklen8)
   {
      /*
        ALPHA16 UDP packet data format from Bryerton: this is packet version 1:
        
        Date: Mon, 20 Jun 2016 15:07:08 -0700
        From bryerton@triumf.ca  Mon Jun 20 15:07:05 2016
        From: Bryerton Shaw <bryerton@triumf.ca>
        To: Konstantin Olchanski <olchansk@triumf.ca>
        Subject: Re: eta on udp data?
        
        Hi Konstantin,
        
        Just trying to iron out one last bug, it was (is?) locking up if I
        saturated the link, but I think I just resolved that! So we ve got all
        16 channels outputting pretty steadily up to 40kHz or so, if I reduce
        it to 3 channels, we can get 150+ kHz event rates per channel.
        
        I am going to add a checksum onto the packet structure but it looks as
        follows, broken down by BYTE offset
        
        0 Packet Type - Currently fixed at0x01
        1 Packet Version - Currently fixed at0x01
        2 Accepted Trigger MSB - Inside the firmware logic Accepted Trigger is unsigned 32bits, providing the lower 16bits here for useful as a dropped UDP packet check
        3 Accepted Trigger LSB
        4 MSB Hardware ID - Currently the lower 48 bits of the ArriaV ChipID. It will be the MAC address shortly however
        5 "" ""
        6 "" ""
        7 "" ""
        8 "" ""
        9 LSB Hardware ID
        10 Build Timestamp (UNIX timestamp, aka seconds since Jan 01, 1980 UTC)
        11 "" ""
        12 "" ""
        13 "" ""
        14 0x00
        15 0x00
        16 MSB Event Timestamp
        17 "" ""
        18 "" ""
        19 "" ""
        20 "" ""
        21 LSB Event Timestamp
        22 MSB Trigger Offset - Trigger Point in relation to the start of the waveform packet. Signed 32bit integer
        23
        24
        25 LSB Trigger Offset
        26 ModuleID - Logical Identifier for the Alpha16 board. unsigned byte
        27 [7] Channel Type - Either 0 or 1, for BScint or Anode. The MSB of this byte
        27 [6:0] Channel ID - Unsigned 7bits, identifies the ADC channel (0-15) used
        28 MSB Sample Count - Unsigned 16 bit value indicating the number of samples (1-N)
        29 LSB Sample Count
        30 MSB First Waveform Sample - Signed 16 bit value
        31 LSB First Waveform Sample
        ....
        30 + (SampleCount*2) MSB Checksum
        31 + (SampleCount*2) LSB Checksum
        
        I will give you the checksum details in a moment, I am just adding it in
        now. Most likely will be a crc16 based on 1+x^2+x^15+x^16 .
        The byte positions may not be ideal, but we can sort that out.
        
        Cheers,
        
        Bryerton
      */
      
      //bankLength = bklen8;
      packetType = getUint8(ptr, 0);
      packetVersion = getUint8(ptr, 1);
      acceptedTrigger = getUint16(ptr, 2);
      hardwareId = getUint32(ptr, 4);
      buildTimestamp = getUint32(ptr, 10);
      //int zero = getUint16(ptr, 14);
      eventTimestamp = getUint32(ptr, 18);
      triggerOffset = getUint32(ptr, 22);
      moduleId = getUint8(ptr, 26);
      int chanX = getUint8(ptr, 27);
      channelType = chanX & 0x80;
      channelId = chanX & 0x7F;
      nsamples = getUint16(ptr, 28);
      checksum = getUint16(ptr, 30 + nsamples*2);
      //length = 32 + nsamples*2;
      
      xcrc16 = crc16((const unsigned char*)ptr, 32 + nsamples*2);

      //Print();

      return 0;
   };
};

struct BankBuf
{
   std::string name;
   int tid;
   void* ptr;
   int psize;

   BankBuf(const char* bankname, int xtid, const void* s, int size) // ctor
   {
      name = bankname;
      tid = xtid;
      ptr = malloc(size);
      psize = size;
      memcpy(ptr, s, size);
   }

   ~BankBuf() // dtor
   {
      if (ptr)
         free(ptr);
      ptr = NULL;
      psize = 0;
   }
};

typedef std::vector<BankBuf*> FragmentBuf;

std::deque<FragmentBuf*> gBuf;
std::mutex       gBufLock;

std::mutex       gEvbLock;

struct EvbEventBuf
{
   FragmentBuf* buf;

   uint32_t ts;
   int epoch;
   double time;
   double timeIncr;
};

struct EvbEvent
{
   bool   complete = false; // event is complete
   bool   error = false;    // event has an error
   int    counter = 0;  // event sequential counter
   double time = 0;     // event time, sec
   double timeIncr = 0; // time from previous event, sec

   FragmentBuf *banks = NULL;

   //EvbEvent(); // ctor
   ~EvbEvent(); // dtor
   void Merge(EvbEventBuf* m);
   void Print(int level=0) const;
};

void EvbEvent::Print(int level) const
{
   unsigned nbanks = 0;
   if (banks)
      nbanks = banks->size();
   printf("EvbEvent %d, time %f, incr %f, complete %d, error %d, %d banks: ", counter, time, timeIncr, complete, error, nbanks);
   for (unsigned i=0; i<nbanks; i++) {
      printf(" %s", (*banks)[i]->name.c_str());
   }
}

EvbEvent::~EvbEvent() // dtor
{
   if (banks) {
      // FIXME: delete contents of banks
   }
}

void EvbEvent::Merge(EvbEventBuf* m)
{
   assert(m->buf != NULL);

   if (!banks) {
      banks = new FragmentBuf;
   }

   for (unsigned i=0; i<m->buf->size(); i++) {
      banks->push_back((*(m->buf))[i]);
      (*(m->buf))[i] = NULL;
   }

   delete m->buf;
   m->buf = NULL;
   delete m;
}

class Evb
{
 public: // settings
   unsigned fMaxSkew;
   unsigned fMaxDead;
   double   fEpsSec;
   bool     fClockDrift;
   bool     fTrace = false;

 public: // event builder state
   TsSync fSync;
   int    fCounter;
   std::vector<std::deque<EvbEventBuf*>> fBuf;
   std::deque<EvbEvent*> fEvents;
   //double fLastA16Time;
   //double fLastFeamTime;

 public: // diagnostics
   double fMaxDt;
   double fMinDt;

 public: // counters
   int fCount = 0;
   int fCountComplete   = 0;
   int fCountError      = 0;
   int fCountIncomplete = 0;
   //int fCountIncompleteA16  = 0;
   //int fCountIncompleteFeam = 0;
   //int fCountA16 = 0;
   //int fCountFeam = 0;
   //int fCountRejectedA16 = 0;
   //int fCountRejectedFeam = 0;
   //int fCountCompleteA16 = 0;
   //int fCountCompleteFeam = 0;
   //int fCountErrorA16 = 0;
   //int fCountErrorFeam = 0;

 public: // member functions
   Evb(); // ctor
   ~Evb(); // dtor
   void AddBank(int imodule, uint32_t ts, BankBuf *b);
   EvbEvent* FindEvent(double t);
   void CheckEvent(EvbEvent *e);
   void Build(int index, EvbEventBuf *m);
   void Build();
   void Print() const;
   void UpdateCounters(const EvbEvent* e);
   EvbEvent* Get();
   EvbEvent* GetLastEvent();
};

Evb::Evb()
{
   //double a16_ts_freq, double feam_ts_freq, double eps_sec, int max_skew, int max_dead, bool clock_drift); // ctor

   double eps_sec = 50.0*1e-6;
   int max_skew = 10;
   int max_dead = 10;
   bool clock_drift = true;

   fMaxSkew = max_skew;
   fMaxDead = max_dead;
   fEpsSec = eps_sec;
   fClockDrift = clock_drift;

   fCounter = 0;

   double clk100 = 100.0*1e6; // 100MHz;
   double clk125 = 125.0*1e6; // 125MHz;
   
   double eps = 1000*1e-9;
   double rel = 0;
   int buf_max = 1000;
   
   //fSync.fTrace = 1;
   
   fSync.SetDeadMin(fMaxDead);

   fSync.Configure(0, clk100, eps, rel, buf_max);
   fSync.Configure(1, clk100, eps, rel, buf_max);
   fSync.Configure(2, clk100, eps, rel, buf_max);
   fSync.Configure(3, clk100, eps, rel, buf_max);
   fSync.Configure(4, clk100, eps, rel, buf_max);
   fSync.Configure(5, clk100, eps, rel, buf_max);
   fSync.Configure(6, clk125, eps, rel, buf_max);
   fSync.Configure(7, clk125, eps, rel, buf_max);
   fSync.Configure(8, clk125, eps, rel, buf_max);

   fBuf.resize(9);

   //fLastA16Time = 0;
   //fLastFeamTime = 0;
   fMaxDt = 0;
   fMinDt = 0;
}

Evb::~Evb()
{
   printf("Evb: max dt: %.0f ns, min dt: %.0f ns\n", fMaxDt*1e9, fMinDt*1e9);
}

void Evb::Print() const
{
   printf("Evb status:\n");
   printf("  Sync: "); fSync.Print(); printf("\n");
   //printf("  A16 events: in %d, rejected %d, complete %d, error %d\n", fCountA16, fCountRejectedA16, fCountCompleteA16, fCountErrorA16);
   //printf("  Feam events: in %d, rejected %d, complete %d, error %d\n", fCountFeam, fCountRejectedFeam, fCountCompleteFeam, fCountErrorFeam);
   //printf("  Buffered A16:  %d\n", (int)fBuf[0].size());
   //printf("  Buffered FEAM: %d\n", (int)fBuf[1].size());
   printf("  Buffered output: %d\n", (int)fEvents.size());
   printf("  Output %d events: %d complete, %d with errors, %d incomplete\n", fCount, fCountComplete, fCountError, fCountIncomplete);
}

EvbEvent* Evb::FindEvent(double t)
{
   double amin = 0;
   for (unsigned i=0; i<fEvents.size(); i++) {
      //printf("find event for time %f: event %d, %f, diff %f\n", t, i, fEvents[i]->time, fEvents[i]->time - t);

      double dt = fEvents[i]->time - t;
      double adt = fabs(dt);

      if (adt < fEpsSec) {
         if (adt > fMaxDt) {
            //printf("AgEVB: for time %f found event at time %f, new max dt %.0f ns, old max dt %.0f ns\n", t, fEvents[i]->time, adt*1e9, fMaxDt*1e9);
            fMaxDt = adt;
         }
         //printf("Found event for time %f\n", t);
         //printf("AgEVB: Found event for time %f: event %d, %f, diff %f %.0f ns\n", t, i, fEvents[i]->time, dt, dt*1e9);
         return fEvents[i];
      }

      if (amin == 0)
         amin = adt;
      if (adt < amin)
         amin = adt;
   }
   
   if (fMinDt == 0)
      fMinDt = amin;

   if (amin < fMinDt)
      fMinDt = amin;
   
   EvbEvent* e = new EvbEvent();
   e->complete = false;
   e->error = false;
   e->counter = fCounter++;
   e->time = t;
   
   fEvents.push_back(e);
   
   //printf("New event for time %f\n", t);
   
   return e;
}

void Evb::CheckEvent(EvbEvent *e)
{
   if (e)
      if (e->banks)
         if (e->banks->size() == 128)
            e->complete = true;
#if 0
   e->complete = true;

   if (!e->a16 && !fSync.fModules[0].fDead)
      e->complete = false;

   if (!e->feam && !fSync.fModules[1].fDead)
      e->complete = false;

   e->error = false;

   if (e->a16 && e->a16->error)
      e->error = true;

   if (e->feam && e->feam->error)
      e->error = true;
#endif

   //e->Print();
}

void Evb::Build(int index, EvbEventBuf *m)
{
   m->time = fSync.fModules[index].GetTime(m->ts, m->epoch);

   EvbEvent* e = FindEvent(m->time);

   assert(e);

#if 0
   if (0 && index == 1) {
      printf("offset: %f %f, index %d, ts 0x%08x, epoch %d, feam time %f\n", e->time, m->time, index, m->ts, m->epoch, m->feam->time);
   }
#endif

   if (fClockDrift) { // adjust offset for clock drift
      double off = e->time - m->time;
      //printf("offset: %f %f, diff %f, index %d\n", e->time, m->time, off, index);
      fSync.fModules[index].fOffsetSec += off/2.0;
   }

   e->Merge(m);

   CheckEvent(e);
}

void Evb::Build()
{
   for (unsigned i=0; i<fBuf.size(); i++) {
      while (fBuf[i].size() > 0) {
         EvbEventBuf* m = fBuf[i].front();
         fBuf[i].pop_front();
         Build(i, m);
      }
   }
}

void Evb::UpdateCounters(const EvbEvent* e)
{
   fCount++;
   if (e->error) {
      fCountError++;
   }

   if (e->complete) {
      fCountComplete++;
   } else {
      fCountIncomplete++;
   }
}

EvbEvent* Evb::GetLastEvent()
{
   Build();
   
   if (fEvents.size() < 1)
      return NULL;
   
   EvbEvent* e = fEvents.front();
   fEvents.pop_front();
   UpdateCounters(e);
   return e;
}

EvbEvent* Evb::Get()
{
   if (fSync.fSyncOk)
      Build();
   
   if (fEvents.size() < 1)
      return NULL;

   if (fTrace) {
      printf("Evb::Get: ");
      Print();
      printf("\n");
   }
   
   // check if the oldest event is complete
   if (!fEvents.front()->complete) {
      // oldest event is incomplete,
      // check if any newer events are completed,
      // if they are, pop this incomplete event
      bool c = false;
      for (unsigned i=0; i<fEvents.size(); i++) {
         if (fEvents[i]->complete) {
            c = true;
            break;
         }
      }
      // if there are too many buffered events, all incomplete,
      // something is wrong, push them out anyway
      if (!c && fEvents.size() < fMaxSkew)
         return NULL;
      
      printf("Evb::Get: popping an incomplete event! have %d buffered events, have complete %d\n", (int)fEvents.size(), c);
   }
   
   EvbEvent* e = fEvents.front();
   fEvents.pop_front();
   UpdateCounters(e);
   return e;
}

void Evb::AddBank(int imodule, uint32_t ts, BankBuf* b)
{
   assert(imodule >= 0);
   assert(imodule < (int)fBuf.size());
#if 0
   if (e->error)
      fCountErrorFeam++;

   if (e->complete)
      fCountCompleteFeam++;
#endif

   //uint32_t ts = e->time*fSync.fModules[imodule].fFreqHz;
   //printf("FeamEvent: t %f, ts 0x%08x", e->time, ts);
   //printf("\n");
   fSync.Add(imodule, ts);
   EvbEventBuf* m = new EvbEventBuf;
   m->buf = new FragmentBuf;
   m->buf->push_back(b);
   m->ts = fSync.fModules[imodule].fLastTs;
   m->epoch = fSync.fModules[imodule].fEpoch;
   m->time = 0;
   m->timeIncr = fSync.fModules[imodule].fLastTimeSec - fSync.fModules[imodule].fPrevTimeSec;
   fBuf[imodule].push_back(m);
}

#if 0
struct Event
{
   int eventNo;
   uint32_t timestamp;
   FragmentBuf* buf;

   Event() // ctor
   {
      eventNo = 0;
      timestamp = 0;
      buf = NULL;
   }

   ~Event() // dtor
   {
      // FIXME: memory leak: we are leaking all BankBuf structures inside "buf"
      if (buf)
         delete buf;
   }
};

bool sync_ok = false;

#define MAX_ADC 100

int tscount[MAX_ADC];
uint32_t ts[MAX_ADC][1000];
uint32_t dts[MAX_ADC][1000];

bool tscmp(uint32_t t1, uint32_t t2)
{
   if (t1 == t2)
      return true;
   if (t1+1 == t2)
      return true;
   if (t1 == t2+1)
      return true;
   return false;
}

bool find_sync(int max1, const uint32_t d1[], int max2, const uint32_t d2[], int *p1, int* p2)
{
   for (int i=1; i<max1-5; i++) {
      for (int j=1; j<max2-5; j++) {
         if (tscmp(d1[i], d2[j])) {
            //printf("found candidate at %d %d, dts 0x%08x 0x%08x\n", i, j, d1[i], d2[j]);

            int first_bad = 0;
            for (int k=1; true; k++) {
               if (i+k >= max1)
                  break;
               if (j+k >= max2)
                  break;
               //printf("cmp %d: 0x%08x 0x%08x\n", k, d1[i+k], d2[j+k]);
               if (!tscmp(d1[i+k], d2[j+k])) {
                  first_bad = k;
                  break;
               }
            }

            //printf("first_bad: %d\n", first_bad);

            if (first_bad == 0) {
               *p1 = i;
               *p2 = j;
               return true;
            }
         }
      }
   }
   return false;
}

uint32_t tsoffset[MAX_ADC];

void reset_sync()
{
   sync_ok = false;

   for (int m=0; m<MAX_ADC; m++) {
      tsoffset[m] = 0;
      tscount[m] = 0;
   }
}

typedef std::deque<Event*> EventBuf;

class EVB
{
private:

   int fNextEventNo;
   EventBuf fEvents;

   TsSync *fSync = NULL;

public:
   EVB() // ctor
   {
      Reset();
   }

   void Reset()
   {
      if (fSync)
         delete fSync;

      fSync = new TsSync();

      double clk100 = 100.0*1e6; // 100MHz;
      double clk125 = 125.0*1e6; // 125MHz;

      double eps = 1000*1e-9;
      double rel = 0;
      int buf_max = 1000;

      //fSync->fTrace = 1;

      fSync->SetDeadMin(10);
      //fSync->fEpsSec = 10000/1e9;

      fSync->Configure(0, clk100, eps, rel, buf_max);
      fSync->Configure(1, clk100, eps, rel, buf_max);
      fSync->Configure(2, clk100, eps, rel, buf_max);
      fSync->Configure(3, clk100, eps, rel, buf_max);
      fSync->Configure(4, clk100, eps, rel, buf_max);
      fSync->Configure(5, clk100, eps, rel, buf_max);
      fSync->Configure(6, clk125, eps, rel, buf_max);
      fSync->Configure(7, clk125, eps, rel, buf_max);

      fNextEventNo = 1;
      if (fEvents.size() > 0) {
         cm_msg(MERROR, "EVB::Reset", "Flushing %d events left over from previous run", (int)fEvents.size());
         for (unsigned i=0; i<fEvents.size(); i++) {
            if (fEvents[i])
               delete fEvents[i];
            fEvents[i] = NULL;
         }
         fEvents.clear();
      }
      assert(fEvents.size() == 0);
   }

private:
   Event* FindEvent(int imodule, uint32_t timestamp, bool print)
   {
      for (unsigned i=0; i<fEvents.size(); i++) {
         if (print)
            printf("FindEvent: module %d, ts 0x%08x, try %d 0x%08x\n", imodule, timestamp, i, fEvents[i]->timestamp);
         if (tscmp(fEvents[i]->timestamp, timestamp))
            return fEvents[i];
      }
      return NULL;
   }

   Event* MakeNewEvent(uint32_t timestamp)
   {
      Event* e = new Event();
      e->eventNo = fNextEventNo++;
      e->timestamp = timestamp;
      e->buf = new FragmentBuf();
      fEvents.push_back(e);
      return e;
   }

   void AddToEvent(Event*e, BankBuf* b)
   {
      //printf("Event %d, ts 0x%08x: add bank %s\n", e->eventNo, e->timestamp, b->name.c_str());
      e->buf->push_back(b);
   }

public:

   void PrintEvents() const
   {
      printf("EVB::PrintEvents:\n"); 
      for (unsigned i=0; i<fEvents.size(); i++) {
         Event* e = fEvents[i];
         printf("Event %d, ts 0x%08x, banks: ", e->eventNo, e->timestamp);
         for (unsigned j=0; j<e->buf->size(); j++)
            printf("%s ", (*e->buf)[j]->name.c_str());
         printf("\n");
      }

   }

   void AddBank(int imodule, uint32_t timestamp, BankBuf *b)
   {
      if (imodule >= 1 && imodule <= 8) {
         fSync->Add(imodule-1, timestamp);
         //printf("add module %d ts 0x%08x: ", imodule, timestamp);
         //fSync->Print();
         //printf("\n");
      }

      if (imodule == 7 || imodule == 8) {
         double xts = timestamp*100.0/125.0;
         uint32_t zts = xts;
         //if (imodule == 8)
         //printf("module %d: ts 0x%08x 0x%08x\n", imodule, timestamp, zts);
         timestamp = zts;
      }

      uint32_t xtimestamp = timestamp;

      if (sync_ok && imodule >=0 && imodule < MAX_ADC) {
         xtimestamp -= tsoffset[imodule];
      }

      Event* e = FindEvent(imodule, xtimestamp, false);

      if (!e) {
         if (!sync_ok && imodule>=0 && imodule<MAX_ADC) {
            int ptr = tscount[imodule];
            if (ptr < 20) {
               printf("module %d tscount %d\n", imodule, tscount[imodule]);
               ts[imodule][ptr] = timestamp;
               if (ptr > 0)
                  dts[imodule][ptr] = timestamp - ts[imodule][ptr-1];
               else
                  dts[imodule][ptr] = 0;
               tscount[imodule]++;
            } else {
               int max = 0;
               for (int i=0; i<MAX_ADC; i++)
                  if (tscount[i] > max)
                     max = tscount[i];

               printf("accumulated %d events\n", max);

               for (int i=0; i<max; i++) {
                  printf("%3d: ", i);
                  for (int m=0; m<MAX_ADC; m++) {
                     if (i<tscount[m])
                        printf("  0x%08x/0x%08x", ts[m][i], dts[m][i]);
                     else
                        printf("  0x%08x/0x%08x", -1, -1);
                  }
                  printf("\n");
               }

               int psync[MAX_ADC];

               for (int i=0; i<MAX_ADC; i++) {
                  psync[i] = 0;
               }

               printf("find sync:\n");
               for (int i=0; i<MAX_ADC; i++) {
                  for (int j=i+1; j<MAX_ADC; j++) {
                     if (tscount[i] < 5)
                        continue;
                     if (tscount[j] < 5)
                        continue;
                     int p1, p2;
                     bool s = find_sync(tscount[i], dts[i], tscount[j], dts[j], &p1, &p2);
                     printf("%d-%d: found sync %d, at %d %d\n", i, j, s, p1, p2);

                     if (s) {
                        if (p1 > psync[i])
                           psync[i] = p1;
                        if (p2 > psync[j])
                           psync[j] = p2;
                     }
                  }
               }

               int unitscount = 0;
               std::string units;

               for (int i=0; i<MAX_ADC; i++)
                  if (psync[i]) {
                     unitscount++;
                     units += " " + std::to_string(i);
                  }

               printf("Clock sync: ");
               for (int i=0; i<MAX_ADC; i++) {
                  printf(" %d", psync[i]);
               }
               printf("\n");

               for (int m=0; m<MAX_ADC; m++) {
                  tsoffset[m] = ts[m][psync[m]];
               }

               printf("clock synced!\n");
               cm_msg(MINFO, "sync", "Clock synced, %d units: %s", unitscount, units.c_str());
               sync_ok = true;
            }
         }

         //if (imodule == 8) {
         //   FindEvent(imodule, xtimestamp, true);
         //}
            
         e = MakeNewEvent(xtimestamp);
         //printf("module %d ts 0x%08x, make new event\n", imodule, xtimestamp);
      }

      AddToEvent(e, b);

      //PrintEvents();

      if (fEvents.size() > 10) {
         Event* ee = fEvents.front();
         fEvents.pop_front();
         std::lock_guard<std::mutex> lock(gBufLock);
         gBuf.push_back(ee->buf);
         ee->buf = NULL;
         delete ee;
      }
   }
};

EVB gEVB;
#endif

static Evb* gEvb = NULL;

static int gAlpha16map[] = {
   1,
   2,
   3,
   4,
   5,
   6,
   7,
   8,
   20,
   -1
};

void AddAlpha16bank(int imodule, char cmodule, const void* pbank, int bklen)
{
   Alpha16info info;
   int status = info.Unpack(pbank, bklen);

   if (status != 0) {
      // FIXME: unpacking error
      printf("unpacking error!\n");
      return;
   }
   
   //printf("Unpack info status: %d\n", status);
   //info.Print();

   char newname[5];
   sprintf(newname, "%c%c%02d", 'A', cmodule, info.channelId);
   //printf("bank name [%s]\n", newname);

   BankBuf *b = new BankBuf(newname, TID_BYTE, pbank, bklen);

#if 0
   gEVB.AddBank(imodule, info.eventTimestamp, b);
#endif

   int islot = -1;
   for (int i=0; gAlpha16map[i] >= 0; i++) {
      if (gAlpha16map[i] == imodule) {
         islot = i;
         break;
      }
   }

   if (islot >= 0) {
      std::lock_guard<std::mutex> lock(gEvbLock);
      if (gEvb)
         gEvb->AddBank(islot, info.eventTimestamp, b);
      else
         printf("AddBank but no gEvb!\n");
   } else {
      FragmentBuf* buf = new FragmentBuf();
      buf->push_back(b);
      std::lock_guard<std::mutex> lock(gBufLock);
      gBuf.push_back(buf);
   }
};

void AddAlpha16bank(int imodule, const void* pbank, int bklen)
{
   Alpha16info info;
   int status = info.Unpack(pbank, bklen);

   if (status != 0) {
      // FIXME: unpacking error
      printf("unpacking error!\n");
      return;
   }

#if 0
   if (imodule == 20) {
      printf("Unpack info status: %d\n", status);
      info.Print();
   }
#endif

#if 0
   if (imodule == 20) {
      printf("type %3d, chan %2d, TS 0x%08x\n", info.channelType, info.channelId, info.eventTimestamp);
      //info.Print();
      gEvb->fSync.fModules[8].DumpBuf();
   }
#endif

   int islot = -1;
   for (int i=0; gAlpha16map[i] >= 0; i++) {
      if (gAlpha16map[i] == imodule) {
         islot = i;
         break;
      }
   }

   char cname = 0;
   if (info.channelId <= 9) {
      cname = '0' + info.channelId;
   } else {
      cname = 'A' + info.channelId - 10;
   }

   char newname[5];

   if (info.channelType == 0) {
      sprintf(newname, "%c%02d%c", 'B', imodule, cname);
      //printf("bank name [%s]\n", newname);
   } else if (info.channelType == 128) {
      sprintf(newname, "%c%02d%c", 'C', imodule, cname);
   } else {
      sprintf(newname, "XX%02d", imodule);
   }

   BankBuf *b = new BankBuf(newname, TID_BYTE, pbank, bklen);

   if (islot < 0) {
      FragmentBuf* buf = new FragmentBuf();
      buf->push_back(b);
      std::lock_guard<std::mutex> lock(gBufLock);
      gBuf.push_back(buf);
      return;
   }

#if 0
   gEVB.AddBank(imodule, info.eventTimestamp, b);
#endif
   std::lock_guard<std::mutex> lock(gEvbLock);
   if (gEvb)
      gEvb->AddBank(islot, info.eventTimestamp, b);
   else
      printf("AddBank but no gEvb!\n");
};

// NOTE: event hander runs from the main thread!

void event_handler(HNDLE hBuf, HNDLE id, EVENT_HEADER *pheader, void *pevent)
{
   gEventReadCount++;

   char banklist[STRING_BANKLIST_MAX];
   int nbanks = bk_list(pevent, banklist);

   if (verbose)
      printf("event_handler: Evid: 0x%x, Mask: 0x%x, Serial: %d, Size: %d, Banks: %d (%s)\n", pheader->event_id, pheader->trigger_mask, pheader->serial_number, pheader->data_size, nbanks, banklist);
   
   if (nbanks < 1)
      return;
   
   FragmentBuf* buf = new FragmentBuf();

   for (int i=0; i<nbanks; i++) {
      int status;
      DWORD bklen, bktype;
      void* pbank;
      std::string name;
      name += banklist[i*4+0];
      name += banklist[i*4+1];
      name += banklist[i*4+2];
      name += banklist[i*4+3];
      
      status = bk_find((BANK_HEADER*)pevent, name.c_str(), &bklen, &bktype, &pbank);

      if (status != SUCCESS)
         continue;

      //printf("bk_find status %d, name [%s], bklen %d, bktype %d\n", status, &banklist[i*4], bklen, bktype);

      if (name[0]=='A' && name[1]=='A') {
         int imodule = (name[2]-'0')*10 + (name[3]-'0')*1;
         AddAlpha16bank(imodule, pbank, bklen);
      } else if (name == "WIRE") {
         AddAlpha16bank(99, 'Z', pbank, bklen);
      } else if (name == "ADC1") {
         AddAlpha16bank(0, '1', pbank, bklen);
      } else if (name == "ADC2") {
         AddAlpha16bank(1, '2', pbank, bklen);
      } else if (name == "ADC3") {
         AddAlpha16bank(2, '3', pbank, bklen);
      } else if (name == "ADC4") {
         AddAlpha16bank(3, '4', pbank, bklen);
      } else if (name == "ADC5") {
         AddAlpha16bank(4, '5', pbank, bklen);
      } else if (name == "ADC6") {
         AddAlpha16bank(5, '6', pbank, bklen);
      } else if (name == "ADC7") {
         AddAlpha16bank(6, '7', pbank, bklen);
      } else if (name == "ADC8") {
         AddAlpha16bank(7, '8', pbank, bklen);
      } else {
         BankBuf *bank = new BankBuf(name.c_str(), bktype, (char*)pbank, bklen);
         buf->push_back(bank);
      }
   }

   if (buf->size() == 0) {
      delete buf;
      return;
   }

   std::lock_guard<std::mutex> lock(gBufLock);
   gBuf.push_back(buf);
}

int interrupt_configure(INT cmd, INT source, PTYPE adr)
{
   return SUCCESS;
}

int frontend_init()
{
   int status;

   status = cm_get_experiment_database(&hDB, NULL);
   if (status != CM_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Cannot connect to ODB, cm_get_experiment_database() returned %d", status);
      return FE_ERR_ODB;
   }

   std::string path;
   path += "/Equipment";
   path += "/";
   path += EQ_NAME;
   path += "/Settings";

#if 0
   std::string path1 = path + "/udp_port";

   int udp_port = 50005;
   int size = sizeof(udp_port);
   status = db_get_value(hDB, 0, path1.c_str(), &udp_port, &size, TID_INT, TRUE);
   
   if (status != DB_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Cannot find \"%s\", db_get_value() returned %d", path1.c_str(), status);
      return FE_ERR_ODB;
   }
   
   status = db_find_key(hDB, 0, path.c_str(), &hKeySet);
   
   if (status != DB_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Cannot find \"%s\", db_find_key() returned %d", path.c_str(), status);
      return FE_ERR_ODB;
   }
   
   gDataSocket = open_udp_socket(udp_port);
   
   if (gDataSocket < 0) {
      printf("frontend_init: cannot open udp socket\n");
      cm_msg(MERROR, "frontend_init", "Cannot open UDP socket for port %d", udp_port);
      return FE_ERR_HW;
   }

   cm_msg(MINFO, "frontend_init", "Frontend equipment \"%s\" is ready, listening on UDP port %d", EQ_NAME, udp_port);
#endif

   int evid = -1;
   int trigmask = 0xFFFF;

   int bh = 0;
   const char *bufname = "BUFUDP";

   status = bm_open_buffer(bufname, 0, &bh);
   if (status != BM_SUCCESS && status != BM_CREATED)
      {
         cm_msg(MERROR, "frontend_init", "Error: bm_open_buffer(\"%s\") status %d", bufname, status);
         return FE_ERR_HW;
      }

   int reqid = 0;
   status = bm_request_event(bh, evid, trigmask, GET_ALL, &reqid, event_handler);
   if (status != BM_SUCCESS)
      {
         cm_msg(MERROR, "frontend_init", "Error: bm_request_event() status %d", status);
         return FE_ERR_HW;
      }

#if 0
   reset_sync();
#endif

   if (gEvb)
      delete gEvb;
   gEvb = new Evb();

   cm_msg(MINFO, "frontend_init", "Event builder started, buffer \"%s\", evid %d, trigmask 0x%x, verbose %d", bufname, evid, trigmask, verbose);

   return SUCCESS;
}

int frontend_loop()
{
   ss_sleep(10);
   return SUCCESS;
}

static int gCountBypass = 0;

int begin_of_run(int run_number, char *error)
{
   set_equipment_status("EVB", "Begin run...", "#00FF00");
   printf("begin_of_run!\n");
   if (gEvb)
      delete gEvb;
   gEvb = new Evb();

   gCountBypass = 0;

#if 0
   reset_sync();
   gEVB.Reset();
#endif
   return SUCCESS;
}

int end_of_run(int run_number, char *error)
{
   if (gEvb) {
      printf("end_of_run: Evb state:\n");
      gEvb->Print();
         
      while (1) {
         EvbEvent *e = gEvb->GetLastEvent();
         if (!e)
            break;
         
         if (1) {
            printf("Unpacked EvbEvent: ");
            e->Print();
            printf("\n");
         }

         //count_event += 1;
         
         delete e;
      }
      
      printf("end_of_run: Evb final state:\n");
      gEvb->Print();
   }

   if (gEvb) {
      delete gEvb;
      gEvb = NULL;
   }
   printf("end_of_run!\n");
   return SUCCESS;
}

int pause_run(INT run_number, char *error)
{
   return SUCCESS;
}

int resume_run(INT run_number, char *error)
{
   return SUCCESS;
}

int frontend_exit()
{
   return SUCCESS;
}

INT poll_event(INT source, INT count, BOOL test)
{
   //printf("poll_event: source %d, count %d, test %d\n", source, count, test);

   if (test) {
      for (int i=0; i<count; i++)
         ss_sleep(10);
      return 1;
   }

   return 1;
}

int read_event(char *pevent, int off)
{
#if 0
   if (gBuf.size() < 1) {
      ss_sleep(10);
      return 0;
   }
#endif

   //printf("in queue: %d\n", (int)gBuf.size());

   FragmentBuf* f = NULL;

   {
      std::lock_guard<std::mutex> lock(gBufLock);
      
      if (gBuf.size() > 0) {
         f = gBuf.front();
         gBuf.pop_front();
         gCountBypass++;
      }
      
      // implicit unlock of gBufLock
   }

   if (!f && gEvb) {
      std::lock_guard<std::mutex> lock(gEvbLock);
      
      EvbEvent* e = gEvb->Get();
      
      if (e) {
         printf("Have EvbEvent: ");
         e->Print();
         printf("\n");
         
         f = e->banks;
         e->banks = NULL;
         delete e;
      }

      // implicit unlock of gEvbLock
   }
   
   if (!f) {
      ss_sleep(10);
      return 0;
   }

   if (gEvb) {
      static time_t last = 0;
      time_t now = time(NULL);

      if (last == 0) {
         last = now;
         set_equipment_status("EVB", "Started...", "#00FF00");
      }

      if (now - last > 10) {
         last = now;
         char buf[256];
         sprintf(buf, "complete %d, incomplete %d, bypass %d", gEvb->fCountComplete, gEvb->fCountIncomplete, gCountBypass);
         set_equipment_status("EVB", buf, "#00FF00");
      }
   }

   bk_init32(pevent);

   //std::string banks = "";

   for (unsigned i=0; i<f->size(); i++) {
      BankBuf* b = (*f)[i];

      char* pdata;
      bk_create(pevent, b->name.c_str(), b->tid, (void**)&pdata);
      memcpy(pdata, b->ptr, b->psize);
      bk_close(pevent, pdata + b->psize);

      //banks += b->name;

      delete b;
   }

   delete f;

   //printf("Sending event: %s\n", banks.c_str());

   return bk_size(pevent); 
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
