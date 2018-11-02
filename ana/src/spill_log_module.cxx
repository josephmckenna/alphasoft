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
#include "TSystem.h"
#include <TEnv.h>

#include "AgFlow.h"
#include "chrono_module.h"
#include "TChrono_Event.h"
#include "TChronoChannelName.h"
#include "TTree.h"

#include <vector>
//MAX DET defined here:
#include "TSpill.h"
#include "TGFont.h"
#include "TGFrame.h"
#include "TGListBox.h"
#include "TGTextEdit.h"
#include "TGNumberEntry.h"
#ifndef ROOT_TGLabel
#include "TGLabel.h"
#endif

#include "TROOT.h"
#include "TEnv.h"


#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

#define HOT_DUMP_LOW_THR 500


#include "AnalysisTimer.h"

time_t gTime; // system timestamp of the midasevent
time_t LastUpdate;
//struct tm LastUpdate = {0};

std::list<TSpill*> Spill_List;

TGMainFrame* fMainFrameGUI = NULL;
TGListBox* fListBoxSeq[USED_SEQ]={NULL,NULL,NULL,NULL}; 
TGListBox* fListBoxLogger;
TGTextEdit* fTextEditBuffer;
TGTextButton *fTextButtonCopy;

TGNumberEntry* fNumberEntryDump[USED_SEQ];
TGNumberEntry* fNumberEntryTS[USED_SEQ];
#ifdef HAVE_XMLSERVER
#include "xmlServer.h"
#include "TROOT.h"
#endif

#ifdef XHAVE_LIBNETDIRECTORY
#include "netDirectoryServer.h"
#endif


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
   bool fWriteElog = false;

};

class SpillLog: public TARunObject
{
public: 
   SpillLogFlags* fFlags;
   bool fTrace = false;
   int gIsOnline = 0;
   Int_t RunState =-1;
   Int_t gRunNumber =0;
   time_t run_start_time=0;
   time_t run_stop_time=0;
   int seqcount[NUMSEQ]; //Count sequences run
   double trans_value = 0.;
   
private:

public:
   
   //Chronobox channels
   Int_t clock[CHRONO_N_BOARDS];

   //Detector data to integrate (From ChronoFlow)
   Int_t DetectorChans[CHRONO_N_BOARDS][MAXDET];
   std::vector<Double_t> DetectorTS[MAXDET];
   std::vector<Int_t> DetectorCounts[MAXDET];
   TString detectorName[MAXDET];

   //Dump Marker counter (From ChronoFlow)
   std::vector<Double_t> StartTime[NUMSEQ];
   std::vector<Double_t> StopTime[NUMSEQ];


   //Channels for Dump markers
   ChronoChannel StartChannel[NUMSEQ];
   ChronoChannel StopChannel[NUMSEQ];
   ChronoChannel StartSeqChannel[NUMSEQ];

   std::vector<DumpMarker> DumpMarkers[NUMSEQ][2];
   uint DumpStarts;
   uint DumpStops;
   //Dump Markers to give timestamps (From DumpFlow)
   //std::vector<TString> Description[4];
   //std::vector<Int_t> DumpType[4]; //1=Start, 2=Stop
   //std::vector<Int_t> fonCount[4];
   // Int_t SequencerNum[4];
   Int_t gADSpillNumber;
   TSpill* gADSpill;
   ChronoChannel gADSpillChannel={-1,-1};
   
   Int_t gPOSSpillNumber;
   ChronoChannel gPOSSpillChannel={-1,-1};
   
   
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
   sprintf(buf,"%33s |","Dump Time");
   *log += buf;
   for (int i=0; i<USED_SEQ; i++)
   {
      int iSeq=USED_SEQ_NUM[i];
      sprintf(buf,"%-10s",SeqNames[iSeq].Data());
      *log +=buf;
   }
   *log+="|";

   for (int iDet = 0; iDet<MAXDET; iDet++)
   {
      sprintf(buf,"%-9s ", detectorName[iDet].Data());
      *log += buf;
   }
}

TString LogSpills() 
{

   TString log = "";
   TGString logstr = "";
   for (int i = 0; i < fListBoxLogger->GetNumberOfEntries(); i++)
   {
      TGString logstr = ((TGTextLBEntry*)fListBoxLogger->GetEntry(i))->GetText(); 
      log += TString::Format("%s\n",logstr.Data());
   }
   std::cout << std::endl << "--- Run summary: ---" << std::endl;
   std::cout << log.Data() << std::endl << std::endl;

   for (int i = 0; i < USED_SEQ; i++)
   {
      int iSeqType=USED_SEQ_NUM[i];
      std::list<TSeq_Dump*>::iterator itd;
      for ( uint j=0; j< DumpMarkers[iSeqType][1].size(); j++ )
      {
         if(DumpMarkers[iSeqType][1].at(j).IsDone) continue;
         log += "LogSpills: Msg: INCOMPLETE EVENT:";
         log += DumpMarkers[iSeqType][1].at(j).Description.Data();
         log += "\n";
      }
   }
   return log;
}


void CheckDumpArraySize(Int_t iSeqType, Int_t DumpType)
//Check if Dump times have a valid dump marker... if not, make one
{
   if (DumpType==1)
      if (StartTime[iSeqType].size()>DumpStarts)
      {
         std::cout<<"Some missing start dump markers found..."<<std::endl;
         DumpMarker a;
         a.Description="NO DUMP NAME!";
         a.DumpType=1;
         a.fonCount=StartTime[iSeqType].size()-1;
         a.IsDone=false;
         DumpMarkers[iSeqType][0].push_back(a);
         DumpStarts++;
      }
   if (DumpType==2)
      if (StopTime[iSeqType].size()>DumpStops)
      {
         std::cout<<"Some missing stop dump markers found in seq "<<iSeqType<<"..."<<std::endl;
         std::cout<<"Chrono triggers: "<<StopTime[iSeqType].size()<<std::endl;
         std::cout<<"Dump Markers: "<<DumpMarkers[iSeqType][1].size()<<std::endl;
         DumpMarker a;
         a.Description="NO DUMP NAME!";
         a.DumpType=2;
         a.fonCount=StopTime[iSeqType].size()-1;
         a.IsDone=false;
         DumpMarkers[iSeqType][1].push_back(a);
         DumpStops++;
      }

}


void CatchUp() 
{
 /* struct DumpMarker {
    TString Description;
    Int_t DumpType; //1= Start, 2=Stop, 3=AD spill?, 4=Positrons?
    Int_t fonCount;
    bool IsDone;
  };*/
/*
   //Dump Marker counter (From ChronoFlow)
   std::vector<Double_t> StartTime[NUMSEQ];
   std::vector<Double_t> StopTime[NUMSEQ];
*/
   time(&LastUpdate);

   for (int i = 0; i < USED_SEQ; i++)
   {
      int iSeqType=USED_SEQ_NUM[i];
      uint dump_starts = DumpMarkers[iSeqType][0].size();
      uint dump_stops = DumpMarkers[iSeqType][1].size();
      uint nentries=dump_starts;
      if (dump_stops<nentries) nentries=dump_stops;
      uint nstart = StartTime[iSeqType].size();
      if (nstart<nentries) nentries=nstart;
      uint nstop = StopTime[iSeqType].size();
      if (nstop<nentries) nentries=nstop;
      if (nstart > DumpStarts)
      {
         std::cout << "Missing start dumps..."<<std::endl;
         CheckDumpArraySize(iSeqType,1);
      }
      
      if (nstop > DumpStops)
      {
         std::cout << "Missing stop dumps..."<<std::endl;
         CheckDumpArraySize(iSeqType,2);
      }
      
      //int start_count=0;
      for( uint j = 0; j < nentries; j++ )
      {
         if (j>=nentries) continue;
         std::cout<<"SIZE:"<<DumpMarkers[iSeqType][0].size()<<"+"<<DumpMarkers[iSeqType][1].size()<<std::endl;
      
         std::cout<<"CatchUp: seq"<<iSeqType<<" dump: "<<j<<std::endl;
         if (DumpMarkers[iSeqType][1].at(j).IsDone) continue;
         uint Count=DumpMarkers[iSeqType][1].at(j).fonCount;
         //If this dump has no time stamp, continue
         //if ( StartTime[iSeqType].size() < start_count ) continue;
         DumpMarkers[iSeqType][0].at(j).IsDone=true;
         std::cout <<j<<" start found"<<std::endl;
         
         
         CheckDumpArraySize(iSeqType,2);
         //if ( StopTime[iSeqType].size() < start_count ) continue;
         std::cout <<j<<" stop found"<<std::endl;
//         if (Count>=StartTime[iSeqType].size()) contine;
//         if (Count>=StopTime[iSeqType].size()) contine;
         DumpMarkers[iSeqType][1].at(j).IsDone=true;
         std::cout << Count <<"-"<<StartTime[iSeqType].size()<<std::endl;
         std::cout << Count <<"-"<<StopTime[iSeqType].size()<<std::endl;
         //if ( StartTime[iSeqType].size() < j +1) continue;
         //if ( StopTime[iSeqType].size() < j +1) continue;
         //std::cout<<"CatchUp DumpMarker Processed"<<std::endl;
         TSeq_Dump * d = new TSeq_Dump();
         //Nested dumps not supported
         d->SetSeqNum(iSeqType);
         std::cout<<"a"<<std::endl;
         d->SetDescription(DumpMarkers[iSeqType][0].at(j).Description);
         std::cout<<"b"<<std::endl;
         std::cout<<"COUNT:"<<Count<<"  "<<StartTime[iSeqType].size()<<std::endl;
         
std::cout<<"starttime:"<<StartTime[iSeqType].at(j)<<std::endl; 
         d->SetStartonTime(StartTime[iSeqType].at(j));
         std::cout<<"c"<<std::endl;
         d->SetStoponTime(StopTime[iSeqType].at(j));
         std::cout<<"d"<<std::endl;
         d->SetDone(kTRUE);
         //fNumberEntryDump[iSeqType]->SetIntNumber(fNumberEntryDump[iSeqType]->GetIntNumber()+1);
         //fNumberEntryDump[iSeqType]->Layout();

         d->Print();
         UpdateDumpIntegrals(d);
         TString log ="";
         TSpill* spill=new TSpill();
         spill->FormatDumpInfo(&log,d,kTRUE);
         Spill_List.push_back(spill);
         std::cout<<log<<std::endl;
         //  gPendingSpill->AddDump(se);
         std::cout <<"Making dump..."<<std::endl;
         fListBoxLogger->AddEntrySort(log.Data(),fListBoxLogger->GetNumberOfEntries());
         LayoutListBox(fListBoxLogger);
//                  cout<<"SyncSeq pending spill"<<endl;
      }

   } // end loop seq type

}

/*
void SyncSeq(Int_t iSeqType, Bool_t useDumps)
{ 
  // update the list of sequencer dumps with info about their completion

  // cout << "Num of dumps: " << ListOfDumps[iSeqType].size() << " ; " << iSeqType << " =? " << RECATCH << endl;
  
  // if checking advancement using dumps, handle the case in which no corresponding sequence xml has been received 
  // in that case, don't even look for a sequence start (assume seqstart and XML come at about the same time)
  if ( ListOfDumps[iSeqType].size() < 1 )
    { 
      //    if ( ListOfDumps[iSeqType].size() < 1 || seq_startevent[iSeqType].size() < ListOfDumps[iSeqType].size() ){ 
      //    if ( (int) ListOfDumps[iSeqType].size() < 1 || (int) seq_startevent[iSeqType].size() < cSeq[iSeqType] ){ 
      //    if (gPendingDump[iSeqType]->GetSeqNum() < 0){
      
      if (gPendingDump[iSeqType] && useDumps)
        {
          if ( gPendingDump[iSeqType]->IsDone() ) 
            {
              gPendingDump[iSeqType]->SetStartonTime( gPendingDump[iSeqType]->GetStartonTime() );
              gPendingDump[iSeqType]->SetStoponTime(  gPendingDump[iSeqType]->GetStoponTime() );
              
              UpdateDumpIntegrals(gPendingDump[iSeqType]);

              if(gPendingSpill)
                {
                  TString log ="";
                  gPendingSpill->FormatDumpInfo(&log,gPendingDump[iSeqType],kTRUE);
                  cout<<log<<endl;
                  //  gPendingSpill->AddDump(se);
                  fListBoxLogger->AddEntrySort(log.Data(),fListBoxLogger->GetNumberOfEntries());
                  LayoutListBox(fListBoxLogger);
//                  cout<<"SyncSeq pending spill"<<endl;
                }

              //              if( !gIsOffline )
              Messages(gPendingDump[iSeqType]);

              gPendingDump[iSeqType] = NULL; // or 0
            }
        }
    } 
  // <<<<<------ end

  // loop over sequencers
  list<TSeq_Dump*>::iterator it;
  for ( it = ListOfDumps[iSeqType].begin() ; it != ListOfDumps[iSeqType].end(); it++ )
    {
      TSeq_Dump * se = (TSeq_Dump*)*it;
      Int_t dumpSeqIndex = se->GetSeqNum() - 1;

      // if the dump is already been completed, skip to next dump
      if ( se->IsDone() )
        continue;

      if(gPendingSeqStart[iSeqType] > 0 && dumpSeqIndex + 1 >= gPendingSeqStart[iSeqType]) 
        { // in the ListOfDumps elements, dumpseqindex should be >= 0
          // Mark start of sequence 
          //          if(gPendingSpill){
          char message[200];
          //      sprintf(message,"-----  %s sequence %d start  -----", SeqNames[iSeqType].Data(),cSeq[iSeqType]);
          if (SeqNames[iSeqType].Data()[0]=='c')
          if (iSeqType == PBAR)
            sprintf(message,"                                -----  %s sequence %d start  -----", SeqNames[iSeqType].Data(),gPendingSeqStart[iSeqType]);
          if (iSeqType == RECATCH)
            sprintf(message,"                                                -----  %s sequence %d start  -----", SeqNames[iSeqType].Data(),gPendingSeqStart[iSeqType]);
          if (iSeqType == ATOM)
            sprintf(message,"                                                                -----  %s sequence %d start  -----", SeqNames[iSeqType].Data(),gPendingSeqStart[iSeqType]); 
          if (iSeqType == POS)
            sprintf(message,"                                                                                -----  %s sequence %d start  -----", SeqNames[iSeqType].Data(),gPendingSeqStart[iSeqType]); 
  
          
          fListBoxLogger->AddEntrySort(message,fListBoxLogger->GetNumberOfEntries());
          LayoutListBox(fListBoxLogger);
          
          cout << message << endl;
          
          gPendingSeqStart[iSeqType] = 0;
        }
      
      //    // if sequence is not yet started, exit loop
      if ( dumpSeqIndex + 1 > (int) seq_startevent[iSeqType].size() )
        break;
      
      if(!useDumps)           // check advancement using timestamps
        {     
          // if timestamp has progressed past the start count of the current dump     
          if ( timestamp[iSeqType] >= seq_counts[iSeqType].at(dumpSeqIndex) + se->GetStartonCount() )
            {
              se->SetStarted( kTRUE );
              se->SetStartOfSeq( seq_startevent[iSeqType].at(dumpSeqIndex) );
            }
        }
      else 
        {    // check advancement with dumps
          if (gPendingDump[iSeqType])
            {
              se->SetStarted( kTRUE );
              se->SetStartOfSeq( seq_startevent[iSeqType].at(dumpSeqIndex) );
//              cout<<"SyncSeq dump started"<<endl;
            }
        }
      
    // if the dump has not yet started (even after the above check), exit loop
    if ( !se->HasStarted() )
      break;
    
    // check if the dump has finished (N.B.: during this call to the function)
    if(!useDumps) 
      {     // check advancement using timestamps                 
        if ( timestamp[iSeqType] >= seq_counts[iSeqType].at(dumpSeqIndex) + se->GetStoponCount() ) 
          {
            
            se->SetDone(kTRUE);        // flag the dump as finished
            
            se->SetStartonTime( entry2time( sis_tree[iSeqType], sis_event[iSeqType], se->GetStartOfSeq() + se->GetStartonCount() ) );
            se->SetStoponTime(  entry2time( sis_tree[iSeqType], sis_event[iSeqType], se->GetStartOfSeq() + se->GetStoponCount() ) );
            
            UpdateDumpIntegrals(se);          
            
            cout << se->GetDescription() << endl;
        }        
    }
    else 
      {     // check advancement using dumps
        if (gPendingDump[iSeqType])
          {
            if ( gPendingDump[iSeqType]->IsDone() ) 
              {
                se->SetDone(kTRUE);        // flag the dump as finished
                
                se->SetStartonTime( gPendingDump[iSeqType]->GetStartonTime() );
                se->SetStoponTime(  gPendingDump[iSeqType]->GetStoponTime() );
                
                UpdateDumpIntegrals(se);          
                
                gPendingDump[iSeqType] = NULL; // or 0            
                
                cout << se->GetDescription() << endl;
//                cout<<"SyncSeq integral"<<endl;
                IsHotDump(se);
                if(IsColdDump(se)) SendData(se);
              }
          }
      }

    // if after the cycle it is done 
    if (se->IsDone())
      {
        if(gPendingSpill)
          {
            TString buf = "(" + SeqNames[iSeqType] + ")";
            TString log =TString::Format(" %-4s ",buf.Data());
            gPendingSpill->FormatDumpInfo(&log,se,kFALSE);
            //        gPendingSpill->FormatDumpInfo(&log,se,kTRUE);
            cout<<log<<endl;
            //  gPendingSpill->AddDump(se);
            fListBoxLogger->AddEntrySort(log.Data(),fListBoxLogger->GetNumberOfEntries());
            LayoutListBox(fListBoxLogger);
  //          cout<<"SyncSeq dump done"<<endl;
          }
        
        //      DrawSpills();
        
        //      if( !gIsOffline )
        Messages(se);
      }
     
  }

  
}
*/



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
      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         if (DetectorChans[board][iDet]>-1) 
         { 
            Int_t val = getIntegral(iDet, se->GetStartonTime(), se->GetStoponTime()); 
            //std::cout <<"Channel " <<detectorCh[iDet] <<"  Integral "<< val << std::endl; 
            se->SetDetIntegral( iDet, val); 
         }
      }
   }
}
 


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      //Is running, RunState==3
      //Is idle, RunState==1
      RunState=runinfo->fOdb->odbReadInt("/runinfo/State"); //The odb isn't in its 'final' state before run, so this is useless
      gRunNumber=runinfo->fRunNo;
      std::cout <<"RUN STATE!:"<<RunState<<std::endl;
      run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      std::cout<<"START:"<< run_start_time<<std::endl;
      std::cout<<"STOP: "<< run_stop_time<<std::endl;
      
      for (int i=0; i<CHRONO_N_BOARDS; i++)
         clock[i]=CHRONO_CLOCK_CHANNEL;

     //Save chronobox channel names
     TChronoChannelName* name = new TChronoChannelName();
     TString ChannelName;

     for (int i=0; i<USED_SEQ; i++)
     {

        int iSeq=USED_SEQ_NUM[i];
        seqcount[iSeq]=0;
        std::cout<<i<<" is " << iSeq <<std::endl;
        StartChannel[iSeq].Channel=-1;
        StartChannel[iSeq].Board=-1;
        StopChannel[iSeq].Channel=-1;
        StopChannel[iSeq].Board=-1;
        StartSeqChannel[iSeq].Channel=-1;
        StartSeqChannel[iSeq].Board=-1;

           //fListBoxSeq[i]->RemoveAll();
           //LayoutListBox(fListBoxSeq[i]);
        
     }
     if (run_start_time>0 && run_stop_time==0) //Start run
     {
        for (int i=0; i<USED_SEQ; i++)
        {
           gIsOnline=1;
           //           int iSeq=USED_SEQ_NUM[i]; // unsed  -- AC
           fListBoxSeq[i]->RemoveAll();
           LayoutListBox(fListBoxSeq[i]);
        }
     }
     else
     {
        gIsOnline=0;
     }
     for (int board=0; board<CHRONO_N_BOARDS; board++)
     {
        name->SetBoardIndex(board+1);
        for (int det=0; det<MAXDET; det++)
          DetectorChans[board][det]=-1;
        for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
        {
            
            TString OdbPath="/Equipment/cbms0";
            OdbPath+=board+1;
            OdbPath+="/Settings/ChannelNames";
            //std::cout<<runinfo->fOdb->odbReadString(OdbPath.Data(),chan)<<std::endl;
            if (runinfo->fOdb->odbReadString(OdbPath.Data(),chan))
               name->SetChannelName(runinfo->fOdb->odbReadString(OdbPath.Data(),chan),chan);
         }
         //name->Print();
         Int_t channel=name->GetChannel("CATCH_OR");
         if (channel>0) DetectorChans[board][0]=channel;
         detectorName[0]="CATCH_OR";

         channel=name->GetChannel("SiPM_B");
         if (channel>0) DetectorChans[board][1]=channel;
         detectorName[1]="TOP PM";

         channel=name->GetChannel("SiPM_E");
         if (channel>0) DetectorChans[board][2]=channel;
         detectorName[2]="BOT PM";

         channel=name->GetChannel("TPC_TRIG");
         if (channel>0) DetectorChans[board][3]=channel;
         detectorName[3]="TPC";

         channel=name->GetChannel("SiPM_A_AND_D");
         if (channel>0) DetectorChans[board][4]=channel;
         detectorName[4]="SiPM_A_AND_D";

         channel=name->GetChannel("SiPM_C_AND_F");
         if (channel>0) DetectorChans[board][5]=channel;
         detectorName[5]="SiPM_C_AND_F";

         channel=name->GetChannel("SiPM A_OR_C-AND-D_OR_F");
         if (channel>0) DetectorChans[board][6]=channel;
         detectorName[6]="SiPM A_OR_C-AND-D_OR_F";

         channel=name->GetChannel("SiPM_C");
         if (channel>0) DetectorChans[board][7]=channel;
         detectorName[7]="SiPM_C";

         channel=name->GetChannel("SiPM_D");
         if (channel>0) DetectorChans[board][8]=channel;
         detectorName[8]="SiPM_D";

         channel=name->GetChannel("SiPM_F");
         if (channel>0) DetectorChans[board][9]=channel;
         detectorName[9]="SiPM_F";



         for (int i=0; i<USED_SEQ; i++)
         {
            int iSeq=USED_SEQ_NUM[i];
            channel=name->GetChannel(StartDumpName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]:"<<StartDumpName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               StartChannel[iSeq].Channel=channel;
               StartChannel[iSeq].Board=board;
               std::cout<<"Start Channel:"<<channel<<std::endl;
            }
            
            channel=name->GetChannel(StopDumpName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]:"<<StopDumpName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               StopChannel[iSeq].Channel=channel;
               StopChannel[iSeq].Board=board;
               std::cout<<"Stop Channel:"<<channel<<std::endl;
            }

            channel=name->GetChannel(StartSeqName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]"<<StartSeqName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               StartSeqChannel[iSeq].Channel=channel;
               StartSeqChannel[iSeq].Board=board;
            }
         }

         channel=name->GetChannel("AD_TRIG");
         if (channel>0)
         {
            gADSpillChannel.Channel=channel;
            gADSpillChannel.Board=board;
            std::cout<<"AD_TRIG:"<<channel<<" board:"<<board<<std::endl;
         }
         channel=name->GetChannel("POS_TRIG");
         if (channel>0 )
         {
            gPOSSpillChannel.Channel=channel;
            gPOSSpillChannel.Board=board;
         }
      }

      gADSpillNumber=0;
      gADSpill=NULL;
      gPOSSpillNumber=0;

      Spill_List.clear();

      for (int i=0; i<MAXDET; i++)
      {
         DetectorTS[i].clear();
         DetectorCounts[i].clear();
      }
      DumpStarts=0;
      DumpStops=0;
      for (int i = 0; i < USED_SEQ; i++)
      {
         int iSeqType=USED_SEQ_NUM[i];
         StartTime[iSeqType].clear();
         StopTime[iSeqType].clear();
         DumpMarkers[iSeqType][0].clear();
         DumpMarkers[iSeqType][1].clear();
      }
      if (gIsOnline)
      {
         fListBoxLogger->RemoveAll();
         //Print first line of header
         TString log;
         FormatHeader(&log);
         fListBoxLogger->AddEntrySort(TGString(log.Data()),fListBoxLogger->GetNumberOfEntries());
         LayoutListBox(fListBoxLogger);  
         log = "";
         for (int i=0; i<174; i++)
         {
            log+="-";
         }

         fListBoxLogger->AddEntrySort(TGString(log.Data()),fListBoxLogger->GetNumberOfEntries());
         LayoutListBox(fListBoxLogger);

         char message[30];
         sprintf(message,"Run %d",gRunNumber);
         fListBoxLogger->AddEntrySort(TGString(message),fListBoxLogger->GetNumberOfEntries());
         LayoutListBox(fListBoxLogger);
      }
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::EndRun, run %d\n", runinfo->fRunNo);
      //runinfo->State

      if (!gIsOnline) return;

      char message[30];
      sprintf(message,"End run %d",gRunNumber);
      fListBoxLogger->AddEntrySort(TGString(message),fListBoxLogger->GetNumberOfEntries());
      LayoutListBox(fListBoxLogger);

      // write results to file
      TString log = "";
      TGString logstr = "";
      for (int i = 0; i < fListBoxLogger->GetNumberOfEntries(); i++)
      {
         TGString logstr = ((TGTextLBEntry*)fListBoxLogger->GetEntry(i))->GetText(); 
         log += TString::Format("%s\n",logstr.Data());
      }
      std::cout << std::endl << "--- Run summary: ---" << std::endl;
      std::cout << log.Data() << std::endl << std::endl;
   
      char cmd[1024000];
      //if (fileCache)
      {
         //TString DataLoaderPath=outfileName(0,outfileName.Length()-5);
         TString spillLogName="R";
         spillLogName+=gRunNumber;
         spillLogName+=".log";
         std::cout <<"Log file: "<<spillLogName<<std::endl;
         std::ofstream spillLog (spillLogName);
         spillLog<<"[code]"<<log.Data()<<"[/code]"<<std::endl;
         spillLog.close();
         sprintf(cmd,"cat %s | ssh -x alpha@alphadaq /home/alpha/packages/elog/elog -h localhost -p 8080 -l SpillLog -a Run=%d -a Author=ALPHAgdumps &",spillLogName.Data(),gRunNumber);
         printf("--- Command: \n%s\n", cmd);
         if ( fFlags->fWriteElog )
            system(cmd);
      }

      for (int i=0; i<MAXDET; i++)
      {
         for (int j=0; j<CHRONO_N_BOARDS; j++)
         {
            if (DetectorChans[j][i]<0) continue;
            printf("Detector: %s\n",detectorName[i].Data());
            printf("Channel: %d\n",DetectorChans[j][i]);
            printf("DETSIZE:%zu\n",DetectorTS[i].size());
            printf("COUNTSIZE:%zu\n",DetectorCounts[i].size());
         }
      }
      for (int i = 0; i < USED_SEQ; i++)
      {
         int iSeqType=USED_SEQ_NUM[i];
         std::cout<<"Seq dumps (starts and stops)"<<iSeqType<<" - " << DumpMarkers[iSeqType][0].size()<<"+"<< DumpMarkers[iSeqType][1].size()<<std::endl;
         std::cout<<"Start triggers: "<< StartTime[iSeqType].size();
         std::cout<< " - Stop Triggers: " << StopTime[iSeqType].size()<<std::endl;
      }

      LogSpills();
      Spill_List.clear();
      for (int i=0; i<MAXDET; i++)
      {
         DetectorTS[i].clear();
         DetectorCounts[i].clear();
      }
      for (int i = 0; i < USED_SEQ; i++)
      {
         int iSeqType=USED_SEQ_NUM[i];
         StartTime[iSeqType].clear();
         StopTime[iSeqType].clear();
         DumpMarkers[iSeqType][0].clear();
         DumpMarkers[iSeqType][1].clear();
         //fListBoxSeq[i]->Clear();
      }
   }
   
   void DrawADSpill()
   {
      if (gADSpill)
      {
         if (trans_value>0 || gTime > *(gADSpill->GetTime())+10) //Veto'd beam: trans_value is NULL
         {
            gADSpill->SetTransformer( trans_value );
            Spill_List.push_back(gADSpill);
            printf("AD spill\n");
            TGFont* lfont = gClient->GetFontPool()->GetFont("Courier",12,kFontWeightNormal,kFontSlantItalic); 
            if (!lfont){
               exit(123);
            }
            TString log = "";
            gADSpill->FormatADInfo(&log);
            Int_t SpillLogEntry = fListBoxLogger->GetNumberOfEntries();
            TGTextLBEntry* lbe = new TGTextLBEntry(fListBoxLogger->GetContainer(), new TGString(log.Data()), SpillLogEntry,  TGTextLBEntry::GetDefaultGC()(), lfont->GetFontStruct());
            TGLayoutHints *lhints = new TGLayoutHints(kLHintsExpandX | kLHintsTop);
            fListBoxLogger->AddEntrySort(lbe,lhints);
            //fListBoxLogger->AddEntrySort(TGString(log.Data()),gSpillLogEntry);
            LayoutListBox(fListBoxLogger);
            trans_value=-999.;
            gADSpill=NULL;
         }
      }
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
#define FAKE_SOME_DUMPS 0
#if FAKE_SOME_DUMPS
Int_t DemoDump=1;
#endif
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if (!gIsOnline) return flow;
       time(&gTime);  /* get current time; same as: timer = time(NULL)  */

      //Periodically update spill even if no data has arrived
      Double_t seconds = difftime(gTime,LastUpdate);
      if (seconds>10)
      {
#if FAKE_SOME_DUMPS
         //std::cout <<"FAKE AN EVENT"<<std::endl;
         DumpMarker m;
         
         m.Description="Demo Dump ";
         m.Description+=DemoDump;
         m.DumpType=1;
         m.fonCount=DemoDump;
         m.IsDone=false;
         DumpMarkers[3].push_back(m);
         m.Description="Demo Dump ";
         m.Description+=DemoDump;
         m.DumpType=2;
         m.fonCount=DemoDump;
         m.IsDone=false;
         DumpMarkers[3].push_back(m);
         StartTime[3].push_back(2.71828+10*(DemoDump-1));
         StopTime[3].push_back(3.141+10*(DemoDump-1));
         DemoDump++;
#endif
         std::cout<<"Update"<<std::endl;
         CatchUp();
      }

      if( me->event_id == 6 )  //labview
         {
            const TMBank* b = me->FindBank("ADE1");
            if( b )
               {
                  std::cout<<"SpillLog::Analyze   BANK NAME: "<<b->name<<std::endl;
                  int ade0size = b->data_size;
                  if( ade0size ) std::cout<<"SpillLog::Analyze   BANK SIZE: "<<ade0size<<std::endl;
                  const char* ade0ptr = me->GetBankData(b);
                  double * ade0data = (double*)ade0ptr;
                  
                  // timing ?
                  time_t unixtime =  (time_t) round(ade0data[0] - 2082844800.);
                  struct tm  *ts;
                  char ade0time[80];
                  ts = localtime(&unixtime);
                  strftime(ade0time, sizeof(ade0time), "%H:%M:%S", ts);
                  printf("ade0data for spill at [%ld / %s] : %lf\t%lf\t%lf\n",
                         unixtime, ade0time, ade0data[0], ade0data[1], ade0data[2]);
                  
                  // transformer
                  trans_value = ade0data[2];  // -- 2014
                  std::cout<<"AD Transformer Value: "<<trans_value<<std::endl;
                  DrawADSpill();
               }
         }

      const AgChronoFlow* ChronoFlow = flow->Find<AgChronoFlow>();
      if (ChronoFlow) 
      {
         for (uint iEvent=0; iEvent<ChronoFlow->events->size(); iEvent++)
         {
            ChronoEvent* ChronoE=ChronoFlow->events->at(iEvent);
            //if (!ChronoE->ChronoBoard>0) continue;
            //Add start dump time stamps when they happen
            //for (int i=0; i<4; i++) // Loop over sequencers
            for (int i = 0; i < USED_SEQ; i++)
            {
               int iSeqType=USED_SEQ_NUM[i];
               if (StartChannel[iSeqType].Channel<0) continue;
               if (ChronoE->Channel!=StartChannel[iSeqType].Channel) continue;
               if (ChronoE->ChronoBoard!=StartChannel[iSeqType].Board) continue;
               std::cout <<SeqNames[i]<<" StartDump["<<iSeqType<<"(position"<<i<<")]->at("<<StartTime[iSeqType].size()<<") at "<<ChronoE->RunTime<<std::endl;
               StartTime[iSeqType].push_back(ChronoE->RunTime);
            }
            //Add stop dump time stamps when they happen
            //for (int i=0; i<4; i++)
            for (int i = 0; i < USED_SEQ; i++)
            {
               int iSeqType=USED_SEQ_NUM[i];
               if (StopChannel[iSeqType].Channel<0) continue;
               if (ChronoE->Channel!=StopChannel[iSeqType].Channel) continue;
               if (ChronoE->ChronoBoard!=StopChannel[iSeqType].Board) continue;
               std::cout <<SeqNames[i]<<" StopDump["<<iSeqType<<"(position"<<i<<")]->at("<<StopTime[iSeqType].size()<<") "<<ChronoE->RunTime<<std::endl;
               StopTime[iSeqType].push_back(ChronoE->RunTime);
               printf("PAIR THIS: %f\n",ChronoE->RunTime);
               //printf("START STOP PAIR!: %f - %f \n",StartTime[i].at(0),StopTime[i].at(0));
               CatchUp();
            }
            if (ChronoE->ChronoBoard==gADSpillChannel.Board)
               if (ChronoE->Channel==gADSpillChannel.Channel)
               //if (gADSpillChannel>0)
                  if (ChronoE->Counts)
                  {
                     gADSpillNumber++;
                     gADSpill = new TSpill( runinfo->fRunNo, gADSpillNumber, gTime, MAXDET );
                     DrawADSpill();
                  }

            if (ChronoE->ChronoBoard==gPOSSpillChannel.Board)
               if (ChronoE->Channel==gPOSSpillChannel.Channel)
               //if (gPOSSpillChannel>0)
               {
                  gPOSSpillNumber++;
                  TSpill *s = new TSpill( runinfo->fRunNo, gADSpillNumber, gTime, MAXDET );
                  Spill_List.push_back(s);
                  printf("POS spill\n");
                  TGFont* lfont = gClient->GetFontPool()->GetFont("Courier",12,kFontWeightNormal,kFontSlantItalic); 
                  if (!lfont)
                  {
                     exit(123);
                  }
                  TString log = "";
                  s->FormatADInfo(&log);
                  Int_t SpillLogEntry = fListBoxLogger->GetNumberOfEntries();
                  TGTextLBEntry* lbe = new TGTextLBEntry(fListBoxLogger->GetContainer(), new TGString(log.Data()), SpillLogEntry,  TGTextLBEntry::GetDefaultGC()(), lfont->GetFontStruct());
                  TGLayoutHints *lhints = new TGLayoutHints(kLHintsExpandX | kLHintsTop);
                  fListBoxLogger->AddEntrySort(lbe,lhints);
                  //fListBoxLogger->AddEntrySort(TGString(log.Data()),gSpillLogEntry);
                  LayoutListBox(fListBoxLogger);
               }
            for (int i=0; i<USED_SEQ; i++)
            {
               int iSeqType=USED_SEQ_NUM[i];
               if (StartSeqChannel[iSeqType].Channel<0) continue;
                  if (ChronoE->Channel==StartSeqChannel[iSeqType].Channel)
                     if (ChronoE->ChronoBoard==StartSeqChannel[iSeqType].Board)
                     {
                        TString log = "      ------------ ";
                        log +=SeqNames[i];
                        log +=" seq start ------------";
                        std::cout<<log<<std::endl;
                        TGFont* lfont = gClient->GetFontPool()->GetFont("courier",12,kFontWeightNormal,kFontSlantItalic); 
                        if (!lfont)
                        {
                           exit(123);
                        }
                        Int_t SpillLogEntry = fListBoxLogger->GetNumberOfEntries();
                        TGTextLBEntry* lbe = new TGTextLBEntry(fListBoxLogger->GetContainer(), new TGString(log.Data()), SpillLogEntry,  TGTextLBEntry::GetDefaultGC()(), lfont->GetFontStruct());
                        TGLayoutHints *lhints = new TGLayoutHints(kLHintsExpandX | kLHintsTop);
                        fListBoxLogger->AddEntrySort(lbe,lhints);
                        //fListBoxLogger->AddEntrySort(TGString(log.Data()),gSpillLogEntry);
                        LayoutListBox(fListBoxLogger);
                     }
            }
            //Fill detector array
            //Int_t DetectorChans[CHRONO_N_BOARDS][MAXDET];
            for (int i=0; i<MAXDET; i++)
            {
               int b=ChronoE->ChronoBoard;
               int c=ChronoE->Channel;
               if (DetectorChans[b][i] < 0) continue;
               if (DetectorChans[b][i]!=c) continue;
               int cnts=ChronoE->Counts;
               if (!cnts) continue;
               DetectorTS[i].push_back(ChronoE->RunTime);
               DetectorCounts[i].push_back(ChronoE->Counts);
            }
         }
      }
      const AgDumpFlow* DumpFlow = flow->Find<AgDumpFlow>();
      if (DumpFlow)
      { // I am a Dump Flow
         for (int i = 0; i < USED_SEQ; i++)
         {
            int iSeq=USED_SEQ_NUM[i];
            //Fix this to insert new vector at back (not this dumb loop)
            uint ndumps=DumpFlow->DumpMarkers[iSeq].size();
            for (uint j=0; j<ndumps; j++)
            {
               //Show list of up-comming start dumps
               char StartStop='#';
               int type_pos=-1;
               if (DumpFlow->DumpMarkers[iSeq].at(j).DumpType==1)
               {
                  StartStop='(';
                  DumpStarts++;
                  type_pos=0;
               }
               if (DumpFlow->DumpMarkers[iSeq].at(j).DumpType==2)
               {
                  StartStop=')';
                  DumpStops++;
                  type_pos=1;
               }
               TString msg = TString::Format("%c  %s", StartStop, DumpFlow->DumpMarkers[iSeq].at(j).Description.Data());
               std::cout<<msg<<std::endl;
               fListBoxSeq[i]->AddEntrySort(msg.Data(),fListBoxSeq[i]->GetNumberOfEntries());
               LayoutListBox(fListBoxSeq[i]);
               //Add the markers to a queue for timestamps later
               if (type_pos==0 || type_pos==1)
                  DumpMarkers[iSeq][type_pos].push_back(DumpFlow->DumpMarkers[iSeq].at(j));
            }
         }
      }

      if (me->serial_number % 1000 == 0 ) //Periodically draw spills
      {
         //std::cout <<"Catchup"<<std::endl;
         CatchUp();
      }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"spill_log_module");
      #endif
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
   SpillLogFactory(): TAFactory()
   {
      std::cout<<"Please run me as: ./ag_events.exe -g -Halphagdaq.cern.ch "<<std::endl;
      gEnv->SetValue("Gui.DefaultFont","-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");  
   }
   SpillLogFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("SpillLogFactory::Init!\n");
      

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--elog")
            fFlags.fWriteElog = true;
      }
  if(gROOT->IsBatch()) {
    printf("Cannot run in batch mode\n");
    exit (1);
  }

  //TApplication *app = new TApplication("alpha2dumps", 0, 0);
  //extern TApplication* xapp;
//  runinfo->fRoot->fgApp->Close();
  //runinfo->fRoot->fgApp = new TApplication("alpha2dumps", 0, 0);
  
  //runinfo->fRoot->fgApp->Run();

  //app->Run(kTRUE);
      // main frame
      //  TGMainFrame 
      alphaFrame* fMainFrameGUI = new alphaFrame();
      fMainFrameGUI->SetName("fMainFrameGUI");
      fMainFrameGUI->SetWindowName("ALPHAg dumps");
      fMainFrameGUI->SetLayoutBroken(kTRUE);
      
      int main_width=1600;
      int gap=5;
      int spacing=(main_width-25-(USED_SEQ-1)*gap)/USED_SEQ;
      //int spacing=300;
      int width=spacing-gap;
      
      for (int i=0; i<USED_SEQ; i++)
      {
         // list boxs
         TGLabel *fLabelSeq = new TGLabel(fMainFrameGUI,SeqNames[USED_SEQ_NUM[i]].Data());
         fLabelSeq->SetTextJustify(36);
         fLabelSeq->SetMargins(0,0,0,0);
         fLabelSeq->SetWrapLength(-1);
         fMainFrameGUI->AddFrame(fLabelSeq, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
         fLabelSeq->MoveResize(spacing*i+25,8,62,16);
         fListBoxSeq[i] = new TGListBox(fMainFrameGUI);
         TString boxname="fListBoxSeq";
         boxname+=i+1;
         fListBoxSeq[i]->SetName(boxname.Data());
         fListBoxSeq[i]->Resize(width,116);
         fMainFrameGUI->AddFrame(fListBoxSeq[i], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
         fListBoxSeq[i]->MoveResize(spacing*i+25,24,width,116);
      }


      // list box
      fListBoxLogger = new TGListBox(fMainFrameGUI);
      fListBoxLogger->SetName("fListBoxLogger");
      //  fListBoxLogger->Resize(1150,326);
      fListBoxLogger->Resize(main_width-50,326);
      fMainFrameGUI->AddFrame(fListBoxLogger, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      //  fListBoxLogger->MoveResize(25,208,1150,600);
      fListBoxLogger->MoveResize(25,208,main_width-50,600);
      fListBoxLogger->SetMultipleSelections(kTRUE);
    
    
      for (int i=0; i<USED_SEQ; i++)
      {
         fNumberEntryDump[i] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,7,-1,(TGNumberFormat::EStyle) 5);
         fNumberEntryDump[i]->SetName("fNumberEntryDump1");
         fMainFrameGUI->AddFrame(fNumberEntryDump[i], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
         fNumberEntryDump[i]->MoveResize(spacing*i+25,144,64,20);
         fNumberEntryTS[i] = new TGNumberEntry(fMainFrameGUI, (Double_t) 0,7,-1,(TGNumberFormat::EStyle) 5);
         fNumberEntryTS[i]->SetName("fNumberEntryTS1");
         fMainFrameGUI->AddFrame(fNumberEntryTS[i], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
         fNumberEntryTS[i]->MoveResize(spacing*i+25,168,64,20);
      }
      fMainFrameGUI->MapSubwindows();

      //fMainFrameGUI->Resize(fMainFrameGUI->GetDefaultSize());
      fMainFrameGUI->MapWindow();
      fMainFrameGUI->Resize(main_width,916);
      //fMainFrameGUI->Resize(1200,916);
      //fMainFrameGUI->Resize(896,916);

      //xapp->Run(kTRUE);
 
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
