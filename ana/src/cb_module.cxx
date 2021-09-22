//
// MIDAS analyzer for chronobox data
//
// K.Olchanski
//

#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>

#include "manalyzer.h"
#include "midasio.h"

class CbChan
{
public:
   int fChan = 0;
   int fLastTe = 0;
   int fLastTs = 0;
   double fEpoch = 0;
   double fLastTime = 0;
   double fLastLeTime = 0;
   double fLastTeTime = 0;
   int fCountHits = 0;
   int fCountMissingEdge = 0;
   bool fExpectWraparound = false;
   int fCountMissingWraparound = 0;
   int fCountUnexpectedWraparound = 0;
};

class CbModule: public TARunObject
{
public:
   const double kTsFreq = 10.0e6; // 10 MHz

   bool fPrintData = false;

   uint32_t fT0 = 0;
   uint32_t fTE = 0;
   int fCountWC = 0;
   int fCountMissingWC = 0;

   std::vector<CbChan*> fCbChan;

   CbModule(TARunInfo* runinfo, bool printData)
      : TARunObject(runinfo)
   {
      printf("ctor, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fRunEventCounter = 0;
      fPrintData = printData;
   }

   ~CbModule()
   {
      printf("dtor!\n");
   }
  
   void BeginRun(TARunInfo* runinfo)
   {
      printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fRunEventCounter = 0;
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("EndRun, run %d\n", runinfo->fRunNo);
      printf("Counted %d events in run %d\n", fRunEventCounter, runinfo->fRunNo);
      double elapsed = fTE - fT0;
      double minutes = elapsed/60.0;
      double hours = minutes/60.0;
      printf("Elapsed time %d -> %d is %.0f sec or %f minutes or %f hours\n", fT0, fTE, elapsed, minutes, hours);
      if (fCountMissingWC) {
         printf("Wraparounds: %d, missing %d, cannot compute TS frequency\n", fCountWC, fCountMissingWC);
      } else {
         double tsbits = (1<<24);
         printf("Wraparounds: %d, approx rate %f Hz, ts freq %.1f Hz\n", fCountWC, fCountWC/elapsed, 0.5*fCountWC/elapsed*tsbits);
      }
      for (unsigned chan = 0; chan < fCbChan.size(); chan++) {
         CbChan* c = fCbChan[chan];
         if (c == NULL)
            continue;
         printf("Channel %3d: %d hits, %d missing edges, %d missing wrap, %d unexpected wrap\n", chan, c->fCountHits, c->fCountMissingEdge, c->fCountMissingWraparound, c->fCountUnexpectedWraparound);
      }
   }

   void NextSubrun(TARunInfo* runinfo)
   {
      printf("NextSubrun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   int fPrevWC = 0;

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //      printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      //event->FindAllBanks();
      //printf("Event: %s\n", event->HeaderToString().c_str());
      //printf("Banks: %s\n", event->BankListToString().c_str());

      fRunEventCounter++;

      std::string bank_name = "CBS";
      TMBank* cbbank = 0;
      event->FindAllBanks();
      for( auto it = event->banks.begin(); it != event->banks.end(); ++it )
         {
            if( it->name.compare(0,3,bank_name) == 0 )
               {
                  cbbank = event->FindBank(it->name.c_str());
                  if (cbbank->name[3] == '2' || cbbank->name[3] == '1' )
                     return flow;
                  std::cout<<"Analyze: Found CB bank "<<cbbank->name<<std::endl;
                  break; // assuming 1 chronoboard
               }
         }
      //      TMBank* cbbank = event->FindBank(bank_name.c_str());
      if (cbbank) {
         if (fT0 == 0) {
            fT0 = event->time_stamp;
         }
         fTE = event->time_stamp;
         int nwords = cbbank->data_size/4; // byte to uint32_t words
         uint32_t* cbdata = (uint32_t*)event->GetBankData(cbbank);
         for (int i=0; i<nwords; i++) {
            uint32_t w = cbdata[i];
            if ((w & 0xFF000000) == 0xFF000000) {
               int tsbits = (w>>16)&0xFF;
               int wc = w & 0xFFFF;

               if (fPrintData) {
                  printf("data %d: 0x%08x, timestamp wraparound counter %d, ts bits 0x%02x\n", i, w, wc, tsbits);
               }

               if (fPrevWC == 0) {
                  fPrevWC = wc;
               } else if (wc != fPrevWC + 1) {
                  printf("ERROR: timestamp wraparound counter jump %d -> %d\n", fPrevWC, wc);
                  fCountMissingWC ++;
                  fPrevWC = wc;
               } else {
                  fPrevWC = wc;
                  fCountWC ++;
               }
               if (tsbits == 0x80) {
                  for (unsigned chan = 0; chan < fCbChan.size(); chan++) {
                     CbChan* c = fCbChan[chan];
                     if (c == NULL)
                        continue;
                     c->fExpectWraparound = true;
                  }
               }
            } else if ((w & 0xFF000000) == 0xFE000000) {
               int scwc = w & 0xFFFF;

               if (fPrintData) {
                  printf("data %d: 0x%08x, scalers block %d words\n", i, w, scwc);
               }

               // this code is incomplete

            } else if ((w & 0x80000000) == 0x80000000) {
               int chan = (w>>24)&0x7F;
               int ts = w&0xFFFFFE;
               int te = w&1;

               while (chan >= (int)fCbChan.size()) {
                  fCbChan.push_back(NULL);
               }
               CbChan* c = fCbChan[chan];

               if (c == NULL) {
                  c = new CbChan;
                  c->fChan = chan;
                  c->fLastTe = te;
                  c->fLastTs = ts;
                  c->fEpoch = 0;
                  c->fCountHits = 1;

                  double time = ts/kTsFreq;
                  c->fLastTime = time;
                  if (te)
                     c->fLastTeTime = time;
                  else
                     c->fLastLeTime = time;

                  fCbChan[chan] = c;

                  if (fPrintData) {
                     printf("data %d: 0x%08x, chan %d, te %d, time 0x%06x, %f sec, first hit\n", i, w, chan, te, ts, time);
                  }
               } else {
                  if (te == c->fLastTe) {
                     printf("ERROR: missing edge: chan %d, edge %d -> %d\n", c->fChan, c->fLastTe, te);
                     c->fCountMissingEdge++;
                  }
                  if (c->fExpectWraparound) {
                     if (ts > c->fLastTs) {
                        printf("ERROR: timestamp did not wraparound: chan %d, ts %d -> %d\n", c->fChan, c->fLastTs, ts);
                        c->fCountMissingWraparound++;
                     } else {
                        c->fEpoch += (1<<24)/kTsFreq;
                     }
                     c->fExpectWraparound = false;
                  } else if (ts < c->fLastTs) {
                     printf("ERROR: timestamp wraparound: chan %d, ts %d -> %d\n", c->fChan, c->fLastTs, ts);
                     c->fCountUnexpectedWraparound++;
                     c->fEpoch += (1<<24)/kTsFreq;
                  }

                  double time = c->fEpoch + ts/kTsFreq;
                  double dt = time - c->fLastTime;
                  
                  if (fPrintData) {
                     printf("data %d: 0x%08x, chan %d, te %d, time 0x%06x, %f sec, plus %f usec, from last le %f, te %f\n", i, w, chan, te, ts, time, dt*1e6, (time-c->fLastLeTime)*1e6, (time-c->fLastTeTime)*1e6);
                  }

                  c->fCountHits ++;
                  c->fLastTe = te;
                  c->fLastTs = ts;
                  c->fLastTime = time;
                  if (te)
                     c->fLastTeTime = time;
                  else
                     c->fLastLeTime = time;
               }
            } else {
               printf("data %d: 0x%08x, do not know what this is\n", i, w);
            }
         }
      }
      // else
      //    std::cout<<"no bank: "<<bank_name<<std::endl;

      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   int fRunEventCounter;
};

class CbModuleFactory: public TAFactory
{
public:
   bool fPrintCbData = false;

   void Usage()
   {
      printf("\tCbModuleFactory Usage:\n");
      printf("\t--print-cb-data : print chronobox raw data\n");
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("Init!\n");
      printf("Arguments:\n");
      for (unsigned i=0; i<args.size(); i++) {
         printf("arg[%d]: [%s]\n", i, args[i].c_str());
         if (args[i] == "--print-cb-data") {
            fPrintCbData = true;
         }
      }
   }
   
   void Finish()
   {
      printf("Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new CbModule(runinfo, fPrintCbData);
   }
};

static TARegister tar(new CbModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
