#ifndef _DUMPHANDLING_
#define _DUMPHANDLING_

#include "TSequencerState.h"
#include "TSpill.h"
#include <deque>
// yes, then we can have A2 and AG classes that do fancy things with chronobox / SIS data...

class DumpMarker
{
   public:
   enum DumpTypes { Info, Start, Stop, ADSpill, Positrons, Mixing, FRD, Other};

   std::string Description;
   int seqNum;
   int DumpType;
   int fonCount;
   int fonState;
   double fRunTime; //SIS/Chronobox time stamp (official time)

   void Print()
   {
      std::cout<<"Seq:"<<seqNum
               <<"\tDescription:"<<Description.c_str()
               <<"\tType:"<<DumpType
               <<"\tonCount:"<<fonCount
               <<"\tonState:"<<fonState
               <<"\tRunTime:"<<fRunTime<<std::endl;
   }
   DumpMarker()
   {
      seqNum=-1;
      DumpType=-1;
      fonCount=-1;
      fonState=-1;
      fRunTime=-1.;
   }
   DumpMarker(DumpMarker* d)
   {
       Description=d->Description;
       seqNum     =d->seqNum;
       DumpType   =d->DumpType; //0= Start, 1=Stop, 2=AD spill?, 3=Positrons etc
       fonCount   =d->fonCount;
       fonState   =d->fonState;
       fRunTime   =d->fRunTime;
   }
};
#include "TSequencerDriver.h"

#include "../../a2lib/include/TSISEvent.h"
class DumpPair
{
public:
   int dumpID;
   DumpMarker* StartDumpMarker;
   DumpMarker* StopDumpMarker;
   enum SIS_Status {NO_SIS, NOT_FILLED, FILLED};
   int SIS_Filled[NUM_SIS_MODULES];
   TSISEvent* IntegratedSISCounts[NUM_SIS_MODULES];
   bool IsPaired = false;
   bool IsFinished = false; //Only true if I have been printed (thus safely destroyed)
   std::vector<TSequencerState*> states;
   DumpPair()
   {
      dumpID=-1;
      StartDumpMarker=NULL;
      StopDumpMarker=NULL;
      for (int i=0; i<NUM_SIS_MODULES; i++)
      {
         IntegratedSISCounts[i]=new TSISEvent();
         IntegratedSISCounts[i]->SetSISModuleNo(i);
         SIS_Filled[i]=NO_SIS;
      }
      IsPaired=false;
   }
   DumpPair(DumpMarker* startDump): DumpPair()
   {
      AddStartDump(startDump);
   }
   bool Ready()
   {
      if (!IsPaired) return false;
      if (!StartDumpMarker) return false;
      if (!StopDumpMarker) return false;
      if (StartDumpMarker->fRunTime<0) return false;
      if (StopDumpMarker->fRunTime<0) return false;
      for ( int i=0; i<NUM_SIS_MODULES; i++)
         if (SIS_Filled[i]==NOT_FILLED) return false;
      return true;
   }
   void Print()
   {
      std::cout<<"DumpID: "<<dumpID<<std::endl;
      std::cout<<"IsPaired?:"<<IsPaired<<std::endl;
      if (StartDumpMarker)
      {
         std::cout<<"StartTime:"<<StartDumpMarker->fRunTime<<std::endl;
         StartDumpMarker->Print();
      }
      if (StopDumpMarker)
      {
         std::cout<<"StopTime: "<<StopDumpMarker->fRunTime<<std::endl;
         StopDumpMarker->Print();
      }
      std::cout<<"Number of states:"<<states.size()<<std::endl;
   }
   //Maybe we should add MIDAS speaker announcements inside this 
   //function, most of these failure modes are critical
   std::vector<std::string> check(int DumpStart, int DumpStop)
   {
       std::vector<std::string> errors;
       char buf[200];
       //Check if dumps are paired
       if (!StartDumpMarker)
       {
          sprintf(buf,"Dump %d is a pair with no start dump... this should never happen",dumpID);
          errors.push_back(buf);
       }
       if (!StopDumpMarker)
       {
          if (!StartDumpMarker)
             sprintf(buf,"Dump %d is a pair with no start or stop dump... this should never happen",dumpID);
          else
             sprintf(buf,"Dump %s is a pair with no stop dump... ",StartDumpMarker->Description.c_str());
          errors.push_back(buf);
       }
       //We need states to investigate the Digital output of the 
       //sequencer (to the SIS/ Chronoboxes)
       if (!states.size())
       {
          sprintf(buf,"Dump %d has no states... this should never happen",dumpID);
          errors.push_back(buf);
       }
       else //Check that the SIS is triggered
       {
          TSequencerState* SISStartMarker  =states.front();
          if (!SISStartMarker->GetDigitalOut()->Channels[DumpStart])
             errors.push_back("ERROR: Start dump has no SIS trigger!");
          TSequencerState* SISStopMarker  =states.back();
          if (!SISStopMarker->GetDigitalOut()->Channels[DumpStop])
             errors.push_back("ERROR: Stop dump has no SIS trigger!");
       }
       return errors;
   }
   void clear()
   {
      for (size_t i=0; i<states.size(); i++)
         delete states.at(i);
      states.clear();
   }
   void AddStartDump(DumpMarker* d)
   {
      StartDumpMarker=new DumpMarker(d);
   }
   bool AddStopDump(DumpMarker* d)
   {
      //std::cout<<d->Description <<"\t==\t"<<StartDumpMarker->Description<<"\t?"<<std::endl;
      if (strcmp(d->Description.c_str(),StartDumpMarker->Description.c_str())==0)
      {
         StopDumpMarker=new DumpMarker(d);
         IsPaired=true;
         return true;
      }
      else
      {
         //Descriptions did not match... reject
         return false;
      }
   }
   int AddState(TSequencerState* s)
   {
      //State before dump starts... do not add
      //std::cout<<StartDumpMarker->fonState << " <= "<<s->GetState() <<" <= "<< StopDumpMarker->fonState <<std::endl;
      if (s->GetState()<StartDumpMarker->fonState)
         return -1;
      if (StopDumpMarker)
      {
         //State after dump stops... do not add
         if (s->GetState()>StopDumpMarker->fonState)
            return 1;
         //Create copy of state put into vector
         states.push_back(new TSequencerState(s));
         //std::cout<<"Added state to "<<StartDumpMarker->Description<<std::endl;
         return 0;
      }
      else
      {
         std::cout<<"Warning, adding states to unpaired dump... either a bug or a dump that spans multiple sequences"<<std::endl;
      }
      return 0;
   }
   int AddSISEvent(TSISEvent* s)
   {
      int SISModule=s->GetSISModule();
      //std::cout<<"MODULE:"<<SISModule<<std::endl;
      //Record that there are SIS events...
      SIS_Filled[SISModule]=NOT_FILLED;
      double t=s->GetRunTime();
      if (StartDumpMarker->fRunTime<0)
         return -2;
      if (t<StartDumpMarker->fRunTime)
         return -1;
      if (StopDumpMarker)
         if (StopDumpMarker->fRunTime>0)
            if (t>StopDumpMarker->fRunTime)
            {
               SIS_Filled[SISModule]=FILLED;
               return 1;
            }
      //std::cout<<"POOP"<<std::endl;
      //s->Print();
      *(IntegratedSISCounts[SISModule])+=s;
      return 0;
   }
};

template<typename Spill>
class DumpList
{
public:
   int seqcount=-1;

   std::deque<DumpPair*> dumps;
   //Sequentially sorted pointers to the above dump pairs
   std::deque<DumpMarker*> ordered_starts;
   std::deque<DumpMarker*> ordered_stops;

   Spill* AddStartDump(DumpMarker* d)
   {
      //Construct a new dump at the back of dumps
      dumps.push_back(new DumpPair(d));
      ordered_starts.push_back(dumps.back()->StartDumpMarker);
      //For now no error checking...
      return NULL;
   }
   //Return true if dump is paired
   Spill* AddStopDump(DumpMarker* d)
   {
      for (size_t i=0; i<dumps.size(); i++)
      {
         //Find incomplete dumps, skip paired ones
         if (dumps.at(i)->IsPaired) continue;
         //Add stop dump (if the dump descriptions match)
         if (dumps.at(i)->AddStopDump(d))
         {
            ordered_stops.push_back(dumps.at(i)->StopDumpMarker);
            return NULL;
         }
      }
      //No pair found!
      std::cout<<"ERROR! I did not pair a dump!"<<std::endl;
      char buf[200];
      sprintf(buf,"ERROR! Stop dump:%s did not find a pair",d->Description.c_str()); 
      return new Spill(buf);
   }
   Spill* AddDump(DumpMarker* d)
   {
      switch(d->DumpType)
      {
         case DumpMarker::DumpTypes::Info:
            std::cout<<"Info:"; d->Print();
            return NULL;
         case DumpMarker::DumpTypes::Start:
            return AddStartDump(d);
         case DumpMarker::DumpTypes::Stop:
            return AddStopDump(d);
      }
      return new Spill("Attempted to add dump maker that was neither start not stop...");
   }
   void AddStates(std::vector<TSequencerState*>* s)
   {
      for ( auto &pair : dumps )
      {
         //pair->Print();
         for ( auto & state: *s )
         {
            //state->Print();
            //If the state is after the dump ends.. break this loop
            if (pair->AddState(state)>0) break;
         }
      }
   }
   Spill* AddStartTime(double t)
   {
      if (!ordered_starts.size())
         return new Spill("Error, start dump time stamp given with no dump, did the sequencer start before the run?");
      ordered_starts.front()->fRunTime=t;
      ordered_starts.pop_front();
      return NULL;
   }
   Spill* AddStopTime(double t)
   {
      if (!ordered_stops.size())
         return new Spill("Error, stop dump time stamp given with no dump, did the sequencer start before the run?");
      ordered_stops.front()->fRunTime=t;
      ordered_stops.pop_front();
      return NULL;
   }
   Spill* AddSISEvents(std::vector<TSISEvent*>* events)
   {
      for ( auto &pair : dumps )
      {
         for ( auto &s : *events )
         {
            if (pair->AddSISEvent(s)>0) break;
         }
      }
   }
   void check(TSequencerDriver* d,std::deque<Spill*>* errors)
   {
      //std::cout<<"SIS start dump channel:"<<d->DigitalMap->ChannelDescriptionMap["Dump start"]<<std::endl;
      //std::cout<<"SIS stop dump channel:"<<d->DigitalMap->ChannelDescriptionMap["Dump end"]<<std::endl;
      int start=d->DigitalMap->ChannelDescriptionMap["Dump start"];
      int stop=d->DigitalMap->ChannelDescriptionMap["Dump end"];

      for(auto pair: dumps)
      {
         std::vector<std::string> err=pair->check(start,stop);
         //collect errors and create Spills... 
         if (err.size())
            for(auto error: err)
               errors->push_back(new Spill(error.c_str()));
      }
      return;
   }
   void Print()
   {
      std::cout<<"SequencerCount:"<<seqcount<<std::endl;
      for(auto pair: dumps) 
         pair->Print();
   }
   int countIncomplete()
   {
      int incomplete=0;
      for (size_t i=0;i<dumps.size();i++)
      {
        if (dumps.at(i)->IsPaired) continue;
        if (dumps.at(i)->IsFinished) continue;
        incomplete++;
      }
      return incomplete;
   }
   std::vector<Spill*> flushComplete()
   {
      std::vector<Spill*> complete;
      for (size_t i=0;i<dumps.size();i++)
      {
        DumpPair* pair=dumps.at(i);
        if (!pair->Ready()) continue;
        if (pair->IsFinished) continue;
        pair->IsFinished=true;
        Spill* spill=new Spill(pair);
        complete.push_back(spill);
      }
      return complete;
   }
   void clear()
   {
      for (size_t i=0;i<dumps.size(); i++)
         dumps.at(i)->clear();
      dumps.clear();
   }
   void setup(std::deque<Spill*>* errors)
   {
      seqcount++;
      for (int i=0; i<dumps.size(); i++)
      {
         if (dumps.front()->IsFinished)
         {
            delete dumps.front();
            dumps.pop_front();
         }
         else
         {
            break;
         }
      }
      if (dumps.size())
      {
         //Extra warning to show the dummps were not empty and we need to run tests
         errors->push_back(new Spill("ERROR DUMPS POTENTIALLY THROWN AWAY! Possible aborted sequence detected"));
         for (size_t i=0; i<dumps.size(); i++)
         {
            //If we have a start dump in this pair (we should always have one)
            if (dumps.at(i)->StartDumpMarker)
            {
               //If the start of the dump happened (as marker from SIS)
               if (dumps.at(i)->StartDumpMarker->fRunTime>0)
               {
                  //If start of dump happend, but there is no stop dump
                  if (!dumps.at(i)->StopDumpMarker)
                  {
                     char buf[200];
                     sprintf(buf,
                             "Warning, start dump (%s) being carried from the previous sequence, not paired yet... OK",
                             dumps.at(i)->StartDumpMarker->Description.c_str()
                             );
                     errors->push_back(new Spill(buf));
                  }
                  //Else if there is a valid start dump AND stop dump
                  else
                  {
                     //Check stop dump for valid time... if invalid, sequence was aborted (or buggy)
                     if(dumps.at(i)->StopDumpMarker->fRunTime<0)
                     {
                        errors->push_back(new Spill("ERROR DUMPS THROWN AWAY! Aborted sequence detected"));
                     }
                     //Else is ok... throw a warning anyway
                     else
                     {
                        char buf[200];
                        sprintf(buf,
                           "Warning, dump pair good (%s and %s) in memory, but should have been cleared... this should never happen",
                           dumps.at(i)->StartDumpMarker->Description.c_str(),
                           dumps.at(i)->StopDumpMarker->Description.c_str()
                           );
                        errors->push_back(new Spill(buf));
                     }
                  }
               }
            }
         }
      }
      clear();
      return;
   }
};

#endif
