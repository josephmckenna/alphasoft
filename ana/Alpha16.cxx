//
// ALPHA-g TPC
//
// GRIF-16/ALPHA-16 ADC functions
//
// Class functions for Alpha16.h
//

#include "Alpha16.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MEMZERO(array) memset((array),0,sizeof(array))

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

void Alpha16Packet::Unpack(const void*ptr, int bklen8)
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
   
   bankLength = bklen8;
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
   length = 32 + nsamples*2;
   
   xcrc16 = crc16((const unsigned char*)ptr, 32 + nsamples*2);
}

int Alpha16Packet::PacketType(const void*ptr, int bklen8)
{
   return getUint8(ptr, 0);
}

int Alpha16Packet::PacketVersion(const void*ptr, int bklen8)
{
   return getUint8(ptr, 1);
}

uint32_t Alpha16Packet::PacketTimestamp(const void*ptr, int bklen8)
{
   return getUint32(ptr, 18);
}

int Alpha16Packet::PacketChannel(const void*ptr, int bklen8)
{
   int chanX = getUint8(ptr, 27);
   int channelType = chanX & 0x80;
   int channelId = chanX & 0x7F;
   return channelId;
}

void Alpha16Packet::Print() const
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
   printf("length: %d, bank length %d\n", length, bankLength);
};

void Alpha16Waveform::Unpack(const void* bkptr, int bklen8)
{
   int nsamples = getUint16(bkptr, 28);

   this->reserve(nsamples);
   this->clear();
   
   //printf("Unpacking: "); Print();
   for (int i=0; i<nsamples; i++) {
      int16_t v = getUint16(bkptr, 30 + i*2);
      //printf("sample %d: 0x%02x (%d)\n", i, v, v);
      this->push_back(v);
   }
};

Alpha16Event::Alpha16Event() // ctor
{
   Reset();
}

void Alpha16Event::Reset()
{
   eventNo = 0;
   eventTs = 0;
   prevEventTs = 0;
   MEMZERO(udpPresent);
   MEMZERO(udpEventTs);
   numChan = 0;
   complete = false;
}
   
Alpha16Event::~Alpha16Event() // dtor
{
}

void Alpha16Event::Print() const
{
   printf("ALPHA16 event: %d, ts 0x%08x (incr 0x%08x), channels: %d, complete %d\n", eventNo, eventTs, eventTs - prevEventTs, numChan, complete);
}

void Alpha16EVB::Reset()
{
   fEventCount = 0;
   MEMZERO(fFirstEventTs);
   MEMZERO(fLastEventTs);
   
   while (fEvents.size() > 0) {
      Alpha16Event* e = fEvents.back();
      fEvents.pop_back();
      if (e)
         delete e;
   }
}

void Alpha16EVB::Print() const
{
   printf("EVB contents:\n");
   for (unsigned i=0; i<fEvents.size(); i++) {
      printf("Entry %d: ", i);
      fEvents[i]->Print();
   }
}

bool Alpha16EVB::Match(const Alpha16Event* e, int imodule, uint32_t udpTs)
{
   return (e->udpEventTs[imodule] == udpTs);
}

void Alpha16EVB::AddBank(Alpha16Event* e, int imodule, const void* bkptr, int bklen)
{
   int xchan = Alpha16Packet::PacketChannel(bkptr, bklen);

   assert(xchan>=0);
   assert(xchan<NUM_CHAN_ALPHA16);

   int chan = imodule*NUM_CHAN_ALPHA16 + xchan;
   
   assert(!e->udpPresent[chan]);
   
   e->udpPacket[chan].Unpack(bkptr, bklen);
   e->waveform[chan].Unpack(bkptr, bklen);
   e->udpPresent[chan] = true;
   e->numChan++;

   if (e->numChan == fConfNumChan)
      e->complete = true;
}

Alpha16Event* Alpha16EVB::FindEvent(int imodule, uint32_t udpTs)
{
   // loop over buffered events, look for match
   
   for (unsigned i=0; i<fEvents.size(); i++) {
      if (Match(fEvents[i], imodule, udpTs)) {
         return fEvents[i];
      }
   }
   
   Alpha16Event *e = new Alpha16Event();
   fEventCount++;
   e->eventNo = fEventCount;
   e->udpEventTs[imodule] = udpTs;
   if (fFirstEventTs[imodule] == 0) {
      fFirstEventTs[imodule] = udpTs;
      fLastEventTs[imodule] = fFirstEventTs[imodule];
   }
   e->eventTs = udpTs - fFirstEventTs[imodule];
   e->prevEventTs = fLastEventTs[imodule] - fFirstEventTs[imodule];
   
   fLastEventTs[imodule] = udpTs;
   
   //printf("Event %d ts 0x%08x: new!\n", e->eventNo, e->eventTs);
   
   fEvents.push_front(e);

   return e;
}

Alpha16Event* Alpha16EVB::GetNextEvent()
{
   if (fEvents.size() < 1)
      return NULL;
   
   Alpha16Event* e = fEvents.back();
   
   if (e->complete || fEvents.size() > 10) {
      fEvents.pop_back();
      return e;
   }
  
   return NULL;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
