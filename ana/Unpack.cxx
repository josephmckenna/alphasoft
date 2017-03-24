//
// Unpack.cxx
// K.Olchanski
//


#include <stdio.h>
#include <assert.h> // assert()

#include "Unpack.h"

Alpha16Event* UnpackAlpha16Event(Alpha16EVB* evb, const TMidasEvent* me)
{
   Alpha16Event* e = NULL;

   for (int imodule = 1; imodule <= 20; imodule++) {
      for (int i=0; i<NUM_CHAN_ALPHA16; i++) {
         void *ptr;
         int bklen, bktype;
         int status;

         char c = 0;
         if (i<=9)
            c = '0' + i;
         else
            c = 'A' + i - 10;
         
         char bname[5];
         sprintf(bname, "B%02d%c", imodule, c);
         
         status = me->FindBank(bname, &bklen, &bktype, &ptr);
         if (status == 1) {
            //printf("ALPHA16 bname %s, pointer: %p, status %d, len %d, type %d\n", bname, ptr, status, bklen, bktype);
            
            // print header
            
            int packetType = Alpha16Packet::PacketType(ptr, bklen);
            int packetVersion = Alpha16Packet::PacketVersion(ptr, bklen);

#if 0
            const int xmap_140[] = { 4, 10, 13, 15, 1, 16, 17, 2, 0 };
            const int xmap_147[] = { 1, 2, 3, 4, 6, 7, 8, 16, 0 };
            const int xmap_151[] = { 1, 2, 4, 7, 8, 10, 15, 16, 0 };
            const int xmap_171[] = { 9, 12, 4, 19, 8, 10, 15, 16, 0 };
            const int xmap_173[] = { 9, 12, 4, 6, 8, 10, 15, 16, 0 };
            const int xmap_177[] = { 9, 12, 4, 7, 8, 10, 15, 16, 0 };
            const int xmap_183[] = { 9, 12, 4, 11, 14, 10, 15, 18, 0 };
            const int xmap_184[] = { 9, 12, 4, 11, 10, 15, 17, 18, 0 };
            const int xmap_186[] = { 1, 2, 12, 4,  10, 15, 17, 18, 0 };
            const int xmap_194[] = { 1, 2, 13, 4,  9, 10, 11, 12, 0 };

            const int *xmap = NULL;

            xmap = xmap_194;

            int xmodule = -1;

            for (int x=0; xmap[x]; x++)
               if (xmap[x] == imodule) {
                  xmodule = x;
                  break;
               }
#endif

            if (0) {
               printf("Header:\n");
               printf("  packet type:    0x%02x (%d)\n", packetType, packetType);
               printf("  packet version: 0x%02x (%d)\n", packetVersion, packetVersion);
            }
            
            if (packetType == 1 && packetVersion == 1) {
               if (!e)
                  e = evb->NewEvent();
               evb->AddBank(e, imodule, ptr, bklen);
            } else {
               printf("unknown packet type %d, version %d\n", packetType, packetVersion);
            }
         }
      }
   }

#if 0
   for (int imodule = 0; imodule < 9; imodule++) {
      for (int i=0; i<NUM_CHAN_ALPHA16; i++) {
         void *ptr;
         int bklen, bktype;
         int status;
         
         char bname[5];
         sprintf(bname, "A%01d%02d", 1+imodule, i);
         
         status = me->FindBank(bname, &bklen, &bktype, &ptr);
         if (status == 1) {
            //printf("ALPHA16 bname %s, pointer: %p, status %d, len %d, type %d\n", bname, ptr, status, bklen, bktype);
            
            // print header
            
            int packetType = Alpha16Packet::PacketType(ptr, bklen);
            int packetVersion = Alpha16Packet::PacketVersion(ptr, bklen);
            
            if (0) {
               printf("Header:\n");
               printf("  packet type:    0x%02x (%d)\n", packetType, packetType);
               printf("  packet version: 0x%02x (%d)\n", packetVersion, packetVersion);
            }
            
            if (packetType == 1 && packetVersion == 1) {
               evb->AddBank(e, imodule, ptr, bklen);
            } else {
               printf("unknown packet type %d, version %d\n", packetType, packetVersion);
            }
         }
      }
   }
#endif

   if (e) {
      evb->CheckEvent(e);
   }

   return e;
};

Alpha16Event* UnpackAlpha16Event(Alpha16EVB* evb, TMEvent* me)
{
   Alpha16Event* e = NULL;

   for (int imodule = 1; imodule <= 20; imodule++) {
      for (int i=0; i<NUM_CHAN_ALPHA16; i++) {

         char c = 0;
         if (i<=9)
            c = '0' + i;
         else
            c = 'A' + i - 10;
         
         char bname[5];
         sprintf(bname, "B%02d%c", imodule, c);

         TMBank* b = me->FindBank(bname);

         if (b) {
            int bklen = b->data_size;
            //printf("ALPHA16 bname %s, bank %p, len %d\n", bname, b, bklen);
            // print header

            const char* ptr = me->GetBankData(b);

            if (!ptr)
               continue;
            
            int packetType = Alpha16Packet::PacketType(ptr, bklen);
            int packetVersion = Alpha16Packet::PacketVersion(ptr, bklen);

            if (0) {
               printf("Header:\n");
               printf("  packet type:    0x%02x (%d)\n", packetType, packetType);
               printf("  packet version: 0x%02x (%d)\n", packetVersion, packetVersion);
            }
            
            if (packetType == 1 && packetVersion == 1) {
               if (!e) {
                  e = evb->NewEvent();
               }
               evb->AddBank(e, imodule, ptr, bklen);
            } else {
               printf("unknown packet type %d, version %d\n", packetType, packetVersion);
            }
         }
      }
   }

   if (e) {
      evb->CheckEvent(e);
   }

   return e;
};


FeamEvent* UnpackFeamEvent(FeamEVB* evb, TMEvent* event, const std::vector<std::string> &banks)
{
   //printf("event id %d\n", event->event_id);
   
   if (event->event_id != 1)
      return NULL;
      
   //const char* banks[] = { "BB01", "BB02", "BB03", "BB04", "BB05", "BB06", "BB07", "BB08", NULL };
   //const char* banks[] = { "BB10", "BB07", "BB04", "BB06", "BB01", "BB08", "BB11", "BB05", NULL };
   char *data = NULL;
   
   for (unsigned i=0; i<banks.size(); i++) {
      TMBank* b = event->FindBank(banks[i].c_str());
      if (b) {
         data = event->GetBankData(b);
         if (data) {
            //printf("Have bank %s\n", banks[i]);
            //HandleFeam(i, data, b->data_size);
            
            if (b->data_size < 26) {
               printf("bad FEAM %d packet length %d\n", i, b->data_size);
               continue;
            }
            
            FeamPacket* p = new FeamPacket();
            
            p->Unpack(data, b->data_size);
               
            assert(!p->error);
            
            evb->AddPacket(i, p, data + p->off, p->buf_len);
         }
      }
   }
   
   return evb->Get();
}

AgEVB* agevb = NULL;

void agevb_init()
{
   if (!agevb) {
      agevb = new AgEVB(100.0*1e6/100.0, 125.0*1e6/100.0);
      //agevb->fSync.fTrace = true;
   }
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
