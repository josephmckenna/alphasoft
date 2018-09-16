//
// handle_sequencer
//
// A. Capra
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include "TTree.h"
#include "TSeq_Event.h"
#include "Sequencer2.h"

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

class HandleSequencerFlags
{
public:
   bool fPrint = false;
};

class HandleSequencer: public TARunObject
{
private:
   const static int numseq = 4;
   TString SeqNames[numseq]={"cat","beamline","g-trap","pos"};
   enum {PBAR,RECATCH,ATOM,POS};
   enum {NOTADUMP,DUMP,EPDUMP}; 
   int totalcnts[numseq]={0};
   int cSeq[numseq]={0}; // contatore del numero di sequenze, per tipo
   //Add aditional type for 'other' dumps... Used only for Laser Experiment dumps so far
   int cID[2][numseq]={{0}}; //counter for assignment of unique sequencer ID's (One for starts, the other for stops)
   int cIDextra=0;

public:
   HandleSequencerFlags* fFlags;
   TSeq_Event* fSeqEvent;
   TTree* SequencerTree;
   bool fTrace = false;
   
   HandleSequencer(TARunInfo* runinfo, HandleSequencerFlags* flags)
      : TARunObject(runinfo), fFlags(flags),
        fSeqEvent(0), SequencerTree(0)
   {
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
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      fSeqEvent = new TSeq_Event;
      SequencerTree = new TTree("SequencerEventTree", "SequencerEventTree");
      SequencerTree->Branch("SequencerEvent", &fSeqEvent, 32000, 0);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HandleSequencer::EndRun, run %d\n", runinfo->fRunNo);
      delete SequencerTree;
      if (fSeqEvent) delete fSeqEvent;
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
         return flow;
      
      fSeqEvent->Reset();
      
      // if( fSeqAsm )
      //    fSeqEvent = fSeqAsm->UnpackEvent(event);
      // SequencerTree->Fill();

      const TMBank* b = me->FindBank("SEQ2");
      if( b ) std::cout<<"HandleSequencer::Analyze   BANK NAME: "<<b->name<<std::endl;
      const char* bkptr = me->GetBankData(b);
      int bklen = b->data_size;
      if( bkptr ) 
         {
            std::string test(bkptr);
            std::cout<<"HandleSequencer::Analyze   BANK DATA ("<<test.size()<<"): "<<test<<std::endl;
            std::cout<<"HandleSequencer::Analyze   BANK SIZE: "<<bklen<<std::endl;
         }

      // Sequencer XML parsing interface
      TString sequheader="";
      for(int i = 0;i<bklen && (*bkptr!=60);i++) 
         { //get the first line ; char 60 is "<"      //  && (*bkptr!=10) && (*bkptr!=0)(*bkptr!=13) 
            sequheader+=*bkptr++;
         }
      sequheader+=0;
      std::cout<<"HandleSequencer::Analyze Sequence Header: "<<sequheader<<std::endl;
  
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
         return flow;
         }  
      free(buf);
  
      TXMLNode * node = fParser->GetXMLDocument()->GetRootNode();
      SeqXML* mySeq = new SeqXML(node);
      delete fParser;
  
      Int_t iSeqType;
      // PBAR, MIX, POS definiti in un enum, precedentemente

      if( strcmp( ((TString)mySeq->getSequencerName()).Data(), SeqNames[PBAR].Data()) == 0 ) 
         iSeqType = PBAR;
      else if( strcmp( ((TString)mySeq->getSequencerName()).Data(), SeqNames[RECATCH].Data()) == 0 ) 
         iSeqType = RECATCH;
      else if( strcmp( ((TString)mySeq->getSequencerName()).Data(), SeqNames[ATOM].Data()) == 0 ) 
         iSeqType = ATOM;
      else if( strcmp( ((TString)mySeq->getSequencerName()).Data(), SeqNames[POS].Data()) == 0 ) 
         iSeqType = POS;
  
      else {
         iSeqType = -1;
         std::cerr << "unknown sequencer name: " << ((TString)mySeq->getSequencerName()).Data() <<" seq names "<<SeqNames[PBAR].Data()<<"  "<<SeqNames[RECATCH].Data()<<"  "<<SeqNames[ATOM].Data() <<"  " << SeqNames[POS].Data() <<std::endl;
         //   assert(0);

      }
      std::cout<<"HandleSequencer::Analyze  Sequence: "<<iSeqType<<"   name: "<<((TString)mySeq->getSequencerName()).Data()<<std::endl;
      cSeq[iSeqType]++;


      TIter myChains((TObjArray*)mySeq->getChainLinks(), true);
      SeqXML_ChainLink *cl;
      AgDumpFlow* DumpFlow=new AgDumpFlow(flow);
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
                  Int_t dumpType=0;
                  if (event->GetNameTS()=="startDump") dumpType=1;
                  if (event->GetNameTS()=="stopDump")  dumpType=2;
                  if (dumpType>0 && dumpType<=2)
                  {
                     fSeqEvent->SetID( cID[dumpType-1][iSeqType]++ );
                  }
                  else
                  {
                     fSeqEvent->SetID(cIDextra++);
                  } //Assign to an additional sequencer counter... 
                  fSeqEvent->SetEventName( event->GetNameTS() );
                  fSeqEvent->SetDescription( event->GetDescription() );
                  fSeqEvent->SetonCount( event->GetCount() );
                  fSeqEvent->SetonState( event->GetStateID() );
                  DumpFlow->AddEvent(iSeqType,event->GetDescription(),dumpType,cID[dumpType-1][iSeqType]-1);
                  SequencerTree->Fill();
               }
         }

      return DumpFlow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("HandleSequencer::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class HandleSequencerFactory: public TAFactory
{
public:
   HandleSequencerFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("HandleSequencerFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
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
