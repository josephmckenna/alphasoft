//
// handle_sequencer
//
// A. Capra
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "RecoFlow.h"

#include "TTree.h"
#include "TSeq_Event.h"
#include "Sequencer2.h"

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

#define HANDLE_SEQ_IN_SIDE_THREAD 0

class SequencerSyncCheckFlags
{
public:
   bool fPrint = false; 
   bool fPrintSEQ2 = false;
   bool fPrintSeqDriver = false;
   bool fPrintSeqState = false; 
   bool fPrintSeqEvent = false; 
   bool fFindNsyncs = false;
};

class SequencerSyncCheck: public TARunObject
{
private:

   //int totalcnts[NUMSEQ]={0};
   int cSeq[NUMSEQ]={0}; // contatore del numero di sequenze, per tipo
   //Add aditional type for 'other' dumps... Used only for Laser Experiment dumps so far
   int sID[NUMSEQ]={0}; 
   int dID[NUMSEQ]={0};//Sequencer event ID (for dump markers)
   int cIDextra=0;
   int NSyncsTotal_Digital=0;
   int NSyncsTotal_HV=0;
   void PrintTotalNSyncs(std::array<std::map<TString,int>,NUMSEQ> Seq_syncs_Nsyncsset, int NSyncsTotal)
   {
      std::cout<<std::endl;
      std::cout<<"   Total Number of Syncs "<<NSyncsTotal<<std::endl<<std::endl;

      for(int i=0; i<Seq_syncs_Nsyncsset.size(); i++)
      {
         std::cout<<std::endl;
         std::cout<<"------------------------- Sequence "<<i<<" -------------------------"<<std::endl;
         std::map<TString,int>::iterator it;
         for (it = Seq_syncs_Nsyncsset.at(i).begin(); it != Seq_syncs_Nsyncsset.at(i).end(); it++)
         {
            std::cout << it->first    
               << " : set "
               << it->second
               << " times" 
               << std::endl;
         }
         std::cout<<std::endl;
      }
      std::cout<<"========================================================================="<<std::endl;
      //Seq_syncs_Nsyncsset.clear();
   };
   std::array<std::map<TString,int>,NUMSEQ> Seq_syncs_Nsyncsset_Digital; //std::array<std::map<TString,int>,NUMSEQ> Seq_syncs_Nsyncsset;
   std::array<std::map<TString,int>,NUMSEQ> Seq_syncs_Nsyncsset_HV; //std::array<std::map<TString,int>,NUMSEQ> Seq_syncs_Nsyncsset;

public:
   bool fTrace = false;
   SequencerSyncCheckFlags* fFlags;

   SequencerSyncCheck(TARunInfo* runinfo, SequencerSyncCheckFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="SequencerSyncCheck";
#endif
      if (fTrace)
         printf("SequencerSyncCheck::ctor!\n");
   }

   ~SequencerSyncCheck()
   {
      if (fTrace)
         printf("SequencerSyncCheck::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SequencerSyncCheck::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SequencerSyncCheck::EndRun, run %d\n", runinfo->fRunNo);

      if(fFlags->fFindNsyncs)
      {
         std::cout<<"============================== Digital Map =============================="<<std::endl;
         PrintTotalNSyncs(Seq_syncs_Nsyncsset_Digital, NSyncsTotal_Digital);
         std::cout<<"================================= HV Map ================================"<<std::endl;
         PrintTotalNSyncs(Seq_syncs_Nsyncsset_HV, NSyncsTotal_HV);
      }
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SequencerSyncCheck::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      DumpFlow* df=flow->Find<DumpFlow>();
      if (!df)
      {
         // No data from the handle_sequencer module... nothing to do
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      //Grab data from the flow (from the handle_sequencer module)
      const int iSeqType = df->SequencerNum;
      const std::vector<DumpMarker>& DumpMarkers = df->DumpMarkers;
      const std::vector<TSequencerState>& states = df->states;
      TSequencerDriver* driver= df->driver;

      if(fFlags->fPrintSeqDriver)
      {
         driver->PrintDatamembers();
      }

      std::map<TString,int> syncsmap_Digital;
      std::map<TString,int> syncsmap_HV;
      if(fFlags->fFindNsyncs)
      {
         std::cout<<std::endl;
         std::cout<<"______________________________________DigitalMap syncs_____________________________________"<<std::endl;
         syncsmap_Digital = driver->DigitalMap->FindSyncs();
         syncsmap_HV = driver->HVMap->FindSyncs();
         std::cout<<std::endl;
      }

      for (const TSequencerState& state: states)
      {
         int NumberSyncsSet_Digital=0;   
         int NumberSyncsSet_HV=0;   
         if(fFlags->fPrintSeqState)
         {
            std::cout<<std::endl;
            std::cout<<"========================================================================= Sequencer State ========================================================================="<<std::endl;
            state.Print();
            std::cout<<"==================================================================================================================================================================="<<std::endl;
         }

         NumberSyncsSet_Digital += state.syncs_Nsyncsset_Digital->AddSyncs(syncsmap_Digital);
         NumberSyncsSet_HV += state.syncs_Nsyncsset_HV->AddSyncs(syncsmap_HV);

         if(fFlags->fFindNsyncs)
         {
            std::cout<<std::endl;
            std::cout<<"Number of Syncs Set "<<NumberSyncsSet_Digital<<std::endl;
            state.syncs_Nsyncsset_Digital->Print();
            std::cout<<"______________________________________"<<std::endl;
            std::cout<<std::endl;
            std::cout<<"Number of Syncs Set "<<NumberSyncsSet_HV<<std::endl;
            state.syncs_Nsyncsset_HV->Print();
            std::cout<<"___________________________________________________________________________________________"<<std::endl<<std::endl;
            NSyncsTotal_Digital+=NumberSyncsSet_Digital;
            NSyncsTotal_HV+=NumberSyncsSet_HV;
            Seq_syncs_Nsyncsset_Digital[iSeqType]=state.syncs_Nsyncsset_Digital->map;
            Seq_syncs_Nsyncsset_HV[iSeqType]=state.syncs_Nsyncsset_HV->map;
         }
         state.syncs_Nsyncsset_Digital->map.clear();
         state.syncs_Nsyncsset_HV->map.clear();
      }
      return flow;
   }

};

class SequencerSyncCheckFactory: public TAFactory
{
public:
   SequencerSyncCheckFlags fFlags;

public:


   void Usage()
   {
      printf("SequencerSyncCheckFactory Usage:\n");
      printf("\t--printSEQ2 Display the full XML block the sequencer is sending\n");
      printf("\t--printSeqDriver Display the drivers that sequencers are sending\n");
      printf("\t--printSeqEvent Display the sequencers events\n");
      printf("\t--printSeqState Display the states of the sequencers\n");
      printf("\t--findNsyncs Display the number of times a sync digital line is set\n");
   }

   void Init(const std::vector<std::string> &args)
   {
      printf("SequencerSyncCheckFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print") 
            fFlags.fPrint = true;
         if(args[i] == "--printSEQ2") 
            fFlags.fPrintSEQ2 = true;
         if(args[i] == "--printSeqDriver") 
            fFlags.fPrintSeqDriver = true;
         if(args[i] == "--printSeqState") 
            fFlags.fPrintSeqState = true;
         if(args[i] == "--printSeqEvent") 
            fFlags.fPrintSeqEvent = true;
         if(args[i] == "--findNsyncs") 
            fFlags.fFindNsyncs = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("SequencerSyncCheckFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("SequencerSyncCheckFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new SequencerSyncCheck(runinfo, &fFlags);
   }
};

static TARegister tar(new SequencerSyncCheckFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
