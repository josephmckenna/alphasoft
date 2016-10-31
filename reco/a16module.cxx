//
// MIDAS analyzer example 2: ROOT analyzer
//
// K.Olchanski
//

#include "manalyzer.h"
#include "midasio.h"

#include <assert.h> // assert()

#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

#include "Alpha16.h"
#include "Unpack.h"

#include "anaCommon.cxx"

#include "TrackViewer.hh"
TrackViewer *gView;
TApplication* xapp;

#include "AGAnalyze.hh"

class A16Module: public TAModuleInterface
{
public:
   void Init(const std::vector<std::string> &args);
   void Finish();
   TARunInterface* NewRun(TARunInfo* runinfo);

   int fTotalEventCounter;
   bool fPlotWF;
};

struct A16Run: public TARunInterface
{
   A16Module* fModule;
   int fCounter;
   AlphaTpcX* fATX;
   AGAnalyze *fAgan = nullptr;

   A16Run(TARunInfo* runinfo, A16Module* m)
      : TARunInterface(runinfo)
   {
      printf("A16Run::ctor!\n");
      fModule = m;
      fATX = new AlphaTpcX();
      if (m->fPlotWF)
         fATX->CreateA16Canvas();
      if (true) {
         fAgan = new AGAnalyze;
         fAgan->UpdatePlots();
         bool status;
         status = fAgan->SetGas(0.28);
         if (!status)
            assert(!"SetGas() failed!");
         status = fAgan->SetB(0);
         if (!status)
            assert(!"SetB() failed!");
         fAgan->SetPlotTracks(false);
         if (runinfo->fRoot)
            if (runinfo->fRoot->fgApp)
               xapp = runinfo->fRoot->fgApp;
      }
   }

   ~A16Run()
   {
      printf("A16Run::dtor!\n");
      if (fATX)
         delete fATX;
      fATX = NULL;
      if (fAgan)
         delete fAgan;
      fAgan = NULL;
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      fCounter = 0;
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      fATX->BeginRun(runinfo->fRunNo);
      if (fAgan)
         fAgan->NewRun(runinfo->fRunNo);
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("EndRun, run %d, events %d\n", runinfo->fRunNo, fCounter);
      time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
      fATX->EndRun();
      char fname[1024];
      sprintf(fname, "output%05d.pdf", runinfo->fRunNo);
      fATX->fH->fCanvas->SaveAs(fname);
      if (fAgan) {
         fAgan->Write();
         fAgan->UpdatePlots();
         fAgan->PlotEfficiency();
      }
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);

      if (event->event_id != 1)
         return flow;

      Alpha16Event* e = UnpackAlpha16Event(fATX->fEvb, event);

      if (fAgan && !e->error && e->complete)
         fAgan->Analyze(e);

      int force_plot = fATX->Event(e);

      time_t now = time(NULL);

      if (force_plot) {
         static time_t plot_next = 0;
         if (now > plot_next) {
            fATX->PlotA16Canvas();
            plot_next = time(NULL) + 15;
         }
      }

      static time_t t = 0;

      if (now - t > 15) {
         t = now;
         fATX->Plot();
         fAgan->UpdatePlots();
      }

      fCounter++;
      fModule->fTotalEventCounter++;

      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

void A16Module::Init(const std::vector<std::string> &args)
{
   printf("Init!\n");

   fPlotWF = false;

   for (unsigned i=0; i<args.size(); i++) {
      if (args[i] == "--wf")
         fPlotWF = true;
   }

   fTotalEventCounter = 0;
   TARootHelper::fgDir->cd(); // select correct ROOT directory
}

void A16Module::Finish()
{
   printf("Finish!\n");
   printf("Counted %d events grand total\n", fTotalEventCounter);
}

TARunInterface* A16Module::NewRun(TARunInfo* runinfo)
{
   printf("NewRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
   return new A16Run(runinfo, this);
}

TARegisterModule tarm(new A16Module);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
