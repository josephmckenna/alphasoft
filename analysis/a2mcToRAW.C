///< ##################################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##################################################
///< This class reads the a2MC output and writes a 
///< alpha 2 RAW data file

#define a2mcToRAW_cxx
#include "a2mcToRAW.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
bool verbose = true;

///< Writing the RAW data file (from the MC output data file to the RAW data format)
//=====================
void a2mcToRAW::WriteRAW() {
    if (fChain == 0) return;
    CreateOutputFile();
    fTotEvents = fChain->GetEntriesFast();
///< Loop on the events -> select the hits and the MC tracks to write to 
    Int_t nOutEvents = 0;
    if(verbose) cout << "Number of events " << fTotEvents << endl;
    for (fEvent=0; fEvent<fTotEvents; fEvent++) {
        Long64_t ientry = LoadTree(fEvent);
        if (ientry < 0) break;
        fChain->GetEntry(fEvent);
        if(!GoodEvent()) continue;
        if(verbose) {
            cout << "__________________o " << fEvent << " o_____________________" << endl;
            cout << "Number of silicon detector hits " << SilHits_ << endl;
        }
        for(UInt_t ih=0; ih<SilHits_; ih++) {
            ///< Loop on the hits
//            SilHits_fnStrp[ih] ///< e.g. hit n-strip
        }
        nOutEvents++;
    }
    cout << "Writing " << nOutEvents << " \'good\' events in the RAW output file" << endl;
}

///< Filtering the events
//=====================
Bool_t a2mcToRAW::GoodEvent() {
    ///< Putting here all the filter (cut) parameters
    if(SilHits_<=0) return false; ///< Skipping events with no hits in the silicon
    return true;
}

///< Creating/Opening the RAW data output file 
//=====================
void a2mcToRAW::CreateOutputFile() {
///< Create the RAW output file
    std::ostringstream fileName;
    fileName << "./root/a2mcRAW_" << fRunNumber << ".bin";
    cout << "Creating a2mcRAW file " << fileName.str() << endl;
}

