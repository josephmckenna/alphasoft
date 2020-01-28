#ifndef _DUMPHANDLING_
#define _DUMPHANDLING_

#include "TSequencerState.h"
#include <deque>
// yes, then we can have A2 and AG classes that do fancy things with chronobox / SIS data...

class DumpMarker
{
   public:
   enum DumpTypes { Info, Start, Stop, ADSpill, Positrons, Mixing, FRD, Other};

   std::string Description;
   int fSequencerID; //Unique to each sequencer
   int fSequenceCount; //Sequence number in run
   int DumpType;
   int fonCount;
   int fonState;
   double fRunTime; //SIS/Chronobox time stamp (official time)
   uint32_t MidasTime; //Sequence start time
   void Print()
   {
      std::cout<<"SequencerID:"<<fSequencerID
               <<"\tSequenceCount:"<<fSequenceCount
               <<"\tDescription:"<<Description.c_str()
               <<"\tType:"<<DumpType
               <<"\tonCount:"<<fonCount
               <<"\tonState:"<<fonState
               <<"\tRunTime:"<<fRunTime
               <<"\tSequenceStartTime:"<<MidasTime<<std::endl;
   }
   DumpMarker()
   {
      Description="NULL";
      fSequencerID=-1;
      fSequenceCount=-1;
      DumpType=-1;
      fonCount=-1;
      fonState=-1;
      fRunTime=-1.;
      MidasTime=0;
   }
   DumpMarker(const char* name,DumpTypes type): DumpMarker()
   {
      Description=name;
      DumpType=type;
   }
   DumpMarker(DumpMarker* d)
   {
       Description   =d->Description;
       fSequencerID  =d->fSequencerID;
       fSequenceCount=d->fSequenceCount;
       DumpType      =d->DumpType; //0= Start, 1=Stop, 2=AD spill?, 3=Positrons etc
       fonCount      =d->fonCount;
       fonState      =d->fonState;
       fRunTime      =d->fRunTime;
       MidasTime     =d->MidasTime;
   }
};
//Universal headers
#include "TSequencerDriver.h"
//ALPHA 2 headers..
#include "../../a2lib/include/TSISEvent.h"
template <typename VertexType>
class SVDCounts
{
   public:
   int FirstVF48Event=-1;
   int LastVF48Event=-1;
   int VF48Events=0;
   int Verticies=0;
   int PassCuts=0;
   int PassMVA=0;
   void AddEvent(VertexType* e)
   {
      LastVF48Event=e->VF48NEvent;
      if (FirstVF48Event<0) FirstVF48Event=e->VF48NEvent;
      VF48Events++;
      Verticies+=e->NVertices;
      PassCuts+=e->NPassedCuts;
      PassMVA+=e->MVA;
   }
};



//ALPHA g headers...
template<typename VertexType>
class DumpPair
{
public:
   int dumpID;
   DumpMarker* StartDumpMarker;
   DumpMarker* StopDumpMarker;
   //Enum for SIS, SVD and Chronnobox
   enum STATUS {NO_EQUIPMENT, NOT_FILLED, FILLED};
   //ALPHA 2:
   STATUS SIS_Filled[NUM_SIS_MODULES];
   TSISEvent* IntegratedSISCounts[NUM_SIS_MODULES];
   STATUS SVD_Filled;
   SVDCounts<VertexType> IntegratedSVDCounts;
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
         SIS_Filled[i]=NO_EQUIPMENT;
      }
      SVD_Filled=NO_EQUIPMENT;
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
      if (SVD_Filled==NOT_FILLED) return false;
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
          {
             char buf[100];
             sprintf(buf,"Warning: Start dump %s has no SIS trigger yet!",StartDumpMarker->Description.c_str());
             errors.push_back(buf);
          }
          TSequencerState* SISStopMarker  =states.back();
          if (!SISStopMarker->GetDigitalOut()->Channels[DumpStop])
          {
             char buf[100];
             if (StopDumpMarker)
             {
                sprintf(buf,"Warning: Stop dump %s has no SIS trigger yet!",StopDumpMarker->Description.c_str());
             }
             else
             {
                if (StartDumpMarker)
                    sprintf(buf,"Warning: Start dump %s has no stop dump yet!",StartDumpMarker->Description.c_str());
                else
                    sprintf(buf,"Warning: No start dump, no stop dump...!");
             }
             errors.push_back(buf);
          }
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
      if (StartDumpMarker)
      {
         if (s->GetState()<StartDumpMarker->fonState && states.size()==0)
            return -1;
      }
      else
      {
         return 0;
      }
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
         if (states.size()%1000==0)
            std::cout<<"Warning, adding states to unpaired dump("<<StartDumpMarker->Description.c_str()<<") either a bug or a dump that spans multiple sequences (I will ignore the next 1000 states in this dump)"<<std::endl;
         states.push_back(new TSequencerState(s));
         
      }
      return 0;
   }
   //ALPHA 2 function
   int AddSISEvent(TSISEvent* s)
   {
      int SISModule=s->GetSISModule();
      //std::cout<<"MODULE:"<<SISModule<<std::endl;
      //Record that there are SIS events...
      SIS_Filled[SISModule]=NOT_FILLED;
      double t=s->GetRunTime();
      if (StartDumpMarker)
      {
         if (StartDumpMarker->fRunTime<0)
            return -2;
         if (t<StartDumpMarker->fRunTime)
            return -1;
      }
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
   int AddSVDEvent(VertexType* s)
   {
      SVD_Filled=NOT_FILLED;
      double t=s->GetTime();
      if (StartDumpMarker)
      {
         if (StartDumpMarker->fRunTime<0)
            return -2;
         if (t<StartDumpMarker->fRunTime)
            return -1;
      }
      if (StopDumpMarker)
         if (StopDumpMarker->fRunTime>0)
            if (t>StopDumpMarker->fRunTime)
            {
               SVD_Filled=FILLED;
               return 1;
            }
      IntegratedSVDCounts.AddEvent(s);
      return 0;
   }
};

template<typename SpillType, typename VertexType>//, typename ScalerType, typename VertexType >
class DumpList
{
public:
   int seqcount=-1;
   int SequencerID;
   std::deque<DumpPair<VertexType>*> dumps;
   //Sequentially sorted pointers to the above dump pairs
   std::deque<DumpMarker*> ordered_starts;
   std::deque<DumpMarker*> ordered_stops;
   //Hold errors in queue for ansyncronus error reporting
   std::deque<SpillType*> error_queue;
   //Sequence ID and Start time:
   std::deque<std::pair<int,uint32_t>> SequenceStartTimes;

   DumpList()
   {
      seqcount=-1;
      SequencerID=-1;
   }
   DumpPair<VertexType>* GetPairOfStart(DumpMarker* d)
   {
      for ( auto &pair : dumps )
      {
         if (!pair) continue;
         if (pair->StartDumpMarker==d)
         return pair;
      }
      return NULL;
   }
   DumpPair<VertexType>* GetPairOfStop(DumpMarker* d)
   {
      for ( auto &pair : dumps )
      {
         if (!pair) continue;
         if (pair->StopDumpMarker==d)
         return pair;
      }
      return NULL;
   }
   SpillType* GetError()
   {
      if (!error_queue.size()) return NULL;
      SpillType* s=error_queue.front();
      error_queue.pop_front();
      return s;
   }

   bool AddStartDump(DumpMarker* d)
   {
      //Construct a new dump at the back of dumps
      dumps.push_back(new DumpPair<VertexType>(d));
      ordered_starts.push_back(dumps.back()->StartDumpMarker);
      //For now no error checking...
      return true;
   }
   //Return true if dump is paired
   bool AddStopDump(DumpMarker* d)
   {
      for (size_t i=0; i<dumps.size(); i++)
      {
         if (!dumps.at(i)) continue;
         //Find incomplete dumps, skip paired ones
         if (dumps.at(i)->IsPaired) continue;
         //Add stop dump (if the dump descriptions match)
         if (dumps.at(i)->AddStopDump(d))
         {
            ordered_stops.push_back(dumps.at(i)->StopDumpMarker);
            return true;
         }
      }
      //No pair found!
      std::cout<<"ERROR! I did not pair a dump!"<<std::endl;
      error_queue.push_back(new SpillType("ERROR! Stop dump:%s did not find a pair",
                                          d->Description.c_str()));
      return false;
   }
   void AddSequencerStartTime(DumpMarker* d)
   {
      //If I have a invalid midas timestamp, do nothing
      if (!d->MidasTime) return;
      if (d->DumpType==DumpMarker::DumpTypes::Info) return;
      if (SequenceStartTimes.size())
      {
         uint32_t last_time=SequenceStartTimes.back().second;
         //And I am larger than the previous start (I came afterwood
         if (last_time<d->MidasTime)
         {
            SequenceStartTimes.push_back({d->fSequenceCount,d->MidasTime});
            return;
         }
         if (last_time>d->MidasTime)
         {
            error_queue.push_back(new SpillType("Sequence started before the last one? This should never happen"));
            return;
         }
      }
      else 
      {
         SequenceStartTimes.push_back({d->fSequenceCount,d->MidasTime});
         return;
      }
      return;
   }
   
   bool AddDump(DumpMarker* d)
   {
      AddSequencerStartTime(d);
      switch(d->DumpType)
      {
         case DumpMarker::DumpTypes::Info:
            std::cout<<"Info:"; d->Print();
            return true;
         case DumpMarker::DumpTypes::Start:
            return AddStartDump(d);
         case DumpMarker::DumpTypes::Stop:
            return AddStopDump(d);
      }
      error_queue.push_back(new SpillType("Attempted to add dump maker that was neither start not stop..."));
      return false;
   }
   void AddStates(std::vector<TSequencerState*>* s)
   {
      for ( auto &pair : dumps )
      {
         //pair->Print();
         for ( auto & state: *s )
         {
            //state->Print();
            if (!pair) continue;
            //If the state is after the dump ends.. break this loop
            if (pair->AddState(state)>0) break;
         }
      }
   }
   void RemoveAndCleanAbortedSequence(int SequenceCount)
   {
      assert(SequenceStartTimes.front().first< SequenceCount);
      //     for (int i=0; i<SequenceStartTimes.size(); i++)
      while (1)
	{
	  if (SequenceStartTimes.front().first>= SequenceCount) return;
	  int badseq=SequenceStartTimes.front().first;
      SequenceStartTimes.pop_front();
      for ( auto &pair : dumps )
      {
         if (!pair) continue;
         if (!pair->StartDumpMarker) continue;
         if (pair->StartDumpMarker->fSequenceCount == badseq )
         {
            error_queue.push_back(new SpillType("Delete pair %s", 
                                                pair->StartDumpMarker->Description.c_str()));
	    //            delete pair;
            pair=NULL;
         }
      }
      for ( auto &start : ordered_starts )
      {
         if (!start) continue;
         if (start->fSequenceCount == badseq )
         {
            error_queue.push_back(new SpillType("Delete start %s",
                                                start->Description.c_str()));
	    //            delete start;
            start=NULL;
         }
      }
      for (auto &stop : ordered_stops )
      {
         if (!stop) continue;
         if (stop->fSequenceCount == badseq )
         {
            error_queue.push_back(new SpillType("Delete stop %s",
                                                stop->Description.c_str()));
	    //            delete stop;
            stop=NULL;
         }
      }
	}
   }
   
   void AddStartTime(uint32_t midas_time, double t)
   {
      if (!ordered_starts.size())
      {
         error_queue.push_back(new SpillType("Error, start dump time stamp given with no dump, did the sequencer start before the run?"));
         AddStartDump(new DumpMarker("NO NAME DUMP",DumpMarker::DumpTypes::Start));
         //return;
      }
      // Clear up pointers at front set to NULL... 
      // we dont care about them any more, they've been nuked elsewhere,
      // and when nuked, and error was logged
      if (!ordered_starts.front())
      {
         ordered_starts.pop_front();
         return AddStartTime(midas_time,t);
      }
      // Find sequence start time stamp that closes matches this start 
      // dump... note, negative values will be ignored as are 
      // sequences in the future
      
      int best_i=SequenceStartTimes.front().first;
      int best_diff=SequenceStartTimes.front().second;
      for (int i=0; i<SequenceStartTimes.size(); i++)
      {
         int dt=midas_time-SequenceStartTimes.at(i).second;
         if (dt<=2) continue;
         if (dt<best_diff)
         {
            best_diff=dt;
            best_i=SequenceStartTimes.at(i).first;
         }
         //std::cout<<"iiii  "<< i<<"\t"<<midas_time<<" - "<<SequenceStartTimes.at(i).second<<" = "<< dt <<std::endl;
      }
      if (best_i!=SequenceStartTimes.front().first)
      {
         RemoveAndCleanAbortedSequence(best_i);
         return AddStartTime(midas_time,t);
      }
      if (ordered_starts.front()->MidasTime > midas_time)
      {
         error_queue.push_back(new SpillType("Error, bad unix time of dump... Aborted sequence detected? Skipping dump"));
         ordered_starts.front()->Print();
         std::cout<<ordered_starts.front()->MidasTime <<" > "<< midas_time <<std::endl;
         ordered_starts.pop_front();
         return AddStartTime(midas_time,t);
      }
      ordered_starts.front()->fRunTime=t;
      ordered_starts.pop_front();
      return;
   }
   void AddStopTime(uint32_t midas_time, double t)
   {
      if (!ordered_stops.size())
      {
         error_queue.push_back(new SpillType("Error, stop dump time stamp given with no dump, did the sequencer start before the run?"));
         AddStopDump(new DumpMarker("NO NAME DUMP",DumpMarker::DumpTypes::Stop));
         return;
      }
      if (!ordered_stops.front())
      {
          ordered_stops.pop_front();
          return AddStopTime(midas_time,t);
      }
      // Find sequence start time stamp that closes matches this start 
      // dump... note, negative values will be ignored as are 
      // sequences in the future
      
      int best_i=SequenceStartTimes.front().first;
      int best_diff=99999;
      for (int i=0; i<SequenceStartTimes.size(); i++)
      {
         int dt=midas_time-SequenceStartTimes.at(i).second;
         if (dt<=2) continue;
         if (dt<best_diff)
         {
            best_diff=dt;
            best_i=SequenceStartTimes.at(i).first;
         }
         //std::cout<<"iiii  "<< i<<"\t"<<midas_time<<" - "<<SequenceStartTimes.at(i).second<<" = "<< dt <<std::endl;
      }
      if (best_i!=SequenceStartTimes.front().first)
      {
         RemoveAndCleanAbortedSequence(best_i);
         return AddStopTime(midas_time,t);
      }
      if (ordered_stops.front()->MidasTime > midas_time && ordered_stops.front()->MidasTime!=0)
      {
         error_queue.push_back(new SpillType("Error, bad unix time of dump... Aborted sequence detected? Skipping dump"));
         ordered_starts.front()->Print();

         std::cout<<ordered_stops.front()->MidasTime <<" > "<< midas_time <<std::endl;
         ordered_stops.pop_front();
         return AddStopTime(midas_time,t);
      }
      DumpPair<VertexType>* pair=GetPairOfStop(ordered_stops.front());
      if (pair)
      {
         if (pair->StartDumpMarker)
         {
            if (pair->StartDumpMarker->fRunTime>t)
            {
               error_queue.push_back(new SpillType("XXXX Error... stop dump (%s) happened before start?",
                                                   ordered_stops.front()->Description.c_str()));
               int BadSeq=ordered_starts.front()->fSequenceCount;
              std::cout<<"Deleteing bad sequence:"<<BadSeq;
         for (int i=0; i<ordered_starts.size(); i++)
         {
            if (ordered_starts.at(i)->fSequenceCount == BadSeq)
            {
               std::cout<<"REMOVING START:"<<ordered_starts.at(i)->Description.c_str()<<std::endl;
               DumpPair<VertexType>* bad_pair=GetPairOfStart(ordered_starts.at(i));
               //JOE DO SOME PROPER DELETING!!!
               bad_pair->StartDumpMarker=NULL;
               bad_pair->StopDumpMarker=NULL;
               ordered_starts.at(i)=NULL;
            }
         }
         for (int i=0; i<ordered_stops.size(); i++)
         {
            if (ordered_stops.at(i)->fSequenceCount == BadSeq)
            {
               std::cout<<"REMOVING STOP:"<<ordered_stops.at(i)->Description.c_str()<<std::endl;
               DumpPair<VertexType>* bad_pair=GetPairOfStop(ordered_stops.at(i));
               //JOE DO SOME PROPER DELETING!!!
               bad_pair->StartDumpMarker=NULL;
               bad_pair->StopDumpMarker=NULL;
               ordered_stops.at(i)=NULL;
            }
         }
         return;
            }
            if (pair->StartDumpMarker->fRunTime<0)
            {

               error_queue.push_back(new SpillType("XXXX %s has no start time!... deleting %s start dump",
                                                   ordered_stops.front()->Description.c_str(),
                                                   ordered_starts.front()->Description.c_str()));
               //ordered_starts.pop_front();
               //ordered_stops.pop_front();
               //return AddStopTime(midas_time,t);
               return;
            }
         }
      }
      ordered_stops.front()->fRunTime=t;
      ordered_stops.pop_front();
      //pair->Print();
      return;
   }
   void AddSISEvents(std::vector<TSISEvent*>* events)
   {
      for ( auto &pair : dumps )
      {
         if (!pair) continue;
         for ( auto &s : *events )
         {
            if (pair->AddSISEvent(s)>0) break;
         }
      }
   }
   void AddSVDEvents(std::vector<VertexType*>* events)
   {
      for ( auto &pair : dumps )
      {
         if (!pair) continue;
         for ( auto &s : *events )
         {
            if (pair->AddSVDEvent(s)>0) break;
         }
      }
   }
   
   void check(TSequencerDriver* d)
   {
      //std::cout<<"SIS start dump channel:"<<d->DigitalMap->ChannelDescriptionMap["Dump start"]<<std::endl;
      //std::cout<<"SIS stop dump channel:"<<d->DigitalMap->ChannelDescriptionMap["Dump end"]<<std::endl;
      int start=d->DigitalMap->ChannelDescriptionMap["Dump start"];
      int stop=d->DigitalMap->ChannelDescriptionMap["Dump end"];

      for(auto pair: dumps)
      {
         if (!pair) continue;
         std::vector<std::string> err=pair->check(start,stop);
         //collect errors and create SpillTypes... 
         if (err.size())
            for(auto error: err)
               error_queue.push_back(new SpillType(error.c_str()));
      }
      return;
   }
   void Print()
   {
      std::cout<<"SequencerCount:"<<seqcount<<std::endl;
      for(auto pair: dumps)
      {
         if (!pair) continue;
         pair->Print();
      }
   }
   int countIncomplete()
   {
      int incomplete=0;
      for (size_t i=0;i<dumps.size();i++)
      {
        if (!dumps.at(i)) continue;
        if (dumps.at(i)->IsPaired) continue;
        if (dumps.at(i)->IsFinished) continue;
        incomplete++;
      }
      return incomplete;
   }
   std::vector<SpillType*> flushComplete()
   {
      std::vector<SpillType*> complete;
      for (size_t i=0;i<dumps.size();i++)
      {
        DumpPair<VertexType>* pair=dumps.at(i);
        if (!pair) continue;
        if (!pair->Ready()) continue;
        if (pair->IsFinished) continue;
        pair->IsFinished=true;
        SpillType* spill=new SpillType(pair);
        complete.push_back(spill);
        //Item flushed... delete it
        delete pair;
        dumps.at(i)=NULL;
      }
      if (ordered_starts.size()==0 && 
          ordered_stops.size()==0 &&
          complete.size())
      {
         complete.push_back(
            new SpillType(
               "Sequencer %d: Sequenece %d finished",
               SequencerID, 
               seqcount
            )
         );
      }
      return complete;
   }
   void clear()
   {
      for (size_t i=0;i<dumps.size(); i++)
      {
         if (dumps.at(i))
            dumps.at(i)->clear();
      }
      dumps.clear();
   }
   void setup()
   {
      seqcount++;
      error_queue.push_back(
         new SpillType(
            "Sequencer %d: Sequenece %d queued",
            SequencerID,
            seqcount
         )
      );
      if (SequencerID==2)
      Print();
      return;
   }
   void finish()
   {
      for (int i=0; i<dumps.size(); i++)
      {
         if (!dumps.front())
         {
            dumps.pop_front();
            continue;
         }
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
         error_queue.push_back(new SpillType("ERROR DUMPS POTENTIALLY THROWN AWAY! Possible aborted sequence detected"));
         for (size_t i=0; i<dumps.size(); i++)
         {
            if (!dumps.at(i)) continue;
            //If we have a start dump in this pair (we should always have one)
            if (dumps.at(i)->StartDumpMarker)
            {
               //If the start of the dump happened (as marker from SIS)
               if (dumps.at(i)->StartDumpMarker->fRunTime>0)
               {
                  //If start of dump happend, but there is no stop dump
                  if (!dumps.at(i)->StopDumpMarker)
                  {
                     error_queue.push_back(new SpillType("Warning, start dump (%s) being carried from the previous sequence, not paired yet... OK",
                                                         dumps.at(i)->StartDumpMarker->Description.c_str()));
                  }
                  //Else if there is a valid start dump AND stop dump
                  else
                  {
                     //Check stop dump for valid time... if invalid, sequence was aborted (or buggy)
                     if(dumps.at(i)->StopDumpMarker->fRunTime<0)
                     {
                        error_queue.push_back(new SpillType("ERROR DUMPS THROWN AWAY! Aborted sequence detected"));
                     }
                     //Else is ok... throw a warning anyway
                     else
                     {
                        error_queue.push_back( new SpillType("Warning, dump pair good (%s and %s) in memory, but should have been cleared... this should never happen",
                                                             dumps.at(i)->StartDumpMarker->Description.c_str(),
                                                             dumps.at(i)->StopDumpMarker->Description.c_str()));
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
