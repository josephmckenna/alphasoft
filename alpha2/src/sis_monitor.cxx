//
// SIS data montior, uses a circular buffer to histogram 
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "A2Flow.h"
#include "TSISEvent.h"


#define SECONDS_TO_BUFFER 60


//Time to group SIS Events (seconds)
#define INTEGRATION_TIME 0.05

//#define BUFFER_DEPTH 2000
#define BUFFER_DEPTH SECONDS_TO_BUFFER / INTEGRATION_TIME

#define BINS_PER_SECOND ?


#include "TCanvas.h"

#include "TSISChannels.h"

class TSIS
{
   public:
   int fBin;
   double fRunTime;
   std::vector<uint32_t> fCounts;
   TSIS(int _bin): fCounts(NUM_SIS_CHANNELS * NUM_SIS_MODULES,0)
   {
      fBin = _bin;
      fRunTime = fBin * INTEGRATION_TIME;
   }
   ~TSIS()
   {

   }
   void operator +=(const TSISEvent& data)
   {
      for (int i = 0; i < NUM_SIS_CHANNELS * NUM_SIS_MODULES; i++)
      {
         fCounts[i] += data.GetCountsInChannel(i);
      }
   }
   double GetRunTime() const
   {
      return fRunTime;
   }
};

class SisMonitor: public TARunObject
{
private:
   // Ring buffer would probably be quicker... but lets just get this working
   std::deque<TSIS> fFIFO;

   TCanvas fLiveCanvas;
   std::vector<TH1I> fLiveHisto;
   std::vector<int> fSISChannel;

public:
   SisMonitor(TARunInfo* runinfo)
      : TARunObject(runinfo),
         fFIFO(
               std::deque<TSIS>()
            ),
         fLiveCanvas("LiveSIS","LiveSIS")
   {
      fModuleName = "SisMonitor";
      for (int i = 0; i < BUFFER_DEPTH; i++)
      {
         fFIFO.emplace_back(
            TSIS(
                  i - BUFFER_DEPTH 
               )
            );
      }

      //Stolen from spill_log_module... should be upgraded to ODB reads
      TSISChannels* sisch = new TSISChannels(runinfo->fRunNo);


      for (int i = 0; i < NUM_SIS_MODULES * NUM_SIS_CHANNELS; i++)
      {
         TString name = sisch->GetDescription(i, runinfo->fRunNo);
         
         fLiveHisto.emplace_back(
            TH1I(
               name,
               name,
               BUFFER_DEPTH,
               fFIFO.front().GetRunTime(),
               fFIFO.back().GetRunTime()
            )
         );  
      }


      std::vector<std::pair<std::string,std::string>> channels=
      {
         {"SIS_PMT_CATCH_OR", "Catch OR"},
         {"SIS_PMT_CATCH_AND","Catch AND"},
         {"CT_SiPM1",         "CT SiPM1"},
         {"CT_SiPM2",         "CT SiPM2"},
         {"CT_SiPM_OR",       "CT SiOR"},
         {"CT_SiPM_AND",      "CT SiAND"},
         {"PMT_12_AND_13",    "CT Stick"},
         //{"IO32_TRIG_NOBUSY","IO32_TRIG"},
         //{"PMT_10","PMT 10"},
         //{"ATOMSTICK","Atom Stick"}
      };

      fLiveCanvas.Divide(1,channels.size());

      gDirectory->cd();
      for (size_t i=0; i<channels.size(); i++)
      {
         fSISChannel.push_back(sisch->GetChannel(channels.at(i).first.c_str()));
      }
   }
   ~SisMonitor()
   {
      printf("SisMonitor::dtor!\n");
   }
  
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("SisMonitor::Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
      return flow;
   }
   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SISEventFlow* sf=flow->Find<SISEventFlow>();
      if (!sf)
         return flow;

      // Obtain time range for incoming data
      double mostmax = 0;
      for ( int j = 0; j < NUM_SIS_MODULES; j++ )
      {
         if (sf->sis_events[0].size())
            if ( mostmax < sf->sis_events[j].back()->GetRunTime())
               mostmax = sf->sis_events[j].back()->GetRunTime();
      }
      // Reserve space for all incoming TSISEvents
      int i = fFIFO.back().fBin;
      while (fFIFO.back().GetRunTime() < mostmax)
      {
         fFIFO.emplace_back(TSIS(i));
         fFIFO.pop_front();
         i++;
      }

      //Find bin of the first event
      for (int j = 0; j < NUM_SIS_MODULES; j++)
      {
         int bin = 0;
         for (TSISEvent* e: sf->sis_events[j]) 
         {
            while ( e->GetRunTime() > fFIFO.at(bin).GetRunTime())
               bin++;
            fFIFO.at(bin) += *e;
         }
      }
      //Resise histograms
      for (int i = 0; i < fLiveHisto.size(); i++)
      {
         fLiveHisto[i].GetXaxis()->Set(BUFFER_DEPTH,fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());

         fLiveHisto[i].Reset();
      }

      //Update the histograms
      for (TSIS& s: fFIFO)
      {
         for (int i = 0; i < fLiveHisto.size(); i++)
         {
            if (s.fCounts[i])
            {
               fLiveHisto[i].Fill(s.fRunTime, s.fCounts[i]);
               //std::cout<<s.fRunTime << "\t" << s.fCounts[i] << std::endl;
            }
         }
      }
      for (int i = 0; i < fSISChannel.size(); i++)
      {
         if (fSISChannel[i] > 0)
         {
            fLiveCanvas.cd(i + 1);
            fLiveHisto[fSISChannel[i]].Draw("HIST");
         }
      }
      return flow;
   }
};


static TARegister tar1(new TAFactoryTemplate<SisMonitor>);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
