#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include "AnalysisTimer.h"
#include "AnaSettings.h"

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
   Deconv d;
    
public:

   DeconvAWModule(TARunInfo* runinfo, DeconvAwFlags* f): TARunObject(runinfo),
                                                         fFlags( f ),
                                                         d( f->ana_settings )
   {
      ModuleName="DeconvAWModule";
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
      
      d.SetupADCs( runinfo->fRunNo, 
                   fFlags->fADCnorm,   // dis/en-able normalization of WF
                   fFlags->fDiag );    // dis/en-able histogramming
      d.SetDisplay( !fFlags->fBatch ); // dis/en-able wf storage for aged

      d.PrintADCsettings();
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
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }

      if(fTrace)
         printf("DeconvAWModule::Analyze, run %d, counter %d\n",
                runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      
      const AgEvent* e = ef->fEvent;
      if (fFlags->fTimeCut)
      {
         if (e->time<fFlags->start_time)
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
         if (e->time>fFlags->stop_time)
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
      }

      if (fFlags->fEventRangeCut)
      {
         if (e->counter<fFlags->start_event)
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
         if (e->counter>fFlags->stop_event)
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
      }

      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif   

      const Alpha16Event* aw = e->a16;
      if( !aw )
         {
            std::cout<<"DeconvAWModule::AnalyzeFlowEvent(...) No Alpha16Event in AgEvent # "
                     <<e->counter<<std::endl;
            flow = new UserProfilerFlow(flow,"deconv_aw_module (No Alpha16Event)",timer_start);
            return flow;
         }
      else
         {
            int stat = d.FindAnodeTimes( aw );
            if(fTrace) printf("DeconvAWModule::AnalyzeFlowEvent() status: %d\n",stat);

            AgSignalsFlow* flow_sig = new AgSignalsFlow(flow, d.GetAnodeSignal());
             
            if( fFlags->fDiag )
               {
                  d.AWdiagnostic();
                  flow_sig->AddAdcPeaks( d.GetAdcPeaks() );
                  //               flow_sig->adc32range = d.GetAdcRange();
               }
            
            if( !fFlags->fBatch ) flow_sig->AddAWWaveforms( d.GetAWwaveforms() );
            
            flow = flow_sig;
         }
      ++fCounter;
      //d.Reset();
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
