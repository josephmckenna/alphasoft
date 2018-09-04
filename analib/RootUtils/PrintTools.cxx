#include "PrintTools.h"



void PrintSequences(int runNumber, int SeqNum)
{
  TTree *sequencerTree = Get_Seq_Event_Tree(runNumber);
  TSeq_Event *seqEvent = new TSeq_Event();
  sequencerTree->SetBranchAddress("SequencerEvent", &seqEvent);
  for (Int_t i = 0; i < sequencerTree->GetEntries(); i++)
  {
    sequencerTree->GetEntry(i);
    if (SeqNum>0)
    if (SeqNum!=seqEvent->GetSeqNum())
      continue;
    

      std::cout<< "Sequencer Name:" << seqEvent->GetSeq().Data()
               << "\t Seq Num:" << seqEvent->GetSeqNum()
               << "\t ID:" << seqEvent->GetID()
               << "\t Name:" << seqEvent->GetEventName()
               << "\t Description: " << seqEvent->GetDescription()
               << "\t RunTime: "<< GetRunTimeOfEvent(runNumber,seqEvent)
               << std::endl;
  }
  
    delete seqEvent;
  delete sequencerTree;
}


