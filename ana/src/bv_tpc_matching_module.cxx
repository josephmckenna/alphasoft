#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include "AnaSettings.hh"
#include "json.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TTree.h"

#include "TBarEvent.hh"

class MatchingModuleFlags
{
public:
   double fMagneticField = 0;
   bool fPrint = false;
   bool fDiag = false;
   AnaSettings* ana_settings=0;

   MatchingModuleFlags() // ctor
   { }
   ~MatchingModuleFlags() // dtor
   { }
};

class matchingmodule: public TARunObject
{
public:
   MatchingModuleFlags* fFlags;

private:
   bool fTrace=false;

   // BV geometry
   const double inner_diameter = 446.0; // mm
   const double outer_diameter = 486.0; // mm
   //double radius = (inner_diameter+outer_diameter)/4.; // Use centre of BV
   const double radius = inner_diameter*0.5; // Use inner edge of BV
   const double length = 2604.; // mm
   double c = 2.99792e8*1e-9; // m/ns
   double refrac = 1.93; // From protoTOF tests with time walk correction applied
   double factor = c/refrac * 0.5;


   // Matching parameters
   double max_dz; // mm
   double max_dphi; // rad

   // TOF parameters
   double min_dphi;


public:
   TBarEvent *analyzed_event;
   TTree *BscTree;


public:

   matchingmodule(TARunInfo* runinfo, MatchingModuleFlags* f): TARunObject(runinfo), fFlags(f),
                 max_dz(f->ana_settings->GetDouble("BscModule","max_dz")),
                 max_dphi(f->ana_settings->GetDouble("BscModule","max_dphi")),
                 min_dphi(f->ana_settings->GetDouble("BscModule","min_dphi"))
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="BV/TPC Matching Module";
#endif
      printf("matchingmodule::ctor!\n");
      //      MagneticField=fFlags->fMagneticField<0.?1.:fFlags->fMagneticField;
   }

   ~matchingmodule()
   {
      printf("matchingmodule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      if (fFlags->fPrint) { printf("matchingmodule::begin!"); }

      analyzed_event = new TBarEvent();
      BscTree = new TTree("BscEventTree", "BscEventTree");
      BscTree->Branch("BarrelEvent", &analyzed_event, 32000, 0);
      delete analyzed_event;
      
   }


   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fFlags->fPrint) {printf("PauseRun, run %d\n", runinfo->fRunNo);}
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fFlags->fPrint) {printf("ResumeRun, run %d\n", runinfo->fRunNo);}
   }

   // Main function
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
   

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      AgBarEventFlow *bf = flow->Find<AgBarEventFlow>();
      if(!bf)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TBarEvent *barEvt=bf->BarEvent;
      if (!barEvt)
      {
         if (fFlags->fPrint) {printf("matchingmodule: TBarEvent not found!");}
         return flow;
      }

      AgAnalysisFlow *anaflow = flow->Find<AgAnalysisFlow>();
      if (!anaflow)
      {
         if (fFlags->fPrint) {printf("matchingmodule: AgAnalysisFlow not found!");}
         return flow;
      }
      TStoreEvent* e = anaflow->fEvent;
      if (!e)
      {
         if (fFlags->fPrint) {printf("matchingmodule: TStoreEvent not found!");}
         return flow;
      }

      if (!(fFlags->fDiag)) return flow;

      const TObjArray* LineArray = e->GetLineArray();
      const TObjArray* HelixArray = e->GetHelixArray();
      
      AgEvent* age = ef->fEvent;
      // prepare event to store in TTree
      TBarEvent* analyze_event = new TBarEvent();
      analyze_event->Reset();
      analyze_event->SetID( age->counter );
      analyze_event->SetRunTime( age->time );

      // Main functions
      if( fFlags->fMagneticField > 0. || fFlags->fMagneticField < 0. )
         {
            std::vector<TVector3> helix_points = GetHelices(HelixArray);
            MatchPoints(barEvt, helix_points);
            CalculateTOF(barEvt);
         }
      else
         {
            std::vector<TVector3> line_points = GetLines(LineArray);
            MatchPoints(barEvt, line_points);
            CalculateTOF(barEvt);
         }

      // Saves all the BarEvents to a tree
      if( barEvt )
         {
            for(int i=0; i<barEvt->GetNBars(); ++i)
               analyze_event->AddBarHit(barEvt->GetBars().at(i));

            for(int i=0; i<barEvt->GetNEnds(); ++i)
               analyze_event->AddEndHit(barEvt->GetEndHits().at(i));
            for(int i=0; i<barEvt->GetNTOF(); ++i)
               analyze_event->AddTOF(barEvt->GetTOF().at(i));
            {std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
            BscTree->SetBranchAddress("BarrelEvent", &analyze_event);
            BscTree->Fill();}
         }
         delete analyze_event;

      
      //AgBarEventFlow 
      return flow;
   }

   //________________________________
   // MAIN FUNCTIONS

   std::vector<TVector3> GetLines(const TObjArray* LineArray)
   {
      std::vector<TVector3> line_points;
      for (auto line: *LineArray)
         {
            TStoreLine* l = (TStoreLine*)line;
            TFitLine* lin = new TFitLine(l);
            line_points.push_back(lin->Evaluate(radius*radius));
            delete lin;
         }
      return line_points;
   }
   std::vector<TVector3> GetHelices(const TObjArray* HelixArray)
   {
      std::vector<TVector3> helix_points;
      for (auto helix: *HelixArray)
         {
            TStoreHelix* h = (TStoreHelix*)helix;
            TFitHelix* hel = new TFitHelix(h);
            helix_points.push_back(hel->Evaluate(radius*radius));
            delete hel;
         }
      return helix_points;
   }
   void MatchPoints(TBarEvent* barEvt, std::vector<TVector3> tpc_points)
   {
      for (TVector3 tpc_point: tpc_points) {
         double min_dist = 99999.;
         BarHit* best_barhit;
         for (BarHit* barhit: barEvt->GetBars()) {
            TVector3 bv_point = Get3VectorBV(barhit);
            double dist = GetGeometricDistance(bv_point,tpc_point);
            if (dist>min_dist) continue;
            double dphi = GetDPhi(bv_point,tpc_point);
            if (TMath::Abs(dphi) > max_dphi) continue;
            double dzi = GetDZ(bv_point,tpc_point);
            if (TMath::Abs(dzi) > max_dz) continue;
            min_dist = dist;
            best_barhit = barhit;
         }
         if (min_dist!=99999.) {
            // Match!
            best_barhit->SetTPCHit(tpc_point);
         }
      }
   }
   void CalculateTOF(TBarEvent* barEvt) {
      std::vector<BarHit*> barhits = barEvt->GetBars();
      for (BarHit* barhit: barhits) {
         for (BarHit* barhit2: barhits) {
            if (!(barhit->IsTPCMatched())) continue;
            if (!(barhit2->IsTPCMatched())) continue;
            double TOF = 1e9*(barhit->GetAverageTDCTime()-barhit2->GetAverageTDCTime());
            if (TOF<=0) continue;
            barEvt->AddTOF(TOF);
         }
      }
   }

   //________________________________
   // HELPER FUNCTIONS

   double GetDZ(TVector3 p1, TVector3 p2)
   {
      return p2.z() - p1.z();
   }
   double GetDPhi(TVector3 p1, TVector3 p2)
   {
      //return std::min(TMath::Abs(p1.DeltaPhi(p2)),TMath::Abs(p2.DeltaPhi(p1)));
      return p2.DeltaPhi(p1);
   }
   TVector3 Get3VectorBV(BarHit* hit)
   {
      double xbv,ybv;
      hit->GetXY(xbv,ybv);
      double dt = (hit->GetTDCBot() - hit->GetTDCTop())*1e9;
      double zbv = factor*dt;
      TVector3 bv_point = TVector3(xbv*1000,ybv*1000,zbv*1000); // to mm
      return bv_point;
   }
   double GetGeometricDistance(TVector3 p1, TVector3 p2)
   {
      TVector3 diff = p2-p1;
      return diff.Mag();
   }
};

class matchingModuleFactory: public TAFactory
{
public:
   MatchingModuleFlags fFlags;
public:
   void Help()
   { 
      printf("MatchingModuleFactory::Help\n");
      printf("\t--anasettings /path/to/settings.json\t\t load the specified analysis settings\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("matchingModuleFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++)
         {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
         if( args[i] == "--Bfield" )
            {
               fFlags.fMagneticField = atof(args[++i].c_str());
               printf("Magnetic Field (incompatible with --loadcalib)\n");
            }
         if( args[i] == "--bscprint")
            fFlags.fPrint = true;
         if( args[i] == "--bscdiag")
            fFlags.fDiag = true;
         if( args[i] == "--anasettings" ) 
            json=args[i+1];
         }
      fFlags.ana_settings=new AnaSettings(json.Data());
   }

   void Finish()
   {
      printf("matchingModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("matchingModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new matchingmodule(runinfo,&fFlags);
   }
};

static TARegister tar(new matchingModuleFactory);


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
