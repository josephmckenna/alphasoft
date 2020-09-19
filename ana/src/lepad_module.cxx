#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include "AnalysisTimer.h"
#include "AnaSettings.h"

#include "Ledge.hh"

class LEpadFlags
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
   
public:
   LEpadFlags() // ctor
   { }

   ~LEpadFlags() // dtor
   { }
};


class LEpadModule: public TARunObject
{
public:
   LEpadFlags* fFlags = 0;
   //bool fTrace = true;
   bool fTrace = false;
   int fCounter = 0;

private:
   Ledge l;
 
public:

   LEpadModule(TARunInfo* runinfo, LEpadFlags* f): TARunObject(runinfo),
                                                   fFlags(f)
      
   {
      if (fTrace) printf("LEpadModule::ctor!\n");
   }

   ~LEpadModule()
   {
      if (fTrace) printf("LEpadModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;

      l.SetRMSBaselineCut( fFlags->ana_settings->GetDouble("LEModule","PWBrms") );
      l.SetPulseHeightThreshold( fFlags->ana_settings->GetDouble("LEModule","PWBthr") );
      l.SetCFDfraction( fFlags->ana_settings->GetDouble("LEModule","CFDfrac") );
      l.SetTimeOffset( fFlags->ana_settings->GetDouble("LEModule","PWBtime") );
   }

   void EndRun(TARunInfo* runinfo)
   {      
      printf("LEpadModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace) printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace) printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      // turn off recostruction
      if (fFlags->fRecOff)
         return flow;

      if(fTrace)
         printf("LEpadModule::Analyze, run %d, counter %d\n",
                runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      const AgEvent* e = ef->fEvent;
      if (fFlags->fTimeCut)
      {
        if (e->time<fFlags->start_time)
          return flow;
        if (e->time>fFlags->stop_time)
          return flow;
      }

      if (fFlags->fEventRangeCut)
      {
         if (e->counter<fFlags->start_event)
           return flow;
         if (e->counter>fFlags->stop_event)
           return flow;
      }

      AgSignalsFlow* flow_sig = flow->Find<AgSignalsFlow>();
      if( !flow_sig ) 
         {
            printf("LEpadModule::Analyze NO SignalsFlow?\n");
            return flow;
         }

      #ifdef _TIME_ANALYSIS_
      //clock_t timer_start(clock());
      START_TIMER
      #endif   
      
      const FeamEvent* pwb = e->feam;
      if( !pwb ) // allow for events without pwbs
         {
            std::cout<<"LEpadModule::AnalyzeFlowEvent(...) No FeamEvent in AgEvent # "
                     <<e->counter<<std::endl;
         }
      else
         {
             int stat = l.FindPadTimes(pwb);
             printf("LEpadModule::AnalyzeFlowEvent() status: %d\n",stat);
             if( stat > 0 ) flow_sig->AddPadSignals(l.GetSignal());

             // if( !fFlags->fBatch ) flow_sig->AddPADWaveforms( d.GetPADwaveforms() );

         }
      ++fCounter;
      #ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"LEpad_module",timer_start);
      #endif
 
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class LEpadModuleFactory: public TAFactory
{
public:
   LEpadFlags fFlags;

public:
   void Help()
   {
      printf("LEpadModuleFactory::Help!\n");
      printf("\t--recoff     Turn off reconstruction\n");
      //printf("\t--pwbnorm    Normalize PWB waveforms from rescale file\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {    
      TString json="default";
      printf("LEpadModuleFactory::Init!\n");

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

         // if( args[i] == "--pwbnorm" ) fFlags.fPWBnorm=true;
         // if( args[i] == "--wfnorm" ) fFlags.fPWBnorm=true;
      }

      fFlags.ana_settings=new AnaSettings(json.Data());
      fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("LEpadModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("LEpadModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new LEpadModule(runinfo, &fFlags);
   }
};

static TARegister tar(new LEpadModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
