#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <TH1D.h>
#include <TH2D.h>

#include "TStoreEvent.hh"
#include "TStoreHelix.hh"
#include "TSpacePoint.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"
#include "TFitVertex.hh"

#include "AnalysisTimer.h"

class CosmFlags
{
public:
   bool enabled=false;
   double fMagneticField=1.;
};

class CosmModule: public TARunObject
{
public:

   CosmFlags* fFlags;

private:

   double MagneticField;

   TClonesArray fLinesArray;

   TH1D* hchi2R;
   TH1D* hchi2Z;

   TH1D* hRes2;
   TH1D* hResV;
   TH1D* hResMomProj;
   TH1D* hResMomDiff;
  
   TH2D* hResMom;
  
   TH1D* hDCAeq2;
   TH1D* hDCAgr2;
  
   TH1D* hAngeq2;
   TH1D* hAnggr2;

   TH2D* hAngDCAeq2;
   TH2D* hAngDCAgr2;

   TH1D* hcosaw;
   TH2D* hcospad;
   TH1D* hRes2min;

   double temp;
   TH1D* hpois;

   padmap* pmap;

public:
   CosmModule(TARunInfo* runinfo, CosmFlags* f): TARunObject(runinfo),
                                                 fFlags(f),
                                                 fLinesArray("TFitLine",50)
   {
      printf("CosmModule::ctor!\n");
      MagneticField = fabs(fFlags->fMagneticField);
   }

   ~CosmModule()
   {
      printf("CosmModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if( fFlags->enabled )
         {     
            printf("CosmModule::BeginRun, run %d, file %s\n", 
                   runinfo->fRunNo, runinfo->fFileName.c_str());

            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
            gDirectory->mkdir("cosmics")->cd();

            hchi2R = new TH1D("hchi2R","Hel #chi^{2}_{R}",200,0.,100.);
            hchi2Z = new TH1D("hchi2Z","Hel #chi^{2}_{Z}",500,0.,50.);
	 
            hRes2 = new TH1D("hRes2","Residuals Squared Divide by Number of Spacepoints;#delta [mm^{2}]",1000,0.,500.);
            hResV = new TH1D("hResV","Residuals Point by Point; [mm]",200,0.,50.);
            hResMomProj = new TH1D("hResMomProj","Projection of the Residual Vector onto the Momentum Vector",
                                   1000,-1.e6,1.e6);
            hResMomDiff = new TH1D("hResMomDiff","Magnitude of the Difference between the Residual Vector and the Momentum Vector",
                                   1000,0.,1.e6);

            hResMom = new TH2D("hResMom","Residuals Squared vs Momentum;#delta [mm^{2}];|p| [MeV/c]",
                               500,0.,500.,500,0.,2000.);
	 
            hDCAeq2 = new TH1D("hDCAeq2","Distance of Closest Approach between Helices in =2-tracks Events;DCA [mm]",
                               1000,0.,1.e-3);
            hDCAgr2 = new TH1D("hDCAgr2","Distance of Closest Approach between Helices in >2-tracks Events;DCA [mm]",
                               1000,0.,1.e-3);
	 
            hAngeq2 = new TH1D("hAngeq2","Cosine of the Angle formed by Two Helices in =2-tracks Events;cos(angle)",
                               1000,-1.,1.);
            hAnggr2 = new TH1D("hAnggr2","Cosine of the Angle formed by Two Helices in >2-tracks Events;cos(angle)",
                               1000,-1.,1.);

            hAngDCAeq2 = new TH2D("hAngDCAeq2","DCA and Cosine of Angle between Helices in =2-tracks Events;cos(angle);DCA [mm]",
                                  100,-1.,1.,100,0.,0.1);
            hAngDCAgr2 = new TH2D("hAngDCAgr2","DCA and Cosine of Angle between Helices in >2-tracks Events;cos(angle);DCA [mm]",
                                  100,-1.,1.,100,0.,0.1);

            hcosaw = new TH1D("hcosaw","Occupancy per AW due to cosmics",256,0.,256.);
            hcosaw->SetMinimum(0.);
            hcospad = new TH2D("hcospad","Occupancy per PAD due to cosmics;N",576,0.,576.,32,0.,32.);
            hRes2min = new TH1D("hRes2min","Minimum Residuals Squared Divide by Number of Spacepoints from 2 Helices;#delta [mm^{2}]",1000,0.,1000.);

            // cosmic time distribution
            hpois = new TH1D("hpois","Delta t between cosmics;#Delta t [ms]",2000,0.,2000.);
            temp = 0.;            

            pmap = new padmap;
         }
   } 

   void EndRun(TARunInfo* runinfo)
   {
      printf("CosmModule::EndRun, run %d\n", runinfo->fRunNo);
   }
  
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( !fFlags->enabled ) return flow;
     
      AgAnalysisFlow* af = flow->Find<AgAnalysisFlow>();
      if( !af ) return flow;
      TStoreEvent* e = af->fEvent;
      if( !e ) return flow;

      HelixAnalysis( e );
      CombineHelix( e );

      fLinesArray.Delete();

#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"cosm_module");
#endif
      return flow;
   }

   int HelixAnalysis(TStoreEvent* e)
   {
      const TObjArray* helices = e->GetHelixArray();
      int nHelices = helices->GetEntriesFast();
      std::cout<<"CosmModule::HelixAnalysis Event # "<<e->GetEventNumber()
               <<" Number of Helices: "<<nHelices<<std::endl;
      if(nHelices<2) return 1;
      
      TFitVertex* c = new TFitVertex(-e->GetEventNumber());
      for( int i=0; i<nHelices; ++i )
         {
            TStoreHelix* h = (TStoreHelix*) helices->At(i);
   
            hchi2R->Fill(h->GetRchi2());
            hchi2Z->Fill(h->GetZchi2());

            double nPoints = double(h->GetNumberOfPoints());
	 
            TVector3 p = h->GetMomentumV();
            TVector3 r( h->GetResidual() );
            r *= 1./nPoints;
            hResMomProj->Fill( r.Dot(p) );
            hResMomDiff->Fill( (p-r).Mag() );

            std::vector<double> resv = h->GetResidualsVector();
            for( double& d: resv )
               hResV->Fill(d);

            double res2=h->GetResidualsSquared()/nPoints;
            hRes2->Fill(res2);

            hResMom->Fill( res2, p.Mag() );
	 
            TFitHelix* hh = new TFitHelix(h);
            hh->SetMagneticField(MagneticField);
            c->AddHelix(hh);
         }

      if( c->FindDCA() > 0 )
         {
            double dca = c->GetSeedChi2();

            TFitHelix* h0 = (TFitHelix*) c->GetHelixStack()->At(0);
            TVector3 p0 = h0->GetMomentumV();
            TFitHelix* h1 = (TFitHelix*) c->GetHelixStack()->At(1);
            TVector3 p1 = h1->GetMomentumV();
            double cosangle = p0.Unit().Dot(p1.Unit());
            std::cout<<"CosmModule::HelixAnalysis DCA="<<dca<<"mm Cos(angle)="<<cosangle<<std::endl;
	 
            if( nHelices == 2 )
               {
                  hDCAeq2->Fill( dca );
                  hAngeq2->Fill( cosangle );
                  hAngDCAeq2->Fill( cosangle, dca );
               }
            else
               {
                  hDCAgr2->Fill( dca );
                  hAnggr2->Fill( cosangle );
                  hAngDCAgr2->Fill( cosangle, dca );
               }
         }
      else
         {
            delete c;
            return 2;
         }

      delete c;
      return 0;
   }

   int CombineHelix(TStoreEvent* e)
   {
      const TObjArray* helices = e->GetHelixArray();
      int nHelices = helices->GetEntriesFast();
      std::cout<<"CosmModule::CombineHelix Event # "<<e->GetEventNumber()
               <<" Number of Helices: "<<nHelices<<std::endl;
      if(nHelices<2) return 1;

      int n=0;
      for( int i=0; i<nHelices; ++i )
         {
            TStoreHelix* hi = (TStoreHelix*) helices->At(i);
                for( int j=i+1; j<nHelices; ++j )
               {
                  TStoreHelix* hj = (TStoreHelix*) helices->At(j);
                  TTrack* aTrack = AddAllPoints( hi->GetSpacePoints(), hj->GetSpacePoints() );
                  new(fLinesArray[n]) TFitLine( *aTrack );
                  ( (TFitLine*)fLinesArray.ConstructedAt(n) )->Fit();
                  std::cout<<"CosmModule::CombineHelix n: "<<n
                           <<" nPoints: "<<( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetNumberOfPoints()
                           <<" stat: "<<( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetStat()<<std::endl;
                  if( ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetStat() > 0 )
                     {
                        ( (TFitLine*)fLinesArray.ConstructedAt(n) )->CalculateResiduals();
                        std::cout<<"CosmModule::CombineHelix OK"<<std::endl;
                        ++n;
                     }
                  else
                     {
                        std::cout<<"CosmModule::CombineHelix NO GOOD"<<std::endl;
                        fLinesArray.RemoveAt(n);
                     }
                  delete aTrack;
               }
         }
      fLinesArray.Compress();
      std::cout<<"CosmModule::CombineHelix Cosmic Candidates: "<<fLinesArray.GetEntries()<<std::endl;
      if( fLinesArray.GetEntries() < 1 ) return 2;
      
      double res2=9.e9;
      int idx=-1;
      for( int i=0; i<fLinesArray.GetEntriesFast(); ++i)
         {
            TFitLine* l = (TFitLine*) fLinesArray.At(i);
            double lres2 = l->GetResidualsSquared(),
               nPoints = (double) l->GetNumberOfPoints();
            std::cout<<"CosmModule::CombineHelix Candidate: "<<i<<") delta^2: "<<lres2<<" nPoints: "<<nPoints<<std::endl;
            lres2/=nPoints;
            if( lres2 < res2 )
               {
                  res2=lres2;
                  idx=i;
               }
         }
      fLinesArray.Compress();
      std::cout<<"CosmModule::CombineHelix Cosmic delta^2: "<<res2<<std::endl;

      if( res2 < 9.e9 && idx >= 0 )
         {
            TFitLine* cosmic = (TFitLine*) fLinesArray.At(idx);
            TObjArray points( *cosmic->GetPointsArray() );
            for( int i=0; i<points.GetEntriesFast(); ++i )
               {
                  TSpacePoint* p = (TSpacePoint*) points.At( i );
                  hcosaw->Fill( double(p->GetWire()) );
                  int sec,row;
                  pmap->get( p->GetPad(), sec,row );
                  hcospad->Fill( row, sec );
               }
            hRes2min->Fill(res2);
            e->AddLine( cosmic );
            double delta = (e->GetTimeOfEvent() - temp)*1.e3;
            hpois->Fill( delta );
            temp = e->GetTimeOfEvent();
         }
      else
         return 3;
      return 0;
   }

   TTrack* AddAllPoints(const TObjArray* pcol1, const TObjArray* pcol2)
   {
      int np1 = pcol1->GetEntriesFast(),
         np2 = pcol2->GetEntriesFast();
      TTrack* t = new TTrack;
      for(int i=0; i<np1; ++i)
         t->AddPoint( (TSpacePoint*) pcol1->At(i) );
      for(int i=0; i<np2; ++i)
         t->AddPoint( (TSpacePoint*) pcol2->At(i) );
      return t;
   }
};

class CosmModuleFactory: public TAFactory
{
public:
   CosmFlags fFlags;

   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("CosmModuleFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++) 
         {
            if( args[i] == "--Bfield" )
               {
                  fFlags.fMagneticField = atof(args[i+1].c_str());
               }
            if( args[i] == "--cosm" )
               {
                  fFlags.enabled = true;
               }
         }
   }

   void Finish()
   {
      printf("CosmModuleFactory::Finish!\n");
   }
  
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("CosmModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new CosmModule(runinfo,&fFlags);
   }
};

static TARegister tar(new CosmModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
