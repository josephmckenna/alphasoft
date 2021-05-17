//
// A2 Dumper Module for taking events and saving to super trees.
//
// L.Golino
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

class TA2DumperFlags
{
    public:
        std::string fFileName;
};

class TA2Dumper: public TARunObject
{
public:
    int fRunEventCounter;
    std::vector<std::pair<int,int>> fEventID;
    std::string fFileName;

    //Flags
    bool fEventsLoaded = false;
    bool fEventsSorted = false;

    TA2Dumper(TARunInfo* runInfo, std::string flagFileName) : TARunObject(runInfo), fFileName(flagFileName)
    {
        ModuleName="a2dumper";
        printf("ctor, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
        fRunEventCounter = 0;
    }

    ~TA2Dumper()
    {
        printf("dtor!\n");
    }
    
    void BeginRun(TARunInfo* runInfo)
    {
        printf("BeginRun, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
        fRunEventCounter = 0;
        //TODO Take runNumber as input and filter out all but current run.
        LoadEventIDs();
    }

    void EndRun(TARunInfo* runInfo)
    {
        printf("EndRun, run %d\n", runInfo->fRunNo);
        printf("Counted %d events in run %d\n", fRunEventCounter, runInfo->fRunNo);
    }

    void NextSubrun(TARunInfo* runInfo)
    {
        printf("NextSubrun, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
    }

    void PauseRun(TARunInfo* runInfo)
    {
        printf("PauseRun, run %d\n", runInfo->fRunNo);
    }

    void ResumeRun(TARunInfo* runInfo)
    {
        printf("ResumeRun, run %d\n", runInfo->fRunNo);
    }

    TAFlowEvent* Analyze(TARunInfo* runInfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
    {
        //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runInfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
        //event->old_event.SetBankList();
        //event->old_event.Print();
        START_TIMER;
        fRunEventCounter++;
        flow = new UserProfilerFlow(flow,"custom profiling",timer_start);
        return flow;
    }

    void AnalyzeSpecialEvent(TARunInfo* runInfo, TMEvent* event)
    {
        printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runInfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
    }

    void LoadEventIDs()
    {
        //TODO Take runNumber as input and filter out all but current run. 0 = all
        //std::ofstream myfile (fileName);
        //Hardcoded overwrite of the fileName!! This is obviously illegal, but a little confised how to bring an input from aa.exe into this module, or whether I should be creating a new .exe
        fFileName = "testWithOutOfOrder.list";
        std::ifstream myFile(fFileName);
        std::string line;
        std::cout << "LMG - A2DUMPERMODULE: " << fFileName << std::endl;
        if (myFile.is_open())
        {
            std::cout << "LMG - A2DUMPERMODULE: FILE OPEN" << std::endl;
            while ( std::getline(myFile, line) )
            {
                std::cout << line << std::endl;
                int runNumber, firstEvent, lastEvent;
                //The following is apparantly best method: https://quick-bench.com/q/CWCbHcvWTZBXydPA_mju2r75LX0
                if (3 == std::sscanf(line.c_str(), "%d:%d-%d", &runNumber, &firstEvent, &lastEvent))
                {
                    std::cout << "runNumber=" << runNumber << std::endl;
                    std::cout << "firstEvent=" << firstEvent << std::endl;
                    std::cout << "lastEvent=" << lastEvent << std::endl;
                }
                while(firstEvent <= lastEvent)
                {
                    fEventID.push_back( std::pair<int, int>(runNumber, firstEvent) );
                    std::cout << "LMG - A2DUMPERMODULE: Pushing back: (" << runNumber << ", " << firstEvent << ")" << std::endl;
                    firstEvent++;
                }
            }
            myFile.close();
        }
        fEventsLoaded = true;
        PrintEventIDs();
        SortEvenIDs();
        PrintEventIDs();
    }

    void SortEvenIDs()
    {
        //Sort based on first (tmin) keeping idx in the same location
        std::sort(std::begin(fEventID), std::end(fEventID), 
        [&](const std::pair<double,double>& lhs, const std::pair<double,double>& rhs)
        {
            if(lhs.first == rhs.first)
                return lhs.second < rhs.second;
            else
                return lhs.first < rhs.first;
        } );
        fEventsSorted = true;
    }

    void PrintEventIDs()
    {
        for(int i=0;i<fEventID.size();i++)
        {
            std::cout << "EventID[" << i << "] = (" << fEventID[i].first << ", " << fEventID[i].second << ")" << std::endl;
        }
    }

    void SaveToTrees()
    {
        if(fEventsLoaded && fEventsSorted)
        {
            //Save basically the whole SVD_QOD to .root.
            //Scan run number and eventID and then save it to a root. 
            //I guess we want all these in the same .root and not individuals...?
        }

    }
};

class TA2DumperFactory: public TAFactory
{
public:
    TA2DumperFlags fFlags;
   void Usage()
   {
      printf("\tExample TAFactory Usage!\n");
      printf("\tPrint valid arguements for this modules here!");
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("Init!\n");
      printf("Arguments:\n");
      for (unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--eventlist")
               fFlags.fFileName = args[i+1];
         }
   }
   
   void Finish()
   {
      printf("Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runInfo)
   {
      printf("NewRunObject, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
      return new TA2Dumper(runInfo, fFlags.fFileName);
   }
};

static TARegister tar(new TA2DumperFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
