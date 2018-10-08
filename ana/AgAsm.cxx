//
// AgAsm.cxx
//
// ALPHA-g event assembler
// K.Olchanski
//

#include "AgAsm.h"

#include <stdio.h> // NULL, printf()
#include <math.h> // fabs()
//#include <assert.h> // assert()

AgAsm::AgAsm()
{
   // empty
}

AgAsm::~AgAsm()
{
   //Print();

   if (fAdcAsm) {
      delete fAdcAsm;
      fAdcAsm = NULL;
   }

   if (fPwbModuleMap) {
      delete fPwbModuleMap;
      fPwbModuleMap = NULL;
   }

   if (fPwbAsm) {
      delete fPwbAsm;
      fPwbAsm = NULL;
   }

   if (fFeamAsm) {
      delete fFeamAsm;
      fFeamAsm = NULL;
   }
   
   if (fTrgAsm) {
      delete fTrgAsm;
      fTrgAsm = NULL;
   }

   if (fTdcAsm) {
      delete fTdcAsm;
      fTdcAsm = NULL;
   }

   printf("AgAsm: Total events: %d, complete: %d, with error: %d, incomplete: %d, with error: %d, max timestamp difference trg/adc/pwb/tdc: %.0f/%.0f/%.0f/%.0f ns\n", fCounter, fCountComplete, fCountCompleteWithError, fCountIncomplete, fCountIncompleteWithError, fTrgMaxDt*1e9, fAdcMaxDt*1e9, fPwbMaxDt*1e9, fTdcMaxDt*1e9);
}

void AgAsm::Print() const
{
   printf("AgAsm::Print:\n");
   if (fTrgAsm) {
      //fTrgAsm->Print();
   }
   if (fAdcAsm) {
      //fAdcAsm->Print();
   }
   if (fFeamAsm) {
      fFeamAsm->Print();
   }
   if (fPwbAsm) {
      //fPwbAsm->Print();
   }
   if (fTdcAsm) {
      fTdcAsm->Print();
   }
}

AgEvent* AgAsm::UnpackEvent(TMEvent* me)
{
   bool have_trg  = false;
   bool have_adc  = false;
   bool have_feam = false;
   bool have_pwb  = false;
   bool have_tdc  = false;

   AgEvent* e = new AgEvent();

   e->counter = ++fCounter;
   e->error = false;
   e->complete = true;

   me->FindAllBanks();

   for (unsigned i=0; i<me->banks.size(); i++) {
      const TMBank* b = &me->banks[i];
      //printf("bank %s\n", b->name.c_str());
      if (0) {
      } else if (b->name == "ATAT") {
         if (!fTrgAsm) {
            fTrgAsm = new TrgAsm();
         }
         
         const char* bkptr = me->GetBankData(b);
         int bklen = b->data_size;

         e->trig = fTrgAsm->UnpackBank(bkptr, bklen);

         if (1) {
            printf("Unpacked TRG event: ");
            e->trig->Print();
            printf("\n");
         }

         have_trg = true;
      } else if (b->name == "TRBA") {

         if (!fTdcAsm) {
            fTdcAsm = new TdcAsm();
         }

         const char* bkptr = me->GetBankData(b);
         int bklen = b->data_size;

         if (!e->tdc) {
            e->tdc = fTdcAsm->UnpackBank(bkptr, bklen);
            e->tdc->Print(1);
         }

         have_tdc = true;
      } else if (b->name[0] == 'A') {
         // ADC UDP packet bank from feudp
      } else if (b->name[0] == 'B' && b->name[1] == 'B') {
         // obsolete bank "BBnn" from FEAM rev0 board
      } else if (b->name[0] == 'B' || b->name[0] == 'C') {
         // ADC bank from feevb
         int c1 = b->name[1]-'0';
         int c2 = b->name[2]-'0';
         int module = c1*10 + c2;
         if (module < 1 || module > ADC_MODULE_LAST) {
            printf("AgAsm::UnpackEvent: bank name [%s] has invalid module number %d\n", b->name.c_str(), module);
            e->error = true;
            continue;
         }

         const char* bkptr = me->GetBankData(b);
         int bklen = b->data_size;

         if (!fAdcAsm) {
            fAdcAsm = new Alpha16Asm();
            fAdcAsm->Init(fConfAdc32Rev);
            fAdcAsm->fMap.Init(fAdcMap);
            fAdcAsm->fMap.Print();
         }

         if (!e->a16) {
            e->a16 = fAdcAsm->NewEvent();
         }

         fAdcAsm->AddBank(e->a16, module, b->name.c_str(), bkptr, bklen);
         have_adc = true;
      } else if (b->name[0] == 'P' && ((b->name[1] == 'A') || (b->name[1] == 'B'))) {
         // PWB bank
         int c1 = b->name[2]-'0';
         int c2 = b->name[3]-'0';
         int imodule = c1*10 + c2;

         if (imodule < 0 || imodule > PWB_MODULE_LAST) {
            printf("AgAsm::UnpackEvent: bank name [%s] has invalid module number %d\n", b->name.c_str(), imodule);
            e->error = true;
            continue;
         }

         if (!fPwbModuleMap) {
            fPwbModuleMap = new PwbModuleMap();
            fPwbModuleMap->LoadFeamBanks(fFeamBanks);
            fPwbModuleMap->Print();
         }

         const PwbModuleMapEntry* map = fPwbModuleMap->FindPwb(imodule);
         
         //int ii = 0; // FIXME
         //
         //int column = (ii/8);
         //int ring = (ii%8);
         //
         //bool short_tpc = false; // FIXME (runinfo->fRunNo < 1450);
         //
         //if (short_tpc) {
         //   column = ii;
         //   ring = 0;
         //}
            
         const char* p8 = me->GetBankData(b);
         //const int n32 = b->data_size/4;
         
         if (0) {
            const uint32_t *p32 = (const uint32_t*)p8;
            unsigned nprint = b->data_size/4;
            nprint=10;
            for (unsigned i=0; i<nprint; i++) {
               printf("PB05[%d]: 0x%08x (%d)\n", i, p32[i], p32[i]);
            }
         }

         int f = 0;
         if (b->name[0] == 'P' && b->name[1] == 'A')
            f = 1;
         else if (b->name[0] == 'P' && b->name[1] == 'B')
            f = 2;
         else {
            printf("AgAsm::UnpackEvent: invalid PWB bank name [%s]\n", b->name.c_str());
            e->error = true;
            continue;
         }

         if (!fFeamAsm) {
            fFeamAsm = new FeamAsm();
         }
         
         if (b->data_size < 26) {
            printf("AgAsm::UnpackEvent: bank name [%s] has invalid FEAM packet length %d\n", b->name.c_str(), b->data_size);
            e->error = true;
            continue;
         }
         
         FeamPacket* p = new FeamPacket();
         
         p->Unpack(p8, b->data_size);
         
         if (p->error) {
            printf("AgAsm::UnpackEvent: cannot unpack FeamPacket from bank [%s]\n", b->name.c_str());
            delete p;
            p = NULL;
            e->error = true;
            continue;
         }
         
         fFeamAsm->AddPacket(imodule, map->fColumn, map->fRing, f, p, p8 + p->off, p->buf_len);
         have_feam = true;
      } else if (b->name[0] == 'P' && (b->name[1] == 'C')) {
         // PWB bank
         int c1 = b->name[2]-'0';
         int c2 = b->name[3]-'0';
         int imodule = c1*10 + c2;

         if (imodule < 0 || imodule > PWB_MODULE_LAST) {
            printf("AgAsm::UnpackEvent: bank name [%s] has invalid module number %d\n", b->name.c_str(), imodule);
            e->error = true;
            continue;
         }

         if (!fPwbModuleMap) {
            fPwbModuleMap = new PwbModuleMap();
            fPwbModuleMap->LoadFeamBanks(fFeamBanks);
            fPwbModuleMap->Print();
         }

         const PwbModuleMapEntry* map = fPwbModuleMap->FindPwb(imodule);
         
         const char* p8 = me->GetBankData(b);
         //Unused:
         //const uint32_t *p32 = (const uint32_t*)p8;
         //const int n32 = b->data_size/4;
         
         if (!fPwbAsm) {
            fPwbAsm = new PwbAsm();
         }

         fPwbAsm->AddPacket(imodule, map->fColumn, map->fRing, p8, b->data_size);
         have_pwb = true;
      } else {
         printf("AgAsm::UnpackEvent: unknown bank [%s]\n", b->name.c_str());
      }
   }

   if (e->trig && have_trg) {
      // nothing to do?
   }

   if (e->a16 && have_adc) {
      fAdcAsm->CheckEvent(e->a16);
   }

   if (fPwbAsm && have_pwb) {
      //if (fPwbAsm->CheckComplete()) {
      //printf("PwbAsm ---> complete !!!\n");
      if (!e->feam) {
         e->feam = new FeamEvent();
      }
      fPwbAsm->BuildEvent(e->feam);
      printf("PwbAsm built an event:\n");
      e->feam->Print();
      printf("\n");
      //PrintFeamChannels(e->feam->hits);
   }

   if (fFeamAsm && have_feam) {
      //printf("at end: FeamAsm status:\n");
      //fFeamAsm->Print();
      if (!e->feam) {
         e->feam = new FeamEvent();
      }
      fFeamAsm->BuildEvent(e->feam);

      if ((int)e->feam->modules.size() != fPwbModuleMap->fNumModules) {
         e->feam->complete = false;
      }
      
      //printf("FeamAsm built an event:\n");
      //e->feam->Print();
      //printf("\n");
      //PrintFeamChannels(e->feam->hits);
   }

   if (e->tdc && have_tdc) {
      // nothing to do?
   }

   double time = 0;

   // extract timestamps and event counters

   bool have_time = false;
   
   double trg_time = 0;
   double adc_time = 0;
   double pwb_time = 0;
   double tdc_time = 0;

   int trg_counter = 0;
   int adc_counter = 0;
   int pwb_counter = 0;
   int tdc_counter = 0;

   if (fTrgAsm) {
      if (e->trig) {
         trg_time = e->trig->time;
         trg_counter = e->trig->counter;
         if (!have_time) {
            time = trg_time;
            have_time = true;
         }
      } else {
         e->complete = false;
      }
   }

   if (fAdcAsm) {
      if (e->a16) {
         adc_time = e->a16->time;
         adc_counter = e->a16->counter;
         if (!have_time) {
            time = adc_time;
            have_time = true;
         }
      } else {
         e->complete = false;
      }
   }

   if (fFeamAsm || fPwbAsm) {
      if (e->feam) {
         pwb_time = e->feam->time;
         pwb_counter = e->feam->counter;
         if (!have_time) {
            time = pwb_time;
            have_time = true;
         }
      } else {
         e->complete = false;
      }
   }

   if (fTdcAsm) {
      if (e->tdc) {
         tdc_time = e->tdc->time;
         tdc_counter = e->tdc->counter;
         if (!have_time) {
            time = tdc_time;
            have_time = true;
         }
      } else {
         e->complete = false;
      }
   }

   // assign event time

   e->time = time;
   e->timeIncr = e->time - fLastEventTime;
   fLastEventTime = e->time;

   // check timestamps and event counters

   if (fTrgAsm) {
      if (e->trig) {
         double dt = trg_time - e->time;
         double absdt = fabs(dt);
         if (absdt > fTrgMaxDt)
            fTrgMaxDt = absdt;
         //printf("TRG check: ec %d %d, time %f %f sec, dt %.0f ns\n", e->counter, trg_counter, e->time, trg_time, dt*1e9);
         if (absdt > fConfMaxDt) {
            printf("AgAsm::UnpackEvent: event %d trg timestamp mismatch: time %f should be %f, dt %.0f ns\n", e->counter, trg_time, e->time, dt*1e9);
            e->error = true;
         }
         if (trg_counter != e->counter) {
            printf("AgAsm::UnpackEvent: event %d trg event counter mismatch: %d should be %d\n", e->counter, trg_counter, e->counter);
            e->error = true;
         }
      }
   }

   if (fAdcAsm) {
      if (e->a16) {
         double dt = adc_time - e->time;
         double absdt = fabs(dt);
         if (absdt > fAdcMaxDt)
            fAdcMaxDt = absdt;
         //printf("ADC check: ec %d %d, time %f %f sec, dt %.0f ns\n", e->counter, adc_counter, e->time, adc_time, dt*1e9);
         if (absdt > fConfMaxDt) {
            printf("AgAsm::UnpackEvent: event %d adc timestamp mismatch: time %f should be %f, dt %.0f ns\n", e->counter, adc_time, e->time, dt*1e9);
            e->error = true;
         }
         if (adc_counter != e->counter) {
            printf("AgAsm::UnpackEvent: event %d adc event counter mismatch: %d should be %d\n", e->counter, adc_counter, e->counter);
            e->error = true;
         }
      }
   }

   if (fFeamAsm || fPwbAsm) {
      if (e->feam) {
         double dt = pwb_time - e->time;
         double absdt = fabs(dt);
         if (absdt > fPwbMaxDt)
            fPwbMaxDt = absdt;
         //printf("PWB check: ec %d %d, time %f %f sec, dt %.0f ns\n", e->counter, pwb_counter, e->time, pwb_time, dt*1e9);
         if (absdt > fConfMaxDt) {
            printf("AgAsm::UnpackEvent: event %d pwb timestamp mismatch: time %f should be %f, dt %.0f ns\n", e->counter, pwb_time, e->time, dt*1e9);
            e->error = true;
         }
         if (pwb_counter != e->counter) {
            printf("AgAsm::UnpackEvent: event %d pwb event counter mismatch: %d should be %d\n", e->counter, pwb_counter, e->counter);
            e->error = true;
         }
      }
   }

   if (fTdcAsm) {
      if (e->tdc) {
         double dt = tdc_time - e->time;
         double absdt = fabs(dt);
         if (absdt > fAdcMaxDt)
            fTdcMaxDt = absdt;
         //printf("TDC check: ec %d %d, time %f %f sec, dt %.0f ns\n", e->counter, adc_counter, e->time, adc_time, dt*1e9);
         if (absdt > fConfMaxDt) {
            printf("AgAsm::UnpackEvent: event %d tdc timestamp mismatch: time %f should be %f, dt %.0f ns\n", e->counter, tdc_time, e->time, dt*1e9);
            e->error = true;
         }
         if (tdc_counter != e->counter) {
            printf("AgAsm::UnpackEvent: event %d tdc event counter mismatch: %d should be %d\n", e->counter, tdc_counter, e->counter);
            e->error = true;
         }
      }
   }

   // increment counters

   if (e->complete) {
      if (e->error) {
         fCountCompleteWithError++;
      } else {
         fCountComplete++;
      }
   } else {
      if (e->error) {
         fCountIncompleteWithError++;
      } else {
         fCountIncomplete++;
      }
   }

   // print final result
   
   if (1) {
      printf("AgAsm::UnpackEvent: returning event:\n");
      e->Print();
      printf("\n");
   }
  
   return e;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */


