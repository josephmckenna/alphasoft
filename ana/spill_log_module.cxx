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
#include "TChronoChannelName.h"
#include "TTree.h"

#include <vector>
//MAX DET defined here:
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




TString SeqNames[NUMSEQ]={"cat","rct","atm","pos"};
enum {PBAR,RECATCH,ATOM,POS};
//enum {NOTADUMP,DUMP,EPDUMP}; 


time_t gTime; // system timestamp of the midasevent
time_t LastUpdate;
//struct tm LastUpdate = {0};

std::list<TSpill*> Spill_List;

TGMainFrame* fMainFrameGUI = NULL;
TGListBox* fListBoxSeq[4]; 
TGListBox* fListBoxLogger;
TGTextEdit* fTextEditBuffer;
TGTextButton *fTextButtonCopy;

TGNumberEntry* fNumberEntryDump[4];
TGNumberEntry* fNumberEntryTS[4];
#ifdef HAVE_XMLSERVER
#include "xmlServer.h"
#include "TROOT.h"
#endif

#ifdef XHAVE_LIBNETDIRECTORY
#include "netDirectoryServer.h"
#endif
//TApplication* xapp;


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
   Int_t DetectorChans[CHRONO_N_BOARDS][MAXDET];
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
   uint DumpStarts;
   uint DumpStops;
   //Dump Markers to give timestamps (From DumpFlow)
   //std::vector<TString> Description[4];
   //std::vector<Int_t> DumpType[4]; //1=Start, 2=Stop
   //std::vector<Int_t> fonCount[4];
   // Int_t SequencerNum[4];
   Int_t gADSpillNumber;
   Int_t gADSpillChannel;
   Int_t gADSpillBoard;
   Int_t gPOSSpillNumber;
   Int_t gPOSSpillChannel;
   Int_t gPOSSpillBoard;
   
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


  sprintf(buf,"%-18s","Dump Time");
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
         DumpMarkers[iSeqType].push_back(a);
         DumpStarts++;
      }
   if (DumpType==2)
      if (StopTime[iSeqType].size()>DumpStops)
      {
         std::cout<<"Some missing stop dump markers found in seq "<<iSeqType<<"..."<<std::endl;
         std::cout<<"Chrono triggers: "<<StopTime[iSeqType].size()<<std::endl;
         std::cout<<"Dump Markers: "<<DumpMarkers[iSeqType].size()<<std::endl;
         DumpMarker a;
         a.Description="NO DUMP NAME!";
         a.DumpType=2;
         a.fonCount=StopTime[iSeqType].size()-1;
         a.IsDone=false;
         DumpMarkers[iSeqType].push_back(a);
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

   for (int iSeqType=0; iSeqType<NUMSEQ; iSeqType++)
   {
     
      Int_t nentries;

      nentries = DumpMarkers[iSeqType].size();
      
      uint nstart = StartTime[iSeqType].size();
      uint nstop = StopTime[iSeqType].size();
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
      
      
      for( Int_t i = 0; i < nentries; i++ )
      {
         if (DumpMarkers[iSeqType].at(i).IsDone) continue;
         uint Count=DumpMarkers[iSeqType].at(i).fonCount;
         if( DumpMarkers[iSeqType].at(i).DumpType==1 ) //Start Dump
         {
           //If this dump has no time stamp, continue
           if ( StartTime[iSeqType].size() < Count+1 ) continue;
           DumpMarkers[iSeqType].at(i).IsDone=true;
           std::cout <<i<<" start found"<<std::endl;
           continue; //Wait for a stop dump before drawing...
         }
         else if( DumpMarkers[iSeqType].at(i).DumpType==2 ) 
         {
            CheckDumpArraySize(iSeqType,2);
            if ( StopTime[iSeqType].size() < Count+1 ) continue;
            DumpMarkers[iSeqType].at(i).IsDone=true;
            std::cout <<i<<" stop found"<<std::endl;
         }
         
         TSeq_Dump * d = new TSeq_Dump();
         //Nested dumps not supported
         //std::cout << Count <<"-"<<StartTime[iSeqType].size()<<std::endl;
         //std::cout << Count <<"-"<<StopTime[iSeqType].size()<<std::endl;
         d->SetSeqNum(iSeqType);
         d->SetDescription(DumpMarkers[iSeqType].at(i).Description);
         d->SetStartonTime(StartTime[iSeqType].at(Count));
         d->SetStoponTime(StopTime[iSeqType].at(Count));
         d->SetDone(kTRUE);
         //fNumberEntryDump[iSeqType]->SetIntNumber(fNumberEntryDump[iSeqType]->GetIntNumber()+1);
         //fNumberEntryDump[iSeqType]->Layout();

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
      for (int i=0; i<CHRONO_N_BOARDS; i++)
         clock[i]=CHRONO_CLOCK_CHANNEL;

     //Save chronobox channel names
     TChronoChannelName* name = new TChronoChannelName();
     TString ChannelName;

     for (int board=0; board<CHRONO_N_BOARDS; board++)
     {
        name->SetBoardIndex(board+1);
        for (int det=0; det<MAXDET; det++)
          DetectorChans[board][det]=-1;
        for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
        {
            
            TString OdbPath="/Equipment/cbms0";
            OdbPath+=board+1;
            OdbPath+="/Channels/Channels";
            //std::cout<<runinfo->fOdb->odbReadString(OdbPath.Data(),chan)<<std::endl;
            if (runinfo->fOdb->odbReadString(OdbPath.Data(),chan))
               name->SetChannelName(runinfo->fOdb->odbReadString(OdbPath.Data(),chan),chan);
         }
         //name->Print();
         Int_t channel=name->GetChannel("CT_OR");
         if (channel>0) DetectorChans[board][0]=channel;
         detectorName[0]="CATCH_OR";

         channel=name->GetChannel("CT_AND");
         if (channel>0) DetectorChans[board][1]=channel;
         detectorName[1]="CATCH_AND";

         channel=name->GetChannel("PMT_12_AND_13");
         if (channel>0) DetectorChans[board][2]=channel;
         detectorName[2]="CT_STICK";

         channel=name->GetChannel("AT_OR");
         if (channel>0) DetectorChans[board][3]=channel;
         detectorName[3]="ATOM_OR";

         channel=name->GetChannel("AT_AND");
         if (channel>0) DetectorChans[board][4]=channel;
         detectorName[4]="ATOM_AND";

         channel=name->GetChannel("PMT_10_AND_11");
         if (channel>0) DetectorChans[board][5]=channel;
         detectorName[5]="ATOM_STICK";

         channel=name->GetChannel("SVD_TRIG");
         if (channel>0) DetectorChans[board][6]=channel;
         detectorName[6]="SVD TRIG";

         channel=name->GetChannel("SiPM_1");
         if (channel>0) DetectorChans[board][7]=channel;
         detectorName[7]="SiPM_1";

         channel=name->GetChannel("SiPM_2");
         if (channel>0) DetectorChans[board][8]=channel;
         detectorName[8]="SiPM_2";

         channel=name->GetChannel("SiPM_3");
         if (channel>0) DetectorChans[board][9]=channel;
         detectorName[9]="SiPM_3";

         channel=name->GetChannel("CAT_START_DUMP");
         if (channel>0) StartChannel[0]=channel;
         channel=name->GetChannel("CAT_STOP_DUMP");
         if (channel>0) StopChannel[0]=channel;

         channel=name->GetChannel("BL_START_DUMP");
         if (channel>0) StartChannel[1]=channel;
         channel=name->GetChannel("BL_STOP_DUMP");
         if (channel>0) StopChannel[1]=channel;

         channel=name->GetChannel("AG_START_DUMP");
         if (channel>0) StartChannel[2]=channel;
         channel=name->GetChannel("AG_STOP_DUMP");
         if (channel>0) StopChannel[2]=channel;

         channel=name->GetChannel("POS_START_DUMP");
         if (channel>0) StartChannel[3]=channel;
         channel=name->GetChannel("POS_STOP_DUMP");
         if (channel>0) StopChannel[3]=channel;

         channel=name->GetChannel("AD_TRIG");
         if (channel>0)
         {
            gADSpillChannel=channel;
            gADSpillBoard=board;
         }
         channel=name->GetChannel("POS_TRIG");
         if (channel>0 )
         {
            gPOSSpillChannel=channel;
            gPOSSpillBoard=board;
         }
         
         
      }

      gADSpillNumber=0;
      gPOSSpillNumber=0;
      
      Spill_List.clear();
      
      for (int i=0; i<MAXDET; i++)
      {
         DetectorTS[i].clear();
         DetectorCounts[i].clear();
      }
      DumpStarts=0;
      DumpStops=0;
      for (int i=0; i<NUMSEQ; i++) 
      {
         StartTime[i].clear();
         StopTime[i].clear();
         DumpMarkers[i].clear();
      }
      
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
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::EndRun, run %d\n", runinfo->fRunNo);
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
      for (int iSeq=0; iSeq<NUMSEQ; iSeq++)
      {
         std::cout<<"Seq dumps (starts and stops)"<<iSeq<<" - " << DumpMarkers[iSeq].size()<<std::endl;
         std::cout<<"Start triggers: "<< StartTime[iSeq].size();
         std::cout<< " - Stop Triggers: " << StopTime[iSeq].size()<<std::endl;
      }
      
      LogSpills();
      Spill_List.clear();
      for (int i=0; i<MAXDET; i++)
      {
         DetectorTS[i].clear();
         DetectorCounts[i].clear();
      }
      for (int i=0; i<NUMSEQ; i++) 
      {
         StartTime[i].clear();
         StopTime[i].clear();
         DumpMarkers[i].clear();
      }
      //fMainFrameGUI->CloseWindow();
      //delete app;
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
               //Show list of up-comming start dumps
               char StartStop='#';
               if (DumpFlow->DumpMarkers[iSeq].at(i).DumpType==1)
               {
                  StartStop='(';
                  DumpStarts++;
               }
               if (DumpFlow->DumpMarkers[iSeq].at(i).DumpType==2)
               {
                  StartStop=')';
                  DumpStops++;
               }
               TString msg = TString::Format("%c  %s", StartStop, DumpFlow->DumpMarkers[iSeq].at(i).Description.Data());
               fListBoxSeq[iSeq]->AddEntrySort(msg.Data(),fListBoxSeq[iSeq]->GetNumberOfEntries());
               LayoutListBox(fListBoxSeq[iSeq]);
               //Add the markers to a queue for timestamps later
               DumpMarkers[iSeq].push_back(DumpFlow->DumpMarkers[iSeq].at(i));
             }
           }
        }
      }
      else  //I am a chrono flow
      {
         if (!(ChronoFlow->ChronoBoard>0)) return flow;
         //Add start dump time stamps when they happen
         //for (int i=0; i<4; i++) // Loop over sequencers
         for (int i=0; i<NUMSEQ; i++) // Loop over sequencers
         {
            if (!(ChronoFlow->Counts[StartChannel[i]])) continue;
            std::cout <<"StartDump["<<i<<"] at "<<ChronoFlow->RunTime<<std::endl;
            StartTime[i].push_back(ChronoFlow->RunTime);
         }
         //Add stop dump time stamps when they happen
         //for (int i=0; i<4; i++)
         for (int i=0; i<NUMSEQ; i++)
         {
            if (!(ChronoFlow->Counts[StopChannel[i]])) continue;
            std::cout <<"StopDump["<<i<<"] at "<<ChronoFlow->RunTime<<std::endl;
            StopTime[i].push_back(ChronoFlow->RunTime);
            printf("PAIR THIS: %f\n",ChronoFlow->RunTime);
            //printf("START STOP PAIR!: %f - %f \n",StartTime[i].at(0),StopTime[i].at(0));
            CatchUp();
         }
         if (ChronoFlow->ChronoBoard==gADSpillBoard)
            if (ChronoFlow->Counts[gADSpillChannel])
            {
               gADSpillNumber++;
               TSpill *s = new TSpill( runinfo->fRunNo, gADSpillNumber, gTime, MAXDET );
               Spill_List.push_back(s);
               printf("AD spill\n");
               TGFont* lfont = gClient->GetFontPool()->GetFont("courier",12,kFontWeightNormal,kFontSlantItalic); 
               if (!lfont){
                  exit(123);
               }
               TString log = "";
               s->FormatADInfo(&log);
               Int_t SpillLogEntry = fListBoxLogger->GetNumberOfEntries();
               TGTextLBEntry* lbe = new TGTextLBEntry(fListBoxLogger->GetContainer(), new TGString(log.Data()), SpillLogEntry,  TGTextLBEntry::GetDefaultGC()(), lfont->GetFontStruct());
               TGLayoutHints *lhints = new TGLayoutHints(kLHintsExpandX | kLHintsTop);
               fListBoxLogger->AddEntrySort(lbe,lhints);
               //    fListBoxLogger->AddEntrySort(TGString(log.Data()),gSpillLogEntry);
               LayoutListBox(fListBoxLogger);
            }
         
         if (ChronoFlow->ChronoBoard==gPOSSpillBoard)
            if (ChronoFlow->Counts[gPOSSpillChannel])
            {
               gPOSSpillNumber++;
               TSpill *s = new TSpill( runinfo->fRunNo, gADSpillNumber, gTime, MAXDET );
               Spill_List.push_back(s);
               printf("POS spill\n");
               TGFont* lfont = gClient->GetFontPool()->GetFont("courier",12,kFontWeightNormal,kFontSlantItalic); 
               if (!lfont){
                  exit(123);
               }
               TString log = "";
               s->FormatADInfo(&log);
               Int_t SpillLogEntry = fListBoxLogger->GetNumberOfEntries();
               TGTextLBEntry* lbe = new TGTextLBEntry(fListBoxLogger->GetContainer(), new TGString(log.Data()), SpillLogEntry,  TGTextLBEntry::GetDefaultGC()(), lfont->GetFontStruct());
               TGLayoutHints *lhints = new TGLayoutHints(kLHintsExpandX | kLHintsTop);
               fListBoxLogger->AddEntrySort(lbe,lhints);
               //    fListBoxLogger->AddEntrySort(TGString(log.Data()),gSpillLogEntry);
               LayoutListBox(fListBoxLogger);
            }
         
         //Fill detector array
         for (int i=0; i<MAXDET; i++)
         {
            for (int j=0; j<CHRONO_N_BOARDS; j++)
            {
               if (DetectorChans[j][i]<0) continue;
               if (!(ChronoFlow->Counts[DetectorChans[j][i]])) continue;
               DetectorTS[i].push_back(ChronoFlow->RunTime);
               DetectorCounts[i].push_back(ChronoFlow->Counts[DetectorChans[j][i]]);
            }
         }
      }

      if (me->serial_number % 1000 == 0 ) //Periodically draw spills
      {
         //std::cout <<"Catchup"<<std::endl;
         CatchUp();
      }
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
        gEnv->SetValue("Gui.DefaultFont","-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");


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
