//
// Spit lifetime into spill log. 
//
// L GOLINO
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "RecoFlow.h"
#include "A2Flow.h"
#include "TSpill.h"
#include "TSISChannels.h"


#include <iostream>
class LifetimeModuleFlags
{
public:
   bool fPrint = false;
   bool fLifetimeReady = false;
   bool fLifetimeDone = false;

};
class LifetimeModule: public TARunObject
{
private:
   clock_t start_time;
   uint32_t FirstEvent=0;
   //Once all of these exist and we see another cold dump. We add the lifetime measurment to spill log.
   TA2Spill* FifthDump0=NULL;
   TA2Spill* FifthDump1=NULL;
   TA2Spill* ColdDump=NULL;
   TA2Spill* Lifetime=NULL;


public:
   LifetimeModuleFlags* fFlags;
   bool fTrace = false;
   
   
   LifetimeModule(TARunInfo* runinfo, LifetimeModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="Lifetime Module";
#endif
      if (fTrace)
         printf("LifetimeModule::ctor!\n");
   }

   ~LifetimeModule()
   {
      if (fTrace)
         printf("LifetimeModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("LifetimeModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      start_time=clock();
    }

   void EndRun(TARunInfo* runinfo)
   {
      if (FifthDump0)
         delete FifthDump0;
      if (FifthDump1)
         delete FifthDump1;
      if (ColdDump)
         delete ColdDump;
      if (Lifetime)
         delete Lifetime;

      if(Lifetime && !HaveAllDumps())
         printf("LifetimeModule::EndRun, run %d\n. Lifetime run detected but not enough cold and/or fifth dumps detected for the calculation.", runinfo->fRunNo);
      if(Lifetime && HaveAllDumps() && !fFlags->fLifetimeDone)
         printf("LifetimeModule::EndRun, run %d\n. Lifetime run detected plus all conditions, but for some reason not calculated. Please alert Lukas.", runinfo->fRunNo);


      if (fTrace)
         printf("LifetimeModule::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("LifetimeModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("LifetimeModule::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   bool HaveAllDumps()
   {
       return (FifthDump0 && FifthDump1 && Lifetime && ColdDump);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
     if(fFlags->fLifetimeDone)
     {
        //This ensures we only calculate the lifetime once.
        return flow;;
     }
      A2SpillFlow* SpillFlow= flow->Find<A2SpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            TA2Spill* s=SpillFlow->spill_events.at(i);
            //s->Print();
            if (!s->SeqData) continue;
            int thisSeq=s->SeqData->fSequenceNum;
            if (thisSeq==0) //Catching trap
            //if (strcmp(s->SeqName.c_str(),"cat")==0)
            if (strcmp(s->Name.c_str(),"\"Fifth Dump\"")==0)
            {
               //If fifth dump found, check whether its first or second and save.
               if(FifthDump0 && FifthDump1)
               {
                    FifthDump0 = new TA2Spill(*FifthDump1);
                    FifthDump1 = new TA2Spill(*s);
               }
               else if(FifthDump0)
               {
                   FifthDump1 = new TA2Spill(*s);
               }
               else
               {
                   FifthDump0 = new TA2Spill(*s);
               }
            }
            if (strcmp(s->Name.c_str(),"\"Lifetime\"")==0)
            {
               //If lifetime found, save for later. 
               Lifetime = new TA2Spill(*s);
            }
            if (strcmp(s->Name.c_str(),"\"Cold Dump\"")==0)
            {
               if (HaveAllDumps())
               {
                  fFlags->fLifetimeReady = true;
               }
               else
               {
                   ColdDump = new TA2Spill(*s);
               }
            }

            if (fFlags->fLifetimeReady)
            {
                std::cout<<"lifetime_module::CalculateLifetime"<<std::endl;
                std::cout << "Lifetime run detected..." <<std::endl;
                std::ostringstream lifetimeStream;
                
                //Do lifetime calc. 
                double lifetimeHold = Lifetime->GetStopTime() - Lifetime->GetStartTime();
            
                int bestChannel = FindBestChannel(runinfo);
                double countsFD0 = FifthDump0->ScalerData->DetectorCounts[bestChannel];
                double countsFD1 = FifthDump1->ScalerData->DetectorCounts[bestChannel];
                double countsCD0 = ColdDump->ScalerData->DetectorCounts[bestChannel];
                double countsCD1 = s->ScalerData->DetectorCounts[bestChannel];
                double countslifetime = Lifetime->ScalerData->DetectorCounts[bestChannel];

                double normalised0 = countsCD0/countsFD0;
                double normalised1 = countsCD1/countsFD1;
                double logfactor = ( normalised0 / normalised1 );
                double lifetime = lifetimeHold / TMath::Log( logfactor );
                double lifetimeInMins = lifetime/60;

                lifetimeStream << "Lifetime measurment detected... Normalised to cold dumps the lifetime = " << lifetimeInMins << "m.";
                TA2Spill* LT = new TA2Spill(runinfo->fRunNo,Lifetime->ScalerData->GetStartTime(),lifetimeStream.str().c_str());
                SpillFlow->spill_events.push_back(LT);
                fFlags->fLifetimeDone=true;
            }
            
         }
      }
      else
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
      }
      return flow; 
  }


   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("LifetimeModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   int FindBestChannel(TARunInfo* runinfo) 
   {
       // AUTOMATICALLY FINDS THE GOOD CHANNEL
       // We use the cold dump to see where this lifetime was done.
       // We assume the channel with the most counts is the best place to measure the cold dump.
       // It coudl just do it on every channel and let the user decide but that seems non ideal.
      TSISChannels* channels = new TSISChannels(runinfo->fRunNo);
      std::vector<int> channelsToCheck
      {
         42, 
         channels->GetChannel("SIS_PMT_ATOM_OR"),
         channels->GetChannel("SIS_PMT_CATCH_OR"), 
         channels->GetChannel("SIS_PMT_5_AND_6"), 
         channels->GetChannel("SIS_PMT_7_AND_8"), 
         channels->GetChannel("PMT_12_AND_13"), 
         channels->GetChannel("CT_SiPM_OR"), 
         channels->GetChannel("PMT_10_AND_PMT_11")
      }; 
      int bestChannel = -1;
      int bestCount = 0;
      for(auto i: channelsToCheck)
      {
         if(ColdDump->ScalerData->DetectorCounts[i] > bestCount)
         {
            bestChannel = i;
            bestCount = ColdDump->ScalerData->DetectorCounts[i];
         }
      }
       return bestChannel;
   }
};

class LifetimeModuleFactory: public TAFactory
{
public:
   LifetimeModuleFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("LifetimeModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("LifetimeModuleFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("LifetimeModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new LifetimeModule(runinfo, &fFlags);
   }
};

static TARegister tar(new LifetimeModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
