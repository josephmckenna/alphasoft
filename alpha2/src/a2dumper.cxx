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

class a2dumperFlags
{
    public:
        std::string fileName;
};

class a2dumper: public TARunObject
{
public:
    int fRunEventCounter;
    std::vector<std::pair<int,int>> EventID;
    std::string fileName;

    a2dumper(TARunInfo* runinfo, std::string flagfilename) : TARunObject(runinfo), fileName(flagfilename)
    {
        ModuleName="a2dumper";
        printf("ctor, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
        fRunEventCounter = 0;
    }

    ~a2dumper()
    {
        printf("dtor!\n");
    }
    
    void BeginRun(TARunInfo* runinfo)
    {
        printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
        fRunEventCounter = 0;
    }

    void EndRun(TARunInfo* runinfo)
    {
        printf("EndRun, run %d\n", runinfo->fRunNo);
        printf("Counted %d events in run %d\n", fRunEventCounter, runinfo->fRunNo);
    }

    void NextSubrun(TARunInfo* runinfo)
    {
        printf("NextSubrun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
    }

    void PauseRun(TARunInfo* runinfo)
    {
        printf("PauseRun, run %d\n", runinfo->fRunNo);
    }

    void ResumeRun(TARunInfo* runinfo)
    {
        printf("ResumeRun, run %d\n", runinfo->fRunNo);
    }

    TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
    {
        //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
        //event->old_event.SetBankList();
        //event->old_event.Print();
        START_TIMER;
        fRunEventCounter++;
        flow = new UserProfilerFlow(flow,"custom profiling",timer_start);
        return flow;
    }

    void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
    {
        printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
        //std::ofstream myfile (fileName);
        //Hardcoded overwrite of the fileName!! This is obviously illegal, but a little confised how to bring an input from aa.exe into this module, or whether I should be creating a new .exe
        fileName = "basicsumWrittentoList.list";
        std::ifstream myfile(fileName);
        std::string line;
        std::cout << "LMG - A2DUMPERMODULE: " << fileName << std::endl;
        if (myfile.is_open())
        {
            std::cout << "LMG - A2DUMPERMODULE: FILE OPEN" << std::endl;
            while ( std::getline(myfile, line) )
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
                    EventID.push_back( std::pair<int, int>(runNumber, firstEvent) );
                    std::cout << "LMG - A2DUMPERMODULE: Pushing back: (" << runNumber << ", " << firstEvent << ")" << std::endl;
                    firstEvent++;
                }
            }
            myfile.close();
        }
    }
};

class a2dumperFactory: public TAFactory
{
public:
    a2dumperFlags fFlags;
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
            if( args[i] != "")
               fFlags.fileName = args[i];
         }
   }
   
   void Finish()
   {
      printf("Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new a2dumper(runinfo, fFlags.fileName);
   }
};

static TARegister tar(new a2dumperFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
