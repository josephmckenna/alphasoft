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
   double max_dz = 5000; // mm
   double max_dphi = 0.3; // 0.3 rad, within 3 bars

   // TOF parameters
   double min_dphi = TMath::Pi()*2* 1.5/64; // two bars

   //Histogramm declaration
   TH2D* hdPhiMatch = NULL;
   TH2D* hdZMatch = NULL;
   TH2D* hZBVvZTPC = NULL;
   TH2D* hNHits = NULL;
   TH2D* hBVTPCMatch = NULL;
   TH2D* hAmpTopvZ = NULL;
   TH2D* hAmpBotvZ = NULL;
   TH2D* hQvsDT = NULL;
   TH2D* hQvsDT2 = NULL;
   TH2D* hZvsDT2 = NULL;
   TH2D* hZvsDT3 = NULL;

   std::vector<TString> names{ "2hits2tracksBV", "2hitsNtracksBV", "Nhits2tracksBV", "NhitsNtracksBV", "2hits2tracksTPC", "2hitsNtracksTPC", "Nhits2tracksTPC", "NhitsNtracksTPC", "2hits2tracksBV_NoMatch", "2hitsNtracksBV_NoMatch", "Nhits2tracksBV_NoMatch", "NhitsNtracksBV_NoMatch"  };
   static const int N_names = 12;
   std::vector<TH1D*> h_dZ = std::vector<TH1D*>(N_names);
   std::vector<TH1D*> h_dphi = std::vector<TH1D*>(N_names);
   std::vector<TH1D*> h_TOF = std::vector<TH1D*>(N_names);
   std::vector<TH2D*> h_dZ_dphi = std::vector<TH2D*>(N_names);
   std::vector<TH2D*> h_dZ_TOF = std::vector<TH2D*>(N_names);
   std::vector<TH2D*> h_dphi_TOF = std::vector<TH2D*>(N_names);


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
      hdPhiMatch = new TH2D("hdPhiMatch","Delta phi between BV hit and TPC track;Bar;Delta Phi [rad]",64,-0.5,63.5,32,0,180);
      hdZMatch = new TH2D("hdZMatch","Delta z between BV hit and TPC track;Bar;Delta Z [mm]",64,-0.5,63.5,1500,-3000,3000);
      hZBVvZTPC = new TH2D("hZBVvZTPC","Z position of BV hits and TPC track ends;Z of BV [mm];Z of TPC [mm]",1000,-3000,3000,1000,-3000,3000);
      hNHits = new TH2D("hNHits","Number of hits on BV and TPC per event;BV hits;TPC hits",10,-0.5,9.5,10,-0.5,9.5);
      hBVTPCMatch = new TH2D("hBVTPCMatch","Number of BV hits matched to TPC tracks;Bar;0 = Unmatched, 1 = Matched",64,-0.5,63.5,2,-0.5,1.5);
      hAmpTopvZ = new TH2D("hAmpTopvZ","Top ADC amplitude;Zed (TPC) [mm];Amplitude",1500,-3000,3000,100,0,35000);
      hAmpBotvZ = new TH2D("hAmpBotvZ","Bot ADC amplitude;Zed (TPC) [mm];Amplitude",1500,-3000,3000,100,0,35000);
      hQvsDT = new TH2D("hQvsDT","Q factor versus time difference;1/sqrt(ampl1) - 1/sqrt(ampl2);Bottom TDC time minus top TDC time [ns]",100,-0.03,0.03,100,-30,30);
      hQvsDT2 = new TH2D("hQvsDT2","Q factor versus corrected time difference;1/sqrt(ampl1) - 1/sqrt(ampl2);Bottom TDC time minus top TDC time (corrected) [ns]",100,-0.03,0.03,100,-30,30);
      hZvsDT2 = new TH2D("hZvsDT2","Z position (TPC) versus corrected time difference;Z position from TPC [mm];Bottom TDC time minus top TDC time (corrected) [ns]",100,-3000,-3000,100,-30,30);
      hZvsDT3 = new TH2D("hZvsDT3","Z position (TPC) versus fully corrected time difference;Z position from TPC [mm];Bottom TDC time minus top TDC time (fully corrected) [ns]",100,-3000,-3000,100,-30,30);

      for (int i=0; i<N_names; i++)
         {
            gDirectory->mkdir(names[i])->cd();
            h_dZ[i] = new TH1D(names[i]+"_dZ","Delta Z for "+names[i]+";Delta Z [mm]",1000,-3000,3000);
            h_dphi[i] = new TH1D(names[i]+"_dphi","Delta phi for "+names[i]+";Delta phi [rad]",32,0,180);
            h_TOF[i] = new TH1D(names[i]+"_TOF","Signed time of flight for "+names[i]+";Time of flight [s]",1000,-15e-9,15e-9);
            h_dZ_dphi[i] = new TH2D(names[i]+"_dZ_dphi","Delta Z vs Delta Phi for "+names[i]+";Delta Z [mm];Delta phi [rad]",1000,-3000,3000,32,0,180);
            h_dZ_TOF[i] = new TH2D(names[i]+"_dZ_TOF","Delta Phi vs Time of flight for "+names[i]+";Delta Z [mm];Time of flight [s]",1000,-3000,3000,1000,-15e-9,15e-9);
            h_dphi_TOF[i] = new TH2D(names[i]+"_dphi_TOF","Delta Phi vs Time of flight for "+names[i]+";Delta phi [rad];Time of flight [s]",32,0,180,1000,-15e-9,15e-9);
            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
            gDirectory->cd("bv_tpc_matching_module");
         }
   }


   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
      // Delete histos
      delete hdPhiMatch;
      delete hdZMatch;
      delete hZBVvZTPC;
      delete hNHits;
      delete hBVTPCMatch;
      delete hAmpTopvZ;
      delete hAmpBotvZ;
      delete hQvsDT;
      delete hQvsDT2;
      delete hZvsDT2;
      delete hZvsDT3;

      for (int i=0;i<N_names;i++) delete h_dZ[i];
      for (int i=0;i<N_names;i++) delete h_dphi[i];
      for (int i=0;i<N_names;i++) delete h_TOF[i];
      for (int i=0;i<N_names;i++) delete h_dZ_dphi[i];
      for (int i=0;i<N_names;i++) delete h_dZ_TOF[i];
      for (int i=0;i<N_names;i++) delete h_dphi_TOF[i];

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

/* Commented out to see if this is taking all the time
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif

      // Main functions
      if( fFlags->fMagneticField > 0. )
         {
            std::vector<TVector3> helix_points = GetHelices(flow);
            MatchPoints(flow, helix_points);
            TimeWalkCorrection(flow);
            FillHistos(flow);
            TimeOfFlight(flow, helix_points);
         }
      else
         {
            std::vector<TVector3> line_points = GetLines(flow);
            MatchPoints(flow, line_points);
            TimeWalkCorrection(flow);
            FillHistos(flow);
            TimeOfFlight(flow, line_points);
         }

//      #ifdef _TIME_ANALYSIS_
//         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bv_tpc_matching_module",timer_start);
//      #endif
*/
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
            int bar = hit->GetBar();
            if (bar<16) continue; // Cuts out the first 16 bars for now
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
                  double dphi = GetDPhi(bv_point,tpc_point);
                  double dz = GetDZ(bv_point,tpc_point);
                  hdZMatch->Fill(bar,dz);
                  hdPhiMatch->Fill(bar,180*dphi/TMath::Pi());
                  hZBVvZTPC->Fill(bv_point.z(),tpc_point.z());

                  hit->SetTPCHit(tpc_point);
               }
         }
   }

   void TimeWalkCorrection(TAFlowEvent* flow)
   {
      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>(); // Gets list of bv hits from flow
      if (!bef) {std::cout<<"matchingmodule: AgBarEventFlow not found!"<<std::endl; return; }
      TBarEvent *barEvt=bef->BarEvent;
      for (BarHit* hit: barEvt->GetBars())
         {
            if (not hit->IsTPCMatched()) continue;
            // Excludes saturated amplitude channels
            double topamp = hit->GetAmpTop();
            double botamp = hit->GetAmpBot();
            if (topamp>32000 or botamp>32000) continue;
            // Initial plot of Qfactor vs time difference
            double Qfactor = 1./TMath::Sqrt(botamp) - 1./TMath::Sqrt(topamp);
            double timediff = (hit->GetTDCBot() - hit->GetTDCTop())*1e9;
            hQvsDT->Fill( Qfactor, timediff );
            // Correct for trend
            double p0 = 1601.54;
            double p1 = 1.40796;
            double newtimediff = timediff - (p0*Qfactor+p1);
            hQvsDT2->Fill( Qfactor, newtimediff );
            // Initial plot of Z from TPC vs time difference
            double z = hit->GetTPC().z();
            hZvsDT2->Fill( z, newtimediff );
            // Correct for trend
            double q0 = 0;
            double q1 = 0;
            double newnewtimediff = newtimediff - (q0*z+q1);
            hZvsDT3->Fill ( z, newnewtimediff );
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
            double topamp = hit->GetTopHit()->GetAmp();
            double botamp = hit->GetBotHit()->GetAmp();
            double botminustop = hit->GetTDCBot() - hit->GetTDCTop();
            if (hit->IsTPCMatched())
               {
                  double ztpc = hit->GetTPC().z();
                  hBVTPCMatch->Fill(bar,1);
                  hAmpTopvZ->Fill(ztpc,topamp);
                  hAmpBotvZ->Fill(ztpc,botamp);
               }
            else
               {
                  hBVTPCMatch->Fill(bar,0);
               }
         }
   }

   void TimeOfFlight(TAFlowEvent* flow, std::vector<TVector3> tpc_points)
   {
      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>(); // Gets list of bv hits from flow
      if (!bef) { return; }
      TBarEvent *barEvt=bef->BarEvent;
      std::vector<BarHit*> bars = barEvt->GetBars();
      int nbv = bars.size();
      int ntpc = tpc_points.size();
      int case_name = -1;
      if ( nbv==2 and ntpc==2 ) case_name = 0;
      else if ( nbv==2 and ntpc>2) case_name = 1;
      else if ( nbv>2 and ntpc==2) case_name = 2;
      else if ( nbv>2 and ntpc>2) case_name = 3;
      else return;

      for (int i=0;i<nbv;i++)
         {
            for (int j=i;j<nbv;j++)
               {
                  if (j==i) continue;

                  BarHit* hit1;
                  BarHit* hit2;
                  if (Ordered(bars.at(i),bars.at(j))) { hit1=bars.at(i); hit2=bars.at(j); }
                  else { hit1=bars.at(j); hit2=bars.at(i); }

                  // Require hits with a minimum phi separation
                  double dphiBV = GetDPhi(Get3VectorBV(hit1),Get3VectorBV(hit2));
                  double dzBV = GetDZ(Get3VectorBV(hit1),Get3VectorBV(hit2));
                  double TOF = GetTOF(hit1,hit2);
                  if (TMath::Abs(dphiBV) < min_dphi) continue;

                  // No TPC matching requirements
                  h_dZ[case_name+8]->Fill(dzBV);
                  h_dphi[case_name+8]->Fill((180/TMath::Pi())*dphiBV);
                  h_TOF[case_name+8]->Fill(TOF);
                  h_dZ_dphi[case_name+8]->Fill(dzBV,(180/TMath::Pi())*dphiBV);
                  h_dZ_TOF[case_name+8]->Fill(dzBV,TOF);
                  h_dphi_TOF[case_name+8]->Fill((180/TMath::Pi())*dphiBV,TOF);
                        
                  // Require TPC matching to different tracks
                  if (!(hit1->IsTPCMatched() && hit2->IsTPCMatched())) continue;
                  double distTPC = GetGeometricDistanceTPC(hit1,hit2);
                  if ( distTPC < 0.00001 ) continue;

                  double dphiTPC = GetDPhi(hit1->GetTPC(),hit2->GetTPC());
                  double dzTPC = GetDZ(hit1->GetTPC(),hit2->GetTPC());

                  h_dZ[case_name]->Fill(dzBV);
                  h_dphi[case_name]->Fill((180/TMath::Pi())*dphiBV);
                  h_TOF[case_name]->Fill(TOF);
                  h_dZ_dphi[case_name]->Fill(dzBV,(180/TMath::Pi())*dphiBV);
                  h_dZ_TOF[case_name]->Fill(dzBV,TOF);
                  h_dphi_TOF[case_name]->Fill((180/TMath::Pi())*dphiBV,TOF);
                  h_dZ[case_name+4]->Fill(dzTPC);
                  h_dphi[case_name+4]->Fill((180/TMath::Pi())*dphiTPC);
                  h_TOF[case_name+4]->Fill(TOF);
                  h_dZ_dphi[case_name+4]->Fill(dzTPC,(180/TMath::Pi())*dphiTPC);
                  h_dZ_TOF[case_name+4]->Fill(dzTPC,TOF);
                  h_dphi_TOF[case_name+4]->Fill((180/TMath::Pi())*dphiTPC,TOF);
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
   double GetTOF(BarHit* hit1, BarHit* hit2)
   {
      return hit2->GetAverageTDCTime() - hit1->GetAverageTDCTime();
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
   bool Ordered(BarHit* hit1, BarHit* hit2)
   {
      double phi1 = hit1->GetPhi();
      double phi2 = hit2->GetPhi();
      double dphi = phi2-phi1;
      if (-2*TMath::Pi() < dphi and -1*TMath::Pi() >= dphi) return true;
      if (-1*TMath::Pi() < dphi and 0 >= dphi) return false;
      if (0 < dphi and TMath::Pi() >= dphi) return true;
      if (TMath::Pi() < dphi and 2*TMath::Pi() >= dphi) return false;
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
