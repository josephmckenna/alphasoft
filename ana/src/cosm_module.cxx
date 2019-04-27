#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <TH1D.h>
#include <TH2D.h>

#include "TStoreEvent.hh"
#include "TStoreHelix.hh"
#include "TStoreLine.hh"
#include "TSpacePoint.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"
#include "TFitVertex.hh"

#include "AnalysisTimer.h"
#include "AnaSettings.h"
#include "json.hpp"

class CosmFlags
{
public:
   bool enabled=false;
   double fMagneticField=1.;
   AnaSettings* ana_settings=0;
};

class CosmModule: public TARunObject
{
public:
   bool fTrace = false;
   //bool fTrace = true;
   CosmFlags* fFlags;

private:

   double MagneticField;

   std::vector<TFitLine*> fLines;
   std::vector<double> fDCA;
   std::vector<double> fphi;
   int nTracks;

   unsigned fNspacepointsCut;
   double fLineChi2Cut;
   double fLineChi2Min;

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

   TH1D* hcosphi;
   TH1D* hcostheta;
   TH2D* hcosthetaphi;

   padmap* pmap;

public:
   CosmModule(TARunInfo* runinfo, CosmFlags* f): TARunObject(runinfo),
                                                 fFlags(f),nTracks(0)
   {
      printf("CosmModule::ctor!\n");
      MagneticField = fabs(fFlags->fMagneticField);

      assert( fFlags->ana_settings );
      fNspacepointsCut = fFlags->ana_settings->GetInt("RecoModule","NpointsCut");
      fLineChi2Cut = fFlags->ana_settings->GetDouble("RecoModule","LineChi2Cut");
      fLineChi2Min = fFlags->ana_settings->GetDouble("RecoModule","LineChi2Min");
      printf("CosmModule::ctor Using AnaSettings\n");
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
                               500,0.,50.);
            hDCAgr2 = new TH1D("hDCAgr2","Distance of Closest Approach between Helices in >2-tracks Events;DCA [mm]",
                               500,0.,50.);
	 
            hAngeq2 = new TH1D("hAngeq2","Cosine of the Angle formed by Two Helices in =2-tracks Events;cos(angle)",
                               1000,-1.,1.);
            hAnggr2 = new TH1D("hAnggr2","Cosine of the Angle formed by Two Helices in >2-tracks Events;cos(angle)",
                               1000,-1.,1.);

            hAngDCAeq2 = new TH2D("hAngDCAeq2","DCA and Cosine of Angle between Helices in =2-tracks Events;cos(angle);DCA [mm]",
                                  100,-1.,1.,100,0.,50.);
            hAngDCAgr2 = new TH2D("hAngDCAgr2","DCA and Cosine of Angle between Helices in >2-tracks Events;cos(angle);DCA [mm]",
                                  100,-1.,1.,100,0.,50.);

            hcosaw = new TH1D("hcosaw","Occupancy per AW due to cosmics",256,-0.5,255.5);
            hcosaw->SetMinimum(0.);
            hcospad = new TH2D("hcospad","Occupancy per PAD due to cosmics;Pads Row;Pads Sector",
                               576,-0.5,575.5,32,-0.5,31.5);
            hRes2min = new TH1D("hRes2min","Minimum Residuals Squared Divide by Number of Spacepoints from 2 Helices;#delta [mm^{2}]",1000,0.,1000.);

            // cosmic time distribution
            hpois = new TH1D("hpois","Delta t between cosmics;#Delta t [ms]",300,0.,300.);
            temp = 0.;            

            hcosphi = new TH1D("hcosphi","Direction #phi;#phi [deg]",200,-180.,180.);
            hcosphi->SetMinimum(0.);
            hcostheta = new TH1D("hcostheta","Direction #theta;#theta [deg]",200,0.,180.);
            hcostheta->SetMinimum(0.);

            hcosthetaphi = new TH2D("hcosthetaphi","Direction #theta Vs #phi;#theta [deg];#phi [deg]",
                                    200,0.,180.,200,-180.,180.);

            pmap = new padmap;
         }
   } 

   void EndRun(TARunInfo* runinfo)
   {
      if( fFlags->enabled ) delete pmap;
      printf("CosmModule::EndRun, run %d\n", runinfo->fRunNo);
   }
  
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( !fFlags->enabled ) return flow;
     
      AgAnalysisFlow* af = flow->Find<AgAnalysisFlow>();
      if( !af ) return flow;
      TStoreEvent* e = af->fEvent;
      if( !e ) return flow;
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif   
      int stat=-1;
      if( MagneticField > 0. )
         {
            HelixAnalysis( e );
            stat=CombineHelix( e );
         }
      else
         {
            stat=CombineLine( e );
         }

      if( stat==0 )
         {
            //e->AddLine( cosmic ); <-- I want to do this but TStoreEvent is already flushed
            double delta = (e->GetTimeOfEvent() - temp)*1.e3;
            hpois->Fill( delta );
            temp = e->GetTimeOfEvent();
         }

      fLines.clear();
      fDCA.clear();
      fphi.clear();
      nTracks=0;

#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"cosm_module",timer_start);
#endif
      return flow;
   }

   int HelixAnalysis(TStoreEvent* e)
   {
      const TObjArray* helices = e->GetHelixArray();
      nTracks = helices->GetEntriesFast();
      if( fTrace )
         std::cout<<"CosmModule::HelixAnalysis Event # "<<e->GetEventNumber()
                  <<" Number of Helices: "<<nTracks<<std::endl;
      if(nTracks<2) return 1;
      
      //      TFitVertex* c = new TFitVertex(-e->GetEventNumber());
      //      std::vector<TFitHelix*> hha; 
      for( int i=0; i<nTracks; ++i )
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
	 
            // hha.push_back( new TFitHelix(h) );
            // hha.back()->SetMagneticField(MagneticField);
            // c->AddHelix(hha.back());
         }

      int stat=0;
      // if( c->FindDCA() > 0 )
      //    {
      //       double dca = c->GetNewChi2(); //mis-name since I'm re-using the vertexing algorithm

      //       TFitHelix* h0 = (TFitHelix*) c->GetHelixStack()->At(0);
      //       TVector3 p0 = h0->GetMomentumV();
      //       TFitHelix* h1 = (TFitHelix*) c->GetHelixStack()->At(1);
      //       TVector3 p1 = h1->GetMomentumV();
      //       double cosangle = p0.Unit().Dot(p1.Unit());
      //       if( fTrace )
      //          std::cout<<"CosmModule::HelixAnalysis DCA="<<dca
      //                   <<"mm Cos(angle)="<<cosangle<<std::endl;
	 
      //       if( nHelices == 2 )
      //          {
      //             hDCAeq2->Fill( dca );
      //             hAngeq2->Fill( cosangle );
      //             hAngDCAeq2->Fill( cosangle, dca );
      //          }
      //       else
      //          {
      //             hDCAgr2->Fill( dca );
      //             hAnggr2->Fill( cosangle );
      //             hAngDCAgr2->Fill( cosangle, dca );
      //          }
      //    }
      // else
      //    stat=2;

      // for( auto h: hha )
      //    delete h;
      // hha.clear();
      // delete c;
      return stat;
   }

   int CombineHelix(TStoreEvent* e)
   {
      const TObjArray* helices = e->GetHelixArray();
      nTracks = helices->GetEntriesFast();
      if( fTrace )
         std::cout<<"CosmModule::CombineHelix Event # "<<e->GetEventNumber()
                  <<" Number of Helices: "<<nTracks<<std::endl;
      if(nTracks<2) return 1;
      fLines.clear();
      fDCA.clear();
      fphi.clear();

      int n=0;
      for( int i=0; i<nTracks; ++i )
         {
            TStoreHelix* hi = (TStoreHelix*) helices->At(i);
            for( int j=i+1; j<nTracks; ++j )
               {
                  TStoreHelix* hj = (TStoreHelix*) helices->At(j);
                  TFitLine* l = new TFitLine();
                  AddAllPoints( l, hi->GetSpacePoints(), hj->GetSpacePoints() );
                  l->SetChi2Cut( fLineChi2Cut );
                  l->SetChi2Min( fLineChi2Min );
                  l->SetPointsCut( fNspacepointsCut );
                  l->Fit();
                  if( fTrace )
                     std::cout<<"CosmModule::CombineHelix n: "<<n
                              <<" nPoints: "<<l->GetNumberOfPoints()
                              <<" stat: "<<l->GetStat()<<std::endl;
                  //  if( l->GetStat() > 0 )
                  if( l->IsGood() && !l->IsWeird() )
                     {
                        double rsq = l->CalculateResiduals();
                        if( fTrace )
                           std::cout<<"CosmModule::CombineHelix OK delta^2: "<<rsq<<std::endl;
                        int stat = CalculateHelDCA(hi,hj);
                        if( stat < 0 ) continue; 
                        fLines.push_back(l);
                        ++n;
                     }
                  else
                     {
                        if( fTrace )
                           std::cout<<"CosmModule::CombineHelix NO GOOD"<<std::endl;
                        delete l;
                     }
               }
         }
      
      if( fTrace )
         std::cout<<"CosmModule::CombineHelix Cosmic Candidates: "<<fLines.size()<<std::endl;
      if( fLines.size() < 1 ) return 2;
      
      return Residuals();
   }

   int CalculateHelDCA(TStoreHelix* hi, TStoreHelix* hj)
   {
      TFitVertex* c = new TFitVertex(-1);
      TFitHelix* hel1 = new TFitHelix(hi);
      hel1->SetMagneticField(MagneticField);
      c->AddHelix( hel1 );
      TFitHelix* hel2 = new TFitHelix(hj);
      hel2->SetMagneticField(MagneticField);
      if(fTrace) std::cout<<"CosmModule::CalculateHelDCA Nhel: "
                          <<c->AddHelix( hel2 )<<std::endl;
      else c->AddHelix( hel2 );

      int stat = c->FindDCA();
      if( stat > 0 )
         {
            double dca = c->GetNewChi2(); //mis-name since I'm re-using the vertexing algorithm
            TVector3 p1 = hel1->GetMomentumV();
            TVector3 p2 = hel2->GetMomentumV();
            double cang = p1.Unit().Dot(p1.Unit());
            if( fTrace )
               std::cout<<"CosmModule::HelixAnalysis DCA="<<dca
                        <<"mm Cos(angle)="<<cang<<std::endl;
            fDCA.push_back( dca );
            fphi.push_back( cang );
         }
      else 
         {
            if(fTrace)
               {
                  std::cout<<"CosmModule::CalculateHelDCA FindDCA stat: "<<stat<<std::endl;
                  if( stat == 0 ) 
                     std::cout<<"CosmModule::CalculateHelDCA AddedHelix: "<<c->GetNumberOfAddedHelix()<<std::endl;
               }
            // delete hel1;
            // delete hel2;
            // delete c;
            // return -1;
         }

      delete hel1;
      delete hel2;
      delete c;
      return stat-1;
   }

   int CombineLine(TStoreEvent* e)
   {
      const TObjArray* lines = e->GetLineArray();
      nTracks = lines->GetEntriesFast();
      if( fTrace )
         std::cout<<"CosmModule::CombineLine Event # "<<e->GetEventNumber()
                  <<" Number of Lines: "<<nTracks
            /*<<" Number of Points: "<<e->GetNumberOfPoints()*/<<std::endl;
      if(nTracks<2) return 1;
      fLines.clear();
      fDCA.clear();
      fphi.clear();

      int n=0;
      for( int i=0; i<nTracks; ++i )
         {
            TStoreLine* hi = (TStoreLine*) lines->At(i);
            TVector3 ui = *(hi->GetDirection());
            for( int j=i+1; j<nTracks; ++j )
               {
                  TStoreLine* hj = (TStoreLine*) lines->At(j);
                  TVector3 uj = *(hj->GetDirection());

                  TFitLine* l = new TFitLine();
                  AddAllPoints( l, hi->GetSpacePoints(), hj->GetSpacePoints() );
                  l->SetChi2Cut( fLineChi2Cut );
                  l->SetChi2Min( fLineChi2Min );
                  l->SetPointsCut( fNspacepointsCut );
                  l->Fit();
                  if( fTrace )
                     std::cout<<"CosmModule::CombineLine n: "<<n
                              <<" nPoints: "<<l->GetNumberOfPoints()
                        //<<" stat: "<<l->GetStat()
                              <<std::endl;
                  //if( l->GetStat() > 0 )
                  if( l->IsGood() && !l->IsWeird() )
                     {
                        double rsq = l->CalculateResiduals();
                        if( fTrace )
                           std::cout<<"CosmModule::CombineLine OK delta^2: "<<rsq<<std::endl;
                        
                        double dist = LineDistance(hi,hj);
                        if( dist < 0. ) continue;
                        fDCA.push_back( dist );
                        double cang = ui.Dot(uj);
                        fphi.push_back( cang );

                        fLines.push_back(l);
                        ++n;
                     }
                  else
                     {
                        if( fTrace )
                           std::cout<<"CosmModule::CombineLine NO GOOD status:"
                                    <<l->GetStatus()<<std::endl;
                        delete l;
                     }
               }
         }

      if( fTrace )
         std::cout<<"CosmModule::CombineLine Cosmic Candidates: "<<fLines.size()<<std::endl;
      if( fLines.size() < 1 ) return 2;
      
      return Residuals();
   }
   
   void AddAllPoints( TFitLine* t, const TObjArray* pcol1, const TObjArray* pcol2 )
   {
      int np1 = pcol1->GetEntriesFast(),
         np2 = pcol2->GetEntriesFast();
      if( fTrace )
         std::cout<<"CosmModule::AddAllPoints(TFitLine* t,...) np1: "<<np1<<" np2: "<<np2<<std::endl;
      for(int i=0; i<np1; ++i)
         t->AddPoint( (TSpacePoint*) pcol1->At(i) );
      for(int i=0; i<np2; ++i)
         t->AddPoint( (TSpacePoint*) pcol2->At(i) );

      if( fTrace )
         std::cout<<"CosmModule::AddAllPoints(TFitLine* t,...) track points: "<<t->GetNumberOfPoints()<<std::endl;
   }

   int Residuals()
   {
      double res2=9.e99;
      int idx=-1,i=0;
      for( auto l: fLines )
         {
            double lres2 = l->GetResidualsSquared(),
               nPoints = (double) l->GetNumberOfPoints();
            if( fTrace )
               std::cout<<"CosmModule::Residuals Candidate: "<<i
                        <<") delta^2: "<<lres2
                        <<" nPoints: "<<nPoints<<std::endl;
            lres2/=nPoints;
            if( lres2 < res2 )
               {
                  res2=lres2;
                  idx=i;
               }
            ++i;
         }
      if( fTrace )
         std::cout<<"CosmModule::Residuals Cosmic delta^2: "<<res2<<" @ "<<idx<<std::endl;

      if( idx < 0 )
         return 3;

      hRes2min->Fill(res2);
      FillOccupancyHisto( idx );
      return 0;
   }

   void FillOccupancyHisto(int idx)
   {
      double dca = fDCA.at( idx ), cosangle = fphi.at( idx );
      if( nTracks == 2 )
         {
            hDCAeq2->Fill( dca );
            hAngeq2->Fill( cosangle );
            hAngDCAeq2->Fill( cosangle, dca );
         }
      else if( nTracks > 2 )
         {
            hDCAgr2->Fill( dca );
            hAnggr2->Fill( cosangle );
            hAngDCAgr2->Fill( cosangle, dca );
         }
      else return;
         
      TFitLine* cosmic = fLines.at( idx );
      for( uint i=0; i<cosmic->GetPointsArray()->size(); ++i )
         {
            TSpacePoint* p = (TSpacePoint*) cosmic->GetPointsArray()->at( i );
            int aw = p->GetWire(), sec,row;
            pmap->get( p->GetPad(), sec,row );
            if( fTrace && 0 )
               {
                  double time = p->GetTime(),
                     height = p->GetHeight();
                  std::cout<<aw<<"\t\t"<<sec<<"\t"<<row<<"\t\t"<<time<<"\t\t"<<height<<std::endl;
               }
            hcosaw->Fill( double(aw) );
            hcospad->Fill( double(row), double(sec) );
         }
      
      TVector3 u = cosmic->GetU();
      hcosphi->Fill(u.Phi()*TMath::RadToDeg());
      hcostheta->Fill(u.Theta()*TMath::RadToDeg());
      hcosthetaphi->Fill(u.Theta()*TMath::RadToDeg(),u.Phi()*TMath::RadToDeg());
   }

   double LineDistance(TStoreLine* l0, TStoreLine* l1)
   {
      TVector3 u0 = *(l0->GetDirection());
      TVector3 u1 = *(l1->GetDirection());
      TVector3 p0 = *(l0->GetPoint());
      TVector3 p1 = *(l1->GetPoint());

      TVector3 n0 = u0.Cross( u1 ); // normal to lines
      TVector3 c =  p1 - p0;
      if( n0.Mag() == 0. ) return -1.;
  
      TVector3 n1 = n0.Cross( u1 ); // normal to plane formed by n0 and line1

      double tau = c.Dot( n1 ) / u0.Dot( n1 ); // intersection between
      TVector3 q0 = tau * u0 + p0;             // plane and line0

      double t1 = ( (q0-p0).Cross(n0) ).Dot( u0.Cross(n0) ) / ( u0.Cross(n0) ).Mag2();
      TVector3 q1 = t1 * u0 + p0;

      double t2 = ( (q0-p1).Cross(n0) ).Dot( u1.Cross(n0) ) / ( u1.Cross(n0) ).Mag2();
      TVector3 q2 = t2*u1+p1;

      TVector3 Q = q2 - q1;

      return Q.Mag();
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
            if( args[i] == "--anasettings" ) json=args[i+1];
         }
      fFlags.ana_settings=new AnaSettings(json.Data());
      //fFlags.ana_settings->Print();
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
