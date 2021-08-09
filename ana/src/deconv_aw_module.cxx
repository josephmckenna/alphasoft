#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include "AnaSettings.hh"

#include "Deconv.hh"

class DeconvAwFlags
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

   AnaSettings* ana_settings=0;

   bool fADCnorm=false;
   bool fPersEnabled=false;
   
public:
   DeconvAwFlags() // ctor
   { }

   ~DeconvAwFlags() // dtor
   { }
};


class DeconvAWModule: public TARunObject
{
public:
   DeconvAwFlags* fFlags = 0;
   //bool fTrace = true;
   bool fTrace = false;
   int fCounter = 0;

private:
   DeconvWire d;
    
public:

   DeconvAWModule(TARunInfo* runinfo, DeconvAwFlags* f): TARunObject(runinfo),
	fFlags( f ),
	d( f->ana_settings,runinfo->fRoot->fOutputFile,runinfo->fRunNo,f->fADCnorm,f->fDiag )
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="DeconvAWModule";
#endif
      if (fTrace)
         printf("DeconvAWModule::ctor!\n");
   }

   ~DeconvAWModule()
   {
      if (fTrace)
         printf("DeconvAWModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;
      
      d.SetDisplay( !fFlags->fBatch || fFlags->fPersEnabled ); // dis/en-able wf storage for aged
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("DeconvAWModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
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

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      // turn off recostruction
      if (fFlags->fRecOff)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      if(fTrace)
         printf("DeconvAWModule::Analyze, run %d, counter %d\n",
                runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      const AgEvent* e = ef->fEvent;
      if (fFlags->fTimeCut)
      {
         if (e->time<fFlags->start_time)
         {
#ifdef HAVE_MANALYZER_PROFILER
            *flags|=TAFlag_SKIP_PROFILE;
#endif
            return flow;
         }
         if (e->time>fFlags->stop_time)
         {
#ifdef HAVE_MANALYZER_PROFILER
            *flags|=TAFlag_SKIP_PROFILE;
#endif
            return flow;
         }
      }

      if (fFlags->fEventRangeCut)
      {
         if (e->counter<fFlags->start_event)
         {
#ifdef HAVE_MANALYZER_PROFILER
            *flags|=TAFlag_SKIP_PROFILE;
#endif
            return flow;
         }
         if (e->counter>fFlags->stop_event)
         {
#ifdef HAVE_MANALYZER_PROFILER
            *flags|=TAFlag_SKIP_PROFILE;
#endif
            return flow;
         }
      }
#ifdef HAVE_MANALYZER_PROFILER
      TAClock start_time = TAClockNow();
#endif
      const Alpha16Event* aw = e->a16;
      if( !aw )
         {
            std::cout<<"DeconvAWModule::AnalyzeFlowEvent(...) No Alpha16Event in AgEvent # "
                     <<e->counter<<std::endl;
#ifdef HAVE_MANALYZER_PROFILER
            flow = new TAUserProfilerFlow(flow,"deconv_aw_module (No Alpha16Event)",start_time);
#endif
            return flow;
         }
      else
         {
            int stat = d.FindTimes( aw );
            if(fTrace) printf("DeconvAWModule::AnalyzeFlowEvent() status: %d\n",stat);

            AgSignalsFlow* flow_sig = new AgSignalsFlow(flow, d.GetSignal());
             
            if( fFlags->fDiag )
               {
                  flow_sig->AddAdcPeaks( d.GetPeaks() );
                  //               flow_sig->adc32range = d.GetAdcRange();
               }
            
            if( !fFlags->fBatch || fFlags->fPersEnabled ) 
               flow_sig->AddAWWaveforms( d.GetWaveForms() );

            flow = flow_sig;
         }
      ++fCounter;
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

};

class DeconvAWModuleFactory: public TAFactory
{
public:
   DeconvAwFlags fFlags;

public:
   void Help()
   {
      printf("DeconvAWModuleFactory::Help!\n");
      printf("\t--recoff     Turn off reconstruction\n");
      printf("\t--wfnorm     Normalize all waveforms from rescale file\n");
      printf("\t--adcnorm    Normalize ADC waveforms from rescale file\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {    
      TString json="default";
      printf("DeconvAWModuleFactory::Init!\n");

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

         if( args[i] == "--anasettings" ) json=args[i+1];

         if( args[i] == "--adcnorm" ) fFlags.fADCnorm=true;
         if( args[i] == "--wfnorm" ) fFlags.fADCnorm=true;

         if( args[i] == "--persistency" ) fFlags.fPersEnabled=true;
      }

      fFlags.ana_settings=new AnaSettings(json.Data());
      fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("DeconvAWModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("DeconvAWModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new DeconvAWModule(runinfo, &fFlags);
   }
};

static TARegister tar(new DeconvAWModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
