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
   TH2D* hdPhi = NULL;
   TH2D* hdZ = NULL;
   TH2D* hdistMatch = NULL;
   TH2D* hZBVvZTPC = NULL;
   TH2D* hZBVminusZTPC = NULL;
   TH2D* hPhiBVvPhiTPC = NULL;
   TH2D* hNHits = NULL;
   TH2D* hBVTPCMatch = NULL;
   TH1D* hTOFbv = NULL;
   TH1D* hTOFdiffmatch = NULL;
   TH1D* hDistBV = NULL;
   TH2D* hDistBVvTOF = NULL;
   TH1D* hDistTPC = NULL;
   TH2D* hDistTPCvTOF = NULL;
   TH2D* hdPhivdZBV = NULL;
   TH2D* hdPhivdZBVmatched = NULL;
   TH2D* hdPhivdZTPC = NULL;
   TH2D* hdPhiBVvdZTPC = NULL;
   TH2D* hdPhiBVvdZTPC2hits = NULL;
   TH2D* hdZedBVvTOF = NULL;
   TH2D* hdZedTPCvTOF = NULL;
   TH2D* hAmpTopvZ = NULL;
   TH2D* hAmpBotvZ = NULL;
   std::vector<TH2D*> hdtZ = std::vector<TH2D*>(64);
   std::vector<TH2D*> hdtQ1 = std::vector<TH2D*>(64);
   TH2D* hAdtZ = NULL;
   TH2D* hAdtZ_dc = NULL;
   TH2D* hAdtQ1 = NULL;

   // Correction coefficients
   std::vector<std::vector<double>> cCoefs = std::vector<std::vector<double>>(64);

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
      hZBVminusZTPC = new TH2D("hZBVminusZTPC","Z position of BV hits minus TPC track ends;Bar;Z of BV minus Z of TPC [mm]",64,-0.5,63.5,1000,-3000,3000);
      hPhiBVvPhiTPC = new TH2D("hPhiBVvPhiTPC","Phi position of BV hits and TPC track ends;Phi of BV [rad];Phi of TPC [rad]",360,-TMath::Pi(),TMath::Pi(),360,-TMath::Pi(),TMath::Pi());
      hNHits = new TH2D("hNHits","Number of hits on BV and TPC per event;BV hits;TPC hits",10,-0.5,9.5,10,-0.5,9.5);
      hBVTPCMatch = new TH2D("hBVTPCMatch","Number of BV hits matched to TPC tracks;Bar;0 = Unmatched, 1 = Matched",64,-0.5,63.5,2,-0.5,1.5);
      hTOFbv = new TH1D("hTOFbv","Time of flight of all pairs of BV hits;Time of flight [s];Counts",1500,0,1.5*10e-9);
      hTOFdiffmatch = new TH1D("hTOFdiffmatch","Time of flight of all pairs of BV hits matched to different TPC tracks;Time of flight [s];Counts",1500,0,1.5*10e-9);
      hDistBV = new TH1D("hDistBV","Geometrical distance between BV hits;Distance [mm]",1000,0,2400);
      hDistBVvTOF = new TH2D("hDistBVvTOF","Geometrical distance between BV hits versus TOF;Distance [mm];Time of flight [s]",1000,0,2400,1500,0,1.5*10e-9);
      hDistTPC = new TH1D("hDistTPC","Geometrical distance between associated TPC track ends;Distance [mm]",1000,0,2400);
      hDistTPCvTOF = new TH2D("hDistTPCvTOF","Geometrical distance between associated TPC track ends versus TOF;Distance [mm];Time of flight [s]",1000,0,2400,1500,0,1.5*10e-9);
      hdPhivdZBV = new TH2D("hdPhivdZBV","Delta phi versus delta z for two BV hits;Delta Phi [rad];Delta Zed [mm]",64,0.,TMath::Pi(),1500,-3000,3000);
      hdPhivdZBVmatched = new TH2D("hdPhivdZBVmatched","Delta phi versus delta z for two BV hits after TPC matching;Delta Phi [rad];Delta Zed [mm]",64,0.,TMath::Pi(),1500,-3000,3000);
      hdPhivdZTPC = new TH2D("hdPhivdZTPC","Delta phi versus delta z for two TPC hits;Delta Phi [rad];Delta Zed [mm]",180,0.,TMath::Pi(),1500,-3000,3000);
      hdPhiBVvdZTPC = new TH2D("hdPhiBVvdZTPC","Delta phi (BV) versus delta z (TPC);Delta Phi [rad];Delta Zed [mm]",64,0.,TMath::Pi(),1500,-3000,3000);
      hdPhiBVvdZTPC2hits = new TH2D("hdPhiBVvdZTPC2hits","Delta phi (BV) versus delta z (TPC) for two hits;Delta Phi [rad];Delta Zed [mm]",64,0.,TMath::Pi(),1500,-3000,3000);
      hdZedBVvTOF = new TH2D("hdZedBVvTOF","Delta z between two BV hits vs TOF;Delta Zed [mm];Time of flight [s]",1500,-3000,3000,1500,0,1.5*10e-9);
      hdZedTPCvTOF = new TH2D("hdZedTPCvTOF","Delta z between two TPC hits vs TOF;Delta Zed [mm];Time of flight [s]",1500,-3000,3000,1500,0,1.5*10e-9);
      hAmpTopvZ = new TH2D("hAmpTopvZ","Top ADC amplitude;Zed (TPC) [mm];Amplitude",1500,-3000,3000,100,0,35000);
      hAmpBotvZ = new TH2D("hAmpBotvZ","Bot ADC amplitude;Zed (TPC) [mm];Amplitude",1500,-3000,3000,100,0,35000);
      hAdtZ = new TH2D("hAdtZ","Z_tpc vs TDC time difference;Zed (TPC) [mm];Bottom TDC time - top TDC time [ns]",1000,-1500,1500,1000,-40,40);
      hAdtZ_dc = new TH2D("hAdtZ_dc","Z_tpc vs TDC time difference (after correction);Zed (TPC) [mm];Bottom TDC time - top TDC time [ns]",1000,-1500,1500,1000,-10,10);
      hAdtQ1 = new TH2D("hAdtQ1","Amplitude combination 1 vs TDC time difference;1/#sqrt{Q_{bot}} - 1/#sqrt{Q_{top}} ;Bottom TDC time - top TDC time [ns]",1000,-0.03,0.03,1000,-30,30);

      gDirectory->mkdir("hdtZ")->cd();
      for (int i=0;i<64;i++) hdtZ[i] = new TH2D(Form("hdtZ%d",i),"Z_tpc vs TDC time difference;Zed (TPC) [mm];Bottom TDC time - top TDC time [ns]",1000,-1500,1500,1000,-40,40);
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->cd("bv_tpc_matching_module");
      gDirectory->mkdir("hdtQ1")->cd();
      for (int i=0;i<64;i++) hdtQ1[i] = new TH2D(Form("hdtQ1%d",i),"Amplitude combination 1 vs TDC time difference;1/#sqrt{Q_{bot}} - 1/#sqrt{Q_{top}} ;Bottom TDC time - top TDC time [ns]",1000,-0.03,0.03,1000,-30,30);
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->cd("bv_tpc_matching_module");


      // Load correction coefficients
      std::cout<<"Loading coefs"<<std::endl;
      TString mapfile=getenv("AGRELEASE");
      mapfile+="/ana/bscint/";
      mapfile+="bscint_corrections.coef";
      std::ifstream fbscCoef(mapfile.Data());
      if(fbscCoef)
         {
            std::string comment;
            getline(fbscCoef, comment);
            std::cout<<comment<<std::endl;
            for(int bar_ind=0; bar_ind<64; bar_ind++)
               {
                  std::cout<<"getting bar "<<bar_ind<<std::endl;
                  cCoefs[bar_ind] = std::vector<double>(3);
                  fbscCoef >> cCoefs[bar_ind][0] >> cCoefs[bar_ind][1] >> cCoefs[bar_ind][2];
                  std::cout<<"getting bar "<<bar_ind<<std::endl;
               }
            fbscCoef.close();
         }
      for (int bar_ind=0;bar_ind<64;bar_ind++)
         {
            std::cout<<"Bar: "<<cCoefs[bar_ind][0]<<"\tSlope: "<<cCoefs[bar_ind][1]<<"\tIntercept: "<<cCoefs[bar_ind][2]<<std::endl;
         }

   }

   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
      // Delete histos
      delete hdPhi;
      delete hdZ;
      delete hdistMatch;
      delete hZBVvZTPC;
      delete hZBVminusZTPC;
      delete hPhiBVvPhiTPC;
      delete hBVTPCMatch;
      delete hTOFbv;
      delete hTOFdiffmatch;
      delete hDistBV;
      delete hDistBVvTOF;
      delete hDistTPC;
      delete hDistTPCvTOF;
      delete hdPhivdZBV;
      delete hdPhivdZBVmatched;
      delete hdPhivdZTPC;
      delete hdPhiBVvdZTPC;
      delete hdPhiBVvdZTPC2hits;
      delete hdZedBVvTOF;
      delete hdZedTPCvTOF;
      delete hAmpTopvZ;
      delete hAmpBotvZ;
      for (int i=0;i<64;i++) delete hdtZ[i];
      for (int i=0;i<64;i++) delete hdtQ1[i];
      delete hAdtZ;
      delete hAdtZ_dc;
      delete hAdtQ1;
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
                  hdZ->Fill(bar,dz);
                  hdPhi->Fill(bar,dphi);
                  hdistMatch->Fill(bar,mdist);
                  hZBVvZTPC->Fill(bv_point.z(),tpc_point.z());
                  hZBVminusZTPC->Fill(bar,bv_point.z()-tpc_point.z());
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
            double topamp = hit->GetTopHit()->GetAmp();
            double botamp = hit->GetBotHit()->GetAmp();
            double botminustop = hit->GetTDCBot() - hit->GetTDCTop();
            if (hit->IsTPCMatched())
               {
                  double ztpc = hit->GetTPC().z();
                  double botminustop_dc = decorrelate_dtZ(botminustop,bar,ztpc);
                  hBVTPCMatch->Fill(bar,1);
                  hdtZ[bar]->Fill(ztpc,botminustop*1e9);
                  hAdtZ->Fill(ztpc,botminustop*1e9);
                  hAdtZ_dc->Fill(ztpc,botminustop_dc*1e9);
                  // Exclude saturated hits
                  if (botamp>31000) continue;
                  if (topamp>31000) continue;
                  hdtQ1[bar]->Fill(1/TMath::Sqrt(botamp)-1/TMath::Sqrt(topamp),botminustop*1e9);
                  hAdtQ1->Fill(1/TMath::Sqrt(botamp)-1/TMath::Sqrt(topamp),botminustop*1e9);
                  hAmpTopvZ->Fill(ztpc,topamp);
                  hAmpBotvZ->Fill(ztpc,botamp);
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
      if (nbv==2 && bars[0]->IsTPCMatched() && bars[1]->IsTPCMatched() && GetGeometricDistanceBV(bars[0],bars[1])>1)
         {
            BarHit* hita = bars.at(0);
            BarHit* hitb = bars.at(1);
            BarHit* hit1;
            BarHit* hit2;
            if (hita->GetAverageTDCTime() < hitb->GetAverageTDCTime()) { hit1 = hita; hit2 = hitb; }
            else { hit1 =hitb; hit2 = hita; }
            double dphiBV = TMath::Abs(GetDPhi(Get3VectorBV(hit1),Get3VectorBV(hit2)));
            double dZedTPC = GetDZ(hit1->GetTPC(),hit2->GetTPC());
            double distTPC = GetGeometricDistanceTPC(hit1,hit2);
            if ( distTPC > 1 && dphiBV>min_dphi)  hdPhiBVvdZTPC2hits->Fill(dphiBV,dZedTPC);
         }
      for (int i=0;i<nbv;i++)
         {
            for (int j=i;j<nbv;j++)
               {
                  BarHit* hita = bars.at(i);
                  BarHit* hitb = bars.at(j);
                  BarHit* hit1;
                  BarHit* hit2;
                  if (hita->GetAverageTDCTime() < hitb->GetAverageTDCTime()) { hit1 = hita; hit2 = hitb; }
                  else { hit1 =hitb; hit2 = hita; }
                  double TOF = GetTOF(hit1,hit2);
                  double distBV = GetGeometricDistanceBV(hit1,hit2);
                  double dphiBV = TMath::Abs(GetDPhi(Get3VectorBV(hit1),Get3VectorBV(hit2)));

                  // Require different hits
                  if ( distBV < 1 ) continue;

                  // Require hits with a minimum phi separation
                  if (dphiBV < min_dphi) continue;

                  double dZedBV = GetDZ(Get3VectorBV(hit1),Get3VectorBV(hit2));
                  hTOFbv->Fill(TOF);
                  hDistBV->Fill(distBV);
                  hDistBVvTOF->Fill(distBV,TOF);
                  hdZedBVvTOF->Fill(dZedBV,TOF);
                  hdPhivdZBV->Fill(dphiBV,dZedBV);

                  // Require TPC matching to different tracks
                  if (!(hit1->IsTPCMatched() && hit2->IsTPCMatched())) continue;
                  double distTPC = GetGeometricDistanceTPC(hit1,hit2);
                  double dphiTPC = TMath::Abs(GetDPhi(hit1->GetTPC(),hit2->GetTPC()));
                  double dZedTPC = GetDZ(hit1->GetTPC(),hit2->GetTPC());
                  if ( distTPC < 1 ) continue;

                  hDistTPC->Fill(distTPC);
                  hDistTPCvTOF->Fill(distTPC,TOF);
                  hTOFdiffmatch->Fill(TOF);
                  hdPhivdZBVmatched->Fill(dphiBV,dZedBV);
                  hdZedTPCvTOF->Fill(dZedTPC,TOF);
                  hdPhivdZTPC->Fill(dphiTPC,dZedTPC);
                  hdPhiBVvdZTPC->Fill(dphiBV,dZedTPC);
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
   double GetTOF(double t1_top, double t1_bot, double t2_top, double t2_bot)
   {
      return TMath::Abs(t2_top+t2_bot-t1_top-t1_bot)/2.;
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
   double decorrelate_dtZ(double timediff,int bar,double z_tpc)
   {
      double slope = cCoefs[bar][1];
      double intercept = cCoefs[bar][2];
      double fit = slope*z_tpc + intercept;
      return timediff-fit*1e-9;
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
