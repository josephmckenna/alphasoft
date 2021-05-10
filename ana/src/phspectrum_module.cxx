#include "AgFlow.h"
#include "RecoFlow.h"

#include "TH2D.h"
#include "SignalsType.hh"

#include "TStoreLine.hh"
#include "TStoreHelix.hh"
#include "TSpacePoint.hh"

#include "AnaSettings.hh"

#include <cctype>

class PHspectrumFlags
{
public:  
   bool fEnabled;
   int fNtracks;
   double fMagneticField;
   AnaSettings* ana_settings;
  
   PHspectrumFlags():fEnabled(false),// ctor
                     fNtracks(1),fMagneticField(1.0),
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
   
   ALPHAg::padmap* pmap;

   int fNtracks;
   double fCoincTime; // ns

public:
   PHspectrum(TARunInfo* runinfo, PHspectrumFlags* f):TARunObject(runinfo),
                                                      fFlags(f),pmap(0),
                                                      fNtracks(1),fCoincTime(20.)
   {
#ifdef MANALYZER_PROFILER
      ModuleName="PHspectrum Module";
#endif
   }
   ~PHspectrum() {}

   void BeginRun(TARunInfo* runinfo)
   {
      if(!fFlags->fEnabled) return;
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("phspectrum")->cd();

      hawphspect = new TH2D("hawphspect","Avalanche Size Anodes for Tracks;AW;Ne",256,0.,256.,200,0.,2000.);
      hpadphspect = new TH2D("hpadphspect","Avalanche Size Pads for Tracks;pad;Ne",32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,
                             200,0.,10000.);
      hpwbphspect = new TH2D("hpwbphspect","Max PWB P.H.;pad;PH",32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,1000,0.,4200.);
      hadcphspect = new TH2D("hadcphspect","Max ADC P.H.;AW;PH",256,0.,256.,1000,0.,17000.);

      pmap = new ALPHAg::padmap;

      fNtracks = fFlags->fNtracks;
      if (fFlags->ana_settings)
         fCoincTime = fFlags->ana_settings->GetDouble("MatchModule","coincTime");

      printf("PHspectrum::BeginRun() Run %d, Events with >= %d tracks\n", runinfo->fRunNo,fNtracks);
   }

   void EndRun(TARunInfo* runinfo){ delete pmap; }
   void PauseRun(TARunInfo* runinfo){}
   void ResumeRun(TARunInfo* runinfo){}
   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if(!fFlags->fEnabled)
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      AgAnalysisFlow* AnaFlow = flow->Find<AgAnalysisFlow>();
      if( !AnaFlow )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TStoreEvent* e = AnaFlow->fEvent;
      if( !e )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      std::vector<ALPHAg::signal>* adc32 = SigFlow->adc32max;
      std::vector<ALPHAg::signal>* pwb = SigFlow->pwbMax;
      
      std::vector<ALPHAg::signal>* aws = SigFlow->awSig;
      std::vector<ALPHAg::signal>* pads = SigFlow->pdSig;

      if( fFlags->fMagneticField > 0. )
         HelPHspect(e,*adc32,*pwb,*aws,*pads);
      else
         LinePHspect(e,*adc32,*pwb,*aws,*pads);
      
      return flow;
   }

   void FillHistos(const TObjArray* points,
                   std::vector<ALPHAg::signal> &adc32, std::vector<ALPHAg::signal> &pwb,
                   std::vector<ALPHAg::signal> &aws, std::vector<ALPHAg::signal> &pads)
   {
      int nPoints = points->GetEntriesFast();
      std::cout<<"PHspectrum::FillHistos() # of points: "<<nPoints<<std::endl;
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
                   std::vector<ALPHAg::signal> &adc32, std::vector<ALPHAg::signal> &pwb,
                   std::vector<ALPHAg::signal> &aws, std::vector<ALPHAg::signal> &pads)
   {
      const TObjArray* helices = anEvent->GetHelixArray();
      int nTracks = helices->GetEntriesFast();
      std::cout<<"PHspectrum::HelPHspect event # "<<anEvent->GetEventNumber()<<" @ "<<anEvent->GetTimeOfEvent()<<"s found: "<<nTracks<<" tracks"<<std::endl;
      if( nTracks < fNtracks ) return;
      for( int i=0; i<nTracks; ++i )
         {
            TStoreHelix* h = (TStoreHelix*) helices->At(i);
            const TObjArray* points = h->GetSpacePoints();
            FillHistos( points, adc32, pwb, aws, pads );
         }// tracks loop
   }// function: HelPHspect

   void LinePHspect(TStoreEvent* anEvent,
                    std::vector<ALPHAg::signal> &adc32, std::vector<ALPHAg::signal> &pwb,
                    std::vector<ALPHAg::signal> &aws, std::vector<ALPHAg::signal> &pads)
   {
      const TObjArray* lines = anEvent->GetLineArray();
      int nTracks = lines->GetEntriesFast();
      std::cout<<"PHspectrum::LinePHspect event # "<<anEvent->GetEventNumber()<<" @ "<<anEvent->GetTimeOfEvent()<<"s found: "<<nTracks<<" tracks"<<std::endl;
      if( nTracks < fNtracks ) return;
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

   void Help()
   {
      printf("PHspectrumFactory::Help\n");
      printf("\t--phspect [Ntracks]\tEnable extraction of Pulse Height Spectra for tracks, default number of tracks is Ntracks = %d\n",fFlags.fNtracks);
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {     
      TString json="default";
      printf("PHspectrumFactory::Init!\n");
      for(unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--phspect" ) 
               {
                  fFlags.fEnabled = true;
                  if( std::isdigit(args[i+1][0]) )
                     fFlags.fNtracks = std::stoi(args[i+1]);
               }
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
