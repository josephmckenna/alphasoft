//Std Lib
#include <vector>
//Root stuff
#include "TChain.h"
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"
//Our stuff
#include "TA2Plot.h"
#include "TA2Plot_Filler.h"

R__LOAD_LIBRARY($ROOTSYS/test/libEvent.so)

int main(int argc, char* argv[])
{
    int runNumber;
    int startTime = 0;
    int endTime = -1;
    std::vector<std::string> args;
    enum ERunType
    {
        kMixing,
        kCosmic
    };
    ERunType runType;

    for (int i=0; i<argc; i++) 
    {
        args.push_back(argv[i]);
    }

    for (unsigned int i=1; i<args.size(); i++) // loop over the commandline options
    { 
        const char* arg = args[i].c_str();
        if(args[i] == "--mixing")
            runType = kMixing;
        if(args[i] == "--cosmic")
            runType = kCosmic;
        if(args[i] == "--runnumber")
            runNumber = stoi(args[i+1]);
        if(args[i] == "--firsttime")
            startTime = stoi(args[i+1]);
        if(args[i] == "--lasttime")
            endTime = stoi(args[i+1]);
    }

    std::string filename = "eventlist";
    filename += std::to_string(runNumber);

    //LMG - Switch/case only taking Mixing if Mixing.
    TA2Plot* plot = new TA2Plot();
    switch(runType)
    {
        case kMixing:
            plot->AddDumpGates(runNumber, {"Mixing"}, {-1});
            break;
        case kCosmic:
            plot->AddTimeGate(runNumber,startTime,endTime);
            break;
    }
    plot->LoadData();
    //TCanvas* canvas = plot->DrawCanvas("Not Needed");
    plot->WriteEventList(filename);

    return EXIT_SUCCESS;
}