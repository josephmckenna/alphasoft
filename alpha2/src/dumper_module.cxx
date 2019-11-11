

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "A2Flow.h"

#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"

#include "AnalysisTimer.h"
#include "generalizedspher.h"
class DumperFlags
{
public:
   bool fPrint = false;

};

class Dumper: public TARunObject
{
private:
   OnlineMVAStruct* OnlineVars;
   
  //TString gVarList="nhits,residual,r,S0rawPerp,S0axisrawZ,phi_S0axisraw,nCT,nGT,tracksdca,curvemin,curvemean,lambdamin,lambdamean,curvesign,";
  
public:
   DumperFlags* fFlags;
   bool fTrace = false;
   
   
   Dumper(TARunInfo* runinfo, DumperFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("Dumper::ctor!\n");
   }

   ~Dumper()
   {
      if (fTrace)
         printf("Dumper::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Dumper::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Dumper::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Dumper::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }
  
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      AlphaEventFlow* fe=flow->Find<AlphaEventFlow>();
      if (!fe)
         return flow;
      TAlphaEvent* alphaEvent=fe->alphaevent;
      SilEventsFlow* sf=flow->Find<SilEventsFlow>();
      if (!sf)
         return flow;
      TSiliconEvent* siliconEvent=sf->silevent;
      #ifdef _TIME_ANALYSIS_
         clock_t timer_start=clock();
      #endif
      
      
      OnlineVars=new OnlineMVAStruct();
      
      OnlineVars->nhits=alphaEvent->GetNHits();
      OnlineVars->residual = siliconEvent->GetResidual();
      TVector3* vtx = siliconEvent->GetVertex();
      OnlineVars->r = vtx->Perp();
      OnlineVars->phi = vtx->Phi();
      OnlineVars->tracksdca = siliconEvent->GetDCA();
      //z = vtx->Z();
      std::vector<double> velxraw;
      std::vector<double> velyraw;
      std::vector<double> velzraw;

      Int_t nAT =  alphaEvent->GetNHelices(); // all tracks
      OnlineVars->nCT = 0;
      Int_t nraw = 0;
  
      //Unused
      //Double_t AT_MeanHitSig=0; //Average hit significance
      //Unused
      //Double_t CT_MeanHitSig=0; //Average hit significance
      Double_t AT_SumHitSig=0;
      Double_t CT_SumHitSig=0;
      Int_t AT_HitSigCounter=0;
      Int_t CT_HitSigCounter=0;
      for (int i = 0; i< nAT ; ++i)
      {
        TAlphaEventHelix* aehlx = alphaEvent->GetHelix(i);
        if (!aehlx) continue;
        //Double_t fc = aehlx->Getfc();
        Double_t fphi0 = aehlx->Getfphi();
        Double_t fLambda = aehlx->Getflambda();
        //Double_t s=0.; // calculates velx,y,z at POCA
        // special case for s = 0
        Int_t HelixHits=aehlx->GetNHits();
        for (int j=0; j<HelixHits; j++)
        {
          TAlphaEventHit* aehlx_hit=aehlx->GetHit(j);
          AT_SumHitSig+=aehlx_hit->GetHitSignifance();
          AT_HitSigCounter++;
          //delete aehlx_hit;
        }
        // select good helices, after removal of duplicates
      
        // This seems to be true all the time... please check - Joe
        if (aehlx->GetHelixStatus()==1)
        {
          for (int j=0; j<HelixHits; j++)
          {
            TAlphaEventHit* aehlx_hit=aehlx->GetHit(j);
            CT_SumHitSig+=aehlx_hit->GetHitSignifance();
            CT_HitSigCounter++;
            //delete aehlx_hit;
          }
          ++nraw; // == ntracks
          velxraw.push_back( - TMath::Sin(fphi0));
          velyraw.push_back( TMath::Cos(fphi0)) ;
          velzraw.push_back( fLambda );
          OnlineVars->nCT++;
        }
      }
      std::vector<double> velx;
      std::vector<double> vely;
      std::vector<double> velz;
      // alpha event part
      TAlphaEventVertex* aevtx = alphaEvent->GetVertex();
      Int_t nGTL = aevtx->GetNHelices();// tracks with vertex
      OnlineVars->nGT = 0;
      OnlineVars->curvemin=9999.;
      OnlineVars->curvemean=0.;
      OnlineVars->lambdamin=9999.;
      OnlineVars->lambdamean=0;
      OnlineVars->curvesign=0;
      for (int i = 0; i< nGTL ; ++i)
      {
        TAlphaEventHelix* aehlx = aevtx->GetHelix(i);
        //    if(aehlx->GetHelixStatus()<0) continue;
        Double_t fc = aehlx->Getfc();
        Double_t fphi0 = aehlx->Getfphi();
        Double_t fLambda = aehlx->Getflambda();

        //Unused
        //Double_t s=0.; // calculates velx,y,z at POCA
        // special case for s = 0
        velx.push_back( - TMath::Sin(fphi0) );
        vely.push_back( TMath::Cos(fphi0) ) ;
        velz.push_back( fLambda );

        // select good helices, after removal of duplicates
        if (aehlx->GetHelixStatus()==1)
        {
          OnlineVars->nGT++;
          OnlineVars->curvemin= fabs(fc)>OnlineVars->curvemin? OnlineVars->curvemin:fabs(fc);
          OnlineVars->lambdamin= fabs(fLambda)>OnlineVars->lambdamin? OnlineVars->lambdamin:fabs(fLambda);
          OnlineVars->curvemean+=fabs(fc);
          OnlineVars->lambdamean+=fabs(fLambda);
          OnlineVars->curvesign+=(fc>0)?1:-1;
        }
    }
  if(OnlineVars->nGT>0){
    OnlineVars->lambdamean/=OnlineVars->nGT;
    OnlineVars->curvemean/=OnlineVars->nGT;
  }
      
      //Unused
      //Double_t S0rawl1 = -99.;
      //Unused
      //Double_t S0rawl2 = -99.;
      //Unused
      //Double_t S0rawl3 = -99.;
      //Unused
      //Double_t S0axisrawX = -99.;
      //Unused
      //Double_t S0axisrawY = -99.;
      OnlineVars->S0axisrawZ = -99.;

      if(nraw>0)
      {
        TVector3* S0axisraw;
        TVector3* S0valuesraw;
        sphericity(velxraw, velyraw, velzraw, 0, &S0axisraw, &S0valuesraw); // generalizedspher.h

        OnlineVars->S0rawPerp = S0valuesraw->Perp();
        //Unused
        //S0rawl1 = S0valuesraw->X();
        //Unused
        //S0rawl2 = S0valuesraw->Y();
        //Unused
        //S0rawl3 = S0valuesraw->Z();

        //Unused
        //S0axisrawX = S0axisraw->X();
        //Unused
        //S0axisrawY = S0axisraw->Y();
        OnlineVars->S0axisrawZ = S0axisraw->Z();
        //phi_S0axisraw = TMath::ACos(S0axisrawY/TMath::Sqrt(S0axisrawX*S0axisrawX+S0axisrawY*S0axisrawY));
        delete S0axisraw;
        delete S0valuesraw;
      }
      flow=new A2OnlineMVAFlow(flow,OnlineVars);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"dumper_module",timer_start);
      #endif
      return flow; 
  }
};

class DumperFactory: public TAFactory
{
public:
   DumperFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("DumperFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("DumperFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("DumperFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new Dumper(runinfo, &fFlags);
   }
};

static TARegister tar(new DumperFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
