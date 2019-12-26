
#include "TSequencerState.h"
#include "TSequencerDriver.h"

class DumpMarker
{
public:
   enum DumpTypes
   {
      NULLType,
      StartType,
      StopType,
      OtherType
   };
   //TString Description;
   TString DumpName;
   int seqNum;
   int DumpType; //1= Start, 2=Stop, 3=AD spill?, 4=Positrons?
   int fonCount;
   int fonState;
   bool IsDone;
   DumpMarker()
   {
      DumpName="";
      seqNum=-1;
      DumpType=NULLType;
      fonCount=-1;
      fonState=-1;
      IsDone=false;
   }
   /*DumpMarker(DumpMarker* d)
   {
      Description =d->Description;
      seqNum      =d->seqNum;
      DumpType    =d->DumpType;
      fonCount    =d->fonCount;
      fonState    =d->fonState;
      IsDone      =d->IsDone;
   }*/
};

class DumpPair
{
public:
   int dumpID;
   DumpMarker* StartDumpMarker;
   DumpMarker* StopDumpMarker;
   double startTime  =-1;
   double stopTime   =-1;
   bool IsPaired     = false;
   std::vector<TSequencerState*> states;
   DumpPair()
   {
      dumpID=-1;
      StartDumpMarker=NULL;
      StopDumpMarker=NULL;
      IsPaired=false;
   }
   int check(TSequencerDriver* d, std::vector<std::string>& errors)
   {
       int NumErrors=0;
       if (!StartDumpMarker)
       {
          errors.push_back("Pair with no start dump");
          NumErrors++;
       }
       if (!StopDumpMarker)
       {
          errors.push_back("Pair with no stop dump");
          NumErrors++;
       }
       //The start dump trigger should be the state after the label
       TSequencerState* sis_start=states.at(0+1);
       int start_bit=d->DigitalMap->ChannelDescriptionMap["DAVE"];
       if (!sis_start->GetDigitalOut()->Channels[start_bit])
       {
          errors.push_back("Dump has no SIS trigger to come! Abort the sequence!");
          NumErrors++;
       }
       //The stop dump trigger should be the state in the last place
       TSequencerState* sis_stop =states.back();
       int stop_bit=d->DigitalMap->ChannelDescriptionMap["DAVE"];
       if (!sis_stop->GetDigitalOut()->Channels[stop_bit])
       {
          errors.push_back("Dump has no SIS trigger to come! Abort the sequence!");
          NumErrors++;
       }
       //No need to integriate all the states... we only care about the start and stop
       /*for (size_t i=0; i<states.size(); i++)
       {
          TSequencerState* s=states[i];  
       }*/
       return NumErrors;
   }
   void clear()
   {
      for (size_t i=0; i<states.size(); i++)
         delete states.at(i);
      states.clear();
   }
   void AddStartDump(DumpMarker* d)
   {
      StartDumpMarker=d;
   }
   bool AddStopDump(DumpMarker* d)
   {
      if (strcmp(d->DumpName,StartDumpMarker->DumpName)!=0)
      {
         StopDumpMarker=d;
         IsPaired=true;
         return true;
      }
      else
      {
         //Descriptions did not match... reject
         return false;
      }
   }
   bool AddState(TSequencerState* s, std::vector<std::string>& errors)
   {
      if(!StartDumpMarker) return false;
      if(!StopDumpMarker) return false;
      if(StartDumpMarker->fonState >= s->GetState())
      {
         if(StopDumpMarker->fonState <= s->GetState())
         {
            states.push_back(new TSequencerState(s));
            return true;
         }
      }
      return false;
   }

};

bool compare_fonState (const DumpMarker* first, const DumpMarker* second)
{
   if (first->fonState < second->fonState) return true;
   else return false;
}

class DumpList
{
public:
   int seqcount=-1;
   std::vector<DumpPair> dumps;
   //Queue of order dump markers, This array does not own the objects
   std::list<DumpMarker*> DumpStartsSortedByState;
   std::list<DumpMarker*> DumpStopsSortedByState;

   void SortQueuedStates(int seqNum)
   {
      DumpStartsSortedByState.clear();
      for (size_t i=0; i<dumps.size(); i++)
      {
         DumpMarker* d=dumps.at(i).StartDumpMarker;
         if (d)
            DumpStartsSortedByState.push_back(d);
      }
      DumpStopsSortedByState.clear();
      for (size_t i=0; i<dumps.size(); i++)
      {
         DumpMarker* d=dumps.at(i).StopDumpMarker;
         if (d)
            DumpStopsSortedByState.push_back(d);
      }
      DumpStartsSortedByState.sort(compare_fonState);
      DumpStopsSortedByState.sort(compare_fonState);
   }
   void AddState(TSequencerState* s, std::vector<std::string>& errors)
   {
      for (size_t i=0; i<dumps.size(); i++)
      {
         dumps.at(i).AddState(s,errors);
      }
   }
   void AddStartDump(DumpMarker* d, std::vector<std::string>& errors)
   {
      //Construct a new dump at the back of dumps
      dumps.push_back(DumpPair());
      dumps.back().AddStartDump(d);
   }
   void AddStopDump(DumpMarker* d, std::vector<std::string>& errors)
   {
      for (size_t i=0; i<dumps.size(); i++)
      {
         //Find incomplete dumps
         if (dumps.at(i).IsPaired) continue;
         //Add stop dump (if the dump descriptions match)
         if (dumps.at(i).AddStopDump(d)) return;
      }
      //No pair found!
      errors.push_back("ERROR! I did not pair a dump!");
   }
   void AddDump(DumpMarker* d, std::vector<std::string>& errors)
   {
      switch(d->DumpType){
         case DumpMarker::StartType:
            AddStartDump(d,errors);
            break;
         case DumpMarker::StopType:
            AddStopDump(d,errors);
            break;
         case DumpMarker::OtherType:
            std::cout<<"OtherType dump marker... action unknown... doing nothing";
            errors.push_back("OtherType dump marker... action unknown... doing nothing");
            break;
      }
   }
   int check(TSequencerDriver* d,std::vector<std::string>& errors)
   {
      int NumErrors=0;
      NumErrors+=countIncomplete(errors);
      for (size_t i=0; i<dumps.size(); i++)
      {
         NumErrors+=dumps.at(i).check(d,errors);
      }
   }
   int countIncomplete(std::vector<std::string>& errors)
   {
      int incomplete=0;
      for (size_t i=0;i<dumps.size();i++)
      {
        if (dumps.at(i).IsPaired) continue;
        errors.push_back("Unpaired dump here!");//+dumps.at(i).StartDumpMarker->DumpName);
        incomplete++;
      }
      return incomplete;
   }
   void clear()
   {
      for (size_t i=0;i<dumps.size(); i++)
         dumps.at(i).clear();
      dumps.clear();
   }
   void setup(TSequencerDriver*d,   std::vector<std::string>& errors)
   {
      seqcount++;
      if (dumps.size())
      {
         errors.push_back("ERROR DUMPS THROWN AWAY! Aborted sequence detected");
         check(d,errors);
         /*for (size_t i=0; i<dumps.size(); i++)
         {
            hang on... how does this module know if the dumps have been used up...
         }*/
      }
      clear();
   }
};
