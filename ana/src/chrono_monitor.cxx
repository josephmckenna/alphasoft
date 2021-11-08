//
// Chronbox data montior, uses a circular buffer to histogram 
//
// L GOLINO
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "RecoFlow.h"


#include "TStyle.h"
#include "TColor.h"
#include "TF2.h"
#include "TExec.h"
#include "TCanvas.h"

#define SECONDS_TO_BUFFER 60

//Time to group SIS Events (seconds)
#define INTEGRATION_TIME 0.05

//#define BUFFER_DEPTH 2000
#define BUFFER_DEPTH SECONDS_TO_BUFFER / INTEGRATION_TIME

#define BINS_PER_SECOND ?


#include "TCanvas.h"

#include "store_cb.h"
#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>

#include "manalyzer.h"
#include "midasio.h"
#include "TChronoChannelName.h"
#include "TChronoBoardCounter.h"

#include "unpack_cb.h"
#include <TH1D.h>
#include <TH1I.h>

#include "cb_flow.h"

#include "TTree.h"
#include "store_cb.h"



class ChronoMonitor: public TARunObject
{
private:
   // Ring buffer would probably be quicker... but lets just get this working
   std::deque<TChronoBoardCounter> fFIFO[CHRONO_N_BOARDS];
   int                 fFIFOBin[CHRONO_N_BOARDS];

   TCanvas fLiveCanvas;
   std::vector<TH1I> fLiveHisto;
   std::vector<TChronoChannel> fChronChannels;
   TStyle* fChronStyle;
public:
   ChronoMonitor(TARunInfo* runinfo): TARunObject(runinfo), fLiveCanvas("LiveChron","LiveChron")
   {
      fModuleName = "ChronoMonitor";
      for (int b = 0; b < CHRONO_N_BOARDS; b++)
      {
         for (int i = 0; i < BUFFER_DEPTH; i++)
         {
            fFIFO[b].emplace_back(
               TChronoBoardCounter(
                  INTEGRATION_TIME * ( fFIFOBin[b] - BUFFER_DEPTH),
                  INTEGRATION_TIME * ( fFIFOBin[b] - BUFFER_DEPTH + 1),
                  b
               )
            );
            fFIFOBin[b]++;
            fChronStyle = new TStyle("ChronoStyle","ChronoStyle");
         }
      }
   }
   ~ChronoMonitor()
   {
      printf("ChronoMonitor::dtor!\n");
   }
   

   void BeginRun(TARunInfo* runinfo)
   {
      //Spill log monitor set these ODB entries, set create to false
      
      std::vector<std::string> channel_ID_string;
      std::vector<std::string> channel_display_name;
      MVOdbError* error = new MVOdbError();
      
      runinfo->fOdb->RSA("Equipment/alphagonline/Settings/ChannelIDName",&channel_ID_string,false,20,32,error);
      //Re-read and resize?
      int actual_size = 0;
      for (const std::string& s: channel_ID_string)
         if (s.size())
            actual_size++;
      
      runinfo->fOdb->RSA("Equipment/alphagonline/Settings/ChannelDisplayName",&channel_display_name,false,20,32,error);
      for (const std::pair<std::string, int>& board: TChronoChannel::CBMAP)
      {
         TString OdbPath = "/Equipment/cb0";
         OdbPath += board.first; //TODO Get the correct board number.
         OdbPath += "/Settings/names";
         std::vector<std::string> channel_list;
         runinfo->fOdb->RSA(OdbPath,&channel_list,false,60,32,error);
         for (int c = 0; c < channel_list.size(); c++)
         {
            for (int i = 0; i < channel_ID_string.size(); i++)
               if (channel_ID_string.at(i) == channel_list.at(c) && channel_ID_string.size() )
                  fChronChannels.at(i) = TChronoChannel(board.first,c);
         }

         runinfo->fOdb->RSA(OdbPath,&channel_display_name,false,60,32,error);

         for (int c = 0; c < CHRONO_N_CHANNELS; c++)
         {
            TString name = board.first + 
                           std::string("_") + std::to_string(c) + 
                           std::string("-") + channel_display_name.at(c);

            fLiveHisto.emplace_back(
               TH1I(
                  name,
                  name,
                  BUFFER_DEPTH,
                  fFIFO[board.second].front().GetStartTime(),
                  fFIFO[board.second].back().GetStopTime()
               )
            );  
         }
      }

      fLiveCanvas.Divide(1,actual_size);
   }
  
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("ChronoMonitor::Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
      return flow;
   }
   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      TCbFIFOEventFlow* cbFIFOflow = flow->Find<TCbFIFOEventFlow>();
      if (!cbFIFOflow)
         return flow;
   
      // Obtain time range for incoming data
      double mostmax = 0;
      for (const std::pair<std::string,std::vector<TCbFIFOEvent>>& cb: cbFIFOflow->fCbHits)
      {
         if ( mostmax < cb.second.back().GetRunTime())
            mostmax = cb.second.back().GetRunTime();
      }
      // Reserve space for all incoming TSISEvents
      for (int b = 0; b < CHRONO_N_BOARDS; b++)
      {
         while (fFIFO[b].back().GetStopTime() < mostmax)
         {
            fFIFO[b].emplace_back(
               TChronoBoardCounter(
                  INTEGRATION_TIME * ( fFIFOBin[b] - BUFFER_DEPTH),
                  INTEGRATION_TIME * ( fFIFOBin[b] - BUFFER_DEPTH + 1),
                  b
               )
            );
            fFIFO[b].pop_front();
            fFIFOBin[b]++;
         }
      }

      for ( int b = 0; b < CHRONO_N_BOARDS; b++ )
      {
         char CBNAME[5] = "CB00";
         CBNAME[3] += b;
         int bin = 0;
         const std::vector<TCbFIFOEvent> & events = cbFIFOflow->fCbHits[CBNAME];
         for (const TCbFIFOEvent& e: events)
         {
            while ( e.GetRunTime() > fFIFO[b].at(bin).GetStartTime())
               bin++;
            fFIFO[b].at(bin) += e;
         }
      }

      //Resise histograms
      double tmin = 1E99;
      double tmax = -1.;
      for (int b = 0; b < CHRONO_N_BOARDS; b++)
      {
         if (tmin > fFIFO[b].front().GetStartTime())
            tmin = fFIFO[b].front().GetStartTime();
         if (tmax < fFIFO[b].back().GetStopTime())
            tmax = fFIFO[b].back().GetStopTime();
      }

      for (int i = 0; i < fLiveHisto.size(); i++)
      {
         fLiveHisto[i].GetXaxis()->Set(BUFFER_DEPTH,tmin, tmax);
         fLiveHisto[i].Reset();
      }
      fChronStyle->SetPalette(kCool);
      //Update the histograms

      for ( int b = 0; b < CHRONO_N_BOARDS; b++ )
      {
         for (TChronoBoardCounter& s: fFIFO[b])
         {
            for (int i = 0; i < fLiveHisto.size(); i++)
            {
               if (s.fCounts[i])
               {
                  fLiveHisto[i].Fill(s.GetStartTime(), s.fCounts[i]);
                  //std::cout<<s.fRunTime << "\t" << s.fCounts[i] << std::endl;
               }
            }
         }
      }
      for (int i = 0; i < fChronChannels.size(); i++)
      {
         if (fChronChannels[i].GetChannel() > 0)
         {
            fLiveCanvas.cd(i + 1);
            fLiveHisto[i].Draw("HIST");
         }
      }
      return flow;
   }
};


static TARegister tar1(new TAFactoryTemplate<ChronoMonitor>);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
