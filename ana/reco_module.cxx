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
#include "TFitVertex.hh"

#include "TStoreEvent.hh"


class RecoRunFlags
{
public:
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;
   bool fFieldMap=false;

public:
   RecoRunFlags() // ctor
   { }

   ~RecoRunFlags() // dtor
   { }
};

class RecoRun: public TARunObject
{
public:
   bool do_plot = false;
   bool fTrace = false;
   //bool fTrace = true;

   RecoRunFlags* fFlags;

private:
   TClonesArray fPointsArray;
   TClonesArray fTracksArray;
   TClonesArray fLinesArray;
   TClonesArray fHelixArray;

   LookUpTable* fSTR;

   // useful histos
   TH2D* hsprp; // spacepoints in found tracks

   TH1D* hchi2; // chi^2 of line fit
   TH2D* hchi2sp; // chi^2 of line fit Vs # of spacepoints

   double MagneticField;
   unsigned fNhitsCut;
   unsigned fNspacepointsCut;

public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo, RecoRunFlags* f): TARunObject(runinfo),
                                                 fFlags(f),
                                                 fPointsArray("TSpacePoint",1000),
                                                 fTracksArray("TTrack",50),
                                                 fLinesArray("TFitLine",50),
                                                 fHelixArray("TFitHelix",50),
                                                 MagneticField(1.e-4),
                                                 fNhitsCut(5000),fNspacepointsCut(29)
   {
      printf("RecoRun::ctor!\n");
   }

   ~RecoRun()
   {
      printf("RecoRun::dtor!\n");
      delete fSTR;     
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      if( fFlags->fFieldMap )
         {
            fSTR = new LookUpTable(_co2frac);
            MagneticField = 1.;
         }
      else
         {
            if( _MagneticField == 1. )
               {
                  MagneticField = _MagneticField;
                  fSTR = new LookUpTable(_co2frac,_MagneticField);
               }
            else
               fSTR = new LookUpTable(runinfo->fRunNo);
         }
      std::cout<<"RecoRun reco in B = "<<MagneticField<<" T"<<std::endl;
  
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);

      gDirectory->mkdir("reco")->cd();
      hsprp = new TH2D("hsprp","Spacepoints in Tracks;#phi [deg];r [mm]",
                       180,0.,TMath::TwoPi(),200,0.,175.);
      hchi2 = new TH1D("hchi2","#chi^{2} of Straight Lines",100,0.,100.);
      hchi2sp = new TH2D("hchi2sp","#chi^{2} of Straight Lines Vs Number of Spacepoints",
                         100,0.,100.,100,0.,100.);
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
      
      
      if (fFlags->fTimeCut)
      {
        if (age->time<fFlags->start_time)
          return flow;
        if (age->time>fFlags->stop_time)
          return flow;
      }
      
      if (fFlags->fEventRangeCut)
      {
         if (age->counter<fFlags->start_event)
            return flow;
         if (age->counter>fFlags->stop_event)
            return flow;
      }

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
      //      pattrec.SetSmallRadCut(125.);
      pattrec.SetSmallRadCut(135.);
      pattrec.SetMaxIncreseAdapt(45.1);
      pattrec.SetNpointsCut(fNspacepointsCut);
      //pattrec.SetPointsDistCut(4.1);
      pattrec.SetPointsDistCut(8.1);
      pattrec.AdaptiveFinder();
      AddTracks( pattrec.GetTrackVector() );
      printf("RecoRun Analyze  Tracks: %d\n",fTracksArray.GetEntries());

      int nlin = FitLines();
      std::cout<<"RecoRun Analyze lines count: "<<nlin<<std::endl;
      //      printf("RecoRun Analyze  Lines: %d\n",fLinesArray.GetEntries());

      int nhel = FitHelix();
      std::cout<<"RecoRun Analyze helices count: "<<nhel<<std::endl;
      //      printf("RecoRun Analyze  Helices: %d\n",fHelixArray.GetEntries());

      TFitVertex theVertex(age->counter);
      theVertex.SetChi2Cut(30.);
      int status = RecVertex( &theVertex );
      std::cout<<"RecoRun Analyze Vertexing Status: "<<status<<std::endl;

      analyzed_event->Reset();
      analyzed_event->SetEventNumber( age->counter );
      analyzed_event->SetTimeOfEvent( age->time );
      analyzed_event->SetEvent(&fPointsArray,&fLinesArray,&fHelixArray);
      analyzed_event->SetVertexStatus( status );
      if( status > 0 )
         {
            analyzed_event->SetVertex(*(theVertex.GetVertex()));
            analyzed_event->SetUsedHelices(theVertex.GetHelixStack());
            theVertex.Print("rphi");
         }
      else
         std::cout<<"RecoRun Analyze no vertex found"<<std::endl;


      flow = new AgAnalysisFlow(flow, analyzed_event);
      EventTree->Fill();
 
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
            // STR
            const double time = sp->first.t, zed = sp->second.z;
            double r,correction,err;
            if( fFlags->fFieldMap )// STR: (t,z)->(r,phi)
               {
                  r = fSTR->GetRadius( time , zed );
                  correction = fSTR->GetAzimuth( time , zed );
                  err = fSTR->GetdRdt( time , zed );
               }
            else // STR: t->(r,phi)
               {
                  r = fSTR->GetRadius( time );
                  correction = fSTR->GetAzimuth( time );
                  err = fSTR->GetdRdt( time );
               }
      
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
                                             r,correction,zed,
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
            ( (TFitLine*)fLinesArray.ConstructedAt(n) )->SetPointsCut( fNspacepointsCut );
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
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetChi2RCut( 30. );
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetChi2ZCut( 30. );
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetChi2RMin(1.e-2);
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetChi2ZMin(1.e-2);
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetDCut( 99999. );
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Fit();

            if( ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->IsGood() )
               {
                  // calculate momumentum
                  double pt = ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Momentum();
                  //( (TFitHelix*)fHelixArray.ConstructedAt(n) )->CalculateResiduals();
                  ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Print();
                  std::cout<<"RecoRun::FitHelix()  hel # "<<n
                           <<" p_T = "<<pt
                           <<" MeV/c in B = "<<( (TFitHelix*)fHelixArray.ConstructedAt(n) )->GetMagneticField()<<" T"<<std::endl;
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

   int RecVertex(TFitVertex* Vertex)
   {
      int Nhelices = 0;
      for( int n = 0; n<fHelixArray.GetEntries(); ++n )
         {
            TFitHelix* hel = (TFitHelix*)fHelixArray.ConstructedAt(n);
            if( hel->IsGood() )
               {
                  Vertex->AddHelix(hel);
                  ++Nhelices;
               }
         }
      std::cout<<"RecoRun::RecVertex(  )   # helices: "<<fHelixArray.GetEntries()<<"   # good helices: "<<Nhelices<<std::endl;
      // reconstruct the vertex
      int sv = -2;
      if( Nhelices )// find the vertex!
         {
            sv = Vertex->Calculate();
            std::cout<<"RecoRun::RecVertex(  )   # used helices: "<<Vertex->GetNumberOfHelices()<<std::endl;
            //           Vertex->Print();
         }
      return sv;
   }
};

class RecoModuleFactory: public TAFactory
{
public:
   RecoRunFlags fFlags;
public:
   void Help()
   {
      printf("RecoModuleFactory::Help\n");
      printf("\t---usetimerange 123.4 567.8\t\tLimit reconstruction to a time range\n");
      printf("\t---useeventrange 123 456\t\tLimit reconstruction to an event range\n");
      printf("\t---Bmap xx\t\tSet STR using Babcock Map\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("RecoModuleFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
         if( args[i] == "--usetimerange" )
            {
               fFlags.fTimeCut=true;
               i++;
               fFlags.start_time=atof(args[i].c_str());
               i++;
               fFlags.stop_time=atof(args[i].c_str());
               printf("Using time range for reconstruction: ");
               printf("%f - %fs\n",fFlags.start_time,fFlags.stop_time);
            }
         if( args[i] == "--useeventrange" )
            {
               fFlags.fEventRangeCut=true;
               i++;
               fFlags.start_event=atoi(args[i].c_str());
               i++;
               fFlags.stop_event=atoi(args[i].c_str());
               printf("Using event range for reconstruction: ");
               printf("Analyse from (and including) %d to %d\n",fFlags.start_event,fFlags.stop_event);
            }
         if( args[i] == "--Bmap" )
            {
               fFlags.fFieldMap = true;
               printf("Magnetic Field Map for reconstruction");
            }
      }
   }

   void Finish()
   {
      printf("RecoModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("RecoModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new RecoRun(runinfo,&fFlags);
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
