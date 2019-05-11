#include "AgFlow.h"
#include "TH2D.h"
#include "SignalsType.h"

#include "TStoreLine.hh"
#include "TStoreHelix.hh"
#include "TSpacePoint.hh"

#include "AnalysisTimer.h"
#include "AnaSettings.h"

class PHspectrumFlags
{
public:  
   bool fEnabled;
   double fMagneticField;
   AnaSettings* ana_settings;
  
   PHspectrumFlags():fEnabled(false),// ctor
                     fMagneticField(1.0),
                     ana_settings(0)
   { }
  
   ~PHspectrumFlags() // dtor
   { }
};

class PHspectrum: public TARunObject
{
public:
   PHspectrumFlags* fFlags;

private:
   TH2D* hawphspect;
   TH2D* hpadphspect;
   TH2D* hpwbphspect;
   TH2D* hadcphspect;
   
   padmap* pmap;

   double fCoincTime; // ns

public:
   PHspectrum(TARunInfo* runinfo, PHspectrumFlags* f):TARunObject(runinfo),
                                                      fFlags(f),pmap(0),
                                                      fCoincTime(20.)
   {}
   ~PHspectrum() {}

   void BeginRun(TARunInfo* runinfo)
   {
      if(!fFlags->fEnabled) return;
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("phspectrum")->cd();

      hawphspect = new TH2D("hawphspect","Avalanche Size Anodes for Tracks;AW;Ne",256,0.,256.,200,0.,2000.);
      hpadphspect = new TH2D("hpadphspect","Avalanche Size Pads for Tracks;pad;Ne",32*576,0.,_padcol*_padrow,200,0.,10000.);
      hpwbphspect = new TH2D("hpwbphspect","Max PWB P.H.;pad;PH",32*576,0.,_padcol*_padrow,1000,0.,4200.);
      hadcphspect = new TH2D("hadcphspect","Max ADC P.H.;AW;PH",256,0.,256.,1000,0.,17000.);

      pmap = new padmap;
      
      if (fFlags->ana_settings)
         fCoincTime = fFlags->ana_settings->GetDouble("MatchModule","coincTime");
   }

   void EndRun(TARunInfo* runinfo){}
   void PauseRun(TARunInfo* runinfo){}
   void ResumeRun(TARunInfo* runinfo){}
   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if(!fFlags->fEnabled) return flow;

      AgAnalysisFlow* AnaFlow = flow->Find<AgAnalysisFlow>();
      if( !AnaFlow ) return flow;
      TStoreEvent* e = AnaFlow->fEvent;
      if( !e ) return flow;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) return flow;

      std::vector<signal> adc32 = SigFlow->adc32max;
      std::vector<signal> pwb = SigFlow->pwbMax;
      
      std::vector<signal> aws = SigFlow->awSig;
      std::vector<signal> pads = SigFlow->pdSig;

      if( fFlags->fMagneticField > 0. )
         HelPHspect(e,adc32,pwb,aws,pads);
      else
         LinePHspect(e,adc32,pwb,aws,pads);
      
#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"PHspectrum_module");
#endif
      return flow;
   }

   void FillHistos(const TObjArray* points,
                   std::vector<signal> &adc32, std::vector<signal> &pwb,
                   std::vector<signal> &aws, std::vector<signal> &pads)
   {
      int nPoints = points->GetEntriesFast();
      for(int j=0; j<nPoints; ++j )
         {
            TSpacePoint* sp = (TSpacePoint*) points->At(j);
            double time = sp->GetTime();
            int wire = sp->GetWire();
            for(auto& s: aws)
               {
                  if( s.idx == wire && fabs(s.t - time) < 1.e-3 )
                     {
                        hawphspect->Fill(double(wire),s.height);
                     }
               }
            for(auto& s: adc32)
               {
                  if( s.idx == wire )
                     {
                        hadcphspect->Fill(double(wire),s.height);
                     }
               }
            
            int sec,row,pad=sp->GetPad();
            pmap->get(pad,sec,row);
            for(auto& s: pads)
               {
                  if( s.idx == row && s.sec == sec && fabs(s.t - time) < fCoincTime )
                     {
                        hpadphspect->Fill(double(pad),s.height);
                     }
               }
            for(auto& s: pwb)
               {
                  if( s.idx == row && s.sec == sec )
                     {
                        hpwbphspect->Fill(double(pad),s.height);
                     }
               }
         }// points loop 
   }

   void HelPHspect(TStoreEvent* anEvent,
                   std::vector<signal> &adc32, std::vector<signal> &pwb,
                   std::vector<signal> &aws, std::vector<signal> &pads)
   {
      const TObjArray* helices = anEvent->GetHelixArray();
      int nTracks = helices->GetEntriesFast();
      std::cout<<"PHspectrum::HelPHspect event # "<<anEvent->GetEventNumber()<<" @ "<<anEvent->GetTimeOfEvent()<<"s found: "<<nTracks<<" tracks"<<std::endl;
      for( int i=0; i<nTracks; ++i )
         {
            TStoreHelix* h = (TStoreHelix*) helices->At(i);
            const TObjArray* points = h->GetSpacePoints();
            FillHistos( points, adc32, pwb, aws, pads );
         }// tracks loop
   }// function: HelPHspect

   void LinePHspect(TStoreEvent* anEvent,
                    std::vector<signal> &adc32, std::vector<signal> &pwb,
                    std::vector<signal> &aws, std::vector<signal> &pads)
   {
      const TObjArray* lines = anEvent->GetLineArray();
      int nTracks = lines->GetEntriesFast();
      std::cout<<"PHspectrum::LinePHspect event # "<<anEvent->GetEventNumber()<<" @ "<<anEvent->GetTimeOfEvent()<<"s found: "<<nTracks<<" tracks"<<std::endl;
      for( int i=0; i<nTracks; ++i )
         {
            TStoreLine* l = (TStoreLine*) lines->At(i);
            const TObjArray* points = l->GetSpacePoints();
            FillHistos( points, adc32, pwb, aws, pads );
         }// tracks loop
   }// function: LinePHspect
};


class PHspectrumFactory: public TAFactory
{
public:
   PHspectrumFlags fFlags;
  
   void Init(const std::vector<std::string> &args)
   {     
      TString json="default";
      printf("PHspectrumFactory::Init!\n");
      for(unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--phspect" ) fFlags.fEnabled = true; 
            if( args[i] == "--anasettings" ) json=args[i+1];
         }
      fFlags.ana_settings=new AnaSettings(json.Data());
   }

   void Finish()
   {
      printf("PHspectrumFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("PHspectrumFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PHspectrum(runinfo, &fFlags);
   }
};

static TARegister tar(new PHspectrumFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
