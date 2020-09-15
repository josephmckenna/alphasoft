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

#include <unistd.h> // sleep()

#include <string>
#include <vector>
#include <deque>
#include <mutex>

#include "midas.h"
#include "msystem.h" // rb_get_buffer_level()

#include "TsSync.h"

#include "mvodb.h"
#include "tmfe.h"

#include "atpacket.h"

////////////////////////////////////////////////////////////////////////////

static int verbose = 0;

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
   int tid = 0;
   int waiting_incr = 0; // increment waiting bank count
   void* ptr = NULL;
   int psize = 0;

   int xslot = 0;    // evb->AddBank slot
   uint32_t xts = 0; // evb->AddBank ts

   BankBuf(const char* bankname, int xtid, const void* s, int size, int xwaiting_incr) // ctor
   {
      name = bankname;
      tid = xtid;
      waiting_incr = xwaiting_incr;
      assert(size > 0);
      ptr = malloc(size);
      assert(ptr != NULL);
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

// bypass buffer
static std::deque<FragmentBuf*> gBuf;
static std::mutex       gBufLock;
static int size_gbuf_max = 0;

struct EvbEventBuf
{
   //FragmentBuf* buf;

   uint32_t ts;
   int epoch;
   double time;
   double timeIncr;

   BankBuf* bank = NULL;

   void Print() const;
};

struct EvbEvent
{
   bool   complete = false; // event is complete
   bool   error = false;    // event has an error
   int    counter = 0;  // event sequential counter
   double time = 0;     // event time, sec
   double timeIncr = 0; // time from previous event, sec

   bool maybe_complete = false;

   double fCreateTime = 0;
   double fUpdateTime = 0;

   FragmentBuf *banks = NULL;

   std::vector<int> banks_count;
   std::vector<int> banks_waiting;

   //EvbEvent(); // ctor
   ~EvbEvent(); // dtor
   void MergeSlot(int slot, EvbEventBuf* m);
   void Print(int level=0) const;
};

void EvbEventBuf::Print() const
{
   //printf("ts 0x%08x, epoch %d, time %f, incr %f, banks %d", ts, epoch, time, timeIncr, (int)buf->size());
   //if (buf->size() == 1) {
   //   printf(", bank [%s]", (*buf)[0]->name.c_str());
   //}
   const char* bname = "";
   if (bank)
      bname = bank->name.c_str();
   printf("ts 0x%08x, epoch %d, time %f, incr %f, bank [%s]", ts, epoch, time, timeIncr, bname);
}

void EvbEvent::Print(int level) const
{
   unsigned nbanks = 0;
   if (banks)
      nbanks = banks->size();
   printf("EvbEvent %d, time %f, incr %f, complete %d, maybe %d, error %d, %d banks: ", counter, time, timeIncr, complete, maybe_complete, error, nbanks);
   for (unsigned i=0; i<nbanks; i++) {
      if (i>4) { // truncate the bank list
         printf(" ...");
         break;
      }
      printf(" %s", (*banks)[i]->name.c_str());
   }
   printf(" evb slots: ");
   for (unsigned i=0; i<banks_count.size(); i++) {
      printf(" %d/%d", banks_count[i], banks_waiting[i]);
   }
}

EvbEvent::~EvbEvent() // dtor
{
   if (banks) {
      for (unsigned i=0; i<banks->size(); i++) {
         if ((*banks)[i]) {
            delete (*banks)[i];
            (*banks)[i] = NULL;
         }
      }
      banks->clear();
      delete banks;
      banks = NULL;
   }
}

void EvbEvent::MergeSlot(int slot, EvbEventBuf* m)
{
   //assert(m->buf != NULL);

   if (!banks) {
      banks = new FragmentBuf;
   }

   int bw = banks_waiting[slot];

   //int size = m->buf->size();
   //printf("merge slot %d, %d banks\n", slot, size);
   //for (int i=0; i<size; i++) {
   //BankBuf* b = (*(m->buf))[i];
   BankBuf* b = m->bank;
   banks_count[slot]++;
   if (bw > 0) {
      bw -= b->waiting_incr;
   } else {
      cm_msg(MERROR, "EvbEvent::MergeSlot", "Error: slot %d: too many banks or data after complete_this!", slot);
   }
   banks->push_back(b);
   //(*(m->buf))[i] = NULL;
   m->bank = NULL;
   //}
   
   banks_waiting[slot] = bw;

   if (bw == 0)
      maybe_complete = true;

   fUpdateTime = TMFE::GetTime();
   
   //delete m->buf;
   //m->buf = NULL;
   delete m;
}

struct PwbData
{
   uint32_t cnt = 0;
   uint32_t ts = 0;
   uint32_t sent_bits = 0;
   uint32_t threshold_bits = 0;
   uint32_t pkt_seq = 0;
   uint16_t chunk_id = 0;
   uint32_t count_error = 0;
   uint32_t count_bad_pkt_seq = 0;
   uint32_t count_bad_channel_id = 0;
   uint32_t count_bad_format_revision = 0;
   uint32_t count_bad_chunk_id = 0;
   uint32_t count_lost_header = 0;
   uint32_t count_lost_footer = 0;
};

class Evb
{
public:
   std::mutex fLock;

public: // settings
   double   fMaxAgeSec = 2.0;
   double   fMaxDeadSec = 15.0;
   unsigned fMaxDead;
   double   fEpsSec;
   bool     fClockDrift;
   bool     fTrace = false;
   bool     fPrintIncomplete = false;
   bool     fPrintAll = false;

public: // configuration maps, etc
   unsigned fNumSlots = 0;
   std::vector<int> fTrgSlot;   // slot of each module
   std::vector<int> fAdcSlot;  // slot of each module
   std::vector<int> fPwbSlot; // slot of each module
   std::vector<int> fTdcSlot; // slot of each module
   std::vector<int> fNumBanks; // number of banks for each slot
   std::vector<int> fSlotType; // module type for each slot
   std::vector<std::string> fSlotName; // module name for each slot

 public: // event builder state
   TsSync fSync;
   int    fCounter = 0;
   std::vector<std::deque<EvbEventBuf*>> fBuf;
   std::deque<EvbEvent*> fEvents;
   std::vector<PwbData> fPwbData;
   std::vector<int> fDeadSlots;
   int fCountDeadSlots = 0;

 public: // diagnostics
   double fMaxDt;
   double fMinDt;
   unsigned fEventsSize = 0;
   unsigned fMaxEventsSize = 0;

 public: // counters
   int fCountInput      = 0; // count all input events
   int fCountBypass     = 0; // count unknown banks
   int fCountOut        = 0; // count all output events
   int fCount = 0;
   int fCountComplete   = 0; // count complete events e->complete
   int fCountError      = 0; // count events with errors e->error
   int fCountIncomplete = 0; // count incomplete events !e->complete
   int fCountSlotErrors = 0; // errors counted in AddXxxBank() that are not attached to any event. Sum of fCountError[]
   int fCountPopAge     = 0; // count of incomplete events poped by age
   int fCountPopFollowingComplete = 0; // count of incomplete events popped by complete following events
   int fCountPopLast    = 0; // count of events popped at the end of run
   std::vector<int> fCountSlotIncomplete;
   std::vector<double> fLastTimeSec;
   std::vector<double> fSkewTimeSec;
   std::vector<double> fCountPackets;
   std::vector<double> fCountBytes;
   std::vector<double> fPacketsPerSec;
   std::vector<double> fBytesPerSec;
   std::vector<int>    fSentMin;
   std::vector<int>    fSentMax;
   std::vector<double> fCountSent0;
   std::vector<double> fCountSent1;
   std::vector<double> fSentAve;
   std::vector<int>    fThrMin;
   std::vector<int>    fThrMax;
   std::vector<double> fCountThr0;
   std::vector<double> fCountThr1;
   std::vector<double> fThrAve;
   std::vector<int>    fCountErrors;
   std::vector<int>    fPwbScaFifoMaxUsed;
   std::vector<int>    fPwbEventFifoWrUsed;
   std::vector<int>    fPwbEventFifoRdUsed;
   std::vector<int>    fPwbEventFifoWrMaxUsed;
   std::vector<int>    fPwbEventFifoRdMaxUsed;
   std::vector<int>    fPwbEventFifoOverflows;

   int fPwbScaFifoMaxUsedEver = 0;
   int fPwbEventFifoWrMaxUsedAll = 0;
   int fPwbEventFifoWrMaxUsedEver = 0;
   int fPwbEventFifoOverflowsAll  = 0;
   int fPwbEventFifoOverflowsEver = 0;

   int fMaxEventSize = 0;
   int fMaxEventSizeEver = 0;

   double fEventCount = 0;
   double fByteCount = 0;

   double fSkewMax = 0;
   double fSkewMaxEver = 0;

 public: // rate counters
   double fPrevTime = 0;
   std::vector<double> fPrevCountPackets;
   std::vector<double> fPrevCountBytes;
   std::vector<double> fPrevCountSent0;
   std::vector<double> fPrevCountSent1;
   std::vector<double> fPrevCountThr0;
   std::vector<double> fPrevCountThr1;

 public: // member functions
   Evb(MVOdb* settings, MVOdb* config, MVOdb* status); // ctor
   ~Evb(); // dtor
   void AddBank(int imodule, uint32_t ts, BankBuf *b);
   EvbEvent* FindEvent(double t, int index, EvbEventBuf *m);
   void CheckEvent(EvbEvent *e, bool last_event);
   void BuildSlot(int slot, EvbEventBuf *m);
   void Build();
   void Print() const;
   void PrintEvents() const;
   void LogPwbCounters() const;
   void WriteSyncStatus(MVOdb* odb) const;
   void WriteEvbStatus(MVOdb* odb) const;
   void WriteVariables(MVOdb* odb);
   void ResetPerSecond();
   void ComputePerSecond();
   void UpdateCounters(const EvbEvent* e);
   void CheckDeadSlots();
   EvbEvent* GetNext();
   EvbEvent* Get();
   EvbEvent* GetLastEvent();
};

void set_vector_element(std::vector<int>* v, unsigned i, int value, bool overwrite = true)
{
   assert(i<1000); // protect against crazy value
   while (i>=v->size()) {
      v->push_back(-1);
   }
   assert(i<v->size());
   if (overwrite || (*v)[i] < 0) {
      (*v)[i] = value;
   }
}

int get_vector_element(const std::vector<int>& v, unsigned i)
{
   if (i>=v.size())
      return -1;
   else
      return v[i];
}

static bool gKludgeTdcKillFirstEvent = false;
static bool gKludgeTdcLastEvent = false;

Evb::Evb(MVOdb* settings, MVOdb* config, MVOdb* status) // ctor
{
   printf("Evb: constructor!\n");

   //double adc_ts_freq, double feam_ts_freq, double eps_sec, int max_skew, int max_dead, bool clock_drift); // ctor

   // race condition against fectrl... fNumBanks = GetNumBanks();

   double eps_sec = 50.0*1e-6;
   //int max_skew = 10;
   int max_dead = 5;
   bool clock_drift = true;
   int pop_threshold = fSync.fPopThreshold;

   settings->RD("eps_sec", &eps_sec, true);
   //settings->RI("max_skew", &max_skew, true);
   settings->RD("max_age_sec", &fMaxAgeSec, true);
   settings->RD("max_dead_sec", &fMaxDeadSec, true);
   settings->RI("max_dead", &max_dead, true);
   settings->RB("clock_drift", &clock_drift, true);
   settings->RI("sync_pop_threshold", &pop_threshold, true);

   settings->RB("print_incomplete", &fPrintIncomplete, true);
   settings->RB("print_all", &fPrintAll, true);

   fMaxDead = max_dead;
   fEpsSec = eps_sec;
   fClockDrift = clock_drift;
   fSync.fPopThreshold = pop_threshold;

   fCounter = 0;

   double eps = 1000*1e-9;
   double rel = 0;
   int buf_max = 1000;
   
   settings->RD("sync_eps_sec", &eps, true);
   settings->RB("trace_sync", &fSync.fTrace, true);
   
   fSync.SetDeadMin(fMaxDead);

   // Load configuration from ODB

   std::vector<std::string> name;
   std::vector<int> type;
   std::vector<int> module;
   std::vector<int> nbanks;
   std::vector<double> tsfreq;

   config->RSA("name", &name, false, 0, 0);
   config->RIA("type", &type, false, 0);
   config->RIA("module", &module, false, 0);
   config->RIA("nbanks", &nbanks, false, 0);
   config->RDA("tsfreq", &tsfreq, false, 0);

   assert(name.size() == type.size());
   assert(name.size() == module.size());
   assert(name.size() == nbanks.size());
   assert(name.size() == tsfreq.size());

   // Loop over evb slots

   //int count = 0;
   int count_trg = 0;
   int count_adc = 0;
   int count_pwb = 0;
   int count_tdc = 0;

   fNumSlots = name.size();

   fSlotName.resize(fNumSlots);

   for (unsigned i=0; i<name.size(); i++) {
      printf("Slot %2d: [%s] type %d, module %d, nbanks %d, tsfreq %f\n", i, name[i].c_str(), type[i], module[i], nbanks[i], tsfreq[i]);

      switch (type[i]) {
      default:
         break;
      case 1: { // TRG
         fSync.Configure(i, 2.0*0x80000000, tsfreq[i], eps, rel, buf_max);
         set_vector_element(&fTrgSlot, module[i], i);
         set_vector_element(&fNumBanks, i, nbanks[i]);
         set_vector_element(&fSlotType, i, type[i]);
         fSlotName[i] = name[i];
         count_trg++;
         break;
      }
      case 2: { // ADC
         fSync.Configure(i, 2.0*0x80000000, tsfreq[i], eps, rel, buf_max);
         set_vector_element(&fAdcSlot, module[i], i);
         set_vector_element(&fNumBanks, i, nbanks[i]);
         set_vector_element(&fSlotType, i, type[i]);
         fSlotName[i] = name[i];
         count_adc++;
         break;
      }
      case 3: { // FEAMrev0
         //fSync.Configure(i, 2.0*0x80000000, tsfreq[i], eps, rel, buf_max);
         //set_vector_element(&fPwbSlot, module[i], i);
         //set_vector_element(&fNumBanks, i, nbanks[i]);
         //set_vector_element(&fSlotType, i, type[i]);
         //fSlotName[i] = name[i];
         //count_pwb++;
         break;
      }
      case 4: { // PWB rev1
         //fSync.Configure(i, 2.0*0x80000000, tsfreq[i], eps, rel, buf_max);
         //set_vector_element(&fPwbSlot, module[i], i);
         //set_vector_element(&fNumBanks, i, nbanks[i]);
         //set_vector_element(&fSlotType, i, type[i]);
         //fSlotName[i] = name[i];
         //count_pwb++;
         break;
      }
      case 5: { // PWB rev1 with HW UDP
         fSync.Configure(i, 2.0*0x80000000, tsfreq[i], eps, rel, buf_max);
         set_vector_element(&fPwbSlot, module[i], i, false);
         set_vector_element(&fNumBanks, i, nbanks[i]);
         set_vector_element(&fSlotType, i, type[i]);
         fSlotName[i] = name[i];
         count_pwb++;
         break;
      }
      case 6: { // TDC
         fSync.Configure(i, 0x10000000, tsfreq[i], eps, rel, buf_max);
         set_vector_element(&fTdcSlot, module[i], i, false);
         set_vector_element(&fNumBanks, i, nbanks[i]);
         set_vector_element(&fSlotType, i, type[i]);
         fSlotName[i] = name[i];
         count_tdc++;
         gKludgeTdcKillFirstEvent = true;
         gKludgeTdcLastEvent = true;
         break;
      }
      }
   }

   printf("For each module:\n");

   printf("TRG map:   ");
   for (unsigned i=0; i<fTrgSlot.size(); i++)
      printf(" %2d", fTrgSlot[i]);
   printf("\n");

   printf("ADC map:  ");
   for (unsigned i=0; i<fAdcSlot.size(); i++)
      printf(" %2d", fAdcSlot[i]);
   printf("\n");

   printf("PWB map: ");
   for (unsigned i=0; i<fPwbSlot.size(); i++)
      printf(" %2d", fPwbSlot[i]);
   printf("\n");

   printf("TDC map: ");
   for (unsigned i=0; i<fTdcSlot.size(); i++)
      printf(" %2d", fTdcSlot[i]);
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

   fPwbData.resize(fNumSlots);
   fBuf.resize(fNumSlots);

   fDeadSlots.resize(fNumSlots);

   fCountSlotIncomplete.resize(fNumSlots);

   fLastTimeSec.resize(fNumSlots);
   fSkewTimeSec.resize(fNumSlots);

   fCountPackets.resize(fNumSlots);
   fCountBytes.resize(fNumSlots);

   fPacketsPerSec.resize(fNumSlots);
   fBytesPerSec.resize(fNumSlots);

   fPrevCountPackets.resize(fNumSlots);
   fPrevCountBytes.resize(fNumSlots);

   fSentMin.resize(fNumSlots);
   fSentMax.resize(fNumSlots);
   fSentAve.resize(fNumSlots);
   fCountSent0.resize(fNumSlots);
   fCountSent1.resize(fNumSlots);
   fPrevCountSent0.resize(fNumSlots);
   fPrevCountSent1.resize(fNumSlots);

   fThrMin.resize(fNumSlots);
   fThrMax.resize(fNumSlots);
   fThrAve.resize(fNumSlots);
   fCountThr0.resize(fNumSlots);
   fCountThr1.resize(fNumSlots);
   fPrevCountThr0.resize(fNumSlots);
   fPrevCountThr1.resize(fNumSlots);

   fCountErrors.resize(fNumSlots);

   fPwbScaFifoMaxUsed.resize(fNumSlots);
   fPwbEventFifoWrUsed.resize(fNumSlots);
   fPwbEventFifoRdUsed.resize(fNumSlots);
   fPwbEventFifoWrMaxUsed.resize(fNumSlots);
   fPwbEventFifoRdMaxUsed.resize(fNumSlots);
   fPwbEventFifoOverflows.resize(fNumSlots);

   fPrevTime = 0;

   cm_msg(MINFO, "Evb::Evb", "Evb: configured %d slots: %d TRG, %d ADC, %d TDC, %d PWB", fNumSlots, count_trg, count_adc, count_tdc, count_pwb);

   ResetPerSecond();
   WriteSyncStatus(status);
   WriteEvbStatus(status);

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
   printf("  Slot errors: %d\n", fCountSlotErrors);
#if 0
   printf("  Incomplete count for each slot:\n");
   for (unsigned i=0; i<fCountSlotIncomplete.size(); i++) {
      if (fCountSlotIncomplete[i] > 0) {
         printf("    slot %d, module %s: incomplete count: %d\n", i, fSlotName[i].c_str(), fCountSlotIncomplete[i]);
      }
   }
#endif
#if 1
   for (unsigned i=0; i<fPwbData.size(); i++) {
      const PwbData* d = &fPwbData[i];
      if (d->count_error > 0) {
         printf("slot %d: PWB counters: bad_pkt_seq %d, bad_channel_id %d, bad_format_revision %d, bad_chunk_id %d, lost_header %d, lost_footer %d\n",
                i,
                d->count_bad_pkt_seq,
                d->count_bad_channel_id,
                d->count_bad_format_revision,
                d->count_bad_chunk_id,
                d->count_lost_header,
                d->count_lost_footer);
         //uint16_t chunk_id[MAX_PWB_CHAN];
      }
   }
#endif
#if 0
   LogPwbCounters();
#endif
   printf("  Max dt: %.0f ns\n", fMaxDt*1e9);
   printf("  Min dt: %.0f ns\n", fMinDt*1e9);
}

void Evb::PrintEvents() const
{
   printf("Evb dump of buffered events, fEvents size is %d:\n", (int)fEvents.size());
   for (unsigned i=0; i<fEvents.size(); i++) {
      printf("slot %d: ", i);
      fEvents[i]->Print();
      printf("\n");
   }
}

void Evb::LogPwbCounters() const
{
   for (unsigned i=0; i<fPwbData.size(); i++) {
      const PwbData* d = &fPwbData[i];
      if (d->count_error > 0) {
         cm_msg(MINFO, "LogPwbCounters", "slot %d: PWB counters: errors: %d: bad_pkt_seq %d, bad_channel_id %d, bad_format_revision %d, bad_chunk_id %d, lost_header %d, lost_footer %d",
                i,
                d->count_error,
                d->count_bad_pkt_seq,
                d->count_bad_channel_id,
                d->count_bad_format_revision,
                d->count_bad_chunk_id,
                d->count_lost_header,
                d->count_lost_footer);
         //uint16_t chunk_id[MAX_PWB_CHAN];
      }
   }
}

void Evb::WriteSyncStatus(MVOdb* odb) const
{
   odb->WI("sync_min", fSync.fMin);
   odb->WI("sync_max", fSync.fMax);
   odb->WB("sync_ok",  fSync.fSyncOk);
   odb->WB("sync_failed",   fSync.fSyncFailed);
   odb->WB("sync_overflow", fSync.fOverflow);
}

void Evb::WriteEvbStatus(MVOdb* odb) const
{
   odb->WI("events_in",        fCountInput);
   odb->WI("complete",         fCountComplete);
   odb->WI("incomplete",       fCountIncomplete);
   odb->WI("pop_age",          fCountPopAge);
   odb->WI("pop_following",    fCountPopFollowingComplete);
   odb->WI("pop_last",         fCountPopLast);
   odb->WI("error",            fCountError);
   odb->WI("per_slot_errors",  fCountSlotErrors);
   odb->WI("bypass",           fCountBypass);
   odb->WI("count_dead_slots", fCountDeadSlots);
   odb->WSA("names", fSlotName, 32);
   odb->WIA("dead", fDeadSlots);
   odb->WIA("incomplete_count", fCountSlotIncomplete);
   odb->WDA("skew_time", fSkewTimeSec);
   odb->WDA("packets_count", fCountPackets);
   odb->WDA("bytes_count", fCountBytes);
   odb->WDA("packets_per_second", fPacketsPerSec);
   odb->WDA("bytes_per_second", fBytesPerSec);
   odb->WIA("sent_min", fSentMin);
   odb->WIA("sent_max", fSentMax);
   odb->WDA("sent_ave", fSentAve);
   odb->WIA("thr_min", fThrMin);
   odb->WIA("thr_max", fThrMax);
   odb->WDA("thr_ave", fThrAve);
   odb->WIA("errors", fCountErrors);
   odb->WIA("pwb_sca_fifo_max_used", fPwbScaFifoMaxUsed);
   odb->WIA("pwb_event_fifo_wr_max_used", fPwbEventFifoWrMaxUsed);
   odb->WIA("pwb_event_fifo_rd_max_used", fPwbEventFifoRdMaxUsed);
   odb->WIA("pwb_event_fifo_wr_used", fPwbEventFifoWrUsed);
   odb->WIA("pwb_event_fifo_rd_used", fPwbEventFifoRdUsed);
   odb->WIA("pwb_event_fifo_overflows", fPwbEventFifoOverflows);
   
   //for (unsigned i=0; i<fPwbEventFifoWrMaxUsed.size(); i++) {
   //   fPwbEventFifoWrMaxUsed[i] = fPwbEventFifoWrUsed[i];
   //   fPwbEventFifoRdMaxUsed[i] = fPwbEventFifoRdUsed[i];
   //}
}

void Evb::WriteVariables(MVOdb* odb)
{
   odb->WD("skew_sec", fSkewMax);
   fSkewMax = 0;
   odb->WD("skew_max_sec", fSkewMaxEver);

   odb->WD("pwb_sca_fifo_max_used", fPwbScaFifoMaxUsedEver);
   odb->WD("pwb_event_fifo_used", fPwbEventFifoWrMaxUsedAll);
   fPwbEventFifoWrMaxUsedAll = 0;
   odb->WD("pwb_event_fifo_max_used", fPwbEventFifoWrMaxUsedEver);

   odb->WD("pwb_event_fifo_overflows", fPwbEventFifoOverflowsAll);
   fPwbEventFifoOverflowsAll = 0;
   odb->WD("pwb_event_fifo_overflows_sum", fPwbEventFifoOverflowsEver);

   odb->WD("event_size", fMaxEventSize);
   fMaxEventSize = 0;
   odb->WD("event_size_max", fMaxEventSizeEver);

   odb->WD("event_count", fEventCount);
   fEventCount = 0;

   odb->WD("byte_count", fByteCount);
   fByteCount = 0;
}

void Evb::ResetPerSecond()
{
   double now = TMFE::GetTime();
   fPrevTime = now;
   for (unsigned i=0; i<fNumSlots; i++) {
      fPacketsPerSec[i] = 0;
      fBytesPerSec[i] = 0;
      fPrevCountPackets[i] = fCountPackets[i];
      fPrevCountBytes[i] = fCountBytes[i];
      fPrevCountSent0[i] = fCountSent0[i];
      fPrevCountSent1[i] = fCountSent1[i];
      fPrevCountThr0[i] = fCountThr0[i];
      fPrevCountThr1[i] = fCountThr1[i];
   }
}

void Evb::ComputePerSecond()
{
   double now = TMFE::GetTime();
   double elapsed = now - fPrevTime;
   fPrevTime = now;

   for (unsigned i=0; i<fNumSlots; i++) {
      double dp = fCountPackets[i] - fPrevCountPackets[i];
      double db = fCountBytes[i] - fPrevCountBytes[i];

      fPacketsPerSec[i] = dp/elapsed;
      fBytesPerSec[i] = db/elapsed;

      fPrevCountPackets[i] = fCountPackets[i];
      fPrevCountBytes[i] = fCountBytes[i];

      double ds0 = fCountSent0[i] - fPrevCountSent0[i];
      double ds1 = fCountSent1[i] - fPrevCountSent1[i];

      double ave = 0;
      if (ds0 >= 1)
         ave = ds1/ds0;

      fSentAve[i] = ave;

      fPrevCountSent0[i] = fCountSent0[i];
      fPrevCountSent1[i] = fCountSent1[i];

      double dt0 = fCountThr0[i] - fPrevCountThr0[i];
      double dt1 = fCountThr1[i] - fPrevCountThr1[i];

      double avet = 0;
      if (dt0 >= 1)
         avet = dt1/dt0;

      fThrAve[i] = avet;

      fPrevCountThr0[i] = fCountThr0[i];
      fPrevCountThr1[i] = fCountThr1[i];
   }
}

EvbEvent* Evb::FindEvent(double t, int index, EvbEventBuf *m)
{
   double amin = 0;
   
   if (fEvents.size() > 0) {
      for (unsigned i=fEvents.size()-1; ; i--) {
         //printf("find event for time %f: event %d, %f, diff %f\n", t, i, fEvents[i]->time, fEvents[i]->time - t);
         
         double dt = fEvents[i]->time - t;
         double adt = fabs(dt);
         
         if (adt < fEpsSec) {
            if (adt > fMaxDt) {
               //printf("AgEVB: for time %f found event at time %f, new max dt %.0f ns, old max dt %.0f ns\n", t, fEvents[i]->time, adt*1e9, fMaxDt*1e9);
               fMaxDt = adt;
            }
            //printf("Found event for time %f\n", t);
            //printf("Found event for time %f: event %d of %d, %f, diff %f %.0f ns\n", t, i, fEvents.size(), fEvents[i]->time, dt, dt*1e9);
            return fEvents[i];
         }
         
         if (amin == 0)
            amin = adt;
         if (adt < amin)
            amin = adt;
         
         if (i==0)
            break;
      }
   }

   if (0) {
      printf("Creating new event for time %f, already buffered events do not match this time:\n", t);

      for (unsigned i=0; i<fEvents.size(); i++) {
         printf("Slot %d: ", i);
         //printf("find event for time %f: event %d, %f, diff %f\n", t, i, fEvents[i]->time, fEvents[i]->time - t);
         fEvents[i]->Print();
         printf("\n");
      }
   }
   
   if (fMinDt == 0)
      fMinDt = amin;

   if (amin < fMinDt)
      fMinDt = amin;
   
   EvbEvent* e = new EvbEvent();
   e->fCreateTime = TMFE::GetTime();
   e->fUpdateTime = e->fCreateTime;
   e->complete = false;
   e->error = false;
   e->counter = fCounter++;
   e->time = t;

   assert(e->banks_count.size() == 0);
   assert(e->banks_waiting.size() == 0);
   for (unsigned i=0; i<fNumBanks.size(); i++) {
      e->banks_count.push_back(0);
      e->banks_waiting.push_back(fNumBanks[i]);
   }
   
   fEvents.push_back(e);

   unsigned fEventsSize = fEvents.size();
   if (fEventsSize > fMaxEventsSize) {
      fMaxEventsSize = fEventsSize;
   }

   if (0) {
      printf("New event for time %f, index %2d: ", t, index);
      m->Print();
      printf("\n");
   }
   
   return e;
}

void Evb::CheckEvent(EvbEvent *e, bool last_event)
{
   assert(e);

   if (!e->maybe_complete)
      return;

   assert(e->banks);
   assert(e->banks_count.size() == fNumBanks.size());
   assert(e->banks_waiting.size() == fNumBanks.size());

   if (0) {
      printf("check event: ");
      e->Print();
      printf("\n");
   }

   bool complete = true;

   int num_slots = fNumBanks.size();
   for (int i=0; i<num_slots; i++) {
      if (fSync.fModules[i].fDead) {
         continue;
      }

      if (e->banks_waiting[i] > 0) {
         if (last_event && gKludgeTdcLastEvent && fSlotType[i] == 6) {
            cm_msg(MINFO, "Evb::CheckEvent", "Kludge: ignoring lack of TDC data in the last event");
            gKludgeTdcLastEvent = false;
         } else {
            complete = false;
         }
      }

      if (0) {
         printf("slot %d: type %d, should have %d, have %d, waiting %d, complete %d\n", i, fSlotType[i], fNumBanks[i], e->banks_count[i], e->banks_waiting[i], complete);
      }

      if (!complete)
         break;
   }

   e->complete = complete;
   e->maybe_complete = false;

   //e->Print();
}

void Evb::BuildSlot(int slot, EvbEventBuf *m)
{
   m->time = fSync.fModules[slot].GetTime(m->ts, m->epoch);

   if (0 && slot==3) {
      printf("time %f\n", m->time);
   }

#if 0
   static double gLastTime = 0;
   if (!gLastTime)
      gLastTime = m->time;

   if (fabs(gLastTime - m->time) > 10.0) {
      printf("crazy time %f after %f, slot %d, eventbuf: ", m->time, gLastTime, index);
      m->Print();
      printf("\n");
   } else {
      gLastTime = m->time;
   }

   if (fabs(m->timeIncr) > 10.0) {
      printf("crazy incr %f after %f, slot %d, eventbuf: ", m->time, gLastTime, index);
      m->Print();
      //if (m->buf) {
      //   printf(", bank %s", (*m->buf)[0]->name.c_str());
      //}
      if (m->bank) {
         printf(", bank %s", m->bank->name.c_str());
      }
      printf("\n");
   }
#endif

   EvbEvent* e = FindEvent(m->time, slot, m);

   assert(e);

#if 0
   if (0 && index == 1) {
      printf("offset: %f %f, index %d, ts 0x%08x, epoch %d, feam time %f\n", e->time, m->time, index, m->ts, m->epoch, m->feam->time);
   }
#endif

   if (fClockDrift) { // adjust offset for clock drift
      double off = e->time - m->time;
      //printf("offset: %f %f, diff %f, index %d\n", e->time, m->time, off, slot);
      fSync.fModules[slot].fOffsetSec += off/2.0;
   }

   e->MergeSlot(slot, m);

   CheckEvent(e, false);
}

void Evb::Build()
{
   //DWORD t1 = ss_millitime();
   //int loops = 0;
   int num_slots = fBuf.size();
   for (int slot=0; slot<num_slots; slot++) {
      while (!fBuf[slot].empty()) {
         EvbEventBuf* m = fBuf[slot].front();
         fBuf[slot].pop_front();
         BuildSlot(slot, m);
         //loops++;
      }
   }
   //DWORD t2 = ss_millitime();
   //DWORD dt = t2-t1;
   //if (dt > 1) {
   //   printf("Build() took %d ms, %d loops\n", dt, loops);
   //}
}

void Evb::CheckDeadSlots()
{
   double maxtime = 0;

   for (unsigned slot=0; slot<fNumSlots; slot++) {
      if (fSync.fModules[slot].fDead)
         continue;
      if (fLastTimeSec[slot]) {
         if (fLastTimeSec[slot] > maxtime)
            maxtime = fLastTimeSec[slot];
      }
   }

   for (unsigned slot=0; slot<fNumSlots; slot++) {
      if (fSync.fModules[slot].fDead) {
         fSkewTimeSec[slot] = 0; // mark dead slot
      } else {
         fSkewTimeSec[slot] = maxtime - fLastTimeSec[slot];

         if (fSkewTimeSec[slot] > fSkewMax)
            fSkewMax = fSkewTimeSec[slot];

         if (fSkewTimeSec[slot] > fSkewMaxEver)
            fSkewMaxEver = fSkewTimeSec[slot];
         
         if (fSkewTimeSec[slot] > fMaxDeadSec) {
            cm_msg(MERROR, "Evb::CheckDeadSlots", "Slot %d (%s) is now dead, no packets in %.3f sec", slot, fSlotName[slot].c_str(), fSkewTimeSec[slot]);
            fSync.fModules[slot].fDead = true;
         }
      }
   }
}

void Evb::UpdateCounters(const EvbEvent* e)
{
   fCount++;
   if (e->error) {
      if (fCountError == 0) {
         cm_msg(MERROR, "Evb::UpdateCounters", "First event event with errors");
      }
      fCountError++;
   }

   if (e->complete) {
      fCountComplete++;
   } else {
      if (fCountIncomplete == 0) {
         cm_msg(MERROR, "Evb::UpdateCounters", "First incomplete event");
      }

      fCountIncomplete++;

      for (unsigned i=0; i<fNumSlots; i++) {
         if (fSync.fModules[i].fDead) {
            fDeadSlots[i] = true;
            continue;
         }

         if (e->banks_waiting[i] > 0) {
            fCountSlotIncomplete[i]++;
         }

         if (0) {
            printf("slot %d: type %d, should have %d, have %d\n", i, fSlotType[i], fNumBanks[i], e->banks_count[i]);
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

   fCountPopLast += 1;

   CheckEvent(e, true);
   UpdateCounters(e);
   return e;
}

EvbEvent* Evb::GetNext()
{
   EvbEvent* e = fEvents.front();

   // top of queue has a complete event, return it!

   if (e->complete) {
      fEvents.pop_front();
      return e;
   }

   double now = TMFE::GetTime();
   double age = now - e->fUpdateTime;

   if (age > fMaxAgeSec) {
      // this event has not been updated for too long,
      // it will never become complete, pop it out!
      fEvents.pop_front();

      if (fPrintIncomplete) {
         printf("Evb::GetNext: age %.3f sec, popping this incomplete event!\n", age);
         e->Print();
         printf("\n");
      }

      fCountPopAge += 1;
      
      return e;
   }

   // top of queue has an incomplete event,
   // check if there is a complete event down the line

   for (unsigned i=0; i<fEvents.size(); i++) {
      if (fEvents[i]->complete) {
         // a later event has been completed,
         // this event will never become complete.
         fEvents.pop_front();

         if (fPrintIncomplete) {
            printf("Evb::GetNext: found complete event at %d, popping this incomplete event!\n", i);
            e->Print();
            printf("\n");
         }

         fCountPopFollowingComplete += 1;

         return e;
      }
   }

   // marinade this event for some more
   e = NULL;
   return NULL;
}

EvbEvent* Evb::Get()
{
   //DWORD t1 = ss_millitime();
   if (fSync.fSyncOk)
      Build();
   //DWORD t2 = ss_millitime();
   
   if (fEvents.size() < 1) {
      //DWORD dt = t2-t1;
      //printf("Get() t1t2 %d\n", t2-t1);
      return NULL;
   }

   if (fTrace) {
      printf("Evb::Get: ");
      Print();
      printf("\n");
   }
   
   EvbEvent* e = GetNext();

   if (!e) {
      return NULL;
   }

   if (fPrintAll) {
      e->Print();
      printf("\n");
   }
   
   //DWORD t3 = ss_millitime();
   //fEvents.pop_front();
   UpdateCounters(e);
   //DWORD t4 = ss_millitime();
   //DWORD dt = t4-t1;
   //if (dt > 1)
   //   printf("Get() t1t2 %d, t2t3 %d, t3t4 %d, dt %d\n", t2-t1, t3-t2, t4-t3, dt);
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
   //m->buf = new FragmentBuf;
   //m->buf->push_back(b);
   m->ts = fSync.fModules[imodule].fLastTs;
   m->epoch = fSync.fModules[imodule].fEpoch;
   m->time = 0;
   m->timeIncr = fSync.fModules[imodule].fLastTimeSec - fSync.fModules[imodule].fPrevTimeSec;
   m->bank = b;

   if (0 && imodule==3) {
      printf("slot %2d, ts 0x%08x, epoch %d, time %f, incr %f\n", imodule, m->ts, m->epoch, m->time, m->timeIncr);
   }

   fBuf[imodule].push_back(m);
}

std::vector<BankBuf*> gBankBuf;

void XAddBank(int islot, uint32_t ts, BankBuf* b)
{
   b->xslot = islot;
   b->xts = ts;
   gBankBuf.push_back(b);
}

bool XFlushBank(Evb* evb)
{
   bool flushed_at_least_one = false;
   std::lock_guard<std::mutex> lock(evb->fLock);
   int size = gBankBuf.size();
   for (int i=0; i<size; i++) {
      BankBuf* b = gBankBuf[i];
      gBankBuf[i] = NULL;
      evb->AddBank(b->xslot, b->xts, b);
      flushed_at_least_one = true;
   }
   gBankBuf.clear();
   return flushed_at_least_one;
}

bool AddAdcBank(Evb* evb, int imodule, const void* pbank, int bklen)
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
   
   int xmodule = imodule;

   if (info.channelType == 128) {
      xmodule += 100;
   }

   int islot = get_vector_element(evb->fAdcSlot, xmodule);

   //printf("adc module %d slot %d\n", imodule, islot);

   if (islot < 0) {
      return false;
   }

   char cname = 0;
   if (info.channelId <= 9) {
      cname = '0' + info.channelId;
   } else {
      cname = 'A' + info.channelId - 10;
   }

   // FIXME: not locked!
   evb->fLastTimeSec[islot] = TMFE::GetTime();
   evb->fCountPackets[islot] += 1;
   evb->fCountBytes[islot] += bklen;

   char newname[5];

   if (info.channelType == 0) {
      sprintf(newname, "%c%02d%c", 'B', imodule, cname);
      //printf("bank name [%s]\n", newname);
   } else if (info.channelType == 128) {
      sprintf(newname, "%c%02d%c", 'C', imodule, cname);
   } else {
      sprintf(newname, "XX%02d", imodule);
   }

#if 0
   if (info.channelType == 128) {
      printf("bank %s islot %d imodule %d xmodule %d channel %d timestamp 0x%08x\n", newname, islot, imodule, xmodule, info.channelId, info.eventTimestamp);
   }
#endif

   BankBuf *b = new BankBuf(newname, TID_BYTE, pbank, bklen, 1);

   XAddBank(islot, info.eventTimestamp, b);

   return true;
};

int CountBits(uint32_t bitmap)
{
   int count = 0;
   while (bitmap != 0) {
      if (bitmap & 1)
         count++;
      bitmap>>=1;
   }
   return count;
}

bool AddPwbBank(Evb* evb, int imodule, const char* bkname, const char* pbank, int bklen, int bktype)
{
   int jslot = get_vector_element(evb->fPwbSlot, imodule);

   if (jslot < 0) {
      return false;
   }

   const uint32_t *p32 = (const uint32_t*)pbank;
   const int n32 = bklen/4;

   if (0) {
      unsigned nprint = n32;
      nprint=10;
      for (unsigned i=0; i<nprint; i++) {
         printf("PB05[%d]: 0x%08x (%d)\n", i, p32[i], p32[i]);
         //e->udpData.push_back(p32[i]);
      }
   }

   uint32_t DEVICE_ID   = p32[0];
   uint32_t PKT_SEQ     = p32[1];
   uint32_t CHANNEL_SEQ = (p32[2] >>  0) & 0xFFFF;
   uint32_t CHANNEL_ID  = (p32[2] >> 16) & 0xFF;
   uint32_t FLAGS       = (p32[2] >> 24) & 0xFF;
   uint16_t CHUNK_ID    = (p32[3] >>  0) & 0xFFFF;
   uint32_t CHUNK_LEN   = (p32[3] >> 16) & 0xFFFF;
   uint32_t HEADER_CRC  = p32[4];
   uint32_t end_of_payload = 5*4 + CHUNK_LEN;
   uint32_t payload_crc = p32[end_of_payload/4];
   
   if (0) {
      printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, LEN 0x%04x, CRC 0x%08x, bank bytes %d, end of payload %d, CRC 0x%08x\n",
             DEVICE_ID,
             PKT_SEQ,
             CHANNEL_SEQ,
             CHANNEL_ID,
             FLAGS,
             CHUNK_ID,
             CHUNK_LEN,
             HEADER_CRC,
             bklen,
             end_of_payload,
             payload_crc);
   }

   PwbData* dj = &evb->fPwbData[jslot];

   bool bad_pkt_seq = false;

   if (PKT_SEQ < dj->pkt_seq) {
      printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x -- Error: PKT_SEQ jump 0x%08x to 0x%08x\n",
             DEVICE_ID,
             PKT_SEQ,
             CHANNEL_SEQ,
             CHANNEL_ID,
             FLAGS,
             CHUNK_ID,
             dj->pkt_seq,
             PKT_SEQ);
      //d->count_bad_pkt_seq++;
      //d->count_error++;
      //bad_pkt_seq = true;
      cm_msg(MERROR, "AddPwbBank", "UDP packet out of order or counter wraparound: 0x%08x -> 0x%08x", dj->pkt_seq, PKT_SEQ);
   }

   if (dj->pkt_seq != 0) {
      if (dj->pkt_seq+1 != PKT_SEQ) {
         printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x -- Error: PKT_SEQ jump 0x%08x to 0x%08x\n",
                DEVICE_ID,
                PKT_SEQ,
                CHANNEL_SEQ,
                CHANNEL_ID,
                FLAGS,
                CHUNK_ID,
                dj->pkt_seq,
                PKT_SEQ);
         dj->count_bad_pkt_seq++;
         dj->count_error++;
         evb->fCountErrors[jslot]++;
         evb->fCountSlotErrors++; // FIXME: not locked!
         bad_pkt_seq = true;
         if (dj->count_bad_pkt_seq < 100) {
            cm_msg(MERROR, "AddPwbBank", "slot %d name %s: wrong PWB UDP packet order: 0x%08x -> 0x%08x, %d missing? count %d", jslot, evb->fSlotName[jslot].c_str(), dj->pkt_seq, PKT_SEQ, PKT_SEQ-dj->pkt_seq-1, dj->count_bad_pkt_seq);
         }
      }
   }

   dj->pkt_seq = PKT_SEQ;

   if (CHANNEL_ID > 3) {
      printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x -- Error: invalid CHANNEL_ID\n",
             DEVICE_ID,
             PKT_SEQ,
             CHANNEL_SEQ,
             CHANNEL_ID,
             FLAGS,
             CHUNK_ID);
      dj->count_bad_channel_id++;
      dj->count_error++;
      evb->fCountErrors[jslot]++;
      evb->fCountSlotErrors++; // FIXME: no locked!
      return false;
   }
   
   int islot = jslot + CHANNEL_ID;
   
   // FIXME: not locked!
   evb->fLastTimeSec[islot] = TMFE::GetTime();
   evb->fCountPackets[islot] += 1;
   evb->fCountBytes[islot] += bklen;

   //printf("pwb module %d slot %d/%d\n", imodule, jslot, islot);

   PwbData* d = &evb->fPwbData[islot];

   if (0) {
      printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x\n",
             DEVICE_ID,
             PKT_SEQ,
             CHANNEL_SEQ,
             CHANNEL_ID,
             FLAGS,
             CHUNK_ID);
   }
   
   bool trace = false;

   uint32_t ts = 0;

   int waiting_incr = 0;

   if (CHUNK_ID == 0) {
      if (0) {
         for (unsigned i=5; i<20; i++) {
            printf("PB05[%d]: 0x%08x (%d)\n", i, p32[i], p32[i]);
            //e->udpData.push_back(p32[i]);
         }
      }
      
      int FormatRevision  = (p32[5]>> 0) & 0xFF;
      //int ScaId           = (p32[5]>> 8) & 0xFF;
      //int CompressionType = (p32[5]>>16) & 0xFF;
      //int TriggerSource   = (p32[5]>>24) & 0xFF;

      uint32_t TriggerTimestamp1 = 0;
      uint32_t ScaChannelsSent1 = 0;
      uint32_t ScaChannelsSent2 = 0;
      uint32_t ScaChannelsSent3 = 0;
      uint32_t ScaChannelsThreshold1 = 0;
      uint32_t ScaChannelsThreshold2 = 0;
      uint32_t ScaChannelsThreshold3 = 0;
      
      if ((FormatRevision == 0) || (FormatRevision == 1)) {
         //uint32_t HardwareId1 = p32[6];
         //
         //uint32_t HardwareId2 = (p32[7]>> 0) & 0xFFFF;
         //int TriggerDelay     = (p32[7]>>16) & 0xFFFF;
         
         // NB timestamp clock is 125 MHz
         
         TriggerTimestamp1 = p32[8];
         
         //uint32_t TriggerTimestamp2 = (p32[9]>> 0) & 0xFFFF;
         //uint32_t Reserved1         = (p32[9]>>16) & 0xFFFF;
         
         //int ScaLastCell = (p32[10]>> 0) & 0xFFFF;
         //int ScaSamples  = (p32[10]>>16) & 0xFFFF;
         
         ScaChannelsSent1 = p32[11];
         ScaChannelsSent2 = p32[12];
         
         ScaChannelsSent3 = (p32[13]>> 0) & 0xFFFF;
         ScaChannelsThreshold1 = (p32[13]>>16) & 0xFFFF;
         
         ScaChannelsThreshold1 |= ((p32[14] & 0xFFFF) << 16) & 0xFFFF0000;
         ScaChannelsThreshold2 = (p32[14]>>16) & 0xFFFF;
         
         ScaChannelsThreshold2 |= ((p32[15] & 0xFFFF) << 16) & 0xFFFF0000;
         ScaChannelsThreshold3 = (p32[15]>>16) & 0xFFFF;
      } else if ((FormatRevision == 2)) {
         const uint32_t *w32 = p32+4;
         TriggerTimestamp1 = w32[4];

         ScaChannelsSent1 = w32[7];
         ScaChannelsSent2 = w32[8];
         ScaChannelsSent3 = (w32[9]>> 0) & 0xFFFF;

         ScaChannelsThreshold1 = (w32[9]>>16) & 0xFFFF;
         ScaChannelsThreshold1 |= ((w32[10] & 0xFFFF) << 16) & 0xFFFF0000;
         ScaChannelsThreshold2 = (w32[10]>>16) & 0xFFFF;
         ScaChannelsThreshold2 |= ((w32[11] & 0xFFFF) << 16) & 0xFFFF0000;
         ScaChannelsThreshold3 = (w32[11]>>16) & 0xFFFF;

         uint32_t w13 = w32[13];
         int ScaFifoMaxUsed  = (w13 & 0x0000FFFF);
         int EventFifoWrUsed = (w13 & 0xFF000000) >> 24;
         int EventFifoRdUsed = (w13 & 0x00FF0000) >> 16;

         evb->fPwbScaFifoMaxUsed[islot] = ScaFifoMaxUsed;
         if (ScaFifoMaxUsed > evb->fPwbScaFifoMaxUsedEver)
            evb->fPwbScaFifoMaxUsedEver = ScaFifoMaxUsed;

         evb->fPwbEventFifoWrUsed[islot] = EventFifoWrUsed;
         if (EventFifoWrUsed > evb->fPwbEventFifoWrMaxUsed[islot])
            evb->fPwbEventFifoWrMaxUsed[islot] = EventFifoWrUsed;
         if (EventFifoWrUsed > evb->fPwbEventFifoWrMaxUsedAll)
            evb->fPwbEventFifoWrMaxUsedAll = EventFifoWrUsed;
         if (EventFifoWrUsed > evb->fPwbEventFifoWrMaxUsedEver)
            evb->fPwbEventFifoWrMaxUsedEver = EventFifoWrUsed;

         evb->fPwbEventFifoRdUsed[islot] = EventFifoRdUsed;
         if (EventFifoRdUsed > evb->fPwbEventFifoRdMaxUsed[islot])
            evb->fPwbEventFifoRdMaxUsed[islot] = EventFifoRdUsed;
         //if (EventFifoRdUsed > evb->fPwbEventFifoRdMaxUsedAll)
         //   evb->fPwbEventFifoRdMaxUsedAll = EventFifoRdUsed;

         if (EventFifoWrUsed == 31) { // overflow
            evb->fPwbEventFifoOverflows[islot] += 1;
            evb->fPwbEventFifoOverflowsAll += 1;
            evb->fPwbEventFifoOverflowsEver += 1;
         }

         //printf("module %d, slot %d, word 13: 0x%08x, sca fifo max used: %d, event fifo used: %d, %d, max used: %d, %d\n", imodule, islot, w13, ScaFifoMaxUsed, EventFifoWrUsed, EventFifoRdUsed, evb->fPwbEventFifoWrMaxUsed[islot], evb->fPwbEventFifoRdMaxUsed[islot]);
      } else {
         printf("Error: invalid format revision %d\n", FormatRevision);
         d->count_bad_format_revision++;
         d->count_error++;
         evb->fCountErrors[islot]++;
         evb->fCountSlotErrors++; // FIXME: no locked!
         return false;
      }

      ts = TriggerTimestamp1;
      
      d->cnt = 1;
      d->ts  = ts;

#if 1
      int sent_bits = CountBits(ScaChannelsSent1) + CountBits(ScaChannelsSent2) + CountBits(ScaChannelsSent3);
      int threshold_bits = CountBits(ScaChannelsThreshold1) + CountBits(ScaChannelsThreshold2) + CountBits(ScaChannelsThreshold3);

      d->sent_bits = sent_bits;
      d->threshold_bits = threshold_bits;

      //printf("sent_bits: 0x%08x 0x%08x 0x%08x -> %d bits, threshold bits %d\n", ScaChannelsSent1, ScaChannelsSent2, ScaChannelsSent3, sent_bits, threshold_bits);
      
      // FIXME: not locked!
      if (sent_bits > evb->fSentMax[islot])
         evb->fSentMax[islot] = sent_bits;
      if ((evb->fSentMin[islot] == 0) || (sent_bits < evb->fSentMin[islot]))
         evb->fSentMin[islot] = sent_bits;
      evb->fCountSent0[islot] += 1;
      evb->fCountSent1[islot] += sent_bits;

      if (threshold_bits > evb->fThrMax[islot])
         evb->fThrMax[islot] = threshold_bits;
      if ((evb->fThrMin[islot] == 0) || (threshold_bits < evb->fThrMin[islot]))
         evb->fThrMin[islot] = threshold_bits;
      evb->fCountThr0[islot] += 1;
      evb->fCountThr1[islot] += threshold_bits;
#endif

      if (d->chunk_id != 0) {
         printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x -- Error: last chunk_id 0x%04x, lost event footer\n",
                DEVICE_ID,
                PKT_SEQ,
                CHANNEL_SEQ,
                CHANNEL_ID,
                FLAGS,
                CHUNK_ID,
                d->chunk_id);
         d->count_lost_footer++;
         d->count_error++;
         evb->fCountErrors[islot]++;
         evb->fCountSlotErrors++; // FIXME: not locked!
      }

      d->chunk_id = 0;

      if (trace) {
         printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, TS 0x%08x\n",
                DEVICE_ID,
                PKT_SEQ,
                CHANNEL_SEQ,
                CHANNEL_ID,
                FLAGS,
                CHUNK_ID,
                TriggerTimestamp1
                );
      }

      if (0) {
         printf("slot %2d, bank %s, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, TS 0x%08x header\n",
                islot,
                bkname,
                PKT_SEQ,
                CHANNEL_SEQ,
                CHANNEL_ID,
                FLAGS,
                CHUNK_ID,
                TriggerTimestamp1
                );
      }

#if 0      
      if (0) {
         printf("H F 0x%02x, Sca 0x%02x, C 0x%02x, T 0x%02x, H 0x%08x, 0x%04x, Delay 0x%04x, TS 0x%08x, 0x%04x, R1 0x%04x, SCA LastCell 0x%04x, Samples 0x%04x, Sent 0x%08x 0x%08x 0x%08x, Thr 0x%08x 0x%08x 0x%08x, R2 0x%04x\n",
                FormatRevision,
                ScaId,
                CompressionType,
                TriggerSource,
                HardwareId1, HardwareId2,
                TriggerDelay,
                TriggerTimestamp1, TriggerTimestamp2,
                Reserved1,
                ScaLastCell,
                ScaSamples,
                ScaChannelsSent1,
                ScaChannelsSent2,
                ScaChannelsSent3,
                ScaChannelsThreshold1,
                ScaChannelsThreshold2,
                ScaChannelsThreshold3,
                Reserved2);
      }
#endif
   } else {
      ts = d->ts;
      d->cnt++;
      if (!bad_pkt_seq) {
         if (CHUNK_ID <= d->chunk_id) {
            printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x -- Error: last chunk_id 0x%04x, lost event header\n",
                   DEVICE_ID,
                   PKT_SEQ,
                   CHANNEL_SEQ,
                   CHANNEL_ID,
                   FLAGS,
                   CHUNK_ID,
                   d->chunk_id);
            d->count_lost_header++;
            d->count_error++;
            evb->fCountErrors[islot]++;
            evb->fCountSlotErrors++; // FIXME: not locked!
         } else if (CHUNK_ID != d->chunk_id+1) {
            printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x -- Error: bad CHUNK_ID, last chunk_id 0x%04x\n",
                   DEVICE_ID,
                   PKT_SEQ,
                   CHANNEL_SEQ,
                   CHANNEL_ID,
                   FLAGS,
                   CHUNK_ID,
                   d->chunk_id);
            d->count_bad_chunk_id++;
            d->count_error++;
            evb->fCountErrors[islot]++;
            evb->fCountSlotErrors++; // FIXME: not locked!
         }
      }
      d->chunk_id = CHUNK_ID;

      if (0) {
         printf("slot %2d, bank %s, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, TS 0x%08x\n",
                islot,
                bkname,
                PKT_SEQ,
                CHANNEL_SEQ,
                CHANNEL_ID,
                FLAGS,
                CHUNK_ID,
                ts
                );
      }
   }

   if (FLAGS & 1) {
      if (trace) {
         printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, TS 0x%08x, LAST of %d packets\n",
                DEVICE_ID,
                PKT_SEQ,
                CHANNEL_SEQ,
                CHANNEL_ID,
                FLAGS,
                CHUNK_ID,
                d->ts,
                d->cnt);
      }
      d->cnt = 0;
      d->ts = 0;
      d->chunk_id = 0;
      waiting_incr = 1;
   } else {
      if (0 && trace) {
         printf("ID 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, TS 0x%08x, count %d\n",
                DEVICE_ID,
                PKT_SEQ,
                CHANNEL_SEQ,
                CHANNEL_ID,
                FLAGS,
                CHUNK_ID,
                d->ts,
                d->cnt);
      }
   }
      
   if (0) {
      printf("PWB timestamp 0x%08x\n", ts);
      return false;
   }

   if (ts == 0) {
      return false;
   }

   char nbkname[5];
   nbkname[0] = 'P';
   nbkname[1] = 'C';
   nbkname[2] = bkname[2];
   nbkname[3] = bkname[3];
   nbkname[4] = 0;

   BankBuf *b = new BankBuf(nbkname, TID_BYTE, pbank, bklen, waiting_incr);

   XAddBank(islot, ts, b);
   
   return true;
}

bool AddTrgBank(Evb* evb, const char* bkname, const char* pbank, int bklen, int bktype)
{
   //printf("AddTrgBank: name [%s] len %d type %d, tid_size %d\n", bkname, bklen, bktype, rpc_tid_size(bktype));

   AlphaTPacket p;
   p.Unpack(pbank, bklen);

   //p.Print();
   //printf("\n");

   int imodule = 1;

   int islot = get_vector_element(evb->fTrgSlot, imodule);

   if (islot < 0) {
      return false;
   }

   // FIXME: not locked!
   evb->fLastTimeSec[islot] = TMFE::GetTime();
   evb->fCountPackets[islot] += 1;
   evb->fCountBytes[islot] += bklen;

   uint32_t ts = p.ts_625;

   BankBuf *b = new BankBuf(bkname, TID_DWORD, pbank, bklen, 1);

   XAddBank(islot, ts, b);
   
   return true;
}

bool AddTdcBank(Evb* evb, const char* bkname, const char* pbank, int bklen, int bktype)
{
   //printf("AddTdcBank: name [%s] len %d type %d, tid_size %d\n", bkname, bklen, bktype, rpc_tid_size(bktype));

   if (gKludgeTdcKillFirstEvent) {
      gKludgeTdcKillFirstEvent = false;
      cm_msg(MINFO, "AddTdcBank", "Kludge: killing first TDC event");
      return true;
   }
      
   if (0) {
      const uint32_t* p32 = (const uint32_t*)pbank;
      
      for (int i=0; i<20; i++) {
         printf("tdc[%d]: 0x%08x 0x%08x\n", i, p32[i], getUint32(pbank, i*4));
      }
   }
   
   uint32_t fpga0_mark = getUint32(pbank, 6*4);
   uint32_t fpga0_header = getUint32(pbank, 7*4);
   uint32_t fpga0_epoch  = getUint32(pbank, 8*4);
   uint32_t fpga0_hit    = getUint32(pbank, 9*4);

   uint32_t ts = fpga0_epoch & 0x0FFFFFFF;

   if (0) {
      printf("fpga0: mark 0x%08x, h 0x%08x, e 0x%08x, h 0x%08x, ts 0x%08x\n",
             fpga0_mark,
             fpga0_header,
             fpga0_epoch,
             fpga0_hit,
             ts);
   }

   int imodule = 0;

   int islot = get_vector_element(evb->fTdcSlot, imodule);

   if (islot < 0) {
      return false;
   }

   // FIXME: not locked!
   evb->fLastTimeSec[islot] = TMFE::GetTime();
   evb->fCountPackets[islot] += 1;
   evb->fCountBytes[islot] += bklen;

   BankBuf *b = new BankBuf(bkname, TID_DWORD, pbank, bklen, 1);

   XAddBank(islot, ts, b);
   
   return true;
}

static int gFirstEventIn = 0;
static int gFirstEventOut = 0;

void event_handler(Evb* evb, HNDLE hBuf, HNDLE id, EVENT_HEADER *pheader, void *pevent)
{
   bool first_event = false;

   if (gFirstEventIn == 0) {
      cm_msg(MINFO, "event_handler", "Event builder received the first event");
      gFirstEventIn = 1;
      first_event = true;
   }

   evb->fCountInput++;

   //char banklist[STRING_BANKLIST_MAX];
   //int nbanks = bk_list(pevent, banklist);

   if (verbose) {
      //printf("event_handler: Evid: 0x%x, Mask: 0x%x, Serial: %d, Size: %d, Banks: %d (%s)\n", pheader->event_id, pheader->trigger_mask, pheader->serial_number, pheader->data_size, nbanks, banklist);
      printf("event_handler: Evid: 0x%x, Mask: 0x%x, Serial: %d, Size: %d\n", pheader->event_id, pheader->trigger_mask, pheader->serial_number, pheader->data_size);
   }
   
   //if (nbanks < 1)
   //return;

#if 0
   if (gEvb) {
      int sz = gEvb->fEvents.size();
      if (sz > 100) {
         cm_msg(MERROR, "event_handler", "evb stall, fEvents.size is %d", sz);
         {
            std::lock_guard<std::mutex> lock(gEvbLock);
            gEvb->PrintEvents();
         }
         sleep(1);
      }
   }
#endif
   
   FragmentBuf* buf = new FragmentBuf();

   BANK32* bhptr = NULL;
   while (1) {
      char *pdata;
      bk_iterate32(pevent, &bhptr, &pdata);
      if (bhptr == NULL) {
         break;
      }

      char name[5];
      name[0] = bhptr->name[0];
      name[1] = bhptr->name[1];
      name[2] = bhptr->name[2];
      name[3] = bhptr->name[3];
      name[4] = 0;

      if (first_event) {
         printf("first event bank: %s\n", name);
      }

      //printf("bk_iterate32 bhptr %p, pdata %p, name %s [%s], type %d, size %d\n", bhptr, pdata, bhptr->name, name, bhptr->type, bhptr->data_size);

      //int status;
      //DWORD bklen, bktype;
      //void* pbank;
      //std::string name;
      //name += banklist[i*4+0];
      //name += banklist[i*4+1];
      //name += banklist[i*4+2];
      //name += banklist[i*4+3];
      //      
      //status = bk_find((BANK_HEADER*)pevent, name.c_str(), &bklen, &bktype, &pbank);
      //
      //if (status != SUCCESS)
      //continue;

      //printf("bk_find status %d, name [%s], bklen %d, bktype %d\n", status, &banklist[i*4], bklen, bktype);

      const char* pbank = pdata;
      int bktype = bhptr->type;
      int bklen = bhptr->data_size; // new bklen is size in bytes, old bklen returned by bk_find() is in units of rpc_tid_size(tid)

      if (bklen <= 0 || bklen > 10000) {
         cm_msg(MERROR, "event_handler", "Bank [%s] type %d invalid length %d", name, bktype, bklen);
         cm_msg_flush_buffer();
         abort();
      }

      bool handled = false;

      if (name[0]=='A' && name[1]=='A') {
         int imodule = (name[2]-'0')*10 + (name[3]-'0')*1;
         handled = AddAdcBank(evb, imodule, pbank, bklen);
      } else if (name[0]=='P' && name[1]=='B') {
         int imodule = (name[2]-'0')*10 + (name[3]-'0')*1;
         handled = AddPwbBank(evb, imodule, name, (const char*)pbank, bklen, bktype);
      } else if (name[0]=='A' && name[1]=='T') {
         handled = AddTrgBank(evb, name, (const char*)pbank, bklen, bktype);
      } else if (name[0]=='T' && name[1]=='R' && name[2]=='B' && name[3]=='A') {
         handled = AddTdcBank(evb, name, (const char*)pbank, bklen, bktype);
      }

      if (!handled) {
         //printf("bypass bank %s\n", name.c_str());
         BankBuf *bank = new BankBuf(name, bktype, (char*)pbank, bklen, 1);
         buf->push_back(bank);
      }
   }

   if (0) {
      evb->fSync.Dump();
      evb->fSync.Print();
      printf("\n");
   }

   if (1) {
      static bool ok = false;
      static bool failed = false;
      
      if (ok != evb->fSync.fSyncOk) {
         if (evb->fSync.fSyncOk) {
            int count_dead = 0;
            std::string deaders = "";
            for (unsigned i=0; i<evb->fSync.fModules.size(); i++) {
               if (evb->fSync.fModules[i].fDead) {
                  if (count_dead > 0)
                     deaders += ", ";
                  deaders += evb->fSlotName[i];
                  count_dead++;
               }
            }
            if (count_dead > 0) {
               cm_msg(MERROR, "event_handler", "Event builder timestamp sync successful. %d dead modules: %s", count_dead, deaders.c_str());
            } else {
               cm_msg(MINFO, "event_handler", "Event builder timestamp sync successful");
            }
         }
         ok = evb->fSync.fSyncOk;
      }

      if (failed != evb->fSync.fSyncFailed) {
         if (evb->fSync.fSyncFailed) {
            cm_msg(MERROR, "event_handler", "Event builder timestamp sync FAILED");
         }
         failed = evb->fSync.fSyncFailed;
      }
   }
   //printf("EVB %d %d\n", , gEvb->fSync.fSyncFailed);

   if (buf->size() == 0) {
      delete buf;
      return;
   }

   {
      std::lock_guard<std::mutex> lock(gBufLock);
      gBuf.push_back(buf);
      int size = gBuf.size();
      if (size > size_gbuf_max)
         size_gbuf_max = size;
   }
}

std::deque<FragmentBuf*> gCopyBuf;
std::mutex       gCopyBufLock;
int size_gcopybuf_max = 0;

//bool run_threads = true;

bool build(Evb* evb, bool build_last)
{
   FragmentBuf* f = NULL;
   
   //printf("build_thread 111!\n");
   
#if 0
   {
      std::lock_guard<std::mutex> lock(gBufLock);
      
      if (gBuf.size() > 0) {
         f = gBuf.front();
         gBuf.pop_front();
         gCountBypass++;
      }
      
      // implicit unlock of gBufLock
   }
#endif
   
   //printf("build_thread 222! f=%p\n", f);
   
   if (!f) {
      std::lock_guard<std::mutex> lock(evb->fLock);
      
      if (1) {
         //DWORD t1 = ss_millitime();
         
         EvbEvent* e = evb->Get();

         if (!e && build_last) {
            e = evb->GetLastEvent();
         }
         
         //DWORD t2 = ss_millitime();
         //DWORD dt = t2 - t1;
         //if (dt > 0) {
         //   printf("Get() took %d ms\n", dt);
         //}
         
         if (e) {
            if (gFirstEventOut == 0) {
               cm_msg(MINFO, "build_thread", "Event builder built the first event");
               gFirstEventOut = 1;
            }
            
            //printf("Have EvbEvent: ");
            //e->Print();
            //printf("\n");
            
            f = e->banks;
            e->banks = NULL;
            delete e;
         }
      }
      
      // implicit unlock of gEvbLock
   }
   
   //printf("build_thread 333! f=%p\n", f);
   
   if (!f) {
      return false;
   }
   
   {
      std::lock_guard<std::mutex> lock(gCopyBufLock);
      gCopyBuf.push_back(f);
      f = NULL;
      
      int size = gCopyBuf.size();
      if (size > size_gcopybuf_max)
         size_gcopybuf_max = size;
   }
   
   //printf("build_thread 444! f=%p\n", f);
   
   if (f) {
      delete f;
      f = NULL;
   }

   return true;
}

#if 0
int build_thread(void*evbptr)
{
   printf("build_thread started!\n");

   Evb* evb = (Evb*)evbptr;

   while (run_threads) {
      bool done_something = build(evb);
      if (!done_something) {
         ss_sleep(1);
      }
   }

   printf("build_thread finished!\n");
   return 0;
}
#endif

static void handle_event(Evb* evb, EVENT_HEADER* pevent)
{
   // FIXME: gEvb can still be deleted while we are inside the event_handler(), this will cause a crash!
   //DWORD t1 = ss_millitime();

   event_handler(evb, 0, 0, pevent, pevent+1);

   //DWORD t2 = ss_millitime();
   //DWORD dt = t2-t1;
   //if (dt > 1) {
   //   printf("event_handler time %d\n", (int)dt);
   //}
   
   //DWORD t3 = ss_millitime();

   XFlushBank(evb);

   //DWORD t4 = ss_millitime();
   //DWORD dt34 = t4-t3;
   //if (dt34 > 1) {
   //   printf("XFlushBank time %d\n", (int)dt34);
   //}
}

static std::deque<EVENT_HEADER*> gEhBuf;
static std::mutex gEhBufLock;
static int size_gehbuf_max = 0;

bool handle(Evb* evb)
{
   EVENT_HEADER* pevent = NULL;
   {
      std::lock_guard<std::mutex> lock(gEhBufLock);
      if (!gEhBuf.empty()) {
         pevent = gEhBuf.front();
         gEhBuf.pop_front();
      }
   }
   
   if (!pevent) {
      return false;
   }
   
   handle_event(evb, pevent);
   
   if (pevent) {
      free(pevent);
      pevent = NULL;
   }

   return true;
}

#if 0
int handler_thread(void* evbptr)
{
   printf("handler_thread started, arg %p!\n", evbptr);

   Evb* evb = (Evb*)evbptr;

   while (run_threads) {
      bool done_something = handle(evb);
      if (!done_something) {
         //printf("sleep!\n");
         ss_sleep(2);
      }
   }

   printf("handler_thread finished!\n");
   return 0;
}
#endif

struct read_thread_data
{
   int num_bh;
   int bh[10];
};

#if 0
int read_thread(void*arg)
{
   printf("read_thread started, arg %p!\n", arg);

   read_thread_data* data = (read_thread_data*)arg;

   while (run_threads) {
      bool read_something = read_buffers(NULL, data);
         
      if (!read_something) {
         ss_sleep(2);
         continue;
      }
   }

   printf("read_thread finished!\n");
   return 0;
}
#endif

int open_buffer(const char* bufname)
{
   int status;
   int evid = -1;
   int trigmask = 0xFFFF;

   int bh = 0;

   status = bm_open_buffer(bufname, 0, &bh);
   if (status != BM_SUCCESS && status != BM_CREATED) {
      cm_msg(MERROR, "frontend_init", "Error: bm_open_buffer(\"%s\") status %d", bufname, status);
      return -1;
   }
   
   int reqid = 0;
   //status = bm_request_event(bh, evid, trigmask, GET_ALL, &reqid, event_handler);
   status = bm_request_event(bh, evid, trigmask, GET_ALL, &reqid, NULL);
   if (status != BM_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Error: bm_request_event() status %d", status);
      return -1;
   }

   cm_msg(MINFO, "frontend_init", "Event builder reading from buffer \"%s\", evid %d, trigmask 0x%x", bufname, evid, trigmask);

   return bh;
}

static unsigned gMaxEventsSize = 0;

void report_evb_unlocked(TMFeEquipment* eq, Evb* evb, MVOdb* status)
{
   int size_gbuf = 0;
   int size_gcopybuf = 0;
   int size_gehbuf = 0;
   
   {
      std::lock_guard<std::mutex> lock(gBufLock);
      size_gbuf = gBuf.size();
   }
   
   {
      std::lock_guard<std::mutex> lock(gCopyBufLock);
      size_gcopybuf = gCopyBuf.size();
   }
   
   {
      std::lock_guard<std::mutex> lock(gEhBufLock);
      size_gehbuf = gEhBuf.size();
   }
   
   int count_dead_slots = 0;
   for (unsigned i=0; i<evb->fNumSlots; i++) {
      if (evb->fSync.fModules[i].fDead) {
         evb->fDeadSlots[i] = true;
         count_dead_slots++;
      } else {
         evb->fDeadSlots[i] = false;
      }
   }
   evb->fCountDeadSlots = count_dead_slots;
   
   std::string st = msprintf("dead %d, in %d, complete %d, incomplete %d, with errors %d, bypass %d, per-slot errors %d, queue %d/%d, input queue %d/%d, evb %d/%d/%d, copy queue: %d/%d",
           evb->fCountDeadSlots,
           evb->fCountInput,
           evb->fCountComplete, evb->fCountIncomplete, evb->fCountError,
           evb->fCountBypass,
           evb->fCountSlotErrors,
           size_gbuf, size_gbuf_max,
           size_gehbuf, size_gehbuf_max,
           (int)evb->fEventsSize, (int)evb->fMaxEventsSize, gMaxEventsSize,
           size_gcopybuf, size_gcopybuf_max
           );

   if (evb->fCountDeadSlots > 0 || evb->fCountIncomplete > 0 || evb->fCountError > 0 || evb->fCountSlotErrors > 0 || evb->fCountBypass > 0) {
      eq->SetStatus(st.c_str(), "yellow");
   } else {
      eq->SetStatus(st.c_str(), "#00FF00");
   }
   
   if (evb->fMaxEventsSize > gMaxEventsSize) {
      gMaxEventsSize = evb->fMaxEventsSize;
      evb->fMaxEventsSize = 0;
   }
   
   evb->Print();
   
   evb->ComputePerSecond();
   evb->WriteSyncStatus(status);
   evb->WriteEvbStatus(status);
   evb->WriteVariables(eq->fOdbEqVariables);
}

int copy_event(TMFeEquipment* eq, char* event, int max_event_size, FragmentBuf* f)
{
   eq->ComposeEvent(event, max_event_size);
   eq->BkInit(event, max_event_size);

   //std::string banks = "";

   int size = f->size();
   for (int i=0; i<size; i++) {
      BankBuf* b = (*f)[i];

      char* pdata = (char*)eq->BkOpen(event, b->name.c_str(), b->tid);
      memcpy(pdata, b->ptr, b->psize);
      eq->BkClose(event, pdata + b->psize);

      //banks += b->name;

      delete b;
   }

   return eq->BkSize(event);
}

int read_event(TMFeEquipment* eq, Evb* evb, char *event, int max_event_size)
{
#if 0
   int n_bytes = 0;
   rb_get_buffer_level(get_event_rbh(0), &n_bytes);
   double tt = TMFE::GetTime();
   static double ttx = 0;
   if (ttx==0) {
      ttx = tt;
   } else {
      double dt = tt-ttx;
      if (dt > 0.002)
         printf("\n");
      printf("%.3f %d ", dt, n_bytes);
      ttx = tt;
   }
#endif

#if 0
   if (gBuf.size() < 1) {
      ss_sleep(10);
      return 0;
   }
#endif

   //printf("in queue: %d\n", (int)gBuf.size());

   if (evb) {
      //evb->Print();
      //evb->fSync.Dump();
      //evb->fSync.fTrace = true;
   }

#if 0
   if (evb) {
      static time_t last = 0;
      time_t now = time(NULL);

      //if (last == 0) {
      //   last = now;
      //   set_equipment_status("EVB", "Started...", "#00FF00");
      //}

      if (last == 0 || now - last > 2) {
         last = now;
         report_evb_unlocked();
      }
   }
#endif

   FragmentBuf* f = NULL;

   {
      std::lock_guard<std::mutex> lock(gBufLock);
      
      if (gBuf.size() > 0) {
         f = gBuf.front();
         gBuf.pop_front();
         if (evb->fCountBypass == 0) {
            cm_msg(MERROR, "read_event", "First bypass event");
         }
         evb->fCountBypass++;
      }
      
      // implicit unlock of gBufLock
   }

#if 0
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
#endif

   if (!f) {
      std::lock_guard<std::mutex> lock(gCopyBufLock);
      if (!gCopyBuf.empty()) {
         f = gCopyBuf.front();
         gCopyBuf.pop_front();
      }
   }
   
   if (!f) {
      return 0;
   }

   evb->fCountOut++;

   int size = copy_event(eq, event, max_event_size, f);

   delete f;

#if 0
   double tte = TMFE::GetTime();
   double dtte = tte - tt;

   if (dtte > 0.002) {
      printf("E %.3f ", dtte);
   }
#endif

   //printf("Sending event: banks %s\n", banks.c_str());
   //printf("Sending event: serial_number %d\n", ((EVENT_HEADER*)pevent)[-1].serial_number);

   return size; 
}

static const int max_event_size = 64*1024*1024;
static char event[max_event_size];

bool send_event(TMFeEquipment* eq, Evb* evb)
{
   //char* pevent = (event + sizeof(EVENT_HEADER));
   int size = read_event(eq, evb, event, 0);
   if (size == 0) {
      return false;
   }
   if (size > evb->fMaxEventSize)
      evb->fMaxEventSize = size;
   if (size > evb->fMaxEventSizeEver)
      evb->fMaxEventSizeEver = size;
   evb->fEventCount += 1;
   evb->fByteCount += size;
   //printf("Send event: size %d\n", size);
   eq->SendEvent(event);
   return true;
}

class EvbEq:
   public TMFeRpcHandlerInterface,
   public TMFePeriodicHandlerInterface
{
public: // TMFE
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;

public: // methods
   EvbEq(TMFE* mfe, TMFeEquipment* eq); // ctor
   virtual ~EvbEq(); // dtor
   bool Init();
   void InitEvb();
   bool ReadBuffers(void);
   bool Build(bool build_last);

public: // handlers for MIDAS callbacks
   void HandlePeriodic();
   void HandleBeginRun();
   void HandleEndRun();

public:
   Evb* fEvb = NULL;
   struct read_thread_data fData;

public: // ODB
   MVOdb* fSettings = NULL;
   MVOdb* fConfig = NULL;
   MVOdb* fStatus = NULL;
};

EvbEq::EvbEq(TMFE* mfe, TMFeEquipment* eq) // ctor
{
   fMfe = mfe;
   fEq = eq;

   fSettings = eq->fOdbEqSettings;
   fConfig   = mfe->fOdbRoot->Chdir("Equipment/Ctrl/EvbConfig", false);
   fStatus   = eq->fOdbEq->Chdir("EvbStatus", true);
}

EvbEq::~EvbEq()
{
   if (fEvb) {
      fEvb->fLock.lock();
      delete fEvb;
      fEvb = NULL;
   }
}

bool EvbEq::Init()
{
   //xdata.evb = evb;
   fData.num_bh = 3;
   fData.bh[0] = open_buffer("BUFTRG");
   fData.bh[1] = open_buffer("BUFTDC");
   fData.bh[2] = open_buffer("BUFUDP");

   if (fData.bh[0] < 0)
      return false;
   if (fData.bh[1] < 0)
      return false;
   if (fData.bh[2] < 0)
      return false;

   return true;
}

void EvbEq::HandlePeriodic()
{
   //printf("EvbEq::HandlePeriodic!\n");

   if (fEvb) {
      fEvb->CheckDeadSlots();
      report_evb_unlocked(fEq, fEvb, fStatus);
   }

   fEq->WriteStatistics();
}

void EvbEq::HandleBeginRun()
{
   printf("EvbEq::HandleBeginRun!\n");

   fEq->SetStatus("Begin run...", "#00FF00");

   //if (fEvb) {
   //   fEvb->fLock.lock();
   //   delete fEvb;
   //   fEvb = NULL;
   //}

   fEq->ZeroStatistics();
   fEq->WriteStatistics();

   //fEvb = new Evb(fSettings, fConfig, fStatus);
   //report_evb_unlocked(fEq, fEvb, fStatus);

   int countBufFlushed = 0;
   {
      std::lock_guard<std::mutex> lock(gBufLock);
      
      while (gBuf.size() > 0) {
         FragmentBuf* f = gBuf.front();
         gBuf.pop_front();
         delete f;
         countBufFlushed++;
      }
      
      // implicit unlock of gBufLock
   }

   cm_msg(MINFO, "HandleBeginRun", "Flushed %d buffered events", countBufFlushed);

   gFirstEventIn = 0;
   gFirstEventOut = 0;

   size_gbuf_max = 0;
   size_gcopybuf_max = 0;
   size_gehbuf_max = 0;

   gMaxEventsSize = 0;
}

void EvbEq::InitEvb()
{
   if (fEvb) {
      fEvb->fLock.lock();
      delete fEvb;
      fEvb = NULL;
   }

   fEq->ZeroStatistics();
   fEq->WriteStatistics();

   fEvb = new Evb(fSettings, fConfig, fStatus);
   report_evb_unlocked(fEq, fEvb, fStatus);
}

void EvbEq::HandleEndRun()
{
   printf("EvbEq::HandleEndRun!\n");

   int count_gehbuf = 0;
   while (1) {
      std::lock_guard<std::mutex> lock(gEhBufLock);
      if (gEhBuf.empty()) {
         break;
      }
      if (count_gehbuf == 0) {
         printf("waiting for gEhBuf!\n");
      }
      count_gehbuf++;
      ss_sleep(10);
   }
   printf("done waiting for gEhBuf, %d loops\n", count_gehbuf);

   // build the last remaining events
   int count_build = 0;
   while (1) {
      bool done_something = Build(true);
      if (!done_something)
         break;
      count_build++;
   }

   if (count_build) {
      fMfe->Msg(MLOG, "HandleEndRun", "HandleEndRun: %d loops of Build()", count_build);
   }

   if (fEvb) {
      std::lock_guard<std::mutex> lock(fEvb->fLock);

      printf("end_of_run: Evb state:\n");
      fEvb->Print();

      int count_lost = 0;

      while (1) {
         EvbEvent *e = fEvb->GetLastEvent();
         if (!e)
            break;

         count_lost += 1;
         
         delete e;
      }

      if (count_lost) {
         fMfe->Msg(MERROR, "HandleEndRun", "HandleEndRun: %d events lost at end of run", count_lost);
      }
      
      printf("end_of_run: Evb final state:\n");
      fEvb->Print();
      fEvb->LogPwbCounters();
      
      cm_msg(MLOG, "end_of_run", "end_of_run: %d in, complete %d, incomplete %d, with errors %d, bypass %d, out %d, lost at end of run %d, per-slot errors %d", fEvb->fCountInput, fEvb->fCountComplete, fEvb->fCountIncomplete, fEvb->fCountError, fEvb->fCountBypass, fEvb->fCountOut, count_lost, fEvb->fCountSlotErrors);
   }

   if (fEvb) {
      report_evb_unlocked(fEq, fEvb, fStatus);
      fEvb->fLock.lock();
      delete fEvb;
      fEvb = NULL;
   }

   fEq->WriteStatistics();
}

bool EvbEq::ReadBuffers()
{
   bool read_something = false;
   for (int i=0; i<fData.num_bh; i++) {
      int bh = fData.bh[i];
      EVENT_HEADER* pevent = NULL;
      int status = bm_receive_event_alloc(bh, &pevent, BM_NO_WAIT);
      //printf("bh[%d] %d, bufsize %d, size %d, status %d\n", i, bh, bufsize, size, status);
      if (status == BM_ASYNC_RETURN) {
         continue;
      }
      if (status != BM_SUCCESS) {
         printf("bm_receive_event() returned %d\n", status);
         cm_msg(MERROR, "read_thread", "bm_receive_event() returned %d, stopping the thread", status);
         return false;
      }
      
      read_something = true;
      
#if 0
      {
         std::lock_guard<std::mutex> lock(gEhBufLock);
         gEhBuf.push_back(pevent);
         pevent = NULL;
         int size = gEhBuf.size();
         if (size > size_gehbuf_max)
            size_gehbuf_max = size;
      }
#endif

      if (!fEvb) {
         InitEvb();
      }

      handle_event(fEvb, pevent);

      free(pevent);
   }

   return read_something;
}

bool EvbEq::Build(bool build_last)
{
   bool done_something = ReadBuffers();
   if (fEvb) {
      done_something |= build(fEvb, build_last);
      done_something |= handle(fEvb);
      done_something |= send_event(fEq, fEvb);
   }
   return done_something;
}

void HotStart(TMFE*mfe, TMFeRpcHandlerInterface*iface)
{
   int run_state = 0;
   mfe->fOdbRoot->RI("Runinfo/state", &run_state);
   if (run_state == 3) { // STATE_RUNNING
      printf("Hot starting the run!\n");
      iface->HandleBeginRun();
   }
}

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect("feevb", __FILE__);
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *common = new TMFeCommon();
   common->EventID = 1;
   common->LogHistory = 1;
   common->Period = 1000;
   common->Buffer = "SYSTEM";
   
   TMFeEquipment* eq = new TMFeEquipment(mfe, "EVB", common);
   eq->Init();
   eq->SetStatus("Starting...", "white");
   eq->WriteStatistics();

   mfe->RegisterEquipment(eq);

   EvbEq* evbeq = new EvbEq(mfe, eq);

   bool ok = evbeq->Init();
   if (!ok) {
      printf("evb init failed!\n");
      return 1;
   }

   mfe->RegisterRpcHandler(evbeq);

   mfe->SetTransitionSequenceStart(500);
   mfe->SetTransitionSequenceStop(600);
   mfe->DeregisterTransitionPause();
   mfe->DeregisterTransitionResume();

#if 0   
   ss_thread_create(build_thread, evb);
   ss_thread_create(handler_thread, evb);
   ss_thread_create(read_thread, &xdata);
#endif

   mfe->RegisterPeriodicHandler(eq, evbeq);

   eq->SetStatus("Started", "#00FF00");

   HotStart(mfe, evbeq);

   while (!mfe->fShutdownRequested) {
      bool done_something = evbeq->Build(false);
      if (!done_something) {
         mfe->PollMidas(10);
      } else {
         mfe->PollMidas(0);
      }
   }

   delete evbeq;
   evbeq = NULL;

   mfe->Disconnect();

   return 0;
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
