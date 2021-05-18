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
    //The data from each event we want to save.
    Double_t fX;
    Double_t fY; 
    Double_t fZ; 
    Double_t fR;
    Double_t fPhi;
    Int_t fNumTracks;

    //Flags & counters.
    TA2DumperFlags* fFlags;
    int fCurrentEventNumber;
    int fCurrentEventIndex = 0;
    bool fEventsLoaded = false;
    bool fEventsSorted = false;

    TA2Dumper(TARunInfo* runInfo, TA2DumperFlags* flags) : TARunObject(runInfo)
    {
        printf("ctor, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
        fFlags = flags;
        ModuleName="a2dumper";
        
        //Setup the filename for the root file.
        std::string fileName = "dumperoutput";
        fileName += std::to_string(runInfo->fRunNo);
        fileName += ".root";
        fRootFile = TFile::Open(fileName.c_str(),"RECREATE");
        
        //Set up the new tree using name as input.
        if(fFlags->fTreeName == "mixing" || fFlags->fTreeName == "cosmic")
            fTree = new TTree(fFlags->fTreeName.c_str(),"data from siliconEvent");
        else
            fTree = new TTree("UnnamedTree","data from siliconEvent");
        
        //Branch the tree based on our members.
        fTree->Branch("x", &fX, "x/D");
        fTree->Branch("y", &fY, "y/D");
        fTree->Branch("z", &fZ, "z/D");
        fTree->Branch("r", &fR, "r/D");
        fTree->Branch("phi", &fPhi, "phi/D");
        fTree->Branch("numTracks", &fNumTracks, "numTracks/I");
    }

    ~TA2Dumper()
    {
        printf("dtor!\n");
    }
    
    void BeginRun(TARunInfo* runInfo)
    {
        printf("BeginRun, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
        
        
        LoadEventIDs(); //Load event IDs we want to dump
        fCurrentEventNumber = fEventIDs[fCurrentEventIndex].second; //Initialise the current event number to the first in the list.
        
        
    }

    void EndRun(TARunInfo* runInfo)
    {
        printf("EndRun, run %d\n", runInfo->fRunNo);

        //Write the tree and file, then close file.
        fTree->Write();
        fRootFile->Write();
        fRootFile->Close();
    }

    TAFlowEvent* Analyze(TARunInfo* runInfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
    {
        START_TIMER;
        flow = new UserProfilerFlow(flow,"custom profiling",timer_start);
        return flow;
    }

    TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runInfo, TAFlags* flags, TAFlowEvent* flow)
    {
        if(flow) 
        {
            //TAFlowEvent* flowEvent = flow; //Not actually needed maybe?
            SilEventFlow* siliconEventFlow=flow->Find<SilEventFlow>();
            if (siliconEventFlow)
            {
                //TAlphaEvent* alphaevent=siliconEventFlow->alphaevent; //Also not needed?
                TSiliconEvent* siliconEvent=siliconEventFlow->silevent;
                Int_t eventNumber = siliconEvent->GetVF48NEvent();

                if(eventNumber == fCurrentEventNumber)
                {
                    std::cout << "current event Number = " << fCurrentEventNumber << ". Matched with found eventNumber " << eventNumber << std::endl;
                    std::cout << "(x, y, z, r, phi, nT) = (" << fX << ", " << fY << ", " << fZ << ", " << fR << ", " << fPhi << ", " << fNumTracks << ")" << std::endl; 
                                        
                    //Update members
                    fX = siliconEvent->GetVertexX();
                    fY = siliconEvent->GetVertexY();
                    fZ = siliconEvent->GetVertexZ();
                    fR = siliconEvent->GetVertexR();
                    fPhi = siliconEvent->GetVertexPhi();
                    fNumTracks = siliconEvent->GetNTracks();

                    //Fill tree.
                    fTree->Fill();
                
                    //Update current event number to be checked against (remember everything here is in order).
                    fCurrentEventIndex++;
                    fCurrentEventNumber = fEventIDs[fCurrentEventIndex].second;
                }
            }
        }
        return flow;
    }

    void AnalyzeSpecialEvent(TARunInfo* runInfo, TMEvent* event)
    {
        printf("a2dump::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runInfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
    }

    void LoadEventIDs()
    {
        //TODO Take runNumber as input and filter out all but current run. 0 = all
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
