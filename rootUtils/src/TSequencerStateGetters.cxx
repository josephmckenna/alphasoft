#include "TSequencerStateGetters.h"


TSequencerState* Get_Seq_State(Int_t runNumber, int SequencerID, int state)
{

   // Replace with TTree reader... its faster
   TTree* t = Get_Seq_State_Tree(runNumber);
   TSequencerState* seqState = new TSequencerState();
   t->SetBranchAddress("TSequencerState",&seqState);
   std::cout<<"Looking for "<<SequencerID <<" state: " << state <<std::endl;
   
   
   for ( int i=0; i< t->GetEntries(); i++)
   {
      t->GetEntry(i);
      //seqState->Print();
      //Must be from same sequence
      if (seqState->GetSeqNum()!=SequencerID)
         continue;
      //Not at the right state yet:
      if (seqState->GetState()<state)
         continue;
      //Match found:
      if (seqState->GetState()==state)
         return seqState;
   }
   return NULL;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
