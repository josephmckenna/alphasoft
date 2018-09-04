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


void PrintChronoBoards(int runNumber, Double_t tmin, Double_t tmax)
{
   if (tmax<0.) tmax=GetTotalRunTime(runNumber);
   TString Names[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
   Int_t Counts[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
   for (int boards=0; boards<CHRONO_N_BOARDS; boards++)
   {
      for (int chans=0; chans<CHRONO_N_CHANNELS; chans++)
      {
         Names[boards][chans]=Get_Chrono_Name(runNumber, boards, chans);
         Counts[boards][chans]=GetCountsInChannel(runNumber,boards,chans,tmin,tmax);
      }
   }
   std::cout<<"Name\tBoard\tChannel\tCounts\tRate"<<std::endl;
   for (int boards=0; boards<CHRONO_N_BOARDS; boards++)
   {
      for (int chans=0; chans<CHRONO_N_CHANNELS; chans++)
      {
         std::cout<<Names[boards][chans]<<"\t"
                  <<boards<<"\t"
                  <<chans<<"\t"
                  <<Counts[boards][chans]<<"\t"
                  <<Counts[boards][chans]/(tmax-tmin)
                  <<std::endl;
      }
   }

}
