//
// spill_log_module
//
// JTK McKenna
//


#include <list>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include <TEnv.h>

#include "AgFlow.h"
#include "chrono_module.h"
#include "TTree.h"

#include <vector>
#include "TSpill.h"

#include "TGFrame.h"
#include "TGListBox.h"
#include "TGTextEdit.h"
#include "TGNumberEntry.h"
#ifndef ROOT_TGLabel
#include "TGLabel.h"
#endif


#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

#define HOT_DUMP_LOW_THR 500


#define MAXDET 2

TString SeqNames[NUMSEQ]={"cat","rct","atm","pos"};
enum {PBAR,RECATCH,ATOM,POS};
//enum {NOTADUMP,DUMP,EPDUMP}; 


time_t gTime; // system timestamp of the midasevent
std::list<TSpill*> Spill_List;

TGMainFrame* fMainFrameGUI = NULL;
TGListBox* fListBoxSeq[4]; 
TGListBox* fListBoxLogger;
TGTextEdit* fTextEditBuffer;
TGTextButton *fTextButtonCopy;

TGNumberEntry* fNumberEntryDump[4];
TGNumberEntry* fNumberEntryTS[4];


TApplication* xapp;


class alphaFrame: public TGMainFrame {
  
public: 

   alphaFrame();
   alphaFrame(const char* name, const char* description);

   virtual ~alphaFrame();

   Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
   void CloseWindow();

};
  
alphaFrame::alphaFrame(){
  alphaFrame("alphaFrame","alpha Frame");
}

alphaFrame::alphaFrame(const char* name, const char* description)
  : TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame){

}

alphaFrame::~alphaFrame(){

}

Bool_t alphaFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2){

   if ( (int)msg == 259 && parm1 == 100001)
   {
      fTextEditBuffer->RemoveAll();
      fTextEditBuffer->Layout();

      int nentries = fListBoxLogger->GetNumberOfEntries();

      if ( nentries > 0) 
      {
         //     TGTextLBEntry* lbentry = (TGLBEntry*)fListBoxLogger->GetSelectedEntry();
         TGTextLBEntry* lbentry = (TGTextLBEntry*)fListBoxLogger->GetSelectedEntry();
        if (!lbentry)
        {
           lbentry = (TGTextLBEntry*)fListBoxLogger->Select(nentries - 1);
        }
        TGString str = lbentry->GetText();
        fTextEditBuffer->LoadBuffer( str.Data() ) ;
      }
      fTextEditBuffer->Layout();
      fTextEditBuffer->SelectAll();
      fTextEditBuffer->Copy();
      return kTRUE;
   }
   return kFALSE;
}


void alphaFrame::CloseWindow(){
   gApplication->Terminate(0);
   //delete xapp
}

class SpillLogFlags
{
public:
   bool fPrint = false;


};



class SpillLog: public TARunObject
{
public: 
   SpillLogFlags* fFlags;
   bool fTrace = false;
private:

public:


   //Chronobox channels
   Int_t clock[CHRONO_N_BOARDS];

   //Detector data to integrate (From ChronoFlow)
   Int_t DetectorChans[MAXDET];
   std::vector<Double_t> DetectorTS[MAXDET];
   std::vector<Int_t> DetectorCounts[MAXDET];
   TString detectorName[MAXDET];

   //Dump Marker counter (From ChronoFlow)
   std::vector<Double_t> StartTime[NUMSEQ];
   std::vector<Double_t> StopTime[NUMSEQ];

   //Channels for Dump markers
   Int_t StartChannel[NUMSEQ];
   Int_t StopChannel[NUMSEQ];

   std::vector<DumpMarker> DumpMarkers[NUMSEQ];
   //Dump Markers to give timestamps (From DumpFlow)
   //std::vector<TString> Description[4];
   //std::vector<Int_t> DumpType[4]; //1=Start, 2=Stop
   //std::vector<Int_t> fonCount[4];
   // Int_t SequencerNum[4];
   SpillLog(TARunInfo* runinfo, SpillLogFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("SpillLog::ctor!\n");
   }

   ~SpillLog()
   {
      if (fTrace)
         printf("SpillLog::dtor!\n");
   }




void LayoutListBox(TGListBox* fLb)
{
   fLb->Layout();
   TGVScrollBar* fsb = fLb->GetVScrollbar();
   fsb->SetPosition(fsb->GetRange());
   fLb->Layout();
}


void Messages(TSeq_Dump* se){
  
  // perform routine checks and issues vocal alarms
  if( strncmp( se->GetDescription().Data(), "Hot", 3) == 0 &&
      se->GetDetIntegral(0) < HOT_DUMP_LOW_THR ) // [0] -> SIS_PMT_CATCH_OR
    //cm_msg(MTALK,"alpha2dumps","Warning Hot Dump is low");
    std::cout <<"WARNING HOT DUMP IS LOW"<<std::endl;
}

/*Bool_t MatchDumpDescription(TSeq_Event* se1, TSeq_Event* se2) {
  return ( strcmp(se1->GetDescription().Data(), se2->GetDescription().Data()) == 0 ); 
}*/


void FormatHeader(TString* log){

  char buf[300];

  //  *log += "                     | "; // indentation     
  *log += "                "; // indentation     


  sprintf(buf,"%-21s","Dump Time");
  *log += buf;

  sprintf(buf,"| %-33s        | ","CAT Event       RCT Event       ATM Event       POS Event"); // description 
  *log += buf;


  for (int iDet = 0; iDet<MAXDET; iDet++){
    sprintf(buf,"%-9s ", detectorName[iDet].Data());
    *log += buf;
  }


}

TString LogSpills() {

   TString log = "";
   TGString logstr = "";
   for (int i = 0; i < fListBoxLogger->GetNumberOfEntries(); i++)
   {
      TGString logstr = ((TGTextLBEntry*)fListBoxLogger->GetEntry(i))->GetText(); 
      log += TString::Format("%s\n",logstr.Data());
   }
   std::cout << std::endl << "--- Run summary: ---" << std::endl;
   std::cout << log.Data() << std::endl << std::endl;

   for (int iSeqType = 0; iSeqType < NUMSEQ; iSeqType++)
   {
      std::list<TSeq_Dump*>::iterator itd;
      for ( uint i=0; i< DumpMarkers[iSeqType].size(); i++ )
      {
         if(DumpMarkers[iSeqType].at(i).IsDone) continue;
         log += "LogSpills: Msg: INCOMPLETE EVENT:";
         log += DumpMarkers[iSeqType].at(i).Description.Data();
         log += "\n";
      }
   }
   return log;
}


void DrawSpills(Bool_t endofrun = kFALSE) 
{

//  if( gIsRunning == kFALSE && endofrun == kFALSE ) return;

  std::cout<<"Draw Spills"<<std::endl;

  double y = 0.1;
  double y_step = 0.05;
  //  double x = 0.05;

  std::list<TSpill*>::reverse_iterator it;
  for ( it=Spill_List.rbegin() ; it != Spill_List.rend(); ++it ) {
  
    TSpill * s = (TSpill*)*it;

    s->SetYStep(y_step);
    //Colour set but not used... removing Sept 2017
    //Int_t colour = kBlack;
    //if( it == Spill_List.rbegin() ) // lo spill piu` recente (quello corrente) e` evidenziato in rosso
    //  colour = kRed;
    
    y+= y_step;
    
    if( y > 1. )
      break;
    
    int m = s->GetNumDump();
    y += y_step * m;
        
    //    s->PrintSpill( gCanvas, x,y, colour );
    
  }

}

Int_t getIntegral(Int_t DetN,Double_t tmin, Double_t tmax) 
{ 
   // binary search to find the first entry 
   Int_t start_entry = 0; 
   Int_t total_entries = DetectorTS[DetN].size();
   Int_t low = 0; 
   Int_t high = total_entries-1; 
   while (low < high)  
   { 
      Int_t mid = Int_t((low + high)/2.); 
      if (DetectorTS[DetN].at(mid) < tmin) 
         low = mid + 1;  
      else 
         //can't be high = mid-1: here A[mid] >= value, 
         //so high can't be < mid if A[mid] == value 
         high = mid;  
   } 
   if ((low < (total_entries-1)) && (DetectorTS[DetN].at(low) == tmin)) 
      start_entry = low; // found 
   // calculate the integral 
   Int_t integral = 0; 
   for( Int_t i = start_entry; i<total_entries; i++ ) 
   { 
      if( DetectorTS[DetN].at(i) < tmin ) continue; 
      if( DetectorTS[DetN].at(i) > tmax ) break; 
      integral += DetectorCounts[DetN].at(i);      
   } 
   return integral; 
} 

void UpdateDumpIntegrals(TSeq_Dump* se) 
{ 
   // update event container; 
   for (int iDet=0; iDet<MAXDET; iDet++) 
   { 
      if (DetectorChans[iDet]>-1) 
      { 
         Int_t val = getIntegral(iDet, se->GetStartonTime(), se->GetStoponTime()); 
         //std::cout <<"Channel " <<detectorCh[iDet] <<"  Integral "<< val << std::endl; 
         se->SetDetIntegral( iDet, val); 
      } 
   } 
} 
 


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      gEnv->SetValue("Gui.DefaultFont","-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

      //TApplication *app = new TApplication("alphagdumps", &argc, argv);
      //extern TApplication* xapp;
      xapp = new TApplication("alphagdumps",0,0);

      // main frame
      //  TGMainFrame 
      alphaFrame* fMainFrameGUI = new alphaFrame();
      fMainFrameGUI->SetName("fMainFrameGUI");
      fMainFrameGUI->SetWindowName("alphagdumps");
      fMainFrameGUI->SetLayoutBroken(kTRUE);

      // list box
      TGLabel *fLabelSeq1 = new TGLabel(fMainFrameGUI,SeqNames[PBAR].Data());
      fLabelSeq1->SetTextJustify(36);
      fLabelSeq1->SetMargins(0,0,0,0);
      fLabelSeq1->SetWrapLength(-1);
      fMainFrameGUI->AddFrame(fLabelSeq1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fLabelSeq1->MoveResize(25,8,62,16);
      fListBoxSeq[0] = new TGListBox(fMainFrameGUI);
      fListBoxSeq[0]->SetName("fListBoxSeq1");
      fListBoxSeq[0]->Resize(290,116);
      fMainFrameGUI->AddFrame(fListBoxSeq[0], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fListBoxSeq[0]->MoveResize(25,24,290,116);

      // list box
      TGLabel *fLabelSeq2 = new TGLabel(fMainFrameGUI,SeqNames[RECATCH].Data());
      fLabelSeq2->SetTextJustify(36);
      fLabelSeq2->SetMargins(0,0,0,0);
      fLabelSeq2->SetWrapLength(-1);
      fMainFrameGUI->AddFrame(fLabelSeq2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fLabelSeq2->MoveResize(345,8,62,16);
      fListBoxSeq[1] = new TGListBox(fMainFrameGUI);
      fListBoxSeq[1]->SetName("fListBoxSeq2");
      fListBoxSeq[1]->Resize(290,116);
      fMainFrameGUI->AddFrame(fListBoxSeq[1], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fListBoxSeq[1]->MoveResize(345,24,290,116);

      // list box
      TGLabel *fLabelSeq3 = new TGLabel(fMainFrameGUI,SeqNames[ATOM].Data());
      fLabelSeq3->SetTextJustify(36);
      fLabelSeq3->SetMargins(0,0,0,0);
      fLabelSeq3->SetWrapLength(-1);
      fMainFrameGUI->AddFrame(fLabelSeq3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fLabelSeq3->MoveResize(665-20,8,62,16);
      fListBoxSeq[2] = new TGListBox(fMainFrameGUI);
      fListBoxSeq[2]->SetName("fListBoxSeq3");
      fListBoxSeq[2]->Resize(290,116);
      fMainFrameGUI->AddFrame(fListBoxSeq[2], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fListBoxSeq[2]->MoveResize(665,24,290,116);
   
      // list box
      TGLabel *fLabelSeq4 = new TGLabel(fMainFrameGUI,SeqNames[POS].Data());
      fLabelSeq4->SetTextJustify(36);
      fLabelSeq4->SetMargins(0,0,0,0);
      fLabelSeq4->SetWrapLength(-1);
      fMainFrameGUI->AddFrame(fLabelSeq4, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fLabelSeq4->MoveResize(985,8,62,16);
      fListBoxSeq[3] = new TGListBox(fMainFrameGUI);
      fListBoxSeq[3]->SetName("fListBoxSeq4");
      fListBoxSeq[3]->Resize(290,116);
      fMainFrameGUI->AddFrame(fListBoxSeq[3], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fListBoxSeq[3]->MoveResize(985,24,290,116);


      // list box
      fListBoxLogger = new TGListBox(fMainFrameGUI);
      fListBoxLogger->SetName("fListBoxLogger");
      //  fListBoxLogger->Resize(1150,326);
      fListBoxLogger->Resize(1350,326);
      fMainFrameGUI->AddFrame(fListBoxLogger, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      //  fListBoxLogger->MoveResize(25,208,1150,600);
      fListBoxLogger->MoveResize(25,208,1350,600);
      fListBoxLogger->SetMultipleSelections(kTRUE);
    
      fNumberEntryDump[0] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,7,-1,(TGNumberFormat::EStyle) 5);
      fNumberEntryDump[0]->SetName("fNumberEntryDump1");
      fMainFrameGUI->AddFrame(fNumberEntryDump[0], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fNumberEntryDump[0]->MoveResize(25,144,64,20);
      fNumberEntryDump[1] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
      fNumberEntryDump[1]->SetName("fNumberEntryDump2");
      fMainFrameGUI->AddFrame(fNumberEntryDump[1], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fNumberEntryDump[1]->MoveResize(345,144,64,20);
      fNumberEntryDump[2] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
      fNumberEntryDump[2]->SetName("fNumberEntryDump3");
      fMainFrameGUI->AddFrame(fNumberEntryDump[2], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fNumberEntryDump[2]->MoveResize(665,144,64,20);
      fNumberEntryDump[3] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
      fNumberEntryDump[3]->SetName("fNumberEntryDump4");
      fMainFrameGUI->AddFrame(fNumberEntryDump[3], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fNumberEntryDump[3]->MoveResize(985,144,64,20);

      fNumberEntryTS[0] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,7,-1,(TGNumberFormat::EStyle) 5);
      fNumberEntryTS[0]->SetName("fNumberEntryTS1");
      fMainFrameGUI->AddFrame(fNumberEntryTS[0], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fNumberEntryTS[0]->MoveResize(25,168,64,20);
      fNumberEntryTS[1] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
      fNumberEntryTS[1]->SetName("fNumberEntryTS2");
      fMainFrameGUI->AddFrame(fNumberEntryTS[1], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fNumberEntryTS[1]->MoveResize(345,168,64,20);
      fNumberEntryTS[2] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
      fNumberEntryTS[2]->SetName("fNumberEntryTS3");
      fMainFrameGUI->AddFrame(fNumberEntryTS[2], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fNumberEntryTS[2]->MoveResize(665,168,64,20);
      fNumberEntryTS[3] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
      fNumberEntryTS[3]->SetName("fNumberEntryTS4");
      fMainFrameGUI->AddFrame(fNumberEntryTS[3], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      fNumberEntryTS[3]->MoveResize(985,168,64,20);

      fMainFrameGUI->MapSubwindows();

      //fMainFrameGUI->Resize(fMainFrameGUI->GetDefaultSize());
      fMainFrameGUI->MapWindow();
      fMainFrameGUI->Resize(1400,916);
      //fMainFrameGUI->Resize(1200,916);
      //fMainFrameGUI->Resize(896,916);

      xapp->Run(kTRUE);
      for (int i=0; i<CHRONO_N_BOARDS; i++)
        clock[i]=CHRONO_CLOCK_CHANNEL;
      DetectorChans[0]=16;
      DetectorChans[1]=17;
      StartChannel[0]=16+15;
      StopChannel[0]=16+16;
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::EndRun, run %d\n", runinfo->fRunNo);
      for (int i=0; i<2; i++)
      {
         printf("DETSIZE:%zu\n",DetectorTS[i].size());
         printf("COUNTSIZE:%zu\n",DetectorCounts[i].size());
      }
      fMainFrameGUI->CloseWindow();
      delete xapp;
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      
      //AgEventFlow *ef = flow->Find<AgEventFlow>();
      //if (!ef || !ef->fEvent)
      //   return flow;
      const AgChronoFlow* ChronoFlow = flow->Find<AgChronoFlow>();
      if (!ChronoFlow) 
      {
        const AgDumpFlow* DumpFlow = flow->Find<AgDumpFlow>();
        if (!DumpFlow) return flow;
        else
        { // I am a Dump Flow
           for (int iSeq=0; iSeq<NUMSEQ; iSeq++)
           {
             //Fix this to insert new vector at back (not this dumb loop)
             for (uint i=0; i<DumpFlow->DumpMarkers[iSeq].size(); i++)
             {
               DumpMarkers[iSeq].push_back(DumpFlow->DumpMarkers[iSeq].at(i));
             }
           }
        }
      }
      else //I am a chrono flow
      {
         //Add start dump time stamps when they happen
         //for (int i=0; i<4; i++) // Loop over sequencers
         for (int i=0; i<1; i++) // Loop over sequencers
         {
            if (!(ChronoFlow->Counts[StartChannel[i]])) continue;
            StartTime[i].push_back(ChronoFlow->RunTime[StartChannel[i]]);
         }
         //Add stop dump time stamps when they happen
         //for (int i=0; i<4; i++)
         for (int i=0; i<1; i++)
         {
            if (!(ChronoFlow->Counts[StopChannel[i]])) continue;
            StopTime[i].push_back(ChronoFlow->RunTime[StopChannel[i]]);
            printf("PAIR THIS: %f\n",ChronoFlow->RunTime[StopChannel[i]]);
            printf("START STOP PAIR!: %f - %f \n",StartTime[i].at(0),StopTime[i].at(0));
         }
         
         //Fill detector array
         for (int i=0; i<2; i++)
         {
            if (!(ChronoFlow->Counts[DetectorChans[i]])) continue;
            DetectorTS[i].push_back(ChronoFlow->RunTime[DetectorChans[i]]);
            DetectorCounts[i].push_back(ChronoFlow->Counts[DetectorChans[i]]);
         }
      }
      
      if (me->serial_number % 100 == 0 ) //Periodically draw spills
        DrawSpills();
      //delete flow?
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("SpillLog::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class SpillLogFactory: public TAFactory
{
public:
   SpillLogFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("SpillLogFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("SpillLogFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("SpillLogFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new SpillLog(runinfo, &fFlags);
   }
};

static TARegister tar(new SpillLogFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
