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

#include "TStyle.h"
#include "TColor.h"
#include "TF2.h"
#include "TExec.h"
#include "TCanvas.h"

#define SECONDS_TO_BUFFER 15


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
   std::vector<TSISChannel> fSISChannel;
   TStyle* fSISStyle;
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
         fSISStyle = new TStyle("SVDStyle","SVDStyle");
      }


   }
   ~SisMonitor()
   {
      printf("SisMonitor::dtor!\n");
   }
   

   void BeginRun(TARunInfo* runinfo)
   {
      //Spill log monitor set these ODB entries, set create to false
      
      std::vector<std::string> channel_ID_string;
      std::vector<std::string> channel_display_name;
         MVOdbError* error = new MVOdbError();
      
      runinfo->fOdb->RSA("Equipment/alpha2online/Settings/ChannelIDName",&channel_ID_string,false,20,32,error);
      //Re-read and resize?
      int actual_size = 0;
      for (const std::string& s: channel_ID_string)
         if (s.size())
            actual_size++;
      
      runinfo->fOdb->RSA("Equipment/alpha2online/Settings/ChannelDisplayName",&channel_display_name,false,20,32,error);
      
      //Stolen from spill_log_module... should be upgraded to ODB reads
      TSISChannels* sisch = new TSISChannels(runinfo->fRunNo);


      for (int i = 0; i < NUM_SIS_MODULES * NUM_SIS_CHANNELS; i++)
      {
         TString name = std::to_string(i) + std::string("-") + sisch->GetDescription(i, runinfo->fRunNo);
         
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

      fLiveCanvas.Divide(1,actual_size);

      gDirectory->cd();
      for (size_t i=0; i<channel_ID_string.size(); i++)
      {
         if (channel_ID_string.at(i).size())
            fSISChannel.push_back(sisch->GetChannel(channel_ID_string.at(i).c_str()));
      }
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
            if ( mostmax < sf->sis_events[j].back().GetRunTime())
               mostmax = sf->sis_events[j].back().GetRunTime();
      }
      // Reserve space for all incoming TSISEvents
      int i = fFIFO.back().fBin;
      while (fFIFO.back().GetRunTime() < mostmax)
      {
         fFIFO.emplace_back(TSIS(++i));
         fFIFO.pop_front();
      }

      //Find bin of the first event
      for (int j = 0; j < NUM_SIS_MODULES; j++)
      {
         int bin = 0;
         for (const TSISEvent& e: sf->sis_events[j]) 
         {
            while ( e.GetRunTime() > fFIFO.at(bin).GetRunTime())
               bin++;
            fFIFO.at(bin) += e;
         }
      }
      //Resise histograms
      for (int i = 0; i < fLiveHisto.size(); i++)
      {
         fLiveHisto[i].GetXaxis()->Set(BUFFER_DEPTH,fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());

         fLiveHisto[i].Reset();
      }
      fSISStyle->SetPalette(kCool);
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
         if (fSISChannel[i].IsValid())
         {
            fLiveCanvas.cd(i + 1);
            fLiveHisto[fSISChannel[i].toInt()].Draw("HIST");
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
