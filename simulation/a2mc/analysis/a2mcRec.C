#define BUILD_A2 1

#include "a2mcToRAW.C"

class a2mcReco: public a2mcToRAW
{
private:
   TAlphaEventMap* fAlphaEventMap;
   TAlphaEvent* fAlphaEvent;
   TVector3 fMCvertex;
   TH1D* hvtx_z;
   TH1D* hvtx_r;
   TH1D* hvtx_phi;
   TH2D* hvtx_xy;
   TH1D* hNtracks;
   TFile* fRoot;

public:
   a2mcReco(Int_t runNumber=0):a2mcToRAW(runNumber)
   {
      PreInit();
      // create the reconstruction object
      fAlphaEvent = new TAlphaEvent(fAlphaEventMap);
      CreateOutputFile();
      hvtx_z=new TH1D("hvtx_z","MC Reco Vertex Z;z [cm]",1000,-50.,50.);
      hvtx_r=new TH1D("hvtx_r","MC Reco Vertex R;r [cm]",500,0.,50);
      hvtx_phi=new TH1D("hvtx_phi","MC Reco Vertex #phi;#phi [deg]",200,-180.,180.);
      hvtx_xy=new TH2D("hvtx_xy","MC Reco Vertex X-Y;x [mm];y [mm]",100,-100.,100,100,-100.,100);
      hNtracks=new TH1D("hNtrack","MC Number Reco Tracks",10,0.,10.);
   }
   
   ~a2mcReco()
   {
      delete fAlphaEventMap;
      delete fAlphaEvent;
      delete hvtx_z;
      delete hvtx_r;
      delete hvtx_phi;
      delete hvtx_xy;
      delete hNtracks;
   }
   
   void PreInit()
   {
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
   
   void RecEvent()
   {
      if (fChain == 0) return;
      fTotEvents = fChain->GetEntriesFast();
      ///< Loop on the events -> select the hits and the MC tracks to write to 
      Int_t nOutEvents = 0;
      if(verbose) cout << "Number of events " << fTotEvents << endl;
      for (fEvent=0; fEvent<fTotEvents; fEvent++) 
         {
            Long64_t ientry = LoadTree(fEvent);
            if (ientry < 0) break;
            fChain->GetEntry(fEvent);
            if(!GoodEvent()) continue;
            if(verbose) 
               {
                  cout << "____________________ EVENT # " << fEvent 
                       << " ==> Primary PDG Code " << fPdgCode 
                       << " _____________________" << endl;
                  cout << "Number of silicon detector hits " << SilHits_ << endl;
               }
            fAlphaEvent->DeleteEvent();
            if(verbose) fAlphaEvent->SetVerboseLevel(1);
            bool ok=false;
            for(UInt_t ih=0; ih<SilHits_; ih++) 
               {
                  ///< Loop on the hits
                  Int_t nstrp = SilHits_fnStrp[ih]; /// hit n-strip
                  Int_t pstrp = SilHits_fpStrp[ih]; /// hit p-strip
                  Double_t q = SilHits_fEdep[ih]*1.e4;

                  Int_t hybrid_number=SilHits_fSilID[ih];
                  if( hybrid_number < 0 || hybrid_number > 71 )
                     cerr<<"a2mcReco::RecEvent  Hybrid "<<hybrid_number
                         <<" doesn't exist!!! PANIC NOW!"<<endl;

                  // cout<<"sil #"<<hybrid_number<<" n:"<<nstrp<<" p:"<<pstrp<<" Edep: "<<q<<endl;

                  TAlphaEventSil* sil = fAlphaEvent->GetSilByNumber( hybrid_number );
                  if( !sil )
                     {
                        sil = new TAlphaEventSil( hybrid_number, fAlphaEvent, fAlphaEventMap );
                        fAlphaEvent->AddSil( sil );
                     }
	    
                  Double_t* nside=0;
                  Double_t* nrms=0;
                  if( nstrp>=0 && nstrp<128 ) 
                     {
                        nside=sil->GetASIC1();
                        nside[nstrp]+=q;
                        nrms=sil->GetRMS1();
                        nrms[nstrp]=1000.; // To Be Reviewed
                     }
                  else if( nstrp>=128 && nstrp<256 )
                     {
                        nside=sil->GetASIC2();
                        nside[nstrp-128]+=q;
                        nrms=sil->GetRMS2();
                        nrms[nstrp-128]=1000.; // To Be Reviewed
                     }
                  Double_t* pside=0;
                  Double_t* prms=0;
                  if( pstrp>=0 && pstrp<128 ) 
                     {
                        pside=sil->GetASIC3();
                        pside[pstrp]+=q;
                        prms=sil->GetRMS3();
                        pside[pstrp]=1000.;
                     }
                  else if( pstrp>=128 && pstrp<256 ) 
                     {
                        pside=sil->GetASIC4();
                        pside[pstrp-128]+=q;
                        prms=sil->GetRMS4();
                        pside[pstrp-128]=1000.;
                     }
	    
                  if( nside && pside ) ok=true;
               }
            
            if(ok) fAlphaEvent->RecEvent();

            // if( !event->IsTrig() ) return; // To Be Considered

            TAlphaEventVertex *vertex = fAlphaEvent->GetVertex();
            if( vertex->IsGood() )
               {
                  if( verbose ) vertex->Print();
                  Double_t X = vertex->X();
                  Double_t Y = vertex->Y();
                  Double_t Z = vertex->Z();
                  Double_t PHI = vertex->Phi();
                  Double_t R = sqrt(X*X+Y*Y);
                  hvtx_z->Fill(Z);
                  hvtx_r->Fill(R);
                  hvtx_phi->Fill(PHI*TMath::RadToDeg());
                  hvtx_xy->Fill(X,Y);
                  Double_t NTracks = 0.;
                  for( Int_t itrack = 0; itrack < fAlphaEvent->GetNHelices(); itrack++ )
                     {
                        TAlphaEventHelix * h = (TAlphaEventHelix*) fAlphaEvent->GetHelix(itrack);
                        if(!h) continue;  
                        if(h->GetHelixStatus()<=0) continue;
                        ++NTracks;
                     }
                  hNtracks->Fill(NTracks);
               }
            nOutEvents++;
         }
      cout << "Writing " << nOutEvents << " \'good\' events in the reco output file" << endl;
      fRoot->Write();
   }

   void CreateOutputFile() 
   {
      ///< Create the RAW output file
      std::ostringstream fileName;
      fileName << "../root/a2mcReco_" << fRunNumber << ".root";
      cout << "Creating a2mcReco file " << fileName.str() << endl;
      fRoot=TFile::Open(fileName.str().c_str(),"RECREATE");
   }
  
};

a2mcReco* reco=0;

void a2mcRec(Int_t runNumber=0)
{
   reco = new a2mcReco(runNumber);
   reco->RecEvent();
   delete reco;
   gROOT->ProcessLine(".q");

}



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
