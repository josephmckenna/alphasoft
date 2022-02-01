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
#include <sstream>

class LifetimeModuleFlags
{
public:
   bool fPrint = false;
};

class LifetimeModule: public TARunObject
{
private:
   //Once all of these exist and we see another cold dump. We add the lifetime measurment to spill log.
   TA2Spill* fFirstFifthDump=NULL; //These two need to be saved permanently I guess. 
   TA2Spill* fColdDump=NULL;
   
   TA2Spill* fSecondFifthDump=NULL;
   TA2Spill* fLifetime=NULL;
   bool fLifetimeDone = false;


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
      fLifetimeDone = false;
      ResetAllDumps();
   }

   void ResetAllDumps()
   {
      if (fFirstFifthDump)
      {
         delete fFirstFifthDump;
         fFirstFifthDump = NULL;
      }
      if (fSecondFifthDump)
      {
         delete fSecondFifthDump;
         fSecondFifthDump = NULL;
      }
      if (fColdDump)
      {
         delete fColdDump;
         fColdDump = NULL;
      }
      if (fLifetime)
      {
         delete fLifetime;
         fLifetime = NULL;
      }
   }

   void ResetStoredDumps()
   {
      if (fSecondFifthDump)
      {
         delete fSecondFifthDump;
         fSecondFifthDump = NULL;
      }
      if (fLifetime)
      {
         delete fLifetime;
         fLifetime = NULL;
      }
   }

   void EndRun(TARunInfo* runinfo)
   {
      if(fLifetime && !HaveAllDumps())
         printf("LifetimeModule::EndRun, run %d\n. Lifetime run detected but not enough cold and/or fifth dumps found for the calculation. This module requires 2xCold Dumps, 2xFifth Dumps, and 1xLifetime Dump to automate the calculation.", runinfo->fRunNo);
      if(fLifetime && HaveAllDumps())
         printf("LifetimeModule::EndRun, run %d\n. Lifetime run detected plus all conditions, but for some reason not calculated. Please alert Lukas.", runinfo->fRunNo);

      ResetAllDumps();

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

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      A2SpillFlow* SpillFlow= flow->Find<A2SpillFlow>();
      if (!SpillFlow)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      for (const TA2Spill* s: SpillFlow->spill_events)
      {
         //s->Print();
         if (!s->SeqData) continue;
         int thisSeq=s->SeqData->fSequenceNum;
         if (thisSeq==0) //Catching trap
         //if (strcmp(s->SeqName.c_str(),"cat")==0)
         if (strcmp(s->Name.c_str(),"\"Fifth Dump\"")==0)
         {
            //If fifth dump found, check whether its first or second and save.
            if(fFirstFifthDump && fSecondFifthDump)
            {
               //If both are already saved we need to shuffle them forward in momery s.t. 2nd dump is the current, and the 1st was the one before.
               //Instead if both are saved lets just update the newest one. This now means the first 5th and cold dumps we see will be saved throughout the entirety of the run as a baseline. 
               //delete fFirstFifthDump;
               //fFirstFifthDump = fSecondFifthDump;
               delete fSecondFifthDump;
               fSecondFifthDump = new TA2Spill(*s);
            }
            else if(fFirstFifthDump)
            {
               //If we have one, save the other.
               fSecondFifthDump = new TA2Spill(*s);
            }
            else
            {
               //If we have none, save the first.
               fFirstFifthDump = new TA2Spill(*s);
            }
         }
         if (strcmp(s->Name.c_str(),"\"Lifetime\"")==0)
         {
            //If lifetime found, save for later. 
            fLifetime = new TA2Spill(*s);
            std::cout << "Lifetime run detected..." <<std::endl; //We announce detection of lifetime dump here. If the user does not run the sequence with required dumps the end run will let you know. 
         }
         else if (strcmp(s->Name.c_str(),"\"Cold Dump\"")==0)
         {
            //If we haven't saved the initial cold dump save it now, this should only happen once. 
            if (!fColdDump)
            {
               //delete fColdDump;
               fColdDump = new TA2Spill(*s);
            }
            if (HaveAllDumps())
            {
               //Once we have all the conditions to calculate a lifetime dump, lets do it in this AnalyzeFlowEvent(). 
               //Do lifetime calc. 
               const double lifetimeInMins = CalculateLifetime(runinfo, s);
               std::ostringstream lifetimeStream;
               lifetimeStream << "Lifetime measurement detected... Normalized to fifth dumps the lifetime = " << lifetimeInMins << "m.";
               TA2Spill* lifetimeSpill = new TA2Spill(runinfo->fRunNo,fLifetime->ScalerData->GetStartTime(),lifetimeStream.str().c_str());
               SpillFlow->spill_events.push_back(lifetimeSpill);
               // We've finished with all the dumps... lets get ready for another (in case users 
               // decide to run a lifetime multiple times)
               ResetStoredDumps();
            }
            else
            {
               //Else save the cold dump for later and wait until we have everything we need.
            }
         }
      }
      return flow; 
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("LifetimeModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   bool HaveAllDumps() const
   {
       return (fFirstFifthDump && fSecondFifthDump && fLifetime && fColdDump);
   }

   TSISChannel FindBestChannel(TARunInfo* runinfo) 
   {
       // AUTOMATICALLY FINDS THE GOOD CHANNEL
       // We use the cold dump numbers to see where this lifetime was done.
       // We assume the channel with the most counts is the best place to measure the cold dump.
       // We could just do it on every channel and let the user decide which is useful like the efficiency module but I prefer this.
         //Note: To do this we would require a function that takes all our dumps as an input and returns lifetime dump. It would go where the / operator overload in CE module goes.
       // Searching over so many channels could be a major cause of slowdown. Also saving 5 dumps in memory is non ideal. 
      TSISChannels channels(runinfo->fRunNo);
      std::vector<TSISChannel> channelsToCheck
      {
         TSISChannel(42), //??
         channels.GetChannel("SIS_PMT_ATOM_OR"),
         channels.GetChannel("SIS_PMT_CATCH_OR"), 
         channels.GetChannel("SIS_PMT_5_AND_6"), 
         channels.GetChannel("SIS_PMT_7_AND_8"), 
         channels.GetChannel("PMT_12_AND_13"), 
         channels.GetChannel("CT_SiPM_OR"), 
         channels.GetChannel("SiPM_A"), //AlphaG channels?
         channels.GetChannel("SiPM_B"), 
         channels.GetChannel("SiPM_C"), 
         channels.GetChannel("SiPM_D"), 
         channels.GetChannel("SiPM_E"), 
         channels.GetChannel("SiPM_F"), 
         channels.GetChannel("PMT_10_AND_PMT_11")
      }; 
      TSISChannel bestChannel = -1;
      int bestCount = 0;
      for(const auto i: channelsToCheck)
      {
         // toInt() isn't a great solution... do we want a operator overload for [] and TSISChannel / TChronoChannel
         if(fColdDump->ScalerData->DetectorCounts[i.toInt()] > bestCount)
         {
            bestChannel = i;
            bestCount = fColdDump->ScalerData->DetectorCounts[i.toInt()];
         }
      }
       return bestChannel;
   }

   double CalculateLifetime(TARunInfo* runinfo, const TA2Spill* finalColdDump)
   {
      std::cout<<"lifetime_module::CalculateLifetime"<<std::endl;
      TSISChannel bestChannel = FindBestChannel(runinfo);

      size_t smallestChannelsSize = std::min({fFirstFifthDump->ScalerData->DetectorCounts.size(), fSecondFifthDump->ScalerData->DetectorCounts.size(),  
         fColdDump->ScalerData->DetectorCounts.size(), finalColdDump->ScalerData->DetectorCounts.size(), fLifetime->ScalerData->DetectorCounts.size()});
      
      //smallestChannelsSize = std::min(smallestChannelsSize, finalColdDump->ScalerData->DetectorCounts.size(), fLifetime->ScalerData->DetectorCounts.size());

      if(!bestChannel.IsValid())
      {
         std::cout << "Error in lifetime module. Either bad counts or bad channels." <<std::endl;
         return -1;
      }
      
      //Step by step - good for debugging, potentially worse for performance. - No need to use .at() due to the checks above?
      double lifetimeHold = fLifetime->GetStopTime() - fLifetime->GetStartTime();
      // I am not totally happy with toInt() as a solution. the Scaler data should maybe have an overload for the [] operator with a TSISChannel / TChronoChannel?
      double countsFD0 = fFirstFifthDump->ScalerData->DetectorCounts[bestChannel.toInt()];
      double countsFD1 = fSecondFifthDump->ScalerData->DetectorCounts[bestChannel.toInt()];
      double countsCD0 = fColdDump->ScalerData->DetectorCounts[bestChannel.toInt()];
      double countsCD1 = finalColdDump->ScalerData->DetectorCounts[bestChannel.toInt()];
      double countslifetime = fLifetime->ScalerData->DetectorCounts[bestChannel.toInt()];
      double normalised0 = countsCD0/countsFD0;
      double normalised1 = countsCD1/countsFD1;
      double logfactor = ( normalised0 / normalised1 );
      double lifetime = lifetimeHold / TMath::Log( logfactor );
      double lifetimeInMins = lifetime/60;

      //This refuses to work as a one liner. For now we keep the above method but these are here if someone decides to make this work. 
      /*double lifetimeInMins = ((fLifetime->GetStopTime() - fLifetime->GetStartTime()) / 
                              TMath::Log( (((fColdDump->ScalerData->DetectorCounts[bestChannel])/
                              (fFirstFifthDump->ScalerData->DetectorCounts[bestChannel])) / 
                              ((finalColdDump->ScalerData->DetectorCounts[bestChannel])/(fSecondFifthDump->ScalerData->DetectorCounts[bestChannel]))) ))/60; */

      /*double lifetimeInMins = ( fLifetime->GetStopTime() - fLifetime->GetStartTime() ) / 
                                 (60 * TMath::Log( (fColdDump->ScalerData->DetectorCounts[bestChannel] / fFirstFifthDump->ScalerData->DetectorCounts[bestChannel]) / 
                                 (finalColdDump->ScalerData->DetectorCounts[bestChannel] / fSecondFifthDump->ScalerData->DetectorCounts[bestChannel]) ));*/

      return lifetimeInMins;
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