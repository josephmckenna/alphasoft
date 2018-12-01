#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <TH1D.h>
#include <TH2D.h>

#include "TStoreEvent.hh"
#include "TStoreHelix.hh"
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

public:
  CosmModule(TARunInfo* runinfo, CosmFlags* f): TARunObject(runinfo),
						fFlags(f)
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
	 hchi2Z = new TH1D("hchi2Z","Hel #chi^{2}_{Z}",200,0.,100.);
	 
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

     const TObjArray* helices = e->GetHelixArray();
     int nHelices = helices->GetEntriesFast();
     std::cout<<"CosmModule::AnalyzeFlowEvent Event # "<<e->GetEventNumber()
	      <<" Number of Helices: "<<nHelices<<std::endl;
     if(nHelices<2) return flow;

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
	 std::cout<<"CosmModule::AnalyzeFlowEvent DCA="<<dca<<"mm Cos(angle)="<<cosangle<<std::endl;
	 
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

     delete c;

#ifdef _TIME_ANALYSIS_
     if (TimeModules) flow=new AgAnalysisReportFlow(flow,"cosm_module");
#endif
     return flow;
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
