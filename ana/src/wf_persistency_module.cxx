#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include "AnaSettings.hh"

#include <TString.h>
#include <TH1D.h>

#include <cctype>
#include <vector>


class WFpersistencyFlags
{
public:
   bool fEnabled=false;
   int fAnodeWire=-1;
   bool fVerbose=true;
   AnaSettings* fSettings=0;
   
public:
   WFpersistencyFlags() // ctor
   { }

   ~WFpersistencyFlags() // dtor
   { }
};


class WFpersistencyModule: public TARunObject
{
private:
   int fCounter;
   int fTarget;
   bool fTrace;
   double fADCThres;

   std::vector<TH1D*> hwf;

public:
   WFpersistencyFlags* fFlags = 0;
 
   WFpersistencyModule(TARunInfo* runinfo, WFpersistencyFlags* flags): TARunObject(runinfo),
                                                                       fFlags( flags )
   {
      fTrace=fFlags->fVerbose;
      ModuleName="WFpersistencyModule";
      if( fTrace )
         printf("WFpersistencyModule::ctor!\n");
    
      fCounter=0;
      fTarget=fFlags->fAnodeWire;

      hwf.reserve( 256 );
   }

   ~WFpersistencyModule()
   {
      if (fTrace)
         printf("WFpersistencyModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
#ifdef HAVE_CXX11_THREADS
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
#endif
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      fADCThres=fFlags->fSettings->GetDouble("DeconvModule","ADCthr");
      if( fFlags->fEnabled )
         {
            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
            gDirectory->mkdir("awpersistency");
            gDirectory->cd("awpersistency");
            for(int aw=0; aw<256; ++aw)
               {
                  TString hname=TString::Format("hwfaw%03d",aw);
                  TString htitle=TString::Format("Persistency WF for AW %d - Pedestal Subtracted;bin;ADC",aw);
                  hwf[aw] = new TH1D(hname,htitle,412,0.,412.);
                  for(int b=1; b<=412; ++b) hwf[aw]->Fill(b,0.);
                  //printf("WFpersistencyModule Enabled\tADC thrshold %1.0f\tTarget Channel %d\n",fADCThres,aw);
               }
         }
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("WFpersistencyModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
   }
   void PauseRun(TARunInfo* runinfo){}
   void ResumeRun(TARunInfo* runinfo){}

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      // module disabled
      if( !fFlags->fEnabled )
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
      if( fTrace )
         printf("WFpersistencyModule::AnalyzeFlowEvent\n");

      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow )
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
      std::vector<ALPHAg::wf_ref>* awwf=SigFlow->AWwf;
      if( !awwf )
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
#ifdef _TIME_ANALYSIS_
      START_TIMER
#endif   
         if( awwf->size() <= 0 ) 
            {
               *flags|=TAFlag_SKIP_PROFILE;
               return flow;
            }
      
      for(auto it=awwf->begin();it!=awwf->end();++it)
         {
            if( fTrace )
               printf("WFpersistencyModule AW %d\n",it->i);
           
            for(auto jt=it->wf->begin();jt!=it->wf->end();++jt)
               {
                  hwf[it->i]->Fill(std::distance(it->wf->begin(),jt),*jt);
               }
         }
      return flow;
   }

};

class WFpersistencyModuleFactory: public TAFactory
{
public:
   WFpersistencyFlags fFlags;

public:
   void Help()
   {
      printf("WFpersistencyModuleFactory::Help!\n");
      printf("\t--persistency [AW number]    Enable Persistency mode\n");
      printf("\t--verbose                    Printout what's going on\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {    
      TString json="default";
      printf("WFpersistencyModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) 
         {
            if( args[i]=="-h" || args[i]=="--help" )
               Help();
	  
            if( args[i] == "--verbose" )
               fFlags.fVerbose = true;

            if( args[i] == "--anasettings" ) json=args[i+1];

            if( args[i] == "--persistency" ) 
               {
                  fFlags.fEnabled=true;
                  // if( std::isdigit(args[i+1][0]) )
                  //    fFlags.fAnodeWire=std::stoi(args[i+1]);
                  // else
                  //    fFlags.fAnodeWire=0;
               }
    
         }

      fFlags.fSettings=new AnaSettings(json.Data());
      //fFlags.fSettings->Print();
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("WFpersistencyModuleFactory::NewRunObject, run %d, file %s\n", 
             runinfo->fRunNo, runinfo->fFileName.c_str());
      return new WFpersistencyModule(runinfo, &fFlags);
   }
};

static TARegister tar(new WFpersistencyModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
