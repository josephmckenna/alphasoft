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
    if(verbose) cout << "Number of events " << fTotEvents << endl;
    ///< Create the Alpha Event holder

    fAlphaEvent = new TAlphaEvent(fAlphaEventMap);
    if(verbose) fAlphaEvent->SetVerboseLevel(1);

    ///< Loop over the MC events
    for (fEvent=0; fEvent<fTotEvents; fEvent++) { ///< Loop over the events
        Long64_t ientry = LoadTree(fEvent);
        if(!GoodEvent()) continue; ///< Checking if the event is good
        if(verbose) {
            cout << "||| EVENT # " << fEvent << " ==> Primary PDG Code " << fPdgCode << " _" << endl;
            cout << "\t Number of silicon detector hits " << SilHits_ << endl;
        }

        ///< Resetting fAlphaEvent and filling it with the hits of this event
        fAlphaEvent->DeleteEvent(); ///< Resetting fAlphaEvent
        Bool_t ok = FillAlphaEvent(); ///< ok not used for the moment
        if(ok) fAlphaEvent->RecEvent();
        
        ///< Extracting the information of the reconstructed vertex
        isRecV = false;
        TAlphaEventVertex *vertex = fAlphaEvent->GetVertex();
        if(vertex!=NULL&&vertex->IsGood()) {
            isRecV = true;
            fRecVdx = vertex->X();
            fRecVdy = vertex->Y();
            fRecVdz = vertex->Z();
            fRecPhi = vertex->Phi();
            if(verbose) {
                cout << "MC  Vertex [" << fVdx << ", " << fVdy << ", " << fVdz << "]" << endl;
                cout << "Rec Vertex [" << fRecVdx << ", " << fRecVdy << ", " << fRecVdz << "]" << endl;
            }
        }
        ///< Fill the histos
        FillHistos();
    } ///< End of loop over the events
}

///< Filtering the events
//=====================
Bool_t a2mcReco::GoodEvent() {
    ///< Putting here all the filter (cut) parameters
    if(SilHits_<=0) return false; ///< Skipping events with no hits in the silicon
    return true;
}

//Bool_t a2mcReco::GoodHit() {
//    
//}

void a2mcReco::ShowHistos(bool save_histos) {
    gROOT->cd();
    TCanvas *cVertex = new TCanvas("cVertex", "MC/Rec/Diff vertex distributions", 1200,1200);
    cVertex->Divide(3,3);
    int i=0;
    cVertex->cd(++i); hMCPhi->Draw();
    cVertex->cd(++i); hMCVdr->Draw();
    cVertex->cd(++i); hMCVdz->Draw();

    cVertex->cd(++i); hRecPhi->Draw();
    cVertex->cd(++i); hRecVdr->Draw();
    cVertex->cd(++i); hRecVdz->Draw();

    cVertex->cd(++i); hDiffPhi->Draw();
    cVertex->cd(++i); hDiffVdr->Draw();
    cVertex->cd(++i); hDiffVdz->Draw();

    Double_t parPhi[6] = {2.*hDiffPhi->GetMaximum()/3.,0.,15.,hDiffPhi->GetMaximum()/3.,0.,50.};
    TF1 *fPhi = new TF1("fPhi","gaus(0)+gaus(3)",-100.,100.);
    fPhi->SetLineColor(2);
    fPhi->SetParameters(parPhi);
    hDiffPhi->Fit(fPhi,"R+");

    Double_t parR[6] = {4.*hDiffVdr->GetMaximum()/5.,0.,0.8,hDiffVdr->GetMaximum()/5.,-1.2,2.2};
    TF1 *fR = new TF1("fR","gaus(0)+gaus(3)",-5.,5.);
    fR->SetLineColor(2);
    fR->SetParameters(parR);
    hDiffVdr->Fit(fR,"R+");
    
    Double_t parZ[6] = {2.*hDiffVdz->GetMaximum()/3.,0.,0.5,hDiffVdz->GetMaximum()/3.,0.,2.0};
    TF1 *fZ = new TF1("fZ","gaus(0)+gaus(3)",-5.,5.);
    fZ->SetLineColor(2);
    fZ->SetParameters(parZ);
    hDiffVdz->Fit(fZ,"R+");
    
    cVertex->Modified(); cVertex->Update();
    if(save_histos) {
        ostringstream s; 
        s << "cVertex_run_" << fRunNumber << ".root";
        cVertex->Print(s.str().c_str(),".root");
    }
}
void a2mcReco::FillHistos() {
    gROOT->cd();
    if(!isnan(fVdx)) {
        hMCVdx->Fill(fVdx);
        hMCVdy->Fill(fVdy);
        hMCPhi->Fill(TMath::RadToDeg()*atan2(fVdy, fVdx));
        hMCVdz->Fill(fVdz);
        hMCVdr->Fill(sqrt(fVdx*fVdx+fVdy*fVdy));
    }
    if(isRecV) {
        hRecVdx->Fill(fRecVdx);
        hRecVdy->Fill(fRecVdy);
        hRecPhi->Fill(TMath::RadToDeg()*atan2(fRecVdy, fRecVdx));
        hRecVdz->Fill(fRecVdz);
        hRecVdr->Fill(sqrt(fRecVdx*fRecVdx+fRecVdy*fRecVdy));
    }
    if(!isnan(fVdx)&&isRecV) {
        hDiffVdx->Fill(fVdx-fRecVdx);
        hDiffVdy->Fill(fVdy-fRecVdy);
        hDiffPhi->Fill(TMath::RadToDeg()*atan2(fVdy, fVdx)-TMath::RadToDeg()*atan2(fRecVdy, fRecVdx));
        hDiffVdz->Fill(fVdz-fRecVdz);
        hDiffVdr->Fill(sqrt(fVdx*fVdx+fVdy*fVdy)-sqrt(fRecVdx*fRecVdx+fRecVdy*fRecVdy));
    }
}

void a2mcReco::CreateHistos() {
    gROOT->cd();
    Int_t nBinsVd = 100;
    Float_t xMin = -5., xMax = +5.;
    Float_t zMin = -5., zMax = +5.;
    Float_t phiMin = -100., phiMax = +100.;
    hMCVdx   = new TH1F("hMCVdx",   "MC Vdx"  , nBinsVd, xMin, xMax);
    hMCVdy   = new TH1F("hMCVdy",   "MC Vdy"  , nBinsVd, xMin, xMax);
    hMCPhi   = new TH1F("hMCPhi",   "MC Phi"  , nBinsVd, phiMin, phiMax);
    hMCVdz   = new TH1F("hMCVdz",   "MC Vdz"  , nBinsVd, zMin, zMax);
    hMCVdr   = new TH1F("hMCVdr",   "MC Vdr"  , nBinsVd, xMin, xMax);
    hRecVdx  = new TH1F("hRecVdx",  "Rec Vdx" , nBinsVd, xMin, xMax);
    hRecVdy  = new TH1F("hRecVdy",  "Rec Vdy" , nBinsVd, xMin, xMax);
    hRecPhi  = new TH1F("hRecPhi",  "Rec Phi" , nBinsVd, phiMin, phiMax);
    hRecVdz  = new TH1F("hRecVdz",  "Rec Vdz" , nBinsVd, zMin, zMax);
    hRecVdr  = new TH1F("hRecVdr",  "Rec Vdr" , nBinsVd, xMin, xMax);
    hDiffVdx = new TH1F("hDiffVdx", "Diff Vdx", nBinsVd, xMin, xMax);
    hDiffVdy = new TH1F("hDiffVdy", "Diff Vdy", nBinsVd, xMin, xMax);
    hDiffPhi = new TH1F("hDiffPhi", "Diff Phi", nBinsVd, phiMin, phiMax);
    hDiffVdz = new TH1F("hDiffVdz", "Diff Vdz", nBinsVd, zMin, zMax);
    hDiffVdr = new TH1F("hDiffVdr", "Diff Vdr", nBinsVd, xMin, xMax);
}

///< Creating/Opening the RAW data output file 
//=====================
void a2mcReco::CreateOutputFile() {
///< Create the RAW output file
//    std::ostringstream fileName;
//    fileName << "./root/a2mcRAW_" << fRunNumber << ".root";
//    cout << "Creating a2mcRAW file " << fileName.str() << endl;
//    TFile *fRAW = new TFile(fileName.str().c_str(),"NEW");
//    hMCVdx->Write();
//    fRAW->Close();
}

Bool_t a2mcReco::FillAlphaEvent() {
    Int_t nGoodHits = 0;
    for(UInt_t ih=0; ih<SilHits_; ih++) { ///< Loop over the silicon hits
        Int_t nstrp = SilHits_fnStrp[ih]; /// hit n-strip
        Int_t pstrp = SilHits_fpStrp[ih]; /// hit p-strip
        if(SilHits_fEdep[ih]*1000 < 0.07) continue; ///< Threshold of 70 keV of energy released
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
        if(nside && pside) nGoodHits++;
    } ///< End of loop over the silicon hits
    if(nGoodHits>=3) return true;
    return false;
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
