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
#include "TSeqCollection.h"
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TGraph.h>

#include "TPCconstants.hh"
#include "LookUpTable.hh"
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include "AdaptiveFinder.hh"
#include "NeuralFinder.hh"
#include "TTrack.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"
#include "TFitVertex.hh"

#include "TStoreEvent.hh"

#include "AnalysisTimer.h"
#include "AnaSettings.h"

class RecoRunFlags
{
public:
   bool fRecOff = false; //Turn reconstruction off
   bool fDiag=false;
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;
   double fMagneticField=-1.;
   bool fFieldMap=true;

   enum finderChoice { base, adaptive, neural };
   finderChoice finder = adaptive;

   AnaSettings* ana_settings=0;

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
   std::vector<TSpacePoint*> fPointsArray;
   std::vector<TTrack*> fTracksArray;
   std::vector<TFitLine*> fLinesArray;
   std::vector<TFitHelix*> fHelixArray;

   LookUpTable* fSTR;

   // useful histos
   TH2D* hsprp; // spacepoints in found tracks

   TH1D* hchi2; // chi^2 of line fit
   TH2D* hchi2sp; // chi^2 of line fit Vs # of spacepoints

   double MagneticField;

   // general TracksFinder parameters, also used by other finders
   unsigned fNhitsCut;
   unsigned fNspacepointsCut;
   double fPointsDistCut;
   double fSeedRadCut;
   double fSmallRadCut;

   // AdaptiveFinder
   double fMaxIncreseAdapt;
   double fLastPointRadCut;

   // NeuralFinder
   // V_kl = 0.5 * [1 + tanh(c/Temp \sum(T_kln*V_ln) - alpha/Temp{\sum(V_kn) + \sum(V_ml)} + B/Temp)]
   // NN parameters             // ALEPH values (see DOI 10.1016/0010-4655(91)90048-P)
   double fLambda;              // 5.
   double fAlpha;               // 5.
   double fB;                   // 0.2
   double fTemp;                // 1.
   double fC;                   // 10.
   double fMu;                  // 2.
   double fCosCut;              // 0.9     larger kinks between neurons set T value to zero
   double fVThres;              // 0.9     V value above which a neuron is considered active

   double fDNormXY;             // normalization for XY distance and
   double fDNormZ;              // Z distance, different to weight the influence of gaps differently
                                // no good reason for these values
   double fTscale;              // fudge factor to bring T values into range [0,1],
                                // probably has to be changed with other parameters...
   int fMaxIt;                  // number of iterations
   double fItThres;             // threshold defining convergence

   // TFitLine
   double fLineChi2Cut;
   double fLineChi2Min;

   // TFitHelix
   double fHelChi2RCut;
   double fHelChi2ZCut;
   double fHelChi2RMin;
   double fHelChi2ZMin;
   double fHelDcut;
   double fVtxChi2Cut;

   bool diagnostics;

   // Reasons for pattrec to fail:
   int track_not_advancing;
   int points_cut;
   int rad_cut;

public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo, RecoRunFlags* f): TARunObject(runinfo),
                                                 fFlags(f)
   {
      printf("RecoRun::ctor!\n");
      MagneticField = fFlags->fMagneticField;
      diagnostics=fFlags->fDiag; // dis/en-able histogramming

      assert( fFlags->ana_settings );
      fNhitsCut = fFlags->ana_settings->GetInt("RecoModule","NhitsCut");
      fNspacepointsCut = fFlags->ana_settings->GetInt("RecoModule","NpointsCut");
      fPointsDistCut = fFlags->ana_settings->GetDouble("RecoModule","PointsDistCut");
      fMaxIncreseAdapt = fFlags->ana_settings->GetDouble("RecoModule","MaxIncreseAdapt");
      fSeedRadCut = fFlags->ana_settings->GetDouble("RecoModule","SeedRadCut");
      fSmallRadCut = fFlags->ana_settings->GetDouble("RecoModule","SmallRadCut");
      fLastPointRadCut = fFlags->ana_settings->GetDouble("RecoModule","LastPointRadCut");
      fLineChi2Cut = fFlags->ana_settings->GetDouble("RecoModule","LineChi2Cut");
      fLineChi2Min = fFlags->ana_settings->GetDouble("RecoModule","LineChi2Min");;
      fHelChi2RCut = fFlags->ana_settings->GetDouble("RecoModule","HelChi2RCut");
      fHelChi2ZCut = fFlags->ana_settings->GetDouble("RecoModule","HelChi2ZCut");
      fHelChi2RMin = fFlags->ana_settings->GetDouble("RecoModule","HelChi2RMin");
      fHelChi2ZMin = fFlags->ana_settings->GetDouble("RecoModule","HelChi2ZMin");
      fHelDcut = fFlags->ana_settings->GetDouble("RecoModule","HelDcut");
      fVtxChi2Cut = fFlags->ana_settings->GetDouble("RecoModule","VtxChi2Cut");

      fLambda = fFlags->ana_settings->GetDouble("RecoModule","Lambda_NN");
      fAlpha = fFlags->ana_settings->GetDouble("RecoModule","Alpha_NN");
      fB = fFlags->ana_settings->GetDouble("RecoModule","B_NN");
      fTemp = fFlags->ana_settings->GetDouble("RecoModule","T_NN");
      fC = fFlags->ana_settings->GetDouble("RecoModule","C_NN");
      fMu = fFlags->ana_settings->GetDouble("RecoModule","Mu_NN");
      fCosCut = fFlags->ana_settings->GetDouble("RecoModule","CosCut_NN");
      fVThres = fFlags->ana_settings->GetDouble("RecoModule","VThres_NN");

      fDNormXY = fFlags->ana_settings->GetDouble("RecoModule","DNormXY_NN");
      fDNormZ = fFlags->ana_settings->GetDouble("RecoModule","DNormZ_NN");

      fTscale = fFlags->ana_settings->GetDouble("RecoModule","TScale_NN");
      fMaxIt = fFlags->ana_settings->GetInt("RecoModule","MaxIt_NN");
      fItThres = fFlags->ana_settings->GetDouble("RecoModule","ItThres_NN");
   }

   ~RecoRun()
   {
      printf("RecoRun::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      if( fFlags->fFieldMap )
         {
            if( MagneticField < 0. )
               {
                  fSTR = new LookUpTable(_co2frac); // field map version (simulation)
                  MagneticField = 1.;
               }
            else
               fSTR = new LookUpTable(_co2frac, MagneticField); // uniform field version (simulation)
         }
      else
         {
            fSTR = new LookUpTable(runinfo->fRunNo); // no field version (data)
            MagneticField = 1.e-4;
         }
      std::cout<<"RecoRun reco in B = "<<MagneticField<<" T"<<std::endl;

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);

      if( diagnostics )
         {
            gDirectory->mkdir("reco")->cd();
            hsprp = new TH2D("hsprp","Spacepoints in Tracks;#phi [deg];r [mm]",
                             180,0.,TMath::TwoPi(),200,0.,175.);
            hchi2 = new TH1D("hchi2","#chi^{2} of Straight Lines",100,0.,100.);
            hchi2sp = new TH2D("hchi2sp","#chi^{2} of Straight Lines Vs Number of Spacepoints",
                               100,0.,100.,100,0.,100.);
         }

      track_not_advancing = 0;
      points_cut = 0;
      rad_cut = 0;
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("RecoRun::EndRun, run %d\n", runinfo->fRunNo);
      std::cout<<"RecoRun::EndRun pattrec failed\ttrack not advanving: "<<track_not_advancing
               <<"\tpoints cut: "<<points_cut
               <<"\tradius cut: "<<rad_cut<<std::endl;
      if (analyzed_event) delete analyzed_event;
      if (fSTR) delete fSTR;
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
      if( fTrace )
         printf("RecoRun::Analyze, run %d\n", runinfo->fRunNo);

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      AgEvent* age = ef->fEvent;


      if( fFlags->fRecOff )
         {
            analyzed_event->Reset();
            analyzed_event->SetEventNumber( age->counter );
            analyzed_event->SetTimeOfEvent( age->time );
            EventTree->Fill();
            flow = new AgAnalysisFlow(flow, analyzed_event);
            return flow;
         }


      if (fFlags->fTimeCut)
         {
            if (age->time<fFlags->start_time)
               {
                  analyzed_event->Reset();
                  analyzed_event->SetEventNumber( age->counter );
                  analyzed_event->SetTimeOfEvent( age->time );
                  EventTree->Fill();
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
            if (age->time>fFlags->stop_time)
               {
                  analyzed_event->Reset();
                  analyzed_event->SetEventNumber( age->counter );
                  analyzed_event->SetTimeOfEvent( age->time );
                  EventTree->Fill();
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
         }

      if (fFlags->fEventRangeCut)
         {
            if (age->counter<fFlags->start_event)
               {
                  analyzed_event->Reset();
                  analyzed_event->SetEventNumber( age->counter );
                  analyzed_event->SetTimeOfEvent( age->time );
                  EventTree->Fill();
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
            if (age->counter>fFlags->stop_event)
               {
                  analyzed_event->Reset();
                  analyzed_event->SetEventNumber( age->counter );
                  analyzed_event->SetTimeOfEvent( age->time );
                  EventTree->Fill();
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
         }

      std::cout<<"RecoRun::Analyze Event # "<<age->counter<<std::endl;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) return flow;
      if( fTrace )
         {
            printf("RecoModule::Analyze, AW # signals %d\n", int(SigFlow->awSig.size()));
            printf("RecoModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig.size()));
            printf("RecoModule::Analyze, SP # %d\n", int(SigFlow->matchSig.size()));
         }

      if( SigFlow->matchSig.size() > fNhitsCut )
         {
            std::cout<<"RecoRun::Analyze Too Many Points... quitting"<<std::endl;
#ifdef _TIME_ANALYSIS_
            if (TimeModules) flow=new AgAnalysisReportFlow(flow,"reco_module(too many hits)");
#endif
            return flow;
         }

      AddSpacePoint( &SigFlow->matchSig );

      TracksFinder *pattrec;
      switch(fFlags->finder){
      case RecoRunFlags::adaptive:
         pattrec = new AdaptiveFinder( &fPointsArray );
         ((AdaptiveFinder*)pattrec)->SetMaxIncreseAdapt(fMaxIncreseAdapt);
         break;
      case RecoRunFlags::neural:
         pattrec = new NeuralFinder( &fPointsArray );
         ((NeuralFinder*)pattrec)->SetLambda(fLambda);
         ((NeuralFinder*)pattrec)->SetAlpha(fAlpha);
         ((NeuralFinder*)pattrec)->SetB(fB);
         ((NeuralFinder*)pattrec)->SetTemp(fTemp);
         ((NeuralFinder*)pattrec)->SetC(fC);
         ((NeuralFinder*)pattrec)->SetMu(fMu);
         ((NeuralFinder*)pattrec)->SetCosCut(fCosCut);
         ((NeuralFinder*)pattrec)->SetVThres(fVThres);
         ((NeuralFinder*)pattrec)->SetDNormXY(fDNormXY);
         ((NeuralFinder*)pattrec)->SetDNormZ(fDNormZ);
         ((NeuralFinder*)pattrec)->SetTscale(fTscale);
         ((NeuralFinder*)pattrec)->SetMaxIt(fMaxIt);
         ((NeuralFinder*)pattrec)->SetItThres(fItThres);
         break;
      case RecoRunFlags::base:
      default: pattrec = new TracksFinder( &fPointsArray ); break;
      }

      pattrec->SetPointsDistCut(fPointsDistCut);
      pattrec->SetNpointsCut(fNspacepointsCut);
      pattrec->SetSeedRadCut(fSeedRadCut);

      pattrec->RecTracks();
      int tk,npc,rc;
      pattrec->GetReasons(tk,npc,rc);
      track_not_advancing += tk;
      points_cut += npc;
      rad_cut += rc;
#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,
                                                     {"reco_module(AdaptiveFinder)","Points in track"," # Tracks"},
                                                     {(double)fPointsArray.size(),(double)fTracksArray.size()});
#endif

      AddTracks( pattrec->GetTrackVector() );
      delete pattrec;

      if( MagneticField == 0. )
         {
            int nlin = FitLines();
            std::cout<<"RecoRun Analyze lines count: "<<nlin<<std::endl;
         }

      int nhel = FitHelix();
      std::cout<<"RecoRun Analyze helices count: "<<nhel<<std::endl;

      TFitVertex theVertex(age->counter);
      theVertex.SetChi2Cut( fVtxChi2Cut );
      int status = RecVertex( &theVertex );
      std::cout<<"RecoRun Analyze Vertexing Status: "<<status<<std::endl;

      analyzed_event->Reset();

      AgBarEventFlow *bf = flow->Find<AgBarEventFlow>();

      //If have barrel scintilator, add to TStoreEvent
      if (bf)
         {
            //bf->BarEvent->Print();
            analyzed_event->AddBarrelHits(bf->BarEvent);
         }

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

      fHelixArray.clear();
      fLinesArray.clear();
      fTracksArray.clear(); 
      fPointsArray.clear(); 
      std::cout<<"\tRecoRun Analyze EVENT "<<age->counter<<" ANALYZED"<<std::endl;
#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"reco_module");
#endif
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
            // STR: (t,z)->(r,phi)
            const double time = sp->first.t, zed = sp->second.z;
            double r = fSTR->GetRadius( time , zed ),
               correction = fSTR->GetAzimuth( time , zed ),
               err = fSTR->GetdRdt( time , zed );

            if( fTrace )
               {
                  double z = ( double(sp->second.idx) + 0.5 ) * _padpitch - _halflength;
                  std::cout<<"RecoRun::AddSpacePoint "<<n<<" aw: "<<sp->first.idx
                           <<" t: "<<time<<" r: "<<r
                           <<"\tcol: "<<sp->second.sec<<" row: "<<sp->second.idx<<" z: "<<z
                           <<" ~ "<<sp->second.z<<" err: "<<sp->second.errz<<std::endl;
                  //<<time<<" "<<r<<" "<<correction<<" "<<err<<std::endl;
               }
            TSpacePoint* point=new TSpacePoint();
            point->Setup(sp->first.idx,
                         sp->second.sec,sp->second.idx,
                         time,
                         r,correction,zed,
                         err,0.,sp->second.errz,
                         sp->first.height);
            fPointsArray.push_back(point);
            ++n;
         }
      //fPointsArray.Compress();
      TSeqCollection::QSort((TObject**)fPointsArray.data(),0,fPointsArray.size());
      if( fTrace )
         std::cout<<"RecoRun::AddSpacePoint # entries: "<<fPointsArray.size()<<std::endl;
   }


   void AddTracks( const std::vector<track_t>* track_vector )
   {
      int n=0;
      for( auto it=track_vector->begin(); it!=track_vector->end(); ++it)
         {
            TTrack* thetrack=new TTrack();
            thetrack->Clear();
            thetrack->SetMagneticField(MagneticField);
            //std::cout<<"RecoRun::AddTracks Check Track # "<<n<<" "<<std::endl;
            for( auto ip=it->begin(); ip!=it->end(); ++ip)
               {
                  TSpacePoint* ap = (TSpacePoint*) fPointsArray.at(*ip);
                  thetrack->AddPoint( ap );
                  //std::cout<<*ip<<", ";
                  //ap->Print("rphi");
                  if( diagnostics )
                     hsprp->Fill( ap->GetPhi(), ap->GetR() );
               }
            fTracksArray.push_back(thetrack);
            //            std::cout<<"\n";
            ++n;
         }
      //fTracksArray.Compress();
      assert(n==int(track_vector->size()));
      assert(fTracksArray.size()==track_vector->size());
      if( fTrace )
         std::cout<<"RecoRun::AddTracks # entries: "<<fTracksArray.size()<<std::endl;
   }

   int FitLines()
   {
      int n=0;
      int ntracks=fTracksArray.size();
      for(int it=0; it<ntracks; ++it )
         {
            TTrack* at = fTracksArray.at(it);
            //at->Print();
            TFitLine* line=new TFitLine(*at); //Copy constructor
            
            line->SetChi2Cut( fLineChi2Cut );
            line->SetChi2Min( fLineChi2Min );
            line->SetPointsCut( fNspacepointsCut );
            line->Fit();
            if( line->GetStat() > 0 )
               {
                  double ndf= (double) line->GetDoF();
                  if( ndf > 0. && diagnostics )
                     {
                        double chi2 = line->GetChi2();
                        double nn = (double) line->GetNumberOfPoints();
                        hchi2sp->Fill(chi2,nn);
                        hchi2->Fill(chi2/ndf);
                     }
                  line->CalculateResiduals();
               }
            if( line->IsGood() )
               {
                  if( fTrace )
                     line->Print();
                  ++n;
                  fLinesArray.push_back(line);
               }
            else
               {
                  line-> Reason();
                  delete line;
               }
         }
      //fLinesArray.Compress();
      return n;
   }

   int FitHelix()
   {
      int n=0;
      int ntracks=fTracksArray.size();
      for(int it=0; it<ntracks; ++it )
         {
            TTrack* at = (TTrack*) fTracksArray.at(it);
            //at->Print();
            TFitHelix* helix = new TFitHelix(*at); //Copy constructor

            helix->SetChi2ZCut( fHelChi2ZCut );
            helix->SetChi2RCut( fHelChi2RCut );
            helix->SetChi2RMin( fHelChi2RMin );
            helix->SetChi2ZMin( fHelChi2ZMin );
            helix->SetDCut( fHelDcut );
            helix->Fit();

            if( helix-> GetStatR() > 0 &&
                helix-> GetStatZ() > 0 )
               helix->CalculateResiduals();

            if( helix->IsGood() )
               {
                  // calculate momumentum
                  double pt = helix->Momentum();
                  if( fTrace )
                     {
                        helix->Print();
                        std::cout<<"RecoRun::FitHelix()  hel # "<<n
                                 <<" p_T = "<<pt
                                 <<" MeV/c in B = "<<helix->GetMagneticField()
                                 <<" T"<<std::endl;
                     }
            fHelixArray.push_back(helix);
                  ++n;
               }
            else
               {
                  helix->Reason();
                  helix->Clear();
                  delete helix;
               }
         }
      //fHelixArray.Compress();
      return n;
   }

   int RecVertex(TFitVertex* Vertex)
   {
      int Nhelices = 0;
      int nhel=fHelixArray.size();
      for( int n = 0; n<nhel; ++n )
         {
            TFitHelix* hel = (TFitHelix*)fHelixArray.at(n);
            if( hel->IsGood() )
               {
                  Vertex->AddHelix(hel);
                  ++Nhelices;
               }
         }
      if( fTrace )
         std::cout<<"RecoRun::RecVertex(  )   # helices: "<<nhel<<"   # good helices: "<<Nhelices<<std::endl;
      // reconstruct the vertex
      int sv = -2;
      if( Nhelices )// find the vertex!
         {
            sv = Vertex->Calculate();
            if( fTrace )
               std::cout<<"RecoRun::RecVertex(  )   # used helices: "<<Vertex->GetNumberOfHelices()<<std::endl;
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
      printf("\t---Bmap xx\t\tSet STR using Babcock Map OBSOLETE!!! This is now default\n");
      printf("--loadcalib\t\t Load calibration STR file made by this analysis\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
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
         if( args[i] == "--Bfield" )
            {
               fFlags.fMagneticField = atof(args[++i].c_str());
               printf("Magnetic Field (incompatible with --loadcalib)\n");
            }
         if (args[i] == "--loadcalib")
            {
               fFlags.fFieldMap = false;
               printf("Attempting to use calibrated timing for reconstruction\n");
            }
         if (args[i] == "--recoff")
            fFlags.fRecOff = true;
         if( args[i] == "--diag" )
            fFlags.fDiag = true;
         if( args[i] == "--finder" ){
            std::string findString = args[++i];
            if(findString == "base") fFlags.finder = RecoRunFlags::base;
            else if(findString == "neural") fFlags.finder = RecoRunFlags::neural;
            else if(findString == "adaptive") fFlags.finder = RecoRunFlags::adaptive;
            else {
               std::cerr << "Unknown track finder mode \"" << findString << "\", using adaptive" << std::endl;
            }
         }

         if( args[i] == "--anasettings" ) json=args[++i];
      }

      fFlags.ana_settings=new AnaSettings(json.Data());
      fFlags.ana_settings->Print();
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
