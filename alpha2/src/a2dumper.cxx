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
#include "A2Flow.h"
#include "TTree.h"

#include "TA2MVADumper.h"

class TA2DumperFlags
{
    public:
        std::string fFileName;
        std::string fTreeName;
};

class TA2Dumper: public TARunObject
{
public:
    //List of event IDs we want to save.
    std::vector<std::pair<int,int>> fEventIDs; 
    //The location of saving said events.
    TTree *fTree;
    TFile *fRootFile;

    //The data we want to save is all in this object.
    std::vector<TA2MVADumper*> fMVADumpers;

    //Flags & counters.
    TA2DumperFlags* fFlags;
    int fCurrentEventNumber;
    int fCurrentEventIndex = 0;
    bool fEventsLoaded = false;
    bool fEventsSorted = false;

    TA2Dumper(TARunInfo* runInfo, TA2DumperFlags* flags) : TARunObject(runInfo)
    {
        //printf("ctor, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
        fFlags = flags;
        fModuleName="a2dumper";
        
        //Setup the filename for the root file.
        std::string fileName = "dumperoutput";
        fileName += std::to_string(runInfo->fRunNo);
        fileName += ".root";
        fRootFile = TFile::Open(fileName.c_str(),"RECREATE");
        
        //Set up the new tree using name as input.
        if(fFlags->fTreeName == "mixing" || fFlags->fTreeName == "cosmic")
            fTree = new TTree(fFlags->fTreeName.c_str(),"data from siliconEvent");
        else
            fTree = new TTree(fFlags->fTreeName.c_str(),"data from siliconEvent");
        fMVADumpers.push_back( new TA2MVAEventIDDumper(fTree) );
        fMVADumpers.push_back( new TA2MVAClassicDumper(fTree) );
        fMVADumpers.push_back( new TA2MVAXYZ(fTree) );
    }

    ~TA2Dumper()
    {
        //printf("dtor!\n");
    }
    
    void BeginRun(TARunInfo* runInfo)
    {
        fCurrentEventIndex = 0;
        fCurrentEventNumber = -1;
        printf("BeginRun, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
        LoadEventIDs(); //Load event IDs we want to dump
        // Initialise the current event number to the first in the list.
        for ( ; fCurrentEventIndex < fEventIDs.size(); fCurrentEventIndex++)
        {
            if (fEventIDs[fCurrentEventIndex].first == runInfo->fRunNo)
            {
               fCurrentEventNumber = fEventIDs[fCurrentEventIndex].second;
               break;
            }
        }

    }

    void EndRun(TARunInfo* runInfo)
    {
        printf("EndRun, run %d\n", runInfo->fRunNo);

        //Write the tree and file, then close file.
        fTree->Write();
        fRootFile->Write();
        fRootFile->Close();
    }

    //Presently this module doesn't need to touch TMEvent data
    //TAFlowEvent* Analyze(TARunInfo* runInfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
    //{
    //   START_TIMER;
    //    flow = new UserProfilerFlow(flow,"custom profiling",timer_start);
    //    return flow;
    //}

    TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runInfo, TAFlags* flags, TAFlowEvent* flow)
    {
        SilEventFlow* fe=flow->Find<SilEventFlow>();
        if (!fe)
        {
            return flow;
        }
        TAlphaEvent* alphaEvent=fe->alphaevent;
        TSiliconEvent* siliconEvent=fe->silevent;

        //Update variables of the MVA class. This will check whether the eventnumbers match.
        //If they match this function will return true, if not: false. This allows us to know
        //whether to fill the tree and increment the EventIndex and EventNumber.
        int good_flow = 0;
        while (true)
        {
            if (fEventIDs[fCurrentEventIndex].first == runInfo->fRunNo)
                break;
            else
                fCurrentEventIndex++;
            if ( fCurrentEventIndex >= fEventIDs.size() )
                return flow;
        }
        for ( TA2MVADumper* d: fMVADumpers)
        {
            if(d->UpdateVariables(siliconEvent, alphaEvent, fCurrentEventNumber))
            {
                good_flow++;
                //Fill tree.
                
            }
        }
        if (good_flow)
        {
           assert (good_flow == fMVADumpers.size());
           //Update current event number to be checked against (remember everything here is in order).
           fCurrentEventIndex++;
           fCurrentEventNumber = fEventIDs[fCurrentEventIndex].second;            
           fTree->Fill();
        }
        return flow;
    }

    void AnalyzeSpecialEvent(TARunInfo* runInfo, TMEvent* event)
    {
        printf("a2dump::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runInfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
    }

    void LoadEventIDs()
    {
        std::ifstream myFile(fFlags->fFileName);
        std::string line;
        if (myFile.is_open())
        {
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
                    //std::cout <<"Queue "<< firstEvent << std::endl;
                    fEventIDs.push_back( std::pair<int, int>(runNumber, firstEvent) );
                    firstEvent++;
                }
            }
            myFile.close();
        }

        fEventsLoaded = true;
        SortEvenIDs();
    }

    void SortEvenIDs()
    {
        //Sort based on first (tmin) keeping idx in the same location
        std::sort(std::begin(fEventIDs), std::end(fEventIDs), 
        [&](const std::pair<int,int>& lhs, const std::pair<int,int>& rhs)
        {
            if(lhs.first == rhs.first)
                return lhs.second < rhs.second;
            else
                return lhs.first < rhs.first;
        } );
        fEventsSorted = true;
        //for (const std::pair<int,int>& a: fEventIDs)
        //   std::cout<< "\t"<< a.first<<"-"<<a.second<<std::endl;
    }

    void PrintEventIDs()
    {
        for(int i=0;i<fEventIDs.size();i++)
        {
            std::cout << "EventID[" << i << "] = (" << fEventIDs[i].first << ", " << fEventIDs[i].second << ")" << std::endl;
        }
    }
};

class TA2DumperFactory: public TAFactory
{
public:
    TA2DumperFlags fFlags;
   void Usage()
   {
      printf("TA2DumperFactory::Help!\n");
      printf("\t--eventlist filename\tSave events from TA2Plot.WriteEventList() function\n");
      printf("\t--datalabel label\tValid data labels currently: mixing and cosmic\n");
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("Init!\n");
      for (unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--eventlist")
               fFlags.fFileName = args[i+1];
            if( args[i] == "--datalabel")
               fFlags.fTreeName = args[i+1];
         }
   }
   
   void Finish()
   {
      printf("Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runInfo)
   {
      printf("NewRunObject, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
      return new TA2Dumper(runInfo, &fFlags);
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
