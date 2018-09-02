#include "TSeqEventGetters.h"



TSeq_Event* Get_Seq_Event(Int_t runNumber, TString description, Bool_t IsStart, Int_t repetition)
{
   TTree* t=Get_Seq_Event_Tree(runNumber);
   TSeq_Event* e=new TSeq_Event();
   t->SetBranchAddress("SequencerEvent", &e);
   Int_t matches=0;
   TString DumpType="NULL";
   if (IsStart) DumpType="startDump";
   if (!IsStart) DumpType="stopDump";
   
   for (Int_t i=0; i<t->GetEntries(); i++)
   {
      t->GetEntry(i);
      if (e->GetDescription().BeginsWith(description))
      {
         if (e->GetEventName().BeginsWith(DumpType))
            matches++;
         if (matches==repetition) return e;
      }
   }
   return NULL;
}
TSeq_Event* Get_Seq_Event(Int_t runNumber, TString description, TString DumpType, Int_t repetition)
{
   TTree* t=Get_Seq_Event_Tree(runNumber);
   TSeq_Event* e=new TSeq_Event();
   t->SetBranchAddress("SequencerEvent", &e);
   Int_t matches=0;
   
   for (Int_t i=0; i<t->GetEntries(); i++)
   {
      t->GetEntry(i);
      if (e->GetDescription().BeginsWith(description))
      {
         if (e->GetEventName().BeginsWith(DumpType))
            matches++;
         if (matches==repetition) return e;
      }
   }
   return NULL;
}
