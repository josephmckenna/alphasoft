// HandleVF48.cxx

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TH2.h"
#include "TF1.h"
#include "TLatex.h"
#include "TText.h"
#include "TBox.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TTree.h"


#include "manalyzer.h"
#include "midasio.h"

#include "TSettings.h"

#include "SiMod.h"
#include "UnpackVF48.h"

#define MAX_CHANNELS VF48_MAX_CHANNELS // defined in UnpackVF48.h
#define NUM_SI_MODULES nSil // defined in SiMod.h
#define NUM_VF48_MODULES nVF48 // defined in SiMod.h
#define NUM_SI_ALPHA1 nSiAlpha1 // defined in SiMod.h
#define NUM_SI_ALPHA2 nSiAlpha2 // defined in SiMod.h

#define VF48_COINCTIME 0.000010

class UnpackFlags
{
public:
   bool fPrint = false;
   bool fUnpackOff = false; //Turn off unpacking (reconstruction totally off)
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;

   bool fStopUnpackAfterTime = false;
   double fStopUnpackAfter = -1.;

};

class UnpackModule: public TARunObject
{
public:
   UnpackFlags* fFlags = NULL;
   UnpackVF48* vfu; 
   bool fTrace = false;
      
   // VF48 sampling parameters from sqlite db?

   // Sub-sampling settings
   // int soffset(-1);
   // double subsample(-1.);
   //  int offset(-1);
   double gSubSample[NUM_VF48_MODULES];
   int gOffset[NUM_VF48_MODULES];
   int gSOffset[NUM_VF48_MODULES];
   
   double gSettingsFrequencies[VF48_MAX_MODULES];


   UnpackModule(TARunInfo* runinfo, UnpackFlags* flags)
      : TARunObject(runinfo)
   {
      if (fTrace)
         printf("UnpackModule::ctor!\n");
      vfu = new UnpackVF48();
      // load the sqlite3 db
      char dbName[255]; 
      sprintf(dbName,"%s/a2lib/main.db",getenv("AGRELEASE"));
      TSettings *gSettingsDB = new TSettings(dbName,runinfo->fRunNo);      
      for (int m=0; m<NUM_VF48_MODULES; m++)
      {
         gSettingsFrequencies[m]= gSettingsDB->GetVF48Frequency( runinfo->fRunNo, m);
         vfu->SetTsFreq(m,gSettingsFrequencies[m]);
         // extract VF48 sampling parameters from sqlite db
         gSubSample[m] = gSettingsDB->GetVF48subsample( runinfo->fRunNo,m );
         gOffset[m] = gSettingsDB->GetVF48offset( runinfo->fRunNo, m );
         gSOffset[m] = gSettingsDB->GetVF48soffset( runinfo->fRunNo, m );
         if( gSubSample[m] < 1. || gOffset[m] < 0. || gSOffset[m] < 0. )
         {
            printf("PROBLEM: Unphysical VF48 sampling parameters:\n");
            printf("subsample = %f \t offset = %d \t soffset = %d \n", gSubSample[m], gOffset[m], gSOffset[m]);
            exit(0);
         }
      }
      fFlags   = flags;
   }

   ~UnpackModule()
   {
      if (fTrace)
         printf("UnpackModule::dtor!\n");

   }


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("UnpackModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      int module = 0;
      int samples    = runinfo->fOdb->odbReadInt("/equipment/VF48/Settings/VF48_NumSamples",module,0);
      int grpEnabled = runinfo->fOdb->odbReadInt("/equipment/VF48/Settings/VF48_GroupEnable",module,0);

      printf("Module %d, samples: %d, grpEnable: 0x%x\n", module, samples, grpEnabled);
      vfu->SetFlushIncompleteThreshold(40);
      vfu->SetNumModules(NUM_VF48_MODULES);
      vfu->SetGroupEnableMask(-1, grpEnabled);
      vfu->SetNumSamples(-1, samples);
      vfu->SetCoincTime(VF48_COINCTIME);
      vfu->Reset();

      Int_t Module, ASIC, k;
      Module=ASIC=k=0;

      for (int m=0; m<NUM_VF48_MODULES; m++)
      {
      gSettingsFrequencies[m]= gSettingsDB->GetVF48Frequency( gRunNumber, m);
      vfu->SetTsFreq(m,gSettingsFrequencies[m]);
      // extract VF48 sampling parameters from sqlite db
      gSubSample[m] = gSettingsDB->GetVF48subsample( gRunNumber,m );
      gOffset[m] = gSettingsDB->GetVF48offset( gRunNumber, m );
      gSOffset[m] = gSettingsDB->GetVF48soffset( gRunNumber, m );
      if( gSubSample[m] < 1. || gOffset[m] < 0. || gSOffset[m] < 0. )     {
      printf("PROBLEM: Unphysical VF48 sampling parameters:\n");
      printf("subsample = %f \t offset = %d \t soffset = %d \n", gSubSample[m], gOffset[m], gSOffset[m]);
      exit(0);
    }
    }


   }

   void PreEndRun(TARunInfo* runinfo, std::deque<TAFlowEvent*>* flow_queue)
   {
      if (fTrace)
         printf("UnpackModule::PreEndRun, run %d\n", runinfo->fRunNo);
      //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));

      if (fFeamEvb) {
         int count_feam = 0;

         //printf("UnpackModule::PreEndRun: FeamEVB state:\n");
         //fFeamEvb->Print();

         while (1) {
            FeamEvent *e = fFeamEvb->GetLastEvent();
            if (!e)
               break;

            if (1) {
               printf("Unpacked PWB event: ");
               e->Print();
               printf("\n");
            }

            if (0) {
               for (unsigned i=0; i<e->modules.size(); i++) {
                  if (!e->modules[i])
                     break;
                  FeamModuleData* m = e->modules[i];
                  printf("pwb%02d, cnt %4d, ts_trig: 0x%08x %14.3f usec, ts_incr %14.3f usec\n", m->fModule, m->cnt, m->ts_trig, m->fTime*1e6, m->fTimeIncr*1e6);
                  //a->Print();
                  //printf("\n");
               }
            }

            if (fAgEvb) {
               count_feam += 1;
               fAgEvb->AddFeamEvent(e);
               e = NULL;
            }

            DELETE(e);
         }

         printf("UnpackModule::PreEndRun: FeamEVB final state:\n");
         fFeamEvb->Print();

         printf("UnpackModule::PreEndRun: Unpacked %d last FEAM events\n", count_feam);
      }

      // Handle leftover AgEVB events

      if (fAgEvb) {
         int count_agevent = 0;

         printf("UnpackModule::PreEndRun: AgEVB state:\n");
         fAgEvb->Print();

         while (1) {
            AgEvent *e = fAgEvb->GetLastEvent();
            if (!e)
               break;

            if (1) {
               printf("Unpacked AgEvent: ");
               e->Print();
               printf("\n");
            }

            count_agevent += 1;

            flow_queue->push_back(new AgEventFlow(NULL, e));
         }

         printf("UnpackModule::PreEndRun: AgEVB final state:\n");
         fAgEvb->Print();

         printf("UnpackModule::PreEndRun: Unpacked %d last AgEvent events\n", count_agevent);
      }
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("UnpackModule::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("UnpackModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if (fFlags->fUnpackOff)
         return flow;

      if (event->event_id != 1)
         return flow;


      bool short_tpc = (runinfo->fRunNo < 1450);

      if (0) {
         const time_t now = time(NULL);
         const time_t t = event->time_stamp;
         int dt = now - t;
         printf("UnpackModule: serial %d, time %d, age %d, date %s\n", event->serial_number, event->time_stamp, dt, ctime(&t));
      }

      if (0) {
         TMBank* pwb_bank = event->FindBank("PB02");

         if (pwb_bank) {
            const char* p8 = event->GetBankData(pwb_bank);
            const uint32_t *p32 = (const uint32_t*)p8;
            const int n32 = pwb_bank->data_size/4;

            if (0) {
               unsigned nprint = pwb_bank->data_size/4;
               nprint=10;
               for (unsigned i=0; i<nprint; i++) {
                  printf("PB05[%d]: 0x%08x (%d)\n", i, p32[i], p32[i]);
                  //e->udpData.push_back(p32[i]);
               }
            }
#if 1
            uint32_t MYSTERY     = p32[0];
            uint32_t PKT_SEQ     = p32[1];
            uint32_t CHANNEL_SEQ = (p32[2] >>  0) & 0xFFFF;
            uint32_t CHANNEL_ID  = (p32[2] >> 16) & 0xFF;
            uint32_t FLAGS       = (p32[2] >> 24) & 0xFF;
            uint32_t CHUNK_ID    = (p32[3] >>  0) & 0xFFFF;
            uint32_t CHUNK_LEN   = (p32[3] >> 16) & 0xFFFF;
            uint32_t HEADER_CRC  = p32[4];
            uint32_t end_of_payload = 5*4 + CHUNK_LEN;
            uint32_t payload_crc = p32[end_of_payload/4];

            if (0) {
               printf("M 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, LEN 0x%04x, CRC 0x%08x, bank bytes %d, end of payload %d, CRC 0x%08x\n",
                      MYSTERY,
                      PKT_SEQ,
                      CHANNEL_SEQ,
                      CHANNEL_ID,
                      FLAGS,
                      CHUNK_ID,
                      CHUNK_LEN,
                      HEADER_CRC,
                      pwb_bank->data_size,
                      end_of_payload,
                      payload_crc);
            }

            if (0) {
               printf("M 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x\n",
                      MYSTERY,
                      PKT_SEQ,
                      CHANNEL_SEQ,
                      CHANNEL_ID,
                      FLAGS,
                      CHUNK_ID);
            }

            static int xcount[4];

            if (CHUNK_ID == 0) {
               if (0) {
                  for (unsigned i=5; i<20; i++) {
                     printf("PB05[%d]: 0x%08x (%d)\n", i, p32[i], p32[i]);
                     //e->udpData.push_back(p32[i]);
                  }
               }

               int FormatRevision  = (p32[5]>> 0) & 0xFF;
               int ScaId           = (p32[5]>> 8) & 0xFF;
               int CompressionType = (p32[5]>>16) & 0xFF;
               int TriggerSource   = (p32[5]>>24) & 0xFF;

               uint32_t HardwareId1 = p32[6];

               uint32_t HardwareId2 = (p32[7]>> 0) & 0xFFFF;
               int TriggerDelay     = (p32[7]>>16) & 0xFFFF;

               // NB timestamp clock is 125 MHz

               uint32_t TriggerTimestamp1 = p32[8];

               uint32_t TriggerTimestamp2 = (p32[9]>> 0) & 0xFFFF;
               uint32_t Reserved1         = (p32[9]>>16) & 0xFFFF;

               int ScaLastCell = (p32[10]>> 0) & 0xFFFF;
               int ScaSamples  = (p32[10]>>16) & 0xFFFF;

               uint32_t ScaChannelsSent1 = p32[11];
               uint32_t ScaChannelsSent2 = p32[12];

               uint32_t ScaChannelsSent3 = (p32[13]>> 0) & 0xFF;
               uint32_t ScaChannelsThreshold1 = (p32[13]>> 8) & 0xFFFFFF;

               uint32_t ScaChannelsThreshold2 = p32[14];

               uint32_t ScaChannelsThreshold3 = p32[15] & 0xFFFF;
               uint32_t Reserved2             = (p32[15]>>16) & 0xFFFF;

               if (1) {
                  printf("M 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, TS 0x%08x\n",
                         MYSTERY,
                         PKT_SEQ,
                         CHANNEL_SEQ,
                         CHANNEL_ID,
                         FLAGS,
                         CHUNK_ID,
                         TriggerTimestamp1
                         );
               }

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

               if (0) {
                  printf("S Sca 0x%02x, TS 0x%08x, 0x%04x, SCA LastCell 0x%04x, Samples 0x%04x\n",
                         ScaId,
                         TriggerTimestamp1, TriggerTimestamp2,
                         ScaLastCell,
                         ScaSamples);

                  int ptr32 = 16;

                  while (ptr32 < n32) {
                     int channel = p32[ptr32] & 0xFFFF;
                     int samples = (p32[ptr32]>>16) & 0xFFFF;
                     int nw = samples/2;
                     if (samples&1)
                        nw+=1;
                     printf("C ptr %d, channel %d, samples %d, nw %d\n", ptr32, channel, samples, nw);
                     ptr32 += 1+nw;
                     if (nw <= 0)
                        break;
                  }
               }
               xcount[CHANNEL_ID]=1;
            } else if (FLAGS & 1) {
               xcount[CHANNEL_ID]++;
               if (1) {
                  printf("M 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, LAST of %d packets\n",
                         MYSTERY,
                         PKT_SEQ,
                         CHANNEL_SEQ,
                         CHANNEL_ID,
                         FLAGS,
                         CHUNK_ID,
                         xcount[CHANNEL_ID]);
               }
               xcount[CHANNEL_ID]=0;
            } else {
               xcount[CHANNEL_ID]++;
               if (1) {
                  printf("M 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, count %d\n",
                         MYSTERY,
                         PKT_SEQ,
                         CHANNEL_SEQ,
                         CHANNEL_ID,
                         FLAGS,
                         CHUNK_ID,
                         xcount[CHANNEL_ID]);
               }
            }

            return flow;
#endif
         }
      }

      if (fAgAsm) {

         AgEvent* e = fAgAsm->UnpackEvent(event);

      if (fFlags->fStopUnpackAfterTime)
         if (e->time > fFlags->fStopUnpackAfter)
            fFlags->fUnpackOff=true;


      if (fFlags->fTimeCut)
      {
        if (e->time<fFlags->start_time)
          return new AgEventFlow(flow, e);
        if (e->time>fFlags->stop_time)
          return new AgEventFlow(flow, e);
      }

      if (fFlags->fEventRangeCut)
      {
         if (e->counter<fFlags->start_event)
           return new AgEventFlow(flow, e);
         if (e->counter>fFlags->stop_event)
           return new AgEventFlow(flow, e);
      }

      if (0) {
         printf("%s:%d\n",__FILE__,__LINE__);
         printf("Unpacked AgEvent:   ");
         e->Print();
         printf("\n");
      }
         #ifdef _TIME_ANALYSIS_
            if (TimeModules) flow=new AgAnalysisReportFlow(flow,"unpack_module(AgAsm)");
         #endif
         return new AgEventFlow(flow, e);
      }

      if (1) {
         TMBank* atat_bank = event->FindBank("ATAT");

         if (atat_bank) {
            TrigEvent* e = UnpackTrigEvent(event, atat_bank);

            if (e) {
               if (1) {
                  printf("Unpacked TRG event: ");
                  e->Print();
                  printf("\n");
               }

               if (fAgEvb && e->time >= 0) {
                  fAgEvb->AddTrigEvent(e);
                  e = NULL;
               }

               DELETE(e);
            }
         }
      }

      if (1) {
         TMBank* pwb_bank = event->FindBank("PB05");

         if (pwb_bank) {
            const char* p8 = event->GetBankData(pwb_bank);
            const uint32_t *p32 = (const uint32_t*)p8;
            const int n32 = pwb_bank->data_size/4;

            if (0) {
               unsigned nprint = n32;
               nprint=10;
               for (unsigned i=0; i<nprint; i++) {
                  printf("PB05[%d]: 0x%08x (%d)\n", i, p32[i], p32[i]);
                  //e->udpData.push_back(p32[i]);
               }
            }

            if (!gPwbAsm)
               gPwbAsm = new PwbAsm();

            gPwbAsm->AddPacket(5, 1, 2, event->GetBankData(pwb_bank), pwb_bank->data_size);

            if (gPwbAsm->CheckComplete()) {
               printf("PwbAsm ---> complete !!!\n");
               FeamEvent* e = new FeamEvent();
               gPwbAsm->BuildEvent(e);
               e->Print();
               printf("Printing all FEAM channels:\n");
               PrintFeamChannels(e->hits);

               if (fAgEvb) {
                  fAgEvb->AddFeamEvent(e);
                  e = NULL;
               }

               if (e) {
                  delete e;
                  e = NULL;
               }
            }

#if 0
            uint32_t MYSTERY     = p32[0];
            uint32_t PKT_SEQ     = p32[1];
            uint32_t CHANNEL_SEQ = (p32[2] >>  0) & 0xFFFF;
            uint32_t CHANNEL_ID  = (p32[2] >> 16) & 0xFF;
            uint32_t FLAGS       = (p32[2] >> 24) & 0xFF;
            uint32_t CHUNK_ID    = (p32[3] >>  0) & 0xFFFF;
            uint32_t CHUNK_LEN   = (p32[3] >> 16) & 0xFFFF;
            uint32_t HEADER_CRC  = p32[4];
            uint32_t end_of_payload = 5*4 + CHUNK_LEN;
            uint32_t payload_crc = p32[end_of_payload/4];
            printf("M 0x%08x, PKT_SEQ 0x%08x, CHAN SEQ 0x%04x, ID 0x%02x, FLAGS 0x%02x, CHUNK ID 0x%04x, LEN 0x%04x, CRC 0x%08x, bank bytes %d, end of payload %d, CRC 0x%08x\n",
                   MYSTERY,
                   PKT_SEQ,
                   CHANNEL_SEQ,
                   CHANNEL_ID,
                   FLAGS,
                   CHUNK_ID,
                   CHUNK_LEN,
                   HEADER_CRC,
                   pwb_bank->data_size,
                   end_of_payload,
                   payload_crc);

            if (CHUNK_ID == 0) {
               if (0) {
                  for (unsigned i=5; i<20; i++) {
                     printf("PB05[%d]: 0x%08x (%d)\n", i, p32[i], p32[i]);
                     //e->udpData.push_back(p32[i]);
                  }
               }

               int FormatRevision  = (p32[5]>> 0) & 0xFF;
               int ScaId           = (p32[5]>> 8) & 0xFF;
               int CompressionType = (p32[5]>>16) & 0xFF;
               int TriggerSource   = (p32[5]>>24) & 0xFF;

               uint32_t HardwareId1 = p32[6];

               uint32_t HardwareId2 = (p32[7]>> 0) & 0xFFFF;
               int TriggerDelay     = (p32[7]>>16) & 0xFFFF;

               // NB timestamp clock is 125 MHz

               uint32_t TriggerTimestamp1 = p32[8];

               uint32_t TriggerTimestamp2 = (p32[9]>> 0) & 0xFFFF;
               uint32_t Reserved1         = (p32[9]>>16) & 0xFFFF;

               int ScaLastCell = (p32[10]>> 0) & 0xFFFF;
               int ScaSamples  = (p32[10]>>16) & 0xFFFF;

               uint32_t ScaChannelsSent1 = p32[11];
               uint32_t ScaChannelsSent2 = p32[12];

               uint32_t ScaChannelsSent3 = (p32[13]>> 0) & 0xFF;
               uint32_t ScaChannelsThreshold1 = (p32[13]>> 8) & 0xFFFFFF;

               uint32_t ScaChannelsThreshold2 = p32[14];

               uint32_t ScaChannelsThreshold3 = p32[15] & 0xFFFF;
               uint32_t Reserved2             = (p32[15]>>16) & 0xFFFF;

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

               printf("S Sca 0x%02x, TS 0x%08x, 0x%04x, SCA LastCell 0x%04x, Samples 0x%04x\n",
                      ScaId,
                      TriggerTimestamp1, TriggerTimestamp2,
                      ScaLastCell,
                      ScaSamples);

               int ptr32 = 16;

               while (ptr32 < n32) {
                  int channel = p32[ptr32] & 0xFFFF;
                  int samples = (p32[ptr32]>>16) & 0xFFFF;
                  int nw = samples/2;
                  if (samples&1)
                     nw+=1;
                  printf("C ptr %d, channel %d, samples %d, nw %d\n", ptr32, channel, samples, nw);
                  ptr32 += 1+nw;
                  if (nw <= 0)
                     break;
               }
            }
#endif
         }
      }

      if (fAdcAsm) {
         Alpha16Event* e = UnpackAlpha16Event(fAdcAsm, event);

         if (e) {
            if (1) {
               printf("Unpacked ADC event: ");
               e->Print();
               printf("\n");
            }

            if (fAgEvb && e->time >= 0) {
               fAgEvb->AddAlpha16Event(e);
               e = NULL;
            }

            DELETE(e);
         }
      }

      if (fFeamEvb) {
         FeamEvent *e = UnpackFeamEvent(fFeamEvb, event, fFeamBanks, short_tpc);
         while (1) {
            if (!e)
               break;

            if (1) {
               printf("Unpacked PWB event: ");
               e->Print();
               printf("\n");
            }

            if (0) {
               for (unsigned i=0; i<e->modules.size(); i++) {
                  if (!e->modules[i])
                     break;
                  FeamModuleData* m = e->modules[i];
                  printf("pwb%02d, cnt %4d, ts_trig: 0x%08x %14.3f usec, ts_incr %14.3f usec\n", m->fModule, m->cnt, m->ts_trig, m->fTime*1e6, m->fTimeIncr*1e6);
                  //a->Print();
                  //printf("\n");
               }
            }

            if (fAgEvb) {
               fAgEvb->AddFeamEvent(e);
               e = NULL;
            }

            DELETE(e);

            e = fFeamEvb->Get();
         }
      }

      if (fAgEvb) {
         while (1) {
            AgEvent* e = fAgEvb->Get();
            if (!e)
               break;

            if (1) {
               printf("Unpacked AgEvent:   ");
               e->Print();
               printf("\n");
            }

            runinfo->fFlowQueue.push_back(new AgEventFlow(NULL, e));
         }
      }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"unpack_module");
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("UnpackModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class UnpackModuleFactory: public TAFactory
{
public:
   UnpackFlags fFlags;

public:
   void Help()
   {
      printf("UnpackModuleFactory::Help!\n");
      printf("\t--print      Turn printing on\n");
      printf("\t--noadc      Turn adc off\n");
      printf("\t--nopwb      Turn pwd off\n");
      printf("\t--nounpack   Turn unpacking of TPC data (turn off reconstruction completely)\n");
      printf("\t--usetimerange 123.4 567.8\t\tLimit reconstruction to a time range\n");
      printf("\t--useeventrange 123 567\t\tLimit reconstruction to an event range\n");
      printf("\t--stopunpackafter 567.8\t\tStop unpacking after time\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("UnpackModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--nounpack")
            fFlags.fUnpackOff = true;
         if( args[i] == "--usetimerange" )
            {
               fFlags.fTimeCut=true;
               i++;
               fFlags.start_time=atof(args[i].c_str());
               i++;
               fFlags.stop_time=atof(args[i].c_str());
               printf("Using time range for reconstruction: ");
               printf("%f - %fs\n",fFlags.start_time,fFlags.stop_time);
            }
         if( args[i] == "--useeventrange" )
            {
               fFlags.fEventRangeCut=true;
               i++;
               fFlags.start_event=atoi(args[i].c_str());
               i++;
               fFlags.stop_event=atoi(args[i].c_str());
               printf("Using event range for reconstruction: ");
               printf("Analyse from (and including) %d to %d\n",fFlags.start_event,fFlags.stop_event);
            }
         if( args[i] == "--stopunpackafter" )
            {
               fFlags.fStopUnpackAfterTime=true;
               i++;
               fFlags.fStopUnpackAfter=atof(args[i].c_str());
            }
      }
   }

   void Finish()
   {
      printf("UnpackModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("UnpackModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new UnpackModule(runinfo, &fFlags);
   }
};

static TARegister tar(new UnpackModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
