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
    std::vector<int> runNumbers;
    int runNumber;
    std::string runlist;
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
        if(args[i] == "--listfile")
            runlist = args[i+1];
    }

    std::ifstream myFile(runlist);
    std::string line;
    if (myFile.is_open())
    {
        while ( std::getline(myFile, line) )
        {
            int runNumber;
            std::cout << line << std::endl;
            std::sscanf(line.c_str(), "%d", &runNumber);
            runNumbers.push_back(runNumber);
        }
        myFile.close();
    }

    //Create files and trees here
    TChain* chain;
    switch(runType)
    {
        case kMixing:
            chain = new TChain("mixing");
            break;
        case kCosmic:
            chain = new TChain("cosmic");
            break;
    }

    for(int i = 0; i<runNumbers.size(); i++)
    {
        std::string filename = "dumperoutput";
        filename += std::to_string(runNumbers[i]);
        filename += ".root";
        chain->Add(filename.c_str());
    }


    //TFile oldFile(filename.c_str());
    TTree *oldTree;
    //oldFile.GetObject("mixing", oldTree);
    const auto nEntries = chain->GetEntries();

    TFile *trainFile, *validFile, *testFile;
    TTree *trainTree, *validTree, *testTree;
    switch(runType)
    {
        case(kMixing):
            trainFile = new TFile("trainMixing.root", "RECREATE");
            trainTree = chain->CloneTree(0);
            validFile = new TFile("validationMixing.root", "RECREATE");
            validTree = chain->CloneTree(0);
            testFile = new TFile("testMixing.root", "RECREATE");
            testTree = chain->CloneTree(0);
            break;
        case(kCosmic):
            trainFile = new TFile("trainCosmic.root", "RECREATE");
            trainTree = chain->CloneTree(0);
            validFile = new TFile("validationCosmic.root", "RECREATE");
            validTree = chain->CloneTree(0);
            testFile = new TFile("testCosmic.root", "RECREATE");
            testTree = chain->CloneTree(0);
            break;
    }

    for (int i = 0; i < nEntries; i++) 
    {
        chain->GetEntry(i);
        if (i%3 == 0)
            trainTree->Fill();
        if (i%3 == 1)
            validTree->Fill();
        if (i%3 == 2)
            testTree->Fill();
    }

    trainFile->cd();
    trainFile->Write();

    validFile->cd();
    validFile->Write();

    testFile->cd();
    testFile->Write();

    trainFile->Close();
    validFile->Close();
    testFile->Close();

    return EXIT_SUCCESS;
}