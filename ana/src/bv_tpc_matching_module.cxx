#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "AnalysisTimer.h"

#include "TBarEvent.hh"

class MatchingModuleFlags
{
public:
   double fMagneticField = -1.;

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

   // BV geometry
   double inner_diameter = 446.0; // mm
   double outer_diameter = 486.0; // mm
   //double radius = (inner_diameter+outer_diameter)/4.; // Use centre of BV
   double radius = inner_diameter/2; // Use inner edge of BV
   double length = 2604; // mm
   double speed=TMath::C();
   double cFactor=1.58;

   // Matching parameters
   double max_dz = 2000; // mm
   double max_dphi = 2*TMath::Pi()/64 * 64; // All bars

   // TOF parameters
   double min_dphi = 2*TMath::Pi()/64 * 2; // Two bars
   double z_cut = 800; //mm from centre

   //Histogramm declaration
   TH2D* hdPhi = NULL;
   TH2D* hdZ = NULL;
   TH2D* hdistMatch = NULL;
   TH2D* hZBVvZTPC = NULL;
   TH2D* hPhiBVvPhiTPC = NULL;
   TH2D* hNHits = NULL;
   TH2D* hBVTPCMatch = NULL;
   TH1D* hTOFbv = NULL;
   TH1D* hTOFdiffmatch = NULL;
   TH1D* hDistBV = NULL;
   TH2D* hDistBVvTOF = NULL;
   TH1D* hDistTPC = NULL;
   TH2D* hDistTPCvTOF = NULL;
   TH1D* hdPhiBV = NULL;
   TH1D* hdPhiBVmatched = NULL;
   TH1D* hdZedBV = NULL;
   TH1D* hdZedBVmatched = NULL;
   TH1D* hdPhiTPC = NULL;
   TH1D* hdZedTPC = NULL;
   TH2D* hdPhiBVvTOF = NULL;
   TH2D* hdZedBVvTOF = NULL;
   TH2D* hdPhiTPCvTOF = NULL;
   TH2D* hdZedTPCvTOF = NULL;

public:

   matchingmodule(TARunInfo* runinfo, MatchingModuleFlags* f): TARunObject(runinfo), fFlags(f)
   {
      printf("matchingmodule::ctor!\n");
   }

   ~matchingmodule()
   {
      printf("matchingmodule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      printf("matchingmodule::begin!");
      gDirectory->mkdir("bv_tpc_matching_module")->cd();

      // Histogramm setup
      hdPhi = new TH2D("hdPhi","Delta phi between BV hit and TPC track;Bar;Delta Phi [rad]",64,-0.5,63.5,360,-TMath::Pi(),TMath::Pi());
      hdZ = new TH2D("hdZ","Delta z between BV hit and TPC track;Bar;Delta Z [mm]",64,-0.5,63.5,1500,-3000,3000);
      hdistMatch = new TH2D("hdistMatch","Geometric distance between BV hit and TPC track;Bar;Distance [mm]",64,-0.5,63.5,1000,0,3000);
      hZBVvZTPC = new TH2D("hZBVvZTPC","Z position of BV hits and TPC track ends;Z of BV [mm];Z of TPC [mm]",1000,-3000,3000,1000,-3000,3000);
      hPhiBVvPhiTPC = new TH2D("hPhiBVvPhiTPC","Phi position of BV hits and TPC track ends;Phi of BV [rad];Phi of TPC [rad]",360,-TMath::Pi(),TMath::Pi(),360,-TMath::Pi(),TMath::Pi());
      hNHits = new TH2D("hNHits","Number of hits on BV and TPC per event;BV hits;TPC hits",10,-0.5,9.5,10,-0.5,9.5);
      hBVTPCMatch = new TH2D("hBVTPCMatch","Number of BV hits matched to TPC tracks;Bar;0 = Unmatched, 1 = Matched",64,-0.5,63.5,2,-0.5,1.5);
      hTOFbv = new TH1D("hTOFbv","Time of flight of all pairs of BV hits;Time of flight [s];Counts",1500,0,1.5*10e-9);
      hTOFdiffmatch = new TH1D("hTOFdiffmatch","Time of flight of all pairs of BV hits matched to different TPC tracks;Time of flight [s];Counts",1500,0,1.5*10e-9);
      hDistBV = new TH1D("hDistBV","Geometrical distance between BV hits;Distance [mm]",1000,0,2400);
      hDistBVvTOF = new TH2D("hDistBVvTOF","Geometrical distance between BV hits versus TOF;Distance [mm];Time of flight [s]",1000,0,2400,1500,0,1.5*10e-9);
      hDistTPC = new TH1D("hDistTPC","Geometrical distance between associated TPC track ends;Distance [mm]",1000,0,2400);
      hDistTPCvTOF = new TH2D("hDistTPCvTOF","Geometrical distance between associated TPC track ends versus TOF;Distance [mm];Time of flight [s]",1000,0,2400,1500,0,1.5*10e-9);
      hdPhiBV = new TH1D("hdPhiBV","Delta phi between two BV hits;Delta Phi [rad]",180,0.,TMath::Pi());
      hdPhiBVmatched = new TH1D("hdPhiBVmatched","Delta phi between two BV hits after TPC matching;Delta Phi [rad]",180,0.,TMath::Pi());
      hdZedBV = new TH1D("hdZedBV","Delta z between two BV hits;Delta Zed [mm]",1500,-3000,3000);
      hdZedBVmatched = new TH1D("hdZedBVmatched","Delta z between two BV hits after TPC matching;Delta Zed [mm]",1500,-3000,3000);
      hdPhiTPC = new TH1D("hdPhiTPC","Delta phi between two TPC hits;Delta Phi [rad]",180,0,TMath::Pi());
      hdZedTPC = new TH1D("hdZedTPC","Delta z between two TPC hits;Delta Zed [mm]",1500,-3000,3000);
      hdPhiBVvTOF = new TH2D("hdPhiBVvTOF","Delta phi between two BV hits vs TOF;Delta Phi [rad];Time of flight [s]",180,0,TMath::Pi(),1500,0,1.5*10e-9);
      hdZedBVvTOF = new TH2D("hdZedBVvTOF","Delta z between two BV hits vs TOF;Delta Zed [mm];Time of flight [s]",1500,-3000,3000,1500,0,1.5*10e-9);
      hdPhiTPCvTOF = new TH2D("hdPhiTPCvTOF","Delta phi between two TPC hits vs TOF;Delta Phi [rad];Time of flight [s]",180,0,TMath::Pi(),1500,0,1.5*10e-9);
      hdZedTPCvTOF = new TH2D("hdZedTPCvTOF","Delta z between two TPC hits vs TOF;Delta Zed [mm];Time of flight [s]",1500,-3000,3000,1500,0,1.5*10e-9);

   }

   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
      // Delete histos
      delete hdPhi;
      delete hdZ;
      delete hdistMatch;
      delete hZBVvZTPC;
      delete hPhiBVvPhiTPC;
      delete hBVTPCMatch;
      delete hTOFbv;
      delete hTOFdiffmatch;
      delete hDistBV;
      delete hDistBVvTOF;
      delete hDistTPC;
      delete hDistTPCvTOF;
      delete hdPhiBV;
      delete hdPhiBVmatched;
      delete hdZedBV;
      delete hdZedBVmatched;
      delete hdPhiTPC;
      delete hdZedTPC;
      delete hdPhiBVvTOF;
      delete hdZedBVvTOF;
      delete hdPhiTPCvTOF;
      delete hdZedTPCvTOF;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   // Main function
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {

      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif

      // Main functions
      if( fFlags->fMagneticField > 0. )
         {
            std::vector<TVector3> helix_points = GetHelices(flow);
            MatchPoints(flow, helix_points);
         }
      else
         {
            std::vector<TVector3> line_points = GetLines(flow);
            MatchPoints(flow, line_points);
         }
      FillHistos(flow);
      TimeOfFlight(flow);

//      #ifdef _TIME_ANALYSIS_
//         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bv_tpc_matching_module",timer_start);
//      #endif
      return flow;
   }

   //________________________________
   // MAIN FUNCTIONS

   std::vector<TVector3> GetLines(TAFlowEvent* flow)
   {
      std::vector<TVector3> line_points;
      AgAnalysisFlow *anaflow = flow->Find<AgAnalysisFlow>();
      if (!anaflow) {std::cout<<"matchingmodule: AgAnalysisFlow not found!"<<std::endl; return line_points; }
      TStoreEvent* e = anaflow->fEvent;
      const TObjArray* LineArray = e->GetLineArray();
      for (auto line: *LineArray)
         {
            TStoreLine* l = (TStoreLine*)line;
            TFitLine* lin = new TFitLine(l);
            line_points.push_back(lin->Evaluate(radius*radius));
            delete lin;
         }
      return line_points;
   }
   std::vector<TVector3> GetHelices(TAFlowEvent* flow)
   {
      std::vector<TVector3> helix_points;
      AgAnalysisFlow *anaflow = flow->Find<AgAnalysisFlow>();
      if (!anaflow) {std::cout<<"matchingmodule: AgAnalysisFlow not found!"<<std::endl; return helix_points; }
      else std::cout<<"FOUNDED!"<<std::endl;
      TStoreEvent* e = anaflow->fEvent;
      const TObjArray* HelixArray = e->GetHelixArray();
      for (auto helix: *HelixArray)
         {
            TStoreHelix* h = (TStoreHelix*)helix;
            TFitHelix* hel = new TFitHelix(h);
            helix_points.push_back(hel->Evaluate(radius*radius));
            delete hel;
         }
      return helix_points;
   }
   void MatchPoints(TAFlowEvent* flow, std::vector<TVector3> tpc_points)
   {
      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>(); // Gets list of bv hits from flow
      if (!bef) {std::cout<<"matchingmodule: AgBarEventFlow not found!"<<std::endl; return; }
      TBarEvent *barEvt=bef->BarEvent;
      hNHits->Fill(barEvt->GetBars().size(),tpc_points.size());
      if (tpc_points.size()==0) return;
      for (BarHit* hit: barEvt->GetBars())
         {
            TVector3 bv_point = Get3VectorBV(hit);
            double mdist = 999999.;
            TVector3 tpc_point;
            for (TVector3 tpc: tpc_points)
               {
                  double dist = GetGeometricDistance(bv_point,tpc);
                  if (dist>mdist) continue;
                  double dphii = GetDPhi(bv_point,tpc);
                  if (TMath::Abs(dphii) > max_dphi) continue;
                  double dzi = GetDZ(bv_point,tpc);
                  if (TMath::Abs(dzi) > max_dz) continue;
                  mdist=dist;
                  tpc_point=tpc;
               }
            if (mdist!=999999.)
               { 
                  // Match
                  int bar = hit->GetBar();
                  double dphi = GetDPhi(bv_point,tpc_point);
                  double dz = GetDZ(bv_point,tpc_point);
                  hdZ->Fill(bar,dz);
                  hdPhi->Fill(bar,dphi);
                  hdistMatch->Fill(bar,mdist);
                  hZBVvZTPC->Fill(bv_point.z(),tpc_point.z());
                  hPhiBVvPhiTPC->Fill(bv_point.Phi(),tpc_point.Phi());

                  hit->SetTPCHit(tpc_point);
               }
         }
   }

   void FillHistos(TAFlowEvent* flow)
   {
      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>(); // Gets list of bv hits from flow
      if (!bef) { return; }
      TBarEvent *barEvt=bef->BarEvent;
      for (BarHit* hit: barEvt->GetBars())
         {
            int bar = hit->GetBar();
            if (hit->IsTPCMatched())
               {
                  hBVTPCMatch->Fill(bar,1);
               }
            else
               {
                  hBVTPCMatch->Fill(bar,0);
               }
         }
   }

   void TimeOfFlight(TAFlowEvent* flow)
   {
      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>(); // Gets list of bv hits from flow
      if (!bef) { return; }
      TBarEvent *barEvt=bef->BarEvent;
      std::vector<BarHit*> bars = barEvt->GetBars();
      int nbv = bars.size();
      for (int i=0;i<nbv;i++)
         {
            for (int j=i;j<nbv;j++)
               {
                  BarHit* hit1 = bars.at(i);
                  BarHit* hit2 = bars.at(j);
                  double TOF = GetTOF(hit1,hit2);
                  double distBV = GetGeometricDistanceBV(hit1,hit2);

                  // Require different hits
                  if ( distBV < 1 ) continue;

                  hTOFbv->Fill(TOF);
                  hDistBV->Fill(distBV);
                  hDistBVvTOF->Fill(distBV,TOF);
                  hdPhiBV->Fill(TMath::Abs(GetDPhi(Get3VectorBV(hit1),Get3VectorBV(hit2))));
                  hdZedBV->Fill(GetDZ(Get3VectorBV(hit1),Get3VectorBV(hit2)));
                  hdPhiBVvTOF->Fill(TMath::Abs(GetDPhi(Get3VectorBV(hit1),Get3VectorBV(hit2))),TOF);
                  hdZedBVvTOF->Fill(GetDZ(Get3VectorBV(hit1),Get3VectorBV(hit2)),TOF);

                  // Require TPC matching to different tracks
                  if (!(hit1->IsTPCMatched() && hit2->IsTPCMatched())) continue;
                  double distTPC = GetGeometricDistanceTPC(hit1,hit2);
                  if ( distTPC < 1 ) continue;

                  hDistTPC->Fill(distTPC);
                  hDistTPCvTOF->Fill(distTPC,TOF);
                  hTOFdiffmatch->Fill(TOF);
                  hdPhiBVmatched->Fill(TMath::Abs(GetDPhi(Get3VectorBV(hit1),Get3VectorBV(hit2))));
                  hdZedBVmatched->Fill(GetDZ(Get3VectorBV(hit1),Get3VectorBV(hit2)));
                  hdPhiTPC->Fill(TMath::Abs(GetDPhi(hit1->GetTPC(),hit2->GetTPC())));
                  hdZedTPC->Fill(GetDZ(hit1->GetTPC(),hit2->GetTPC()));
                  hdPhiTPCvTOF->Fill(TMath::Abs(GetDPhi(hit1->GetTPC(),hit2->GetTPC())),TOF);
                  hdZedTPCvTOF->Fill(GetDZ(hit1->GetTPC(),hit2->GetTPC()),TOF);
               }
         }
   }

   //________________________________
   // HELPER FUNCTIONS

   double GetDZ(TVector3 p1, TVector3 p2)
   {
      return p1.z() - p2.z();
   }
   double GetDPhi(TVector3 p1, TVector3 p2)
   {
      //return std::min(TMath::Abs(p1.DeltaPhi(p2)),TMath::Abs(p2.DeltaPhi(p1)));
      return p1.DeltaPhi(p2);
   }
   double GetTOF(double t1_top, double t1_bot, double t2_top, double t2_bot)
   {
      return TMath::Abs(t2_top+t2_bot-t1_top-t1_bot)/2.;
   }
   double GetTOF(BarHit* hit1, BarHit* hit2)
   {
      double t1_top = hit1->GetTDCTop();
      double t2_top = hit2->GetTDCTop();
      double t1_bot = hit1->GetTDCBot();
      double t2_bot = hit2->GetTDCBot();
      return GetTOF(t1_top,t1_bot,t2_top,t2_bot);
   }
   TVector3 Get3VectorBV(BarHit* hit)
   {
      double xbv,ybv;
      hit->GetXY(xbv,ybv);
      double zbv = hit->GetTDCZed();
      TVector3 bv_point = TVector3(xbv*1000,ybv*1000,zbv*1000); // to mm
      return bv_point;
   }
   double GetGeometricDistance(TVector3 p1, TVector3 p2)
   {
      TVector3 diff = p2-p1;
      return diff.Mag();
   }
   double GetGeometricDistanceTPC(BarHit* hit1, BarHit* hit2)
   {
      TVector3 p1 = hit1->GetTPC();
      TVector3 p2 = hit2->GetTPC();
      return GetGeometricDistance(p1,p2);
   }
   double GetGeometricDistanceBV(BarHit* hit1, BarHit* hit2)
   {
      TVector3 p1 = Get3VectorBV(hit1);
      TVector3 p2 = Get3VectorBV(hit2);
      return GetGeometricDistance(p1,p2);
   }
};

class matchingModuleFactory: public TAFactory
{
public:
   MatchingModuleFlags fFlags;
public:
   void Help()
   {   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("matchingModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++)
         {
         if( args[i] == "--Bfield" )
            {
               fFlags.fMagneticField = atof(args[++i].c_str());
               printf("Magnetic Field (incompatible with --loadcalib)\n");
            }
         }
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
