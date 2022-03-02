#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include "AnaSettings.hh"

#include "DeconvPAD.h"

class DeconvPadFlags
{
public:
   bool fRecOff = false; //Turn reconstruction off
   bool fDiag=false;
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;
   bool fBatch = true;
   bool fPWBmap = false;

   AnaSettings* ana_settings=0;

   bool fPWBnorm = false;
   bool fTrace = false;
   
public:
   DeconvPadFlags() // ctor
   { }

   ~DeconvPadFlags() // dtor
   { }
};



class DeconvPADModule: public TARunObject
{
public:
   DeconvPadFlags* fFlags = 0;
   bool fTrace = false;
   int fCounter = 0;

private:
   DeconvPAD d;
  
public:

   DeconvPADModule(TARunInfo* runinfo, DeconvPadFlags* f): TARunObject(runinfo),
                                                         fFlags(f),
                                                         d( f->ana_settings )
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="DeconvPADModule";
#endif
      if (fTrace)
         printf("DeconvPADModule::ctor!\n");
   }

   ~DeconvPADModule()
   {
      if (fTrace)
         printf("DeconvPADModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;

      d.SetupPWBs( runinfo->fRoot->fOutputFile,
                   runinfo->fRunNo, 
                   fFlags->fPWBnorm,   // dis/en-able normalization of WF
                   fFlags->fDiag );    // dis/en-able histogramming
      d.SetDisplay( !fFlags->fBatch ); // dis/en-able wf storage for aged

      d.PrintPWBsettings();

      d.SetTrace(fFlags->fTrace);
   }

   void EndRun(TARunInfo* runinfo)
   {
      
      printf("DeconvPADModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   bool SkipAnalysis(const AgEvent* e) const
   {
      if (!e)
         return true;
      if (fFlags->fTimeCut)
      {
         if (e->time < fFlags->start_time)
         {
            return true;
         }
         if (e->time > fFlags->stop_time)
         {
            return true;
         }
      }

      if (fFlags->fEventRangeCut)
      {
         if (e->counter < fFlags->start_event)
         {
            return true;
         }
      
         if (e->counter > fFlags->stop_event)
         {
            return true;
         }
      }
      return false;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      // turn off recostruction
      if (fFlags->fRecOff)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      if(fTrace)
         printf("DeconvPADModule::Analyze, run %d, counter %d\n",
                runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();
      if (!ef )
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      const AgEvent* e = ef->fEvent;
      if (SkipAnalysis(e))
      {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;         
      }
      AgSignalsFlow* flow_sig = flow->Find<AgSignalsFlow>();
      if( !flow_sig ) 
         {
            printf("DeconvPADModule::Analyze NO SignalsFlow?");
            return flow;
         }
      
      const FeamEvent* pwb = e->feam;
      if( !pwb ) // allow for events without pwbs
         {
            std::cout<<"DeconvPADModule::AnalyzeFlowEvent(...) No FeamEvent in AgEvent # "
                     <<e->counter<<std::endl;
            //return flow;
         }
      else
         {
             flow_sig->PadWaves.clear();
             flow_sig->PadIndex.clear();
             d.BuildWFContainer(pwb,flow_sig->PadWaves,flow_sig->PadIndex, flow_sig->PADwf,flow_sig->pwbMax);
             if (flow_sig->PadWaves.size())
             {
                d.Deconvolution(flow_sig->PadWaves, flow_sig->PadIndex, flow_sig->pdSig );
             }
             d.LogDeconvRemaineder(flow_sig->PadWaves);
             ++fCounter;
         }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class DeconvPADModuleFactory: public TAFactory
{
public:
   DeconvPadFlags fFlags;

public:
   void Help()
   {
      printf("DeconvPADModuleFactory::Help!\n");
      printf("\t--recoff     Turn off reconstruction\n");
      printf("\t--wfnorm     Normalize all waveforms from rescale file\n");
      printf("\t--pwbnorm    Normalize PWB waveforms from rescale file\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {    
      TString json="default";
      printf("DeconvPADModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
         if( args[i] == "--diag" )
            fFlags.fDiag = true;
         if( args[i] == "--usetimerange" )
            {
               fFlags.fTimeCut=true;
               i++;
               fFlags.start_time=atof(args[i].c_str());
               i++;
               fFlags.stop_time=atof(args[i].c_str());
               printf("Using time range for reconstruction: ");
               printf("%f - %fs\n",fFlags.start_time,fFlags.stop_time);
            }
         if( args[i] == "--useeventrange" )
            {
               fFlags.fEventRangeCut=true;
               i++;
               fFlags.start_event=atoi(args[i].c_str());
               i++;
               fFlags.stop_event=atoi(args[i].c_str());
               printf("Using event range for reconstruction: ");
               printf("Analyse from (and including) %d to %d\n",fFlags.start_event,fFlags.stop_event);
            }
         if (args[i] == "--recoff")
            fFlags.fRecOff = true;
         if( args[i] == "--aged" )
            fFlags.fBatch = false;

         if( args[i] == "--trace" )
            fFlags.fTrace = true;

         if( args[i] == "--anasettings" ) json=args[i+1];

         if( args[i] == "--pwbnorm" ) fFlags.fPWBnorm=true;
         if( args[i] == "--wfnorm" ) fFlags.fPWBnorm=true;
      }

      fFlags.ana_settings=new AnaSettings(json.Data());
      if(fFlags.fTrace) fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("DeconvPADModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("DeconvPADModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new DeconvPADModule(runinfo, &fFlags);
   }
};

static TARegister tar(new DeconvPADModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
