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
#include "TFitHelix.hh"

#include "TStoreEvent.hh"

class RecoRun: public TARunObject
{
public:
   bool do_plot = false;
   bool fTrace = false;
   //bool fTrace = true;
private:
   TClonesArray fPointsArray;
   TClonesArray fTracksArray;
   TClonesArray fLinesArray;
   TClonesArray fHelixArray;

   LookUpTable* fSTR;

   TH1D* hspz;
   TH1D* hspr;
   TH1D* hspp;
   TH2D* hspxy;
   TH2D* hspzr;
   TH2D* hspzp;

   TH2D* hsprp;

   TH1D* hdsp;

   TH1D* hNspacepoints;
   TH1D* hNtracks;
   TH1D* hpattreceff;

   TH1D* hNlines;
   TH1D* hphi;
   TH1D* htheta;

   TH1D* hchi2;
   TH2D* hchi2sp;

   TH1D* hcosang;
   TH1D* hdist;
   TH2D* hcosangdist;

   // // plots
   // TCanvas* creco;

   double MagneticField;
   unsigned fNhitsCut;

public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo): TARunObject(runinfo), 
                                fPointsArray("TSpacePoint",1000),
                                fTracksArray("TTrack",50),
                                fLinesArray("TFitLine",50),
                                fHelixArray("TFitHelix",50),
                                MagneticField(_MagneticField),
                                fNhitsCut(5000)
   {
      printf("RecoRun::ctor!\n");
      //fSTR = new LookUpTable(runinfo->fRunNo);
      fSTR = new LookUpTable(0.3,MagneticField);
   }

   ~RecoRun()
   {
      printf("RecoRun::dtor!\n");
      delete fSTR;     
      //if(creco) delete creco;
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      // do_plot = (runinfo->fRoot->fgApp != NULL);
      // if(do_plot) 
      //    {
      //       TString ctitle=TString::Format("reco R%d", runinfo->fRunNo);
      //       creco = new TCanvas("creco",ctitle.Data(),1600,1600);
      //    }
  
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);

      gDirectory->mkdir("reco")->cd();
      // hspz = new TH1D("hspz","Spacepoints;z [mm]",1200,-1200.,1200.);
      // hspr = new TH1D("hspr","Spacepoints;r [mm]",80,109.,190.); 
      // hspp = new TH1D("hspp","Spacepoints;#phi [deg]",100,0.,360.);

      // hspxy = new TH2D("hspxy","Spacepoints;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);
      // hspzr = new TH2D("hspzr","Spacepoints;z [mm];r [mm]",600,-1200.,1200.,80,109.,190.);
      // hspzp = new TH2D("hspzp","Spacepoints;z [mm];#phi [deg]",600,-1200.,1200.,90,0.,360.);

      hsprp = new TH2D("hsprp","Spacepoints in Tracks;#phi [deg];r [mm]",
                       180,0.,TMath::TwoPi(),200,0.,175.);

      // hdsp = new TH1D("hdsp","Distance Spacepoints;d [mm]",100,0.,50.);

      // hNspacepoints = new TH1D("hNspacepoints","Good Spacepoints",500,0.,500.);
      // hNtracks = new TH1D("hNtracks","Found Tracks",10,0.,10.);
      // hpattreceff = new TH1D("hpattreceff","Track Finding Efficiency",202,-1.,200.);

      // hNlines = new TH1D("hNlines","Reconstructed Lines",10,0.,10.);
      // hphi = new TH1D("hphi","Direction #phi;#phi [deg]",200,-180.,180.);
      // htheta = new TH1D("htheta","Direction #theta;#theta [deg]",200,0.,180.);

      hchi2 = new TH1D("hchi2","#chi^{2} of Straight Lines",100,0.,100.);
      hchi2sp = new TH2D("hchi2sp","#chi^{2} of Straight Lines Vs Number of Spacepoints",
                         100,0.,100.,100,0.,100.);
      

      // hcosang = new TH1D("hcosang","Cosine of Angle Formed by 2 Lines;cos(#alpha)",200,-1.,1.);
      // hdist = new TH1D("hdist","Distance between  2 Lines;s [mm]",200,0.,20.);

      // hcosangdist = new TH2D("hcosangdist",
      //                        "Correlation Angle-Distance;cos(#alpha);s [mm]",
      //                        100,-1.,1.,100,0.,20.);
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

      if( SigFlow->matchSig.size() > fNhitsCut )
         {
            std::cout<<"RecoModule::Analyze Too Many Points... quitting"<<std::endl;
            return flow;
         }

      AddSpacePoint( &SigFlow->matchSig );
      printf("RecoRun Analyze  Points: %d\n",fPointsArray.GetEntries());

      TracksFinder pattrec( &fPointsArray );
      //      pattrec.SetMaxIncreseAdapt(8.8);
      pattrec.SetMaxIncreseAdapt(28.);
      //pattrec.SetMaxIncreseAdapt(20.);
      pattrec.SetNpointsCut(29);
      //pattrec.SetPointsDistCut(4.1);
      pattrec.SetPointsDistCut(8.1);
      pattrec.AdaptiveFinder();
      AddTracks( pattrec.GetTrackVector() );
      printf("RecoRun Analyze  Tracks: %d\n",fTracksArray.GetEntries());

      int nlin = FitLines();
      std::cout<<"RecoRun Analyze lines count: "<<nlin<<std::endl;
      printf("RecoRun Analyze  Lines: %d\n",fLinesArray.GetEntries());

      // int nhel = FitHelix();
      // std::cout<<"RecoRun Analyze helices count: "<<nhel<<std::endl;
      // printf("RecoRun Analyze  Helices: %d\n",fHelixArray.GetEntries());

      analyzed_event->Reset();
      analyzed_event->SetEventNumber( age->counter );
      analyzed_event->SetTimeOfEvent( age->time );
      analyzed_event->SetEvent(&fPointsArray,&fLinesArray,&fHelixArray);
      // printf("RecoRun Analyze  Pattern Recognition Efficiency: %1.1f\n",
      //        analyzed_event->GetNumberOfPointsPerTrack());
      flow = new AgAnalysisFlow(flow, analyzed_event);
      EventTree->Fill();
      

      // Plot();

      //      if( do_plot ) ShowPlots();

      // fPointsArray.Clear("C");
      // fTracksArray.Clear("C");
      // fLinesArray.Clear("C");
      // fHelixArray.Clear("C");

      fHelixArray.Delete();
      fLinesArray.Delete();
      fTracksArray.Delete();
      fPointsArray.Delete();
      std::cout<<"\tRecoRun Analyze EVENT "<<age->counter<<" ANALYZED"<<std::endl;
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("RecoRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   void AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints )
   {
      int n = 0;
      //std::cout<<"RecoRun::AddSpacePoint  max time: "<<fSTR->GetMaxTime()<<" ns"<<std::endl;
      for( auto sp=spacepoints->begin(); sp!=spacepoints->end(); ++sp )
         {
            // STR: t->(r,phi)
            double time = sp->first.t;
            double r = fSTR->GetRadius( time ),
               correction = fSTR->GetAzimuth( time ),
               err = fSTR->GetdRdt( time );
            
            if( fTrace )
               {
                  double z = ( double(sp->second.idx) + 0.5 ) * _padpitch - _halflength;
                  std::cout<<"RecoRun::AddSpacePoint "<<n<<" aw: "<<sp->first.idx
                           <<" t: "<<time<<" r: "<<r
                           <<"\tcol: "<<sp->second.sec<<" row: "<<sp->second.idx<<" z: "<<z
                           <<" = "<<sp->second.z<<" err: "<<sp->second.errz<<std::endl;
                  //<<time<<" "<<r<<" "<<correction<<" "<<err<<std::endl;
               }

            new(fPointsArray[n]) TSpacePoint(sp->first.idx,
                                             sp->second.sec,sp->second.idx,
                                             time,
                                             r,correction,sp->second.z,
                                             err,0.,sp->second.errz,
                                             sp->first.height);
            ++n;
         }
      fPointsArray.Compress();
      fPointsArray.Sort();
      if( fTrace )
         std::cout<<"RecoRun::AddSpacePoint # entries: "<<fPointsArray.GetEntries()<<std::endl;
   }

   void AddTracks( const std::vector< std::list<int> >* track_vector )
   {
      int n=0;
      for( auto it=track_vector->begin(); it!=track_vector->end(); ++it)
         {
            new(fTracksArray[n]) TTrack(MagneticField);
            //std::cout<<"RecoRun::AddTracks Check Track # "<<n<<" "<<std::endl;
            for( auto ip=it->begin(); ip!=it->end(); ++ip)
               {
                  TSpacePoint* ap = (TSpacePoint*) fPointsArray.At(*ip);
                  ( (TTrack*)fTracksArray.ConstructedAt(n) ) ->AddPoint( ap );
                  //std::cout<<*ip<<", ";
                  //ap->Print("rphi");
                  hsprp->Fill( ap->GetPhi(), ap->GetR() );
               }
            //            std::cout<<"\n";
            ++n;
         }
      fTracksArray.Compress();
      assert(n==int(track_vector->size()));
      assert(fTracksArray.GetEntries()==int(track_vector->size()));
      if( fTrace )
         std::cout<<"RecoRun::AddTracks # entries: "<<fTracksArray.GetEntries()<<std::endl;
   }

   int FitLines()
   {
      int n=0;
      for(int it=0; it<fTracksArray.GetEntries(); ++it )
         {
            TTrack* at = (TTrack*) fTracksArray.At(it);
            //at->Print();
            new(fLinesArray[n]) TFitLine(*at);
            //( (TFitLine*)fLinesArray.ConstructedAt(n) )->SetChi2Cut( 15. );
            ( (TFitLine*)fLinesArray.ConstructedAt(n) )->SetChi2Cut( 25. );
            ( (TFitLine*)fLinesArray.ConstructedAt(n) )->SetPointsCut( 29 );
            ( (TFitLine*)fLinesArray.ConstructedAt(n) )->Fit();
            if( ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetStat() > 0 )
               {
                  double ndf= (double) ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetDoF();
                  if( ndf > 0. )
                     {
                        double chi2 = ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetChi2();
                        hchi2->Fill(chi2/ndf);
                        double nn = (double) ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetNumberOfPoints();
                        hchi2sp->Fill(chi2,nn);
                     }
               }
            if( ( (TFitLine*)fLinesArray.ConstructedAt(n) )->IsGood() )
               {
                  //( (TFitLine*)fLinesArray.ConstructedAt(n) )->CalculateResiduals();
                  ( (TFitLine*)fLinesArray.ConstructedAt(n) )->Print();
                  ++n;
               }
            else
               {
                  ( (TFitLine*)fLinesArray.ConstructedAt(n) ) -> Reason();
                  fLinesArray.RemoveAt(n);
               }
         }
      fLinesArray.Compress();
      return n;
   }

   int FitHelix()
   {
      int n=0;
      for(int it=0; it<fTracksArray.GetEntries(); ++it )
         {
            TTrack* at = (TTrack*) fTracksArray.At(it);
            //at->Print();
            new(fHelixArray[n]) TFitHelix(*at);
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetChi2ZCut( 30. );
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetDCut( 99999. );
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Fit();
            if( ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->IsGood() )
               {
                  // calculate momumentum
                  ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Momentum();
                  //( (TFitHelix*)fHelixArray.ConstructedAt(n) )->CalculateResiduals();
                  ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Print();
                  ++n;
               }
            else
               {
                  ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Reason();
                  fHelixArray.RemoveAt(n);
               }
         }
      fHelixArray.Compress();
      return n;
   }

   // void ShowPlots()
   // {
   //    creco->Clear();
   //    creco->Divide(2,2);

   //    TH1D* htemp1 = new TH1D("htemp1",";x [mm];y [mm]",1,-190.,190.);
   //    htemp1->SetStats(0);
   //    htemp1->SetMinimum(-190.); htemp1->SetMaximum(190.); 
   //    TGraph* gxy = new TGraph;
   //    gxy->SetName("x-y");
   //    gxy->SetTitle(";x [mm];y [mm]");
   //    gxy->SetMarkerColor(kBlue);
   //    gxy->SetMarkerStyle(43);
      
   //    TH1D* htemp2 = new TH1D("htemp2",";z [mm];r [mm]",1,-1200.,1200.);
   //    htemp2->SetStats(0);
   //    htemp2->SetMinimum(0.); htemp2->SetMaximum(190.);
   //    TGraph* gzr = new TGraph;
   //    gzr->SetName("z-r");
   //    gzr->SetTitle(";z [mm];r [mm]");
   //    gzr->SetMarkerColor(kBlue);
   //    gzr->SetMarkerStyle(43);

   //    int np=0;
   //    for(int isp=0; isp<fPointsArray.GetEntries(); ++isp)
   //       {
   //          TSpacePoint* ap = (TSpacePoint*) fPointsArray.At(isp);
   //          gxy->SetPoint(np,ap->GetX(),ap->GetY());
   //          gzr->SetPoint(np,ap->GetZ(),ap->GetR());
   //          // std::cout<<np<<"\t"
   //          //          <<ap->GetX()<<"\t"<<ap->GetY()<<"\t"<<ap->GetZ()<<"\t"
   //          //          <<ap->GetR()<<std::endl;
   //          ++np;
   //       }
      
   //    creco->cd(1);
   //    htemp1->Draw();
   //    if( np > 0 ) gxy->Draw("Psame");
   //    gPad->SetGrid();
   //    gPad->Modified();
   //    gPad->Update();

   //    creco->cd(2);
   //    htemp2->Draw();
   //    if( np > 0 ) gzr->Draw("Psame");
   //    gPad->SetGrid();
   //    gPad->Modified();
   //    gPad->Update();


   //    TH1D* htemp3 = new TH1D("htemp3",";x [mm];y [mm]",1,-190.,190.);
   //    htemp3->SetStats(0);
   //    htemp3->SetMinimum(-190.); htemp3->SetMaximum(190.); 
   //    TGraph* gxy_fit = new TGraph;
   //    gxy_fit->SetName("x-y_fit");
   //    gxy_fit->SetTitle(";x [mm];y [mm]");
   //    gxy_fit->SetMarkerColor(kRed);
   //    gxy_fit->SetMarkerStyle(43);
      
   //    TH1D* htemp4 = new TH1D("htemp4",";z [mm];r [mm]",1,-1200.,1200.);
   //    htemp4->SetStats(0);
   //    htemp4->SetMinimum(0.); htemp4->SetMaximum(190.);
   //    TGraph* gzr_fit = new TGraph;
   //    gzr_fit->SetName("z-r_fit");
   //    gzr_fit->SetTitle(";z [mm];r [mm]");
   //    gzr_fit->SetMarkerColor(kRed);
   //    gzr_fit->SetMarkerStyle(43);

   //    np=0;
   //    for( int it = 0; it<fLinesArray.GetEntries(); ++it )
   //       {
   //          TFitLine* aLine = (TFitLine*) fLinesArray.At(it);
   //          for(int isp=0; isp<aLine->GetPointsArray()->GetEntries(); ++isp)
   //             {
   //                TSpacePoint* ap = (TSpacePoint*) aLine->GetPointsArray()->At(isp);
   //                gxy_fit->SetPoint(np,ap->GetX(),ap->GetY());
   //                gzr_fit->SetPoint(np,ap->GetZ(),ap->GetR());
   //                // std::cout<<np<<"\t"
   //                //          <<ap->GetX()<<"\t"<<ap->GetY()<<"\t"<<ap->GetZ()<<"\t"
   //                //          <<ap->GetR()<<std::endl;
   //                ++np;
   //             }
   //       }      

   //    creco->cd(3);
   //    htemp3->Draw();
   //    if( np > 0 ) gxy_fit->Draw("Psame");
   //    gPad->SetGrid();
   //    gPad->Modified();
   //    gPad->Update();

   //    creco->cd(4);
   //    htemp4->Draw();
   //    if( np > 0 ) gzr_fit->Draw("Psame");
   //    gPad->SetGrid();
   //    gPad->Modified();
   //    gPad->Update();
   // }

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
            double* slope = aLine->GetU();
            TVector3 U(slope);
            //            TVector3 U(aLine->GetU());
            if( fTrace && 0 )
               std::cout<<"RecoRun::Plot Line  dir phi: "
                        <<U.Phi()*TMath::RadToDeg()
                        <<" deg  theta dir: "
                        <<U.Theta()*TMath::RadToDeg()<<" deg"<<std::endl;
            hphi->Fill(U.Phi()*TMath::RadToDeg());
            htheta->Fill(U.Theta()*TMath::RadToDeg());
            delete slope;
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
