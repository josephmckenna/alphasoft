///< ##################################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##################################################
///< This class reads the a2MC output and writes a 
///< alpha 2 RAW data file

#define a2mcReco_cxx
#include "a2mcReco.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

///< Writing the RAW data file (from the MC output data file to the RAW data format)
//=====================
void a2mcReco::Reco(bool verbose=false) {
    if (fChain == 0) return;
    CreateHistos();
    fTotEvents = fChain->GetEntriesFast();
///< Loop on the events -> select the hits and the MC tracks to write to 
    Int_t nOutEvents = 0;
    if(verbose) cout << "Number of events " << fTotEvents << endl;
    fAlphaEvent = new TAlphaEvent(fAlphaEventMap);
    if(verbose) fAlphaEvent->SetVerboseLevel(1);
    for (fEvent=0; fEvent<fTotEvents; fEvent++) { ///< Loop over the events
        Long64_t ientry = LoadTree(fEvent);
        if (ientry < 0) break;
        fChain->GetEntry(fEvent);
        if(!GoodEvent()) continue;
        fAlphaEvent->DeleteEvent(); ///< Resetting fAlphaEvent
        if(verbose) {
            cout << "||| EVENT # " << fEvent << " ==> Primary PDG Code " << fPdgCode << " _" << endl;
            cout << "\t Number of silicon detector hits " << SilHits_ << endl;
        }
        bool ok=false;
        for(UInt_t ih=0; ih<SilHits_; ih++) { ///< Loop over the silicon hits
            ///< Loop on the hits
            Int_t nstrp = SilHits_fnStrp[ih]; /// hit n-strip
            Int_t pstrp = SilHits_fpStrp[ih]; /// hit p-strip
            Double_t q = SilHits_fEdep[ih]*1.e4;
            Int_t hybrid_number=SilHits_fSilID[ih];
            if(hybrid_number < 0 || hybrid_number > 71)
                cerr<<"a2mcReco::RecEvent  Hybrid "<<hybrid_number <<" doesn't exist!!! PANIC NOW!"<<endl;
            TAlphaEventSil* sil = fAlphaEvent->GetSilByNumber(hybrid_number);
            if(!sil) {
                sil = new TAlphaEventSil(hybrid_number, fAlphaEvent, fAlphaEventMap);
                fAlphaEvent->AddSil(sil);
            }
            Double_t* nside=0;
            Double_t* nrms=0;
            if(nstrp>=0 && nstrp<128) {
                nside=sil->GetASIC1();
                nside[nstrp]+=q;
                nrms=sil->GetRMS1();
                nrms[nstrp]=1000.; // To Be Reviewed
            }
            else if(nstrp>=128 && nstrp<256) {
                nside=sil->GetASIC2();
                nside[nstrp-128]+=q;
                nrms=sil->GetRMS2();
                nrms[nstrp-128]=1000.; // To Be Reviewed
            }
            Double_t* pside=0;
            Double_t* prms=0;
            if(pstrp>=0 && pstrp<128) {
                pside=sil->GetASIC3();
                pside[pstrp]+=q;
                prms=sil->GetRMS3();
                pside[pstrp]=1000.; 
            } else if(pstrp>=128 && pstrp<256) {
                pside=sil->GetASIC4();
                pside[pstrp-128]+=q;
                prms=sil->GetRMS4();
                pside[pstrp-128]=1000.;
            }

            if(nside && pside) ok=true;
        } ///< End of loop over the silicon hits

        if(ok) fAlphaEvent->RecEvent();
        isRecV = false;
        TAlphaEventVertex *vertex = fAlphaEvent->GetVertex();
        if( vertex->IsGood() ) {
            isRecV = true;
            fRecVox = vertex->X();
            fRecVoy = vertex->Y();
            fRecVoz = vertex->Z();
            fRecPhi = vertex->Phi();
            if(verbose) {
                cout << "MC  Vertex [" << fVox << ", " << fVoy << ", " << fVoz << "]" << endl;
                cout << "Rec Vertex [" << fRecVox << ", " << fRecVoy << ", " << fRecVoz << "]" << endl;
            }
        }
        FillHistos();
        nOutEvents++;
    } ///< End of loop over the events
//    CreateOutputFile();
    cout << "Writing " << nOutEvents << " \'good\' events in the RAW output file" << endl;
}

///< Filtering the events
//=====================
Bool_t a2mcReco::GoodEvent() {
    ///< Putting here all the filter (cut) parameters
    if(SilHits_<=0) return false; ///< Skipping events with no hits in the silicon
    return true;
}

void a2mcReco::ShowHistos() {
    gROOT->cd();
    TCanvas *cVertex = new TCanvas("cVertex", "MC/Rec/Diff vertex distributions", 1600,1200);
    cVertex->Divide(3,3);
    cVertex->cd(1); hMCVox->Draw();
    cVertex->cd(2); hMCVoy->Draw();
    cVertex->cd(3); hMCVoz->Draw();
    cVertex->cd(4); hRecVox->Draw();
    cVertex->cd(5); hRecVoy->Draw();
    cVertex->cd(6); hRecVoz->Draw();
    cVertex->cd(7); hDiffVox->Draw();
    cVertex->cd(8); hDiffVoy->Draw();
    cVertex->cd(9); hDiffVoz->Draw();

    cVertex->Modified(); cVertex->Update();
}
void a2mcReco::FillHistos() {
    gROOT->cd();
    if(!isnan(fVox)) {
        hMCVox->Fill(fVox);
        hMCVoy->Fill(fVoy);
        hMCVoz->Fill(fVoz);
    }
    if(isRecV) {
        hRecVox->Fill(fRecVox);
        hRecVoy->Fill(fRecVoy);
        hRecVoz->Fill(fRecVoz);
    }
    if(!isnan(fVox)&&isRecV) {
        hDiffVox->Fill(fRecVox-fVox);
        hDiffVoy->Fill(fRecVoy-fVoy);
        hDiffVoz->Fill(fRecVoz-fVoz);
    }
}

void a2mcReco::CreateHistos() {
    gROOT->cd();
    Int_t nBinsVo = 100;
    Float_t xMin = -10., xMax = +10.;
    Float_t zMin = -10., zMax = +10.;
    hMCVox   = new TH1F("hMCVox",   "MC Vox"  , nBinsVo, xMin, xMax);
    hMCVoy   = new TH1F("hMCVoy",   "MC Voy"  , nBinsVo, xMin, xMax);
    hMCVoz   = new TH1F("hMCVoz",   "MC Voz"  , nBinsVo, zMin, zMax);
    hMCVor   = new TH1F("hMCVor",   "MC Vor"  , nBinsVo, xMin, xMax);
    hRecVox  = new TH1F("hRecVox",  "Rec Vox" , nBinsVo, xMin, xMax);
    hRecVoy  = new TH1F("hRecVoy",  "Rec Voy" , nBinsVo, xMin, xMax);
    hRecVoz  = new TH1F("hRecVoz",  "Rec Voz" , nBinsVo, zMin, zMax);
    hRecVor  = new TH1F("hRecVor",  "Rec Vor" , nBinsVo, xMin, xMax);
    hDiffVox = new TH1F("hDiffVox", "Diff Vox", nBinsVo, xMin, xMax);
    hDiffVoy = new TH1F("hDiffVoy", "Diff Voy", nBinsVo, xMin, xMax);
    hDiffVoz = new TH1F("hDiffVoz", "Diff Voz", nBinsVo, zMin, zMax);
    hDiffVor = new TH1F("hDiffVor", "Diff Vor", nBinsVo, xMin, xMax);
}

///< Creating/Opening the RAW data output file 
//=====================
void a2mcReco::CreateOutputFile() {
///< Create the RAW output file
//    std::ostringstream fileName;
//    fileName << "./root/a2mcRAW_" << fRunNumber << ".root";
//    cout << "Creating a2mcRAW file " << fileName.str() << endl;
//    TFile *fRAW = new TFile(fileName.str().c_str(),"NEW");
//    hMCVox->Write();
//    fRAW->Close();
}


void a2mcReco::InitReco() {
    // Initialize geometry
    new TGeoManager("TALPHAGeo", "ALPHA ROOT geometry manager");
  
    // load the material definitions
    TAlphaGeoMaterialXML * materialXML = new TAlphaGeoMaterialXML();
  
    Char_t pathname[128];
    //  sprintf(pathname,"geo/material.xml");
    sprintf(pathname,"%s/a2lib/geo/material2.xml",getenv("AGRELEASE"));
    materialXML->ParseFile(pathname);
    delete materialXML;

    TAlphaGeoEnvironmentXML * environmentXML = new TAlphaGeoEnvironmentXML();
    //  sprintf(pathname,"geo/environment_geo.xml");
    sprintf(pathname,"%s/a2lib/geo/environment2_geo.xml",getenv("AGRELEASE"));
    environmentXML->ParseFile(pathname);
    delete environmentXML;

    TAlphaGeoDetectorXML * detectorXML = new TAlphaGeoDetectorXML();
    //  sprintf(pathname,"geo/detector_geo.xml");
    sprintf(pathname,"%s/a2lib/geo/detector2_geo.xml",getenv("AGRELEASE"));
    detectorXML->ParseFile(pathname);
    delete detectorXML;

    // close geometry
    gGeoManager->CloseGeometry();

    fAlphaEventMap=new TAlphaEventMap();  
}
