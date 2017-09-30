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

#include "tmvodb.h"

#include "atpacket.h"

static TMVOdb* gOdb = NULL; // ODB root
static TMVOdb* gS = NULL; // ODB equipment settings
static TMVOdb* gC = NULL; // ODB /Eq/Ctrl/EvbConfig
static TMVOdb* gEvbStatus = NULL; // ODB /Eq/EVB/EvbStatus

const char *frontend_name = "feevb";                     /* fe MIDAS client name */
const char *frontend_file_name = __FILE__;               /* The frontend file name */

extern "C" {
   BOOL frontend_call_loop = TRUE;       /* frontend_loop called periodically TRUE */
   int display_period = 0;               /* status page displayed with this freq[ms] */
   int max_event_size = 3*1024*1024;     /* max event size produced by this frontend */
   int max_event_size_frag = 5 * 1024 * 1024;     /* max for fragmented events */
   int event_buffer_size = 40*1024*1024;           /* buffer size to hold events */
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
   int evb_slot;
   std::string name;
   int tid;
   void* ptr;
   int psize;

   BankBuf(int islot, const char* bankname, int xtid, const void* s, int size) // ctor
   {
      evb_slot = islot;
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

   int no_bank_count = 0;
   std::vector<int> bank_count;

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
      if (i>10) { // truncate the bank list
         printf(" ...");
         break;
      }
      printf(" %s", (*banks)[i]->name.c_str());
      //printf(" (%d)", (*banks)[i]->evb_slot);
   }
   printf(" evb slots: %d then ", no_bank_count);
   for (unsigned i=0; i<bank_count.size(); i++) {
      printf(" %d", bank_count[i]);
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
      BankBuf* b = (*(m->buf))[i];
      if (b->evb_slot < 0) {
         no_bank_count++;
      } else {
         if (b->evb_slot >= (int)bank_count.size()) {
            bank_count.resize(b->evb_slot+1);
         }
         bank_count[b->evb_slot]++;
      }
      banks->push_back(b);
      (*(m->buf))[i] = NULL;
   }

   delete m->buf;
   m->buf = NULL;
   delete m;
}

struct FeamTsBuf
{
   uint32_t cnt = 0;
   uint32_t n   = 0;
   uint32_t ts  = 0;
   int n_cnt = 0;
};

class Evb
{
public: // settings
   unsigned fMaxSkew;
   unsigned fMaxDead;
   double   fEpsSec;
   bool     fClockDrift;
   bool     fTrace = false;

public: // configuration maps, etc
   std::vector<int> fAtSlot;   // slot of each module
   std::vector<int> fA16Slot;  // slot of each module
   std::vector<int> fFeamSlot; // slot of each module
   std::vector<int> fNumBanks; // number of banks for each slot
   std::vector<int> fSlotType; // module type for each slot
   std::vector<std::string> fSlotName; // module name for each slot

 public: // event builder state
   TsSync fSync;
   int    fCounter;
   std::vector<std::deque<EvbEventBuf*>> fBuf;
   std::deque<EvbEvent*> fEvents;
   std::vector<FeamTsBuf> fFeamTsBuf;

 public: // diagnostics
   double fMaxDt;
   double fMinDt;

 public: // counters
   int fCount = 0;
   int fCountComplete   = 0;
   int fCountError      = 0;
   int fCountIncomplete = 0;
   std::vector<int> fCountSlotIncomplete;

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

void set_vector_element(std::vector<int>* v, unsigned i, int value)
{
   assert(i<1000); // protect against crazy value
   while (i>=v->size()) {
      v->push_back(-1);
   }
   assert(i<v->size());
   (*v)[i] = value;
}

int get_vector_element(const std::vector<int>& v, unsigned i)
{
   if (i>=v.size())
      return -1;
   else
      return v[i];
}

Evb::Evb()
{
   printf("Evb: constructor!\n");

   //double a16_ts_freq, double feam_ts_freq, double eps_sec, int max_skew, int max_dead, bool clock_drift); // ctor

   // race condition against fectrl... fNumBanks = GetNumBanks();

   double eps_sec = 50.0*1e-6;
   int max_skew = 10;
   int max_dead = 5;
   bool clock_drift = true;
   int pop_threshold = fSync.fPopThreshold;

   gS->RD("eps_sec", 0, &eps_sec, true);
   gS->RI("max_skew", 0, &max_skew, true);
   gS->RI("max_dead", 0, &max_dead, true);
   gS->RB("clock_drift", 0, &clock_drift, true);
   gS->RI("sync_pop_threshold", 0, &pop_threshold, true);

   fMaxSkew = max_skew;
   fMaxDead = max_dead;
   fEpsSec = eps_sec;
   fClockDrift = clock_drift;
   fSync.fPopThreshold = pop_threshold;

   fCounter = 0;

   double eps = 1000*1e-9;
   double rel = 0;
   int buf_max = 1000;
   
   gS->RB("trace_sync", 0, &fSync.fTrace, true);
   
   fSync.SetDeadMin(fMaxDead);

   // Load configuration from ODB

   std::vector<std::string> name;
   std::vector<int> type;
   std::vector<int> module;
   std::vector<int> nbanks;
   std::vector<int> tsfreq;

   gC->RSA("name", &name, false, 0, 0);
   gC->RIA("type", &type, false, 0);
   gC->RIA("module", &module, false, 0);
   gC->RIA("nbanks", &nbanks, false, 0);
   gC->RIA("tsfreq", &tsfreq, false, 0);

   assert(name.size() == type.size());
   assert(name.size() == module.size());
   assert(name.size() == nbanks.size());
   assert(name.size() == tsfreq.size());

   // Loop over evb slots

   //int count = 0;
   int count_at = 0;
   int count_a16 = 0;
   int count_feam = 0;

   fSlotName.resize(name.size());

   for (unsigned i=0; i<name.size(); i++) {
      printf("Slot %2d: [%s] type %d, module %d, nbanks %d, tsfreq %d\n", i, name[i].c_str(), type[i], module[i], nbanks[i], tsfreq[i]);

      switch (type[i]) {
      default:
         break;
      case 1: { // AT
         fSync.Configure(i, tsfreq[i], eps, rel, buf_max);
         set_vector_element(&fAtSlot, module[i], i);
         set_vector_element(&fNumBanks, i, nbanks[i]);
         set_vector_element(&fSlotType, i, type[i]);
         fSlotName[i] = name[i];
         count_at++;
         break;
      }
      case 2: { // A16
         fSync.Configure(i, tsfreq[i], eps, rel, buf_max);
         set_vector_element(&fA16Slot, module[i], i);
         set_vector_element(&fNumBanks, i, nbanks[i]);
         set_vector_element(&fSlotType, i, type[i]);
         fSlotName[i] = name[i];
         count_a16++;
         break;
      }
      case 3: { // FEAMrev0
         fSync.Configure(i, tsfreq[i], eps, rel, buf_max);
         set_vector_element(&fFeamSlot, module[i], i);
         set_vector_element(&fNumBanks, i, nbanks[i]);
         set_vector_element(&fSlotType, i, type[i]);
         fSlotName[i] = name[i];
         count_feam++;
         break;
      }
      }
   }
      
   printf("For each module:\n");

   printf("AT map:   ");
   for (unsigned i=0; i<fAtSlot.size(); i++)
      printf(" %2d", fAtSlot[i]);
   printf("\n");

   printf("A16 map:  ");
   for (unsigned i=0; i<fA16Slot.size(); i++)
      printf(" %2d", fA16Slot[i]);
   printf("\n");

   printf("Feam map: ");
   for (unsigned i=0; i<fFeamSlot.size(); i++)
      printf(" %2d", fFeamSlot[i]);
   printf("\n");

   printf("For each evb slot:\n");

   printf("SlotName: ");
   for (unsigned i=0; i<fSlotName.size(); i++)
      printf(" %s", fSlotName[i].c_str());
   printf("\n");

   printf("SlotType: ");
   for (unsigned i=0; i<fSlotType.size(); i++)
      printf(" %d", fSlotType[i]);
   printf("\n");

   printf("NumBanks: ");
   for (unsigned i=0; i<fNumBanks.size(); i++)
      printf(" %d", fNumBanks[i]);
   printf("\n");

   fFeamTsBuf.resize(name.size());
   fBuf.resize(name.size());
   fCountSlotIncomplete.resize(name.size());

   cm_msg(MINFO, "Evb::Evb", "Evb: configured %d modules: %d AT, %d A16, %d FEAM", (int)name.size(), count_at, count_a16, count_feam);

   fMaxDt = 0;
   fMinDt = 0;
}

Evb::~Evb()
{
   printf("Evb: max dt: %.0f ns, min dt: %.0f ns\n", fMaxDt*1e9, fMinDt*1e9);
   printf("Evb: dtor!\n");
}

void Evb::Print() const
{
   printf("Evb status:\n");
   printf("  Sync: "); fSync.Print(); printf("\n");
   printf("  Buffered output: %d\n", (int)fEvents.size());
   printf("  Output %d events: %d complete, %d with errors, %d incomplete\n", fCount, fCountComplete, fCountError, fCountIncomplete);
   printf("  Incomplete count for each slot:\n");
   for (unsigned i=0; i<fCountSlotIncomplete.size(); i++) {
      if (fCountSlotIncomplete[i] > 0) {
         printf("    slot %d, module %s: incomplete count: %d\n", i, fSlotName[i].c_str(), fCountSlotIncomplete[i]);
      }
   }
   printf("  Max dt: %.0f ns\n", fMaxDt*1e9);
   printf("  Min dt: %.0f ns\n", fMinDt*1e9);
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
   assert(e);
   assert(e->banks);

   if (0) {
      printf("check event: ");
      e->Print();
      printf("\n");
   }

   while (e->bank_count.size() < fNumBanks.size()) {
      e->bank_count.push_back(0);
   }

   assert(e->bank_count.size() == fNumBanks.size());

   bool complete = true;

   for (unsigned i=0; i<fNumBanks.size(); i++) {
      if (fSync.fModules[i].fDead) {
         continue;
      }

      if (e->bank_count[i] != fNumBanks[i]) {
         complete = false;
      }

      if (0) {
         printf("slot %d: type %d, should have %d, have %d, complete %d\n", i, fSlotType[i], fNumBanks[i], e->bank_count[i], complete);
      }
   }

   e->complete = complete;

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

      assert(e->bank_count.size() == fNumBanks.size());

      for (unsigned i=0; i<fNumBanks.size(); i++) {
         if (fSync.fModules[i].fDead) {
            continue;
         }

         if (e->bank_count[i] != fNumBanks[i]) {
            fCountSlotIncomplete[i]++;
         }

         if (0) {
            printf("slot %d: type %d, should have %d, have %d\n", i, fSlotType[i], fNumBanks[i], e->bank_count[i]);
         }
      }
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
   
   EvbEvent* e = fEvents.front();

   // check if the oldest event is complete
   if (!e->complete) {
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
      e->Print();
      printf("\n");
   }
   
   fEvents.pop_front();
   UpdateCounters(e);
   return e;
}

void Evb::AddBank(int imodule, uint32_t ts, BankBuf* b)
{
   assert(imodule >= 0);
   assert(imodule < (int)fBuf.size());

   //if (fBuf[imodule].size() == 0) {
   //   printf("Evb::AddBank: first event for module %d\n", imodule);
   //}

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

bool AddAlpha16bank(Evb* evb, int imodule, const void* pbank, int bklen)
{
   Alpha16info info;
   int status = info.Unpack(pbank, bklen);

   if (status != 0) {
      // FIXME: unpacking error
      printf("unpacking error!\n");
      return false;
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

   int islot = get_vector_element(evb->fA16Slot, imodule);

   //printf("a16 module %d slot %d\n", imodule, islot);

   if (islot < 0) {
      return false;
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

   BankBuf *b = new BankBuf(islot, newname, TID_BYTE, pbank, bklen);

   std::lock_guard<std::mutex> lock(gEvbLock);
   evb->AddBank(islot, info.eventTimestamp, b);

   return true;
};

class FeamPacket
{
public:
   uint32_t cnt;
   uint16_t n;
   uint16_t x511;
   uint16_t buf_len;
   uint32_t ts_start;
   uint32_t ts_trig;
   int off;
   bool error;

public:
   FeamPacket(); // ctor
   ~FeamPacket(); // dtor
   void Unpack(const char* data, int size);
   void Print() const;
};

#if 0
static uint8_t getUint8(const void* ptr, int offset)
{
   return *(uint8_t*)(((char*)ptr)+offset);
}

static uint16_t getUint16be(const void* ptr, int offset)
{
   uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
   return ((ptr8[0]<<8) | ptr8[1]);
}

static uint32_t getUint32be(const void* ptr, int offset)
{
   uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
   return (ptr8[0]<<24) | (ptr8[1]<<16) | (ptr8[2]<<8) | ptr8[3];
}
#endif

static uint16_t getUint16le(const void* ptr, int offset)
{
   uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
   return ((ptr8[1]<<8) | ptr8[0]);
}

static uint32_t getUint32le(const void* ptr, int offset)
{
   uint8_t *ptr8 = (uint8_t*)(((char*)ptr)+offset);
   return (ptr8[3]<<24) | (ptr8[2]<<16) | (ptr8[1]<<8) | ptr8[0];
}

FeamPacket::FeamPacket()
{
   //printf("FeamPacket: ctor!\n");
   //printf("FeamPacket: count %d!\n", x1count++);
   error = true;
}

FeamPacket::~FeamPacket()
{
   //printf("FeamPacket: dtor!\n");
   //x1count--;
}

void FeamPacket::Unpack(const char* data, int size)
{
   error = true;

   off = 0;
   cnt = getUint32le(data, off); off += 4;
   n   = getUint16le(data, off); off += 2;
   x511 = getUint16le(data, off); off += 2;
   buf_len = getUint16le(data, off); off += 2;
   if (n == 0) {
      ts_start = getUint32le(data, off); off += 8;
      ts_trig  = getUint32le(data, off); off += 8;
   } else {
      ts_start = 0;
      ts_trig  = 0;
   }

   error = false;
}

void FeamPacket::Print() const
{
   printf("decoded %2d bytes, ", off);
   printf("cnt %6d, n %3d, x511 %3d, buf_len %4d, ts_start 0x%08x, ts_trig 0x%08x, ",
          cnt,
          n,
          x511,
          buf_len,
          ts_start,
          ts_trig);
   printf("error %d", error);
}

bool AddFeamBank(Evb* evb, int imodule, const char* bkname, const char* pbank, int bklen, int bktype)
{
   FeamPacket p;
   p.Unpack(pbank, bklen);

   int islot = get_vector_element(evb->fFeamSlot, imodule);

   //printf("feam module %d slot %d\n", imodule, islot);

   if (0 && p.n == 0) {
      printf("feam module %d: ", imodule);
      p.Print();
      printf("\n");
   }

   if (islot < 0) {
      return false;
   }

   if (p.n == 0) {
      evb->fFeamTsBuf[islot].n_cnt = 0;
      evb->fFeamTsBuf[islot].n   = 0;
      evb->fFeamTsBuf[islot].cnt = p.cnt;
      evb->fFeamTsBuf[islot].ts  = p.ts_trig;
   }
   
   if (p.cnt != evb->fFeamTsBuf[islot].cnt) {
      return false;
   }

   evb->fFeamTsBuf[islot].n = p.n;
   evb->fFeamTsBuf[islot].n_cnt++;
   
   uint32_t ts = evb->fFeamTsBuf[islot].ts;
   
   BankBuf *b = new BankBuf(islot, bkname, TID_BYTE, pbank, bklen);
   
   std::lock_guard<std::mutex> lock(gEvbLock);
   evb->AddBank(islot, ts, b);
   
   return true;
}

bool AddAtBank(Evb* evb, const char* bkname, const char* pbank, int bklen, int bktype)
{
   AlphaTPacket p;
   p.Unpack(pbank, bklen*rpc_tid_size(bktype));

   //p.Print();
   //printf("\n");

   int imodule = 1;

   int islot = get_vector_element(evb->fAtSlot, imodule);

   if (islot < 0) {
      return false;
   }

   uint32_t ts = p.ts_625;

   BankBuf *b = new BankBuf(islot, bkname, TID_DWORD, pbank, bklen*rpc_tid_size(bktype));
      
   std::lock_guard<std::mutex> lock(gEvbLock);
   evb->AddBank(islot, ts, b);
   
   return true;
}

// NOTE: event handler runs from the main thread!

static int gCountInput = 0;

static int gFirstEventIn = 0;
static int gFirstEventOut = 0;

static Evb* gEvb = NULL;

void event_handler(HNDLE hBuf, HNDLE id, EVENT_HEADER *pheader, void *pevent)
{
   if (gFirstEventIn == 0) {
      cm_msg(MINFO, "event_handler", "Received first event");
      gFirstEventIn = 1;
   }

   if (!gEvb) {
      std::lock_guard<std::mutex> lock(gEvbLock);
      gEvb = new Evb();
   }

   // for sure have the event builder from here on
   assert(gEvb);

   gCountInput++;

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

      bool handled = false;

      if (name[0]=='A' && name[1]=='A') {
         int imodule = (name[2]-'0')*10 + (name[3]-'0')*1;
         handled = AddAlpha16bank(gEvb, imodule, pbank, bklen);
      } else if (name[0]=='P' && name[1]=='A') {
         int imodule = (name[2]-'0')*10 + (name[3]-'0')*1;
         handled = AddFeamBank(gEvb, imodule, name.c_str(), (const char*)pbank, bklen, bktype);
      } else if (name[0]=='A' && name[1]=='T') {
         handled = AddAtBank(gEvb, name.c_str(), (const char*)pbank, bklen, bktype);
      }

      if (!handled) {
         BankBuf *bank = new BankBuf(-1, name.c_str(), bktype, (char*)pbank, bklen*rpc_tid_size(bktype));
         buf->push_back(bank);
      }
   }

   if (0) {
      gEvb->fSync.Dump();
      gEvb->fSync.Print();
      printf("\n");
   }

   if (gEvb) {
      static bool ok = false;
      static bool failed = false;
      
      if (ok != gEvb->fSync.fSyncOk) {
         if (gEvb->fSync.fSyncOk) {
            cm_msg(MINFO, "event_handler", "Event builder timestamp sync successful");
         }
         ok = gEvb->fSync.fSyncOk;
      }

      if (failed != gEvb->fSync.fSyncFailed) {
         if (gEvb->fSync.fSyncFailed) {
            cm_msg(MERROR, "event_handler", "Event builder timestamp sync FAILED");
         }
         failed = gEvb->fSync.fSyncFailed;
      }
   }
   //printf("EVB %d %d\n", , gEvb->fSync.fSyncFailed);

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

   gOdb = MakeOdb(hDB);
   TMVOdb* eq_odb = gOdb->Chdir((std::string("Equipment/") + EQ_NAME).c_str(), true);
   gS = eq_odb->Chdir("Settings", true);
   gEvbStatus = eq_odb->Chdir("EvbStatus", true);
   gC = gOdb->Chdir("Equipment/Ctrl/EvbConfig", false);

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
   gEvb = NULL;

   gCountInput = 0;
   gCountBypass = 0;

   gFirstEventIn = 0;
   gFirstEventOut = 0;

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

   if (gEvb) {
      //gEvb->Print();
      //gEvb->fSync.Dump();
      //gEvb->fSync.fTrace = true;
   }

   if (gEvb) {
      static time_t last = 0;
      time_t now = time(NULL);

      //if (last == 0) {
      //   last = now;
      //   set_equipment_status("EVB", "Started...", "#00FF00");
      //}

      if (last == 0 || now - last > 2) {
         last = now;
         char buf[256];
         sprintf(buf, "%d in, complete %d, incomplete %d, bypass %d", gCountInput, gEvb->fCountComplete, gEvb->fCountIncomplete, gCountBypass);
         set_equipment_status("EVB", buf, "#00FF00");

         gEvb->Print();

         gEvbStatus->WI("events_in", gCountInput);
         gEvbStatus->WI("complete", gEvb->fCountComplete);
         gEvbStatus->WI("incomplete", gEvb->fCountIncomplete);
         gEvbStatus->WI("bypass", gCountBypass);
         gEvbStatus->WI("sync_min", gEvb->fSync.fMin);
         gEvbStatus->WI("sync_max", gEvb->fSync.fMax);
         gEvbStatus->WB("sync_ok", gEvb->fSync.fSyncOk);
         gEvbStatus->WB("sync_failed", gEvb->fSync.fSyncFailed);
         gEvbStatus->WB("sync_overflow", gEvb->fSync.fOverflow);
         gEvbStatus->WIA("incomplete_count", gEvb->fCountSlotIncomplete);
      }
   }

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
         if (gFirstEventOut == 0) {
            cm_msg(MINFO, "read_event", "Built the first event");
            gFirstEventOut = 1;
         }

         //printf("Have EvbEvent: ");
         //e->Print();
         //printf("\n");
         
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
