//
// reco_module.cxx
//
// reconstruction of TPC data
//

#include <stdio.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <TTree.h>
#include <TClonesArray.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TGraph.h>

#include "TPCconstants.hh"
#include "LookUpTable.hh"
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include "TTrack.hh"
#include "TFitLine.hh"

#include "TStoreEvent.hh"

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))


class RecoRun: public TARunObject
{
public:
   bool do_plot = false;
private:
   TClonesArray fPointsArray;
   TClonesArray fTracksArray;
   TClonesArray fLinesArray;

   LookUpTable* fSTR;

   TH1D* hspz;
   TH1D* hspr;
   TH1D* hspp;
   TH2D* hspxy;
   TH2D* hspzr;
   TH2D* hspzp;

   TH1D* hdsp;

   TH1D* hNspacepoints;
   TH1D* hNtracks;
   TH1D* hpattreceff;

   TH1D* hNlines;
   TH1D* hphi;
   TH1D* htheta;
   
   TH1D* hlz;
   TH1D* hlp;
   TH2D* hlzp;

   TH1D* hcosang;
   TH1D* hdist;
   TH2D* hcosangdist;

   // plots
   TCanvas* creco;

public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo): TARunObject(runinfo), 
                                fPointsArray("TSpacePoint",1000),
                                fTracksArray("TTrack",50),
                                fLinesArray("TFitLine",50)
   {
      printf("RecoRun::ctor!\n");
      fSTR = new LookUpTable(runinfo->fRunNo);
   }

   ~RecoRun()
   {
      printf("RecoRun::dtor!\n");
      delete fSTR;     
      DELETE(creco);
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      do_plot = (runinfo->fRoot->fgApp != NULL);
      if(do_plot) 
         {
            TString ctitle=TString::Format("reco R%d", runinfo->fRunNo);
            creco = new TCanvas("creco",ctitle.Data(),1600,1600);
         }
  
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);

      gDirectory->mkdir("reco")->cd();
      hspz = new TH1D("hspz","Spacepoints;z [mm]",1200,-1200.,1200.);
      hspr = new TH1D("hspr","Spacepoints;r [mm]",80,109.,190.); 
      hspp = new TH1D("hspp","Spacepoints;#phi [deg]",100,0.,360.);

      hspxy = new TH2D("hspxy","Spacepoints;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);
      hspzr = new TH2D("hspzr","Spacepoints;z [mm];r [mm]",600,-1200.,1200.,80,109.,190.);
      hspzp = new TH2D("hspzp","Spacepoints;z [mm];#phi [deg]",600,-1200.,1200.,90,0.,360.);

      hdsp = new TH1D("hdsp","Distance Spacepoints;d [mm]",100,0.,50.);

      hNspacepoints = new TH1D("hNspacepoints","Good Spacepoints",200,0.,200.);
      hNtracks = new TH1D("hNtracks","Found Tracks",10,0.,10.);
      hpattreceff = new TH1D("hpattreceff","Track Finding Efficiency",202,-1.,200.);

      hNlines = new TH1D("hNlines","Reconstructed Lines",10,0.,10.);
      hphi = new TH1D("hphi","Direction #phi;#phi [deg]",200,-180.,180.);
      htheta = new TH1D("htheta","Direction #theta;#theta [deg]",200,-180.,180.);
  
      hlz = new TH1D("hlz","Intersection with r=0;z [mm]",1200,-1200.,1200.);
      hlp = new TH1D("hlp","Intersection with r=0;#phi [deg]",100,-180.,180.);
      hlzp = new TH2D("hlzp","Intersection with r=0;z [mm];#phi [deg]",600,-1200.,1200.,90,-180.,180.);

      hcosang = new TH1D("hcosang","Cosine of Angle Formed by 2 Lines;cos(#alpha)",200,-1.,1.);
      hdist = new TH1D("hdist","Distance between  2 Lines;s [mm]",200,0.,20.);

      hcosangdist = new TH2D("hcosangdist",
                             "Correlation Angle-Distance;cos(#alpha);s [mm]",
                             100,-1.,1.,100,0.,20.);
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("RecoRun::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("RecoRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("RecoRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      printf("RecoRun::Analyze, run %d\n", runinfo->fRunNo);
      
      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      AgEvent* age = ef->fEvent;
      std::cout<<"RecoRun::Analyze Event # "<<age->counter<<std::endl;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) return flow;

      printf("RecoModule::Analyze, AW # signals %d\n", int(SigFlow->awSig.size()));
      printf("RecoModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig.size()));

      printf("RecoModule::Analyze, SP # %d\n", int(SigFlow->matchSig.size()));

      AddSpacePoint( &SigFlow->matchSig );
      printf("RecoRun Analyze  Points: %d\n",fPointsArray.GetEntries());

      TracksFinder pattrec( &fPointsArray );
      pattrec.AdaptiveFinder();
      AddTracks( pattrec.GetTrackVector() );
      printf("RecoRun Analyze  Tracks: %d\n",fTracksArray.GetEntries());

      FitLines();
      printf("RecoRun Analyze  Lines: %d\n",fLinesArray.GetEntries());

      analyzed_event->Reset();
      analyzed_event->SetEventNumber( age->counter );
      analyzed_event->SetEvent(&fPointsArray,&fLinesArray);
      // printf("RecoRun Analyze  Fake Pattern Recognition Efficiency: %1.1f\n",
      //        analyzed_event->GetNumberOfPointsPerTrack());
      flow = new AgAnalysisFlow(flow, analyzed_event);
      EventTree->Fill();
      //      std::cout<<"\tRecoRun Analyze EVENT "<<age->counter<<" ANALYZED"<<std::endl;

      Plot();

      if( do_plot ) ShowPlots();

      fPointsArray.Clear("C");
      fTracksArray.Clear("C");
      fLinesArray.Clear("C");

      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("RecoRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   void AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints )
   {
      int n = 0;
      for( auto sp=spacepoints->begin(); sp!=spacepoints->end(); ++sp )
         {
            //new(fPointsArray[n]) TSpacePoint(sp.first.idx,sp.second.idx,sp.first.t);

            double time = sp->first.t;
            double r = fSTR->GetRadius( time ),
               correction = fSTR->GetAzimuth( time ),
               err = fSTR->GetdRdt( time );

            // std::cout<<"RecoRun::AddSpacePoint "<<n<<" "<<sp->first.idx
            //          <<" "<<sp->second.sec<<" "<<sp->second.idx<<" "
            //          <<time<<" "<<r<<" "<<correction<<" "<<err<<std::endl;

            new(fPointsArray[n]) TSpacePoint(sp->first.idx,
                                             sp->second.sec,sp->second.idx,
                                             time,
                                             r,correction,err,sp->first.height);
            ++n;
         }
      //      fPointsArray.Compress();
      fPointsArray.Sort();
      std::cout<<"RecoRun::AddSpacePoint # entries: "<<fPointsArray.GetEntries()<<std::endl;
   }

   void AddTracks( const std::vector< std::list<int> >* track_vector )
   {
      int n=0;
      for( auto it=track_vector->begin(); it!=track_vector->end(); ++it)
         {
            new(fTracksArray[n]) TTrack;
            for( auto ip=it->begin(); ip!=it->end(); ++ip)
               {
                  ( (TTrack*)fTracksArray.ConstructedAt(n) ) -> 
                     AddPoint( (TSpacePoint*) fPointsArray.At(*ip) );
               }
            ++n;
         }
      fTracksArray.Compress();
      assert(n==int(track_vector->size()));
      assert(fTracksArray.GetEntries()==int(track_vector->size()));
      //      std::cout<<"RecoRun::AddTracks # entries: "<<fTracksArray.GetEntries()<<std::endl;
   }

   void FitLines()
   {
      int n=0;
      for(int it=0; it<fTracksArray.GetEntries(); ++it )
         {
            TTrack* at = (TTrack*) fTracksArray.At(it);
            //at->Print();
            new(fLinesArray[n]) TFitLine(*at);
            ( (TFitLine*)fLinesArray.ConstructedAt(n) )->Fit();
            if( ( (TFitLine*)fLinesArray.ConstructedAt(n) )->IsGood() )
               {
                  ( (TFitLine*)fLinesArray.ConstructedAt(n) )->CalculateResiduals();
                  ( (TFitLine*)fLinesArray.ConstructedAt(n) )->Print();
                  ++n;
               }
            else
               {
                  ( (TFitLine*)fLinesArray.ConstructedAt(n) ) -> Reason();
                  fLinesArray.RemoveAt(n);
               }
         }
   }

   void ShowPlots()
   {
      creco->Clear();
      creco->Divide(2,2);

      TH1D* htemp1 = new TH1D("htemp1",";x [mm];y [mm]",1,-190.,190.);
      htemp1->SetStats(0);
      htemp1->SetMinimum(-190.); htemp1->SetMaximum(190.); 
      TGraph* gxy = new TGraph;
      gxy->SetName("x-y");
      gxy->SetTitle(";x [mm];y [mm]");
      gxy->SetMarkerColor(kBlue);
      gxy->SetMarkerStyle(43);
      
      TH1D* htemp2 = new TH1D("htemp2",";z [mm];r [mm]",1,-1200.,1200.);
      htemp2->SetStats(0);
      htemp2->SetMinimum(0.); htemp2->SetMaximum(190.);
      TGraph* gzr = new TGraph;
      gzr->SetName("z-r");
      gzr->SetTitle(";z [mm];r [mm]");
      gzr->SetMarkerColor(kBlue);
      gzr->SetMarkerStyle(43);

      int np=0;
      for(int isp=0; isp<fPointsArray.GetEntries(); ++isp)
         {
            TSpacePoint* ap = (TSpacePoint*) fPointsArray.At(isp);
            gxy->SetPoint(np,ap->GetX(),ap->GetY());
            gzr->SetPoint(np,ap->GetZ(),ap->GetR());
            std::cout<<np<<"\t"<<ap->GetX()<<"\t"<<ap->GetY()<<"\t"<<ap->GetZ()<<"\t"<<ap->GetR()<<std::endl;
            ++np;
         }
      
      creco->cd(1);
      htemp1->Draw();
      if( np > 0 ) gxy->Draw("Psame");
      gPad->SetGrid();
      gPad->Modified();
      gPad->Update();

      creco->cd(2);
      htemp2->Draw();
      if( np > 0 ) gzr->Draw("Psame");
      gPad->SetGrid();
      gPad->Modified();
      gPad->Update();
      
   }

   void Plot()
   {
      for(int isp=0; isp<fPointsArray.GetEntries(); ++isp)
         {
            TSpacePoint* ap = (TSpacePoint*) fPointsArray.At(isp);
            hspz->Fill(ap->GetZ());
            hspr->Fill(ap->GetR());
            hspp->Fill(ap->GetPhi()*TMath::RadToDeg());
            hspxy->Fill(ap->GetX(),ap->GetY());
            hspzr->Fill(ap->GetZ(),ap->GetR());
            hspzp->Fill(ap->GetZ(),ap->GetPhi()*TMath::RadToDeg());

            for(int jsp=isp; jsp<fPointsArray.GetEntries(); ++jsp)
               {
                  if( ap->GetR() > _fwradius ) continue;
                  TSpacePoint* bp = (TSpacePoint*) fPointsArray.At(jsp);
                  if( bp->GetR() > _fwradius ) continue;
                  hdsp->Fill( ap->Distance( bp ) );
               }
         }

      double number_tracks = double( fTracksArray.GetEntries() );
      double total_points=0.;
      for( int it=0; it<fTracksArray.GetEntries(); ++it )
         {
            TTrack* at = (TTrack*) fTracksArray.At(it);
            total_points += double(at->GetNumberOfPoints());
         }
      if( total_points )
         hNspacepoints->Fill( total_points );
      else
         hNspacepoints->Fill( -1. );
      hNtracks->Fill( number_tracks );
      if( number_tracks > 0. )
         hpattreceff->Fill( total_points/number_tracks );
      else
         hpattreceff->Fill( -1. );

      hNlines->Fill( double(fLinesArray.GetEntries()) );
      for( int il=0; il<fLinesArray.GetEntries(); ++il )
         {
            TFitLine* aLine = (TFitLine*) fLinesArray.At(il);
            hphi->Fill(TMath::ATan2(aLine->GetUy(),aLine->GetUx()));
            double ur = TMath::Sqrt( aLine->GetUx()*aLine->GetUx() + aLine->GetUy()*aLine->GetUy() );
            if( ur > 0. )
               htheta->Fill(TMath::ACos(aLine->GetUz()/ur));
            
            TVector3 r0 = aLine->Evaluate( 0. );
            hlz->Fill( r0.Z() );
            hlp->Fill( r0.Phi()*TMath::RadToDeg() );
            hlzp->Fill( r0.Z(), r0.Phi()*TMath::RadToDeg() );
         }

      if( fLinesArray.GetEntries() == 2 )
         {
            double cang = ((TFitLine*) fLinesArray.At(0))->CosAngle((TFitLine*) fLinesArray.At(1));
            hcosang->Fill( cang );
            double dist = ((TFitLine*) fLinesArray.At(0))->Distance((TFitLine*) fLinesArray.At(1));
            hdist->Fill( dist );
            
            hcosangdist->Fill( cang, dist );
         }
   }
};

class RecoModuleFactory: public TAFactory
{
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("RecoModuleFactory::Init!\n");
   }
   void Finish()
   {
      printf("RecoModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("RecoModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new RecoRun(runinfo);
   }
};


static TARegister tar(new RecoModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
