#include "TreeGetters.h"


TTree* Get_Chrono_Tree(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel)
{
   TFile* f=Get_File(runNumber);
   TTree *chrono_tree = NULL;
   TString Name="chrono/ChronoEventTree_";
           Name+=Chronoboard;
           Name+="_";
           Name+=ChronoChannel;
   chrono_tree = (TTree *)f->Get(Name);
   if (chrono_tree == NULL)
   {
      Error("Get_Chrono_Tree", "\033[31mChrono Tree %d - %d for run number %d not found\033[00m", Chronoboard, ChronoChannel, runNumber);
      chrono_tree->GetName(); // This is to crash the CINT interface  instead of exiting (deliberately)
   }
   return chrono_tree;
}

TTree* Get_Chrono_Tree(Int_t runNumber, const char* ChannelName)
{
   Int_t chan=-1;
   Int_t board=-1;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
 
   return Get_Chrono_Tree(runNumber,board,chan);
}


TTree* Get_Chrono_Name_Tree(Int_t runNumber)
{
   TFile* f=Get_File(runNumber);
   TTree *chrono_tree = NULL;
   chrono_tree = (TTree *)f->Get("ChronoBoxChannels");
   if (chrono_tree == NULL)
   {
      Error("Get_Chrono_Tree", "\033[31mChrono Tree for run number %d not found\033[00m", runNumber);
      chrono_tree->GetName(); // This is to crash the CINT interface  instead of exiting (deliberately)
   }
   return chrono_tree;
}

TTree* Get_Seq_Event_Tree(Int_t runNumber)
{
   TFile* f=Get_File(runNumber);
   TTree *seq_tree = NULL;
   seq_tree = (TTree *)f->Get("SequencerEventTree");
   if (seq_tree == NULL)
   {
      Error("Get_Seq_Event_Tree", "\033[31mSeq_Event Tree for run number %d not found\033[00m", runNumber);
      seq_tree->GetName(); // This is to crash the CINT interface  instead of exiting (deliberately)
   }
   return seq_tree;
}

