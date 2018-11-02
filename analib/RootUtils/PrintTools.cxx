#include "PrintTools.h"

#include <fstream> 

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

void PrintChronoNames(int runNumber)
{
   TString Names[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
   for (int boards=0; boards<CHRONO_N_BOARDS; boards++)
   {
      for (int chans=0; chans<CHRONO_N_CHANNELS; chans++)
      {
         Names[boards][chans]=Get_Chrono_Name(runNumber, boards, chans);
      }
   }
   std::cout<<"Name\tBoard\tChannel"<<std::endl;
   for (int boards=0; boards<CHRONO_N_BOARDS; boards++)
   {
      for (int chans=0; chans<CHRONO_N_CHANNELS; chans++)
      {
         std::cout<<Names[boards][chans]<<"\t"
                  <<boards<<"\t"
                  <<chans<<std::endl;
      }
   }
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


Int_t PrintTPCEvents(Int_t runNumber, Double_t tmin, Double_t tmax)
{
   if (tmax<0.) tmax=GetTotalRunTime(runNumber);
   double official_time;
   
   TStoreEvent *store_event = new TStoreEvent();
   TTree *t0 = Get_StoreEvent_Tree(runNumber, official_time);
   t0->SetBranchAddress("StoredEvent", &store_event);
   //SPEED THIS UP BY PREPARING FIRST ENTRY!
   store_event->Print("title");
   std::cout<<"OfficialTime"<<std::endl;
   for (Int_t i = 0; i < t0->GetEntries(); ++i)
   {
      t0->GetEntry(i);
      //
      if (!store_event)
      {
         std::cout<<"NULL TStore event: Probably more OfficialTimeStamps than events"<<std::endl;
         break;
      }
      if (official_time <= tmin)
      {
         store_event->Reset();
         continue;
      }
      
      if (official_time > tmax)
      {
         break;
      }
      
      //std::cout<<"Official Time of event "<<i<<", OfficialTime (RunTime):"<<official_time << "("<<store_event->GetTimeOfEvent()<<")"<<std::endl;
      store_event->Print("line");
      std::cout<<official_time<<std::endl;
   
   }
   return 0;
}


Int_t PrintTPCEvents(Int_t runNumber,  const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
   Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
   return PrintTPCEvents(runNumber,tmin,tmax);
}

Int_t PrintSequenceQOD(Int_t runNumber)
{
//I AM MISSING SEQUENCE HEADER INFORMATION!


   Bool_t gProgressBars=false;
   //cout << GetSequenceString(runNumber)<<endl<<endl;
/*
   Int_t ErrorCount=0;
   Int_t ErrorCounter=0;
   //Set Time Cuts on Dumps here (Mixing, FRD and Quench)
   Double_t FRDTCut=5.;
   Double_t QuenchTCut=1.;
   Double_t MixingTCut=10.;
*/
   //Setup output log file (nicely formated output for elog)
   TString LogFileName=MakeAutoPlotsFolder("");
   TString CSVFileName=MakeAutoPlotsFolder("");
   LogFileName+="/R";
   CSVFileName+="/R";
   LogFileName+=runNumber;
   CSVFileName+=runNumber;
   LogFileName+="_SequenceQOD.log";
   CSVFileName+="_SequenceQOD.csv";
   std::ofstream LogFile (LogFileName, std::ofstream::out);
   std::ofstream CSVFile (CSVFileName, std::ofstream::out);

   LogFile << std::endl;
   //LogFile << GetSequenceQueue(runNumber,"cat")<< endl;;
   //LogFile << GetSequenceQueue(runNumber,"rct")<< endl;;
   //LogFile << GetSequenceQueue(runNumber,"atm")<< endl;;
   //LogFile << GetSequenceQueue(runNumber,"pos")<< endl;;

   LogFile << std::endl;
   //LogFile << GetSequenceQueueFull(runNumber, "cat");
   //LogFile << GetSequenceQueueFull(runNumber, "rct");
   //LogFile <<  GetSequenceQueueFull(runNumber, "atm");
   //LogFile <<  GetSequenceQueueFull(runNumber, "pos");

   LogFile<<std::endl<<std::endl;
   //=================================
   //General rules for this funcition:
   //=================================
   //Start / stop dumps must match pairs
   //Quench and FRD dumps must match QUENCH_FLAG SIS trigger
   //Mixing dump must match MIXING_FLAG SIS trigger
   //Quench and FRD lengths must be less than the hard coded settings above

   //Set all flag boolians to false
/*   Bool_t HasFRDDump=kFALSE;
   Bool_t HasQuenchDump=kFALSE;
   Bool_t HasMixingDump=kFALSE;
*/
   Bool_t HasQuenchFlag=kFALSE;
   Bool_t HasMixingFlag=kFALSE;
   Bool_t HasLaserTrigs=kFALSE;

   Bool_t InvalidTimeStamp=kFALSE;

   TTree* sequencerTree = Get_Seq_Event_Tree(runNumber);
   TSeq_Event* seqEvent = new TSeq_Event();
   sequencerTree->SetBranchAddress("SequencerEvent", &seqEvent);
   //  sprintf(search_string, "\"%s", description); // add a " before the name
   Double_t runTimes[100000];
   TString Names[100000];
   TString Descriptions[100000];
   TString Sequencer[100000];
   Int_t NStarts=0;
   Int_t NStops=0;
   Int_t DumpCount=0;
   Int_t sequencerEntries=sequencerTree->GetEntries();
   Int_t MissingTimeStamps=0;
   Int_t StartTriggers=0;
   Int_t StopTriggers=0;
   if (sequencerEntries>0)
     std::cout << "\nRun "<< runNumber <<" has sequencer dumps!" << std::endl;
   for (Int_t i=0; i<sequencerEntries; i++)
   {
      if (gProgressBars)
      {
         std::cout << "Entry: " << i << " of " << sequencerEntries << std::endl;
      }
      sequencerTree->GetEntry(i);
      Descriptions[i]= seqEvent->GetDescription();
      Sequencer[i]=seqEvent->GetSeq();
      Names[i] = seqEvent->GetEventName();
      std::cout << Descriptions[i] << "\t" << Names[i] << "\t SeqEvent ID: " << seqEvent->GetID() << std::endl;
      runTimes[i] = GetRunTimeOfEvent(runNumber, seqEvent, 0); //Turn off redundant timecheck for speed here
      //Get_RunTime_of_SequencerEvent(runNumber, seqEvent);
      std::cout << "\t" << runTimes[i] <<std::endl;
      if (runTimes[i]<0.)
      {
         InvalidTimeStamp=kTRUE;
         MissingTimeStamps++;
      }
      DumpCount++;
      if (Names[i]=="startDump") NStarts++;
      else if (Names[i]=="stopDump") NStops++;
      //else cout << Names[i] << endl;
   }
   delete seqEvent;
   std::cout<<"\n";

   for (Int_t FlagChans=0; FlagChans<9; FlagChans++)
   {
      const char* CHRONO_FLAG_NAME="";
      //List SIS channels of interest here
      if (FlagChans==0)      CHRONO_FLAG_NAME="FRD_FLAG";
      else if (FlagChans==1) CHRONO_FLAG_NAME="MIXING_FLAG";
      else if (FlagChans==2) CHRONO_FLAG_NAME="SEQLASERPULSE";
      else if (FlagChans==3) CHRONO_FLAG_NAME="LASER_SHUTTER_OPEN";
      else if (FlagChans==4) CHRONO_FLAG_NAME="LASER_SHUTTER_CLOSE";
      else if (FlagChans==5) CHRONO_FLAG_NAME="MIC_SYNTH_STEP_START";
      else if (FlagChans==6) CHRONO_FLAG_NAME="MIC_SYNTH_STEP_STOP";
      else if (FlagChans==7) CHRONO_FLAG_NAME="AD_TRIG";
      else continue;
      //if (FlagChans>=9) SIS_FLAG_NAME="SIS_DIX_ALPHA";
      //if (FlagChans>=10) SIS_FLAG_NAME="SIS_DIX_WALPHA";
      //    if (FlagChans>=9) continue;

      if (gProgressBars) {
	printf("                                           \r"); fflush(stdout); //clean up progress bar
      }
      if ( !ChronoboxesHaveChannel(runNumber, CHRONO_FLAG_NAME) )
         continue;
      std::cout << "Setting up " << CHRONO_FLAG_NAME << std::endl;
      double ot;
      TTree* trigger_tree =  Get_Chrono_Tree( runNumber, CHRONO_FLAG_NAME, ot );

      if( trigger_tree == NULL )
         continue; //Error state?
      Int_t Entries=trigger_tree->GetEntries();
      std::cout << Entries << " Entries found in "<< trigger_tree->GetName() << std::endl;
      for (Int_t i=0; i<Entries; i++)
      {
         if (gProgressBars)
         {
            if ( ( i  ) % 1000 == 0 )  {printf("Counting Entries: %.1f %% \r",100.*((double)i)/Entries); fflush(stdout); }
         }
         if (FlagChans==0) HasQuenchFlag=kTRUE;
         if (FlagChans==1) HasMixingFlag=kTRUE;
         if (FlagChans==2) HasLaserTrigs=kTRUE;
         Names[DumpCount]="CHRONO_FLAG";
         Descriptions[DumpCount]=TString(CHRONO_FLAG_NAME);
         Sequencer[DumpCount]="CHRONO_";
         Sequencer[DumpCount]+=99;
         runTimes[DumpCount]= GetRunTimeOfCount(runNumber,CHRONO_FLAG_NAME, i+1);
         DumpCount++;
      }
   }
   //Sort dumps chronoligically (starts should ALWAYS come before stops)
   TString tempName;
   TString tempDescription;
   TString tempSequencer;
   Double_t tempTime;
   for (Int_t i=0; i< DumpCount; i++)
   {
      for (Int_t j=0; j<DumpCount-1; j++)
      {
         if (runTimes[j]> runTimes[j+1]  && runTimes[j+1]>0. )
         {
            tempTime=runTimes[j+1];
            runTimes[j+1] = runTimes[j];
            runTimes[j] = tempTime;

            tempName=Names[j+1];
            Names[j+1] = Names[j];
            Names[j] = tempName;

            tempDescription=Descriptions[j+1];
            Descriptions[j+1] = Descriptions[j];
            Descriptions[j] = tempDescription;

            tempSequencer=Sequencer[j+1];
            Sequencer[j+1]=Sequencer[j];
            Sequencer[j]=tempSequencer;
         }
      }
   }
   CSVFile<<"Run Number, Dump Name, Dump Description, Time,"<<'\n';
   // SIS channel, SIS channel name ?
   for (Int_t i=0; i< DumpCount; i++) // Save All dumps to CSV
   {
      CSVFile<<runNumber<<","<<Names[i]<<","<<Descriptions[i]<<","<<std::setprecision(20)<<runTimes[i]<<'\n';
   }
   CSVFile.close();

  //Print Pairs of dumps:
  Int_t PrintedStarts=0;
  Int_t PrintedStops=0;
  Int_t shutterOpens=0;
  Int_t shutterCloses=0;
  std::cout<<"\n";
 
  Int_t nChannels=6;
  Int_t* channels[CHRONO_N_CHANNELS*CHRONO_N_BOARDS]; 
  Int_t* boards[CHRONO_N_CHANNELS*CHRONO_N_BOARDS];
  Int_t chan=0;
  
   for (Int_t i=0; i<nChannels; i++)
   {
      channels[i]=new Int_t(-1);
      boards[i]=new Int_t(-1);
   }
   for (Int_t board=0; board<CHRONO_N_BOARDS; board++)
     {
      chan=Get_Chrono_Channel(runNumber,board,"CATCH_OR");
      if (chan>-1)
      {
        *channels[0]=chan;
        *boards[0]=board;
      }
      // chan=Get_Chrono_Channel(runNumber,board,"CATCH_AND");
      // if (chan>-1)
      // {
      //   *channels[1]=chan;
      //   *boards[1]=board;
      // }
      // chan=Get_Chrono_Channel(runNumber,board,"SiPM_A");
      // if (chan>-1)
      // {
      //   *channels[2]=chan;
      //   *boards[2]=board;
      // }
      // chan=Get_Chrono_Channel(runNumber,board,"SiPM_C");
      // if (chan>-1)
      // {
      //   *channels[3]=chan;
      //   *boards[3]=board;
      // }
      // chan=Get_Chrono_Channel(runNumber,board,"SiPM_D");
      // if (chan>-1)
      // {
      //   *channels[4]=chan;
      //   *boards[4]=board;
      // }
      // chan=Get_Chrono_Channel(runNumber,board,"SiPM_F");
      // if (chan>-1)
      // {
      //   *channels[5]=chan;
      //   *boards[5]=board;
      // }
      chan=Get_Chrono_Channel(runNumber,board,"TPC_TRIG");
      if (chan>-1)
      {
        *channels[1]=chan;
        *boards[1]=board;
      }
      chan=Get_Chrono_Channel(runNumber,board,"SiPM_B");
      if (chan>-1)
      {
        *channels[2]=chan;
        *boards[2]=board;
      }
      chan=Get_Chrono_Channel(runNumber,board,"SiPM_E");
      if (chan>-1)
      {
        *channels[3]=chan;
        *boards[3]=board;
      }
      chan=Get_Chrono_Channel(runNumber,board,"SiPM_A_AND_D");
      if (chan>-1)
      {
        *channels[4]=chan;
        *boards[4]=board;
      }
      chan=Get_Chrono_Channel(runNumber,board,"SiPM_C_AND_F");
      if (chan>-1)
      {
        *channels[5]=chan;
        *boards[5]=board;
      }
     }
   
   if ( ChronoboxesHaveChannel(runNumber,"CAT_START_DUMP"))
      StartTriggers+=GetCountsInChannel(runNumber,"CAT_START_DUMP");
   if ( ChronoboxesHaveChannel(runNumber,"BL_START_DUMP")) 
      StartTriggers+=GetCountsInChannel(runNumber,"BL_START_DUMP");
   if ( ChronoboxesHaveChannel(runNumber,"AG_START_DUMP"))
      StartTriggers+=GetCountsInChannel(runNumber,"AG_START_DUMP");
   if ( ChronoboxesHaveChannel(runNumber,"POS_START_DUMP"))
      StartTriggers+=GetCountsInChannel(runNumber,"POS_START_DUMP");

   if ( ChronoboxesHaveChannel(runNumber,"CAT_STOP_DUMP"))
      StopTriggers+=GetCountsInChannel(runNumber,"CAT_STOP_DUMP");
   if ( ChronoboxesHaveChannel(runNumber,"BL_STOP_DUMP")) 
      StopTriggers+=GetCountsInChannel(runNumber,"BL_STOP_DUMP");
   if ( ChronoboxesHaveChannel(runNumber,"AG_STOP_DUMP"))
      StopTriggers+=GetCountsInChannel(runNumber,"AG_STOP_DUMP");
   if ( ChronoboxesHaveChannel(runNumber,"POS_STOP_DUMP"))
      StopTriggers+=GetCountsInChannel(runNumber,"POS_STOP_DUMP");
    
   std::cout<<"\nStart Triggers: "<<StartTriggers<<"\tStop Triggers: "<<StopTriggers<<"\n"<<std::endl;

   std::cout << std::setw(25) << "Dump name" << "\t Start (s) \t Stop (s) \t Duration (s) \t"<<SequenceQODDetectorLine(-1,-1,-1,boards,channels,nChannels) <<"\n";
   LogFile << "===================="<<"\n";
   LogFile << "Dump and CB trigger table: " <<"\n";
   LogFile << "===================="<<"\n";
   LogFile << std::setw(25) << "Dump name" << "\t Start (s) \t Stop (s) \t Duration (s) \t"<<SequenceQODDetectorLine(-1,-1,-1,boards,channels,nChannels)<<"\n";
   for (Int_t i=0; i< DumpCount; i++)
   {
      //Pair start and Stop dump makers in table
      if (strcmp(Names[i],"startDump")==0 )
      {
         std::cout <<std::setw(25)  << Descriptions[i] << " \t " << std::setw(10)<< runTimes[i];
         LogFile <<std::setw(25)  << Descriptions[i] << " \t "<< std::setw(10) << runTimes[i];
         PrintedStarts++;
         //Turn off repetition counting in SequenceQOD printing... just find the first matching stop after the start
         /*Int_t repetition=0;
         for (Int_t j=0; j<= i; j++)
         {
            if (Names[j]=="startDump" && strcmp(Descriptions[i],Descriptions[j])==0) repetition++;
         }
         for (Int_t j=0; j< DumpCount; j++)
         */
         for (Int_t j=i; j< DumpCount; j++)
         {
            if (strcmp(Names[j],"stopDump")==0 && strcmp(Descriptions[i],Descriptions[j])==0) // {repetition--; }
            //if (Names[j]=="stopDump" && Descriptions[i]==Descriptions[j])
            {
               TString DetectorData=SequenceQODDetectorLine(runNumber,runTimes[i],runTimes[j],boards,channels,nChannels);
               std::cout << " \t " << std::setw(10)<<runTimes[j] << " \t " <<std::setw(10)<< runTimes[j]-runTimes[i]<<DetectorData <<std::endl;
               LogFile << " \t " << std::setw(10)<<runTimes[j] << " \t "<<std::setw(10) << runTimes[j]-runTimes[i]<<DetectorData <<"\n";
               PrintedStops++;
               break;
            }
         }
         continue;
         //cout << "Name \t"<< Names[i] << endl;
         //cout << "Description \t"<< Descriptions[i] << endl;
         //cout << "Time \t"<< runTimes[i] << endl;
      }
      //Pair Laser shutter open and Laser shutter close flags in table
      if (Names[i]=="SIS_FLAG" && Descriptions[i]=="LASER_SHUTTER_OPEN")
      {
         std::cout <<std::setw(25)  << Descriptions[i] << " \t " << runTimes[i];
         LogFile <<std::setw(25)  << Descriptions[i] << " \t " << runTimes[i];
         shutterOpens++;
         Int_t repetition=0;
         for (Int_t j=0; j<= i; j++)
         {
            if (Names[j]=="SIS_FLAG" && strcmp(Descriptions[i],Descriptions[j])==0) repetition++;
         }
         for (Int_t j=0; j< DumpCount; j++)
         {
            if (Names[j]=="SIS_FLAG" && Descriptions[j]=="LASER_SHUTTER_CLOSE")  { repetition--; }
            if (repetition==0 && Names[j]=="SIS_FLAG" && Descriptions[j]=="LASER_SHUTTER_CLOSE")
            {
               std::cout << " \t " << runTimes[j] << " \t " << runTimes[j]-runTimes[i]<< std::endl;
               LogFile << " \t " << runTimes[j] << " \t " << runTimes[j]-runTimes[i]<< std::endl;
               shutterCloses++;
               break;
            }
         }
      }
      else if (Names[i]=="SIS_FLAG" && Descriptions[i]=="MIC_SYNTH_STEP_START")
      {
         std::cout <<std::setw(25)  << Descriptions[i] << " \t " << runTimes[i];
         LogFile <<std::setw(25)  << Descriptions[i] << " \t " << runTimes[i];
         shutterOpens++;
         Int_t repetition=0;
         for (Int_t j=0; j<= i; j++)
         {
            if (Names[j]=="SIS_FLAG" && strcmp(Descriptions[i],Descriptions[j])==0) repetition++;
         }
         for (Int_t j=0; j< DumpCount; j++)
         {
            if (Names[j]=="SIS_FLAG" && Descriptions[j]=="MIC_SYNTH_STEP_STOP")  { repetition--; }
            if (repetition==0 && Names[j]=="SIS_FLAG" && Descriptions[j]=="MIC_SYNTH_STEP_STOP")
            {
               std::cout << " \t " << runTimes[j] << " \t " << runTimes[j]-runTimes[i]<< std::endl;
               LogFile << " \t " << runTimes[j] << " \t " << runTimes[j]-runTimes[i]<< std::endl;
               shutterCloses++;
               break;
            }
         }
      }
      //Catch all other SIS_FLAGS
      else 
      {
         if (Names[i]=="SIS_FLAG" && Descriptions[i]!="LASER_SHUTTER_CLOSE" && Descriptions[i]!="MIC_SYNTH_STEP_STOP")
         {
            std::cout << std::setw(25) << Descriptions[i]<< " \t " << runTimes[i] << std::endl;
            LogFile << std::setw(25) << Descriptions[i]<< " \t " << runTimes[i] << std::endl;
         }
      }
   }
   //Missing code here?
   
   if (HasQuenchFlag) std::cout <<"HasQuenchFlag"<<std::endl;
   if (HasMixingFlag) std::cout <<"HasMixingFlag"<<std::endl;
   if (HasLaserTrigs) std::cout <<"HasLaserTrigs"<<std::endl;
   if (InvalidTimeStamp) std::cout <<"Has InvalidTimeStamp"<<std::endl;

   return 99;
}
