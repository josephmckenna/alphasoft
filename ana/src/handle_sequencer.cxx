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

class HandleSequencerFlags
{
public:
   bool fPrint = false; 
   bool fPrintSEQ2 = false;
   bool fPrintSeqDriver = false;
   bool fPrintSeqState = false; 
   bool fPrintSeqEvent = false; 
};

class HandleSequencer: public TARunObject
{
private:

   //int totalcnts[NUMSEQ]={0};
   int cSeq[NUMSEQ]={0}; // contatore del numero di sequenze, per tipo
   //Add aditional type for 'other' dumps... Used only for Laser Experiment dumps so far
   int sID[NUMSEQ]={0}; 
   int dID[NUMSEQ]={0};//Sequencer event ID (for dump markers)
   int cIDextra=0;

public:
   HandleSequencerFlags* fFlags;
   TSeq_Event* fSeqEvent;
   TSequencerState* fSeqState;
   TTree* fSequencerEventTree;
   TTree* fSequencerStateTree;
   bool fTrace = false;

   HandleSequencer(TARunInfo* runinfo, HandleSequencerFlags* flags)
      : TARunObject(runinfo), fFlags(flags),
        fSeqEvent(nullptr), fSeqState(nullptr), fSequencerEventTree(nullptr), fSequencerStateTree(nullptr)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="Handle Sequencer";
#endif
      if (fTrace)
         printf("HandleSequencer::ctor!\n");
   }

   ~HandleSequencer()
   {
      if (fTrace)
         printf("HandleSequencer::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HandleSequencer::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      fSeqEvent = new TSeq_Event;
      fSequencerEventTree = new TTree("SequencerEventTree", "SequencerEventTree");
      fSequencerEventTree->Branch("SequencerEvent", &fSeqEvent, 32000, 0);

      fSeqState = new TSequencerState;
      fSequencerStateTree = new TTree("SequencerStateTree", "SequencerStateTree");
      fSequencerStateTree->Branch("TSequencerState",&fSeqState, 32000, 0);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HandleSequencer::EndRun, run %d\n", runinfo->fRunNo);
      gDirectory->cd("/");

      fSequencerEventTree->Write();
      delete fSequencerEventTree;
      if (fSeqEvent) delete fSeqEvent;

      fSequencerStateTree->Write();
      delete fSequencerStateTree;
      if (fSeqState) delete fSeqState;
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HandleSequencer::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      //std::cout<<"HandleSequencer::Analyze   Event # "<<me->serial_number<<std::endl;

      if( me->event_id != 8 ) // sequencer event id
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
#ifdef HAVE_MANALYZER_PROFILER
      TAClock start_time = TAClockNow();
#endif
      //

      // if( fSeqAsm )
      //    fSeqEvent = fSeqAsm->UnpackEvent(event);
      // SequencerTree->Fill();

      const TMBank* b = me->FindBank("SEQ2");
      //if( b ) std::cout<<"HandleSequencer::Analyze   BANK NAME: "<<b->name<<std::endl;
      char* bkptr = me->GetBankData(b);
      int bklen = b->data_size;
      if(fFlags->fPrintSEQ2)
         std::cout<<bkptr<<std::endl;
#if HANDLE_SEQ_IN_SIDE_THREAD
      if( bkptr ) 
      {
        SEQTextFlow* sq=new SEQTextFlow(flow);
        sq->AddData(bkptr,bklen);
        flow=sq;
      }
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"handle_sequencer(main thread)",timer_start);
      return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SEQTextFlow* sq=flow->Find<SEQTextFlow>();
      if (!sq)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      START_TIMER

      const char* bkptr = sq->data;
      int bklen = sq->size;
#endif
      fSeqEvent->Reset();
      // Sequencer XML parsing interface
      TString sequheader="";
      for(int i = 0;i<bklen && (*bkptr!=60);i++) 
         { //get the first line ; char 60 is "<"      //  && (*bkptr!=10) && (*bkptr!=0)(*bkptr!=13) 
            sequheader+=*bkptr++;
         }
      sequheader+=0;
      //std::cout<<"HandleSequencer::Analyze Sequence Header: "<<sequheader<<std::endl;
  
      TDOMParser *fParser = new TDOMParser();
      fParser->SetValidate(false);
	
      int bufLength = strlen(bkptr);
      char*buf = (char*)malloc(bufLength);
      memcpy(buf, bkptr, bufLength);
  
      for (int i=0; i<bufLength; i++)
         if (!isascii(buf[i]))
            buf[i] = 'X';
         else if (buf[i] == 0x1D)
            buf[i] = 'X';
      
      int parsecode = fParser->ParseBuffer(buf,bufLength);
    
      if (parsecode < 0 ) 
         {
         std::cerr << fParser->GetParseCodeMessage(parsecode) << std::endl;
#ifdef HAVE_MANALYZER_PROFILER
         flow = new TAUserProfilerFlow(flow,"handle_sequencer(no parse)",start_time);
#endif
         return flow;
         }  
      free(buf);
      flow=new DumpFlow(flow);
      //((DumpFlow*)flow)->MidasTime=me->time_stamp;
      TXMLNode * node = fParser->GetXMLDocument()->GetRootNode();
      SeqXML* mySeq = new SeqXML(node);
      TSequencerDriver* driver=new TSequencerDriver();
      {
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      driver->Parse(node);
      if(fFlags->fPrintSeqDriver)
         driver->PrintDatamembers();
      }
      ((DumpFlow*)flow)->driver=driver;
      delete fParser;
  
      int iSeqType=-1;
      // PBAR, MIX, POS definiti in un enum, precedentemente
      for (int iSeq=0; iSeq<NUMSEQ; iSeq++)
      {
         if( strcmp( ((TString)mySeq->getSequencerName()).Data(), SeqNames.at(iSeq).c_str()) == 0 ) 
            {
               iSeqType=iSeq;
               break;
            }
      }

      if (iSeqType < 0)
      {
         std::cerr << "unknown sequencer name: " << ((TString)mySeq->getSequencerName()).Data() <<" seq names ";
         for (int iSeq=0; iSeq<NUMSEQ; iSeq++)
            std::cerr<<SeqNames.at(iSeq)<<"  ";
         std::cerr<<std::endl;
         //   assert(0);

      }
      std::cout<<"HandleSequencer::Analyze  Sequence: "<<iSeqType<<"   name: "<<((TString)mySeq->getSequencerName()).Data()<<std::endl;
      TString s="Sequence ";
      s+=cSeq[iSeqType];
      s+=" loaded";
      ((DumpFlow*)flow)->AddDumpEvent(iSeqType,cSeq[iSeqType],me->time_stamp,s.Data(),DumpMarker::DumpTypes::Info,cSeq[iSeqType],0);
      cSeq[iSeqType]++;
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      gDirectory->cd();

      TIter myChains((TObjArray*)mySeq->getChainLinks(), true);
      SeqXML_ChainLink *cl;
      while((cl = (SeqXML_ChainLink *) myChains.Next()))
      {
         SeqXML_Event *event;
         TIter myEvents(cl->getEvents());
         fSeqEvent->Reset();
         while((event = (SeqXML_Event *) myEvents.Next()))
         {
            //Turning off default printing of dumps Sept 2017 JTKM
            //event->Print("");
            fSeqEvent->SetSeq( mySeq->getSequencerName() );
            fSeqEvent->SetSeqNum(iSeqType); 
            fSeqEvent->SetID(dID[iSeqType]++);
            fSeqEvent->SetEventName( event->GetNameTS() );
            fSeqEvent->SetDescription( event->GetDescription() );
            fSeqEvent->SetonCount( event->GetCount() );
            fSeqEvent->SetonState( event->GetStateID() );
            Int_t onState=event->GetStateID();
            if(fFlags->fPrintSeqEvent)
            {
               std::cout<<std::endl;
               std::cout<<"============================= Sequencer Event =================================="<<std::endl;
               fSeqEvent->Print();
               std::cout<<"================================================================================"<<std::endl;
            }
            fSequencerEventTree->Fill();
            ((DumpFlow*)flow)->AddDumpEvent(
                iSeqType,
                cSeq[iSeqType],
                me->time_stamp,
                event->GetDescription(),
                event->GetNameTS(),
                dID[iSeqType]-1,
                onState);
         }

         SeqXML_State* state;
         TIter myStates(cl->getStates());
         while ((state= (SeqXML_State *) myStates.Next()))
         {
            fSeqState->Reset();
            fSeqState->SetSeq( mySeq->getSequencerName() );
            fSeqState->SetSeqNum(iSeqType);
            fSeqState->SetID(sID[iSeqType]++);

            fSeqState->Set(state);

            //Trigger unset for now
            fSeqState->SetComment(*state->getComment() );

            ((DumpFlow*)flow)->AddStateEvent(*fSeqState);
            if(fFlags->fPrintSeqState)
            {
               std::cout<<std::endl;
               std::cout<<"========================================================================= Sequencer State ========================================================================="<<std::endl;
               fSeqState->Print();
               std::cout<<"==================================================================================================================================================================="<<std::endl;
            }
            fSequencerStateTree->Fill();
         }
      }
      delete mySeq;
#if HANDLE_SEQ_IN_SIDE_THREAD
      //I am done with the SEQText, lets free up some memory
      sq->Clear();
#endif
      return flow;
   }

};

class HandleSequencerFactory: public TAFactory
{
public:
   HandleSequencerFlags fFlags;

public:


   void Usage()
   {
      printf("HandleSequencerFactory Usage:\n");
      printf("\t--printSEQ2 Display the full XML block the sequencer is sending\n");
      printf("\t--printSeqDriver Display the drivers that sequencers are sending\n");
      
   }

   void Init(const std::vector<std::string> &args)
   {
      printf("HandleSequencerFactory::Init!\n");

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
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("HandleSequencerFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("HandleSequencerFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HandleSequencer(runinfo, &fFlags);
   }
};

static TARegister tar(new HandleSequencerFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
