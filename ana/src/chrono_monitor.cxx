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
   std::array<std::vector<TH1I>,CHRONO_N_BOARDS> fLiveHisto;
   std::vector<TChronoChannel> fChronChannels;
   TStyle* fChronStyle;
public:
   ChronoMonitor(TARunInfo* runinfo): TARunObject(runinfo), fLiveCanvas("LiveChrono","LiveChrono")
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
         fFIFOBin[b] = 0;
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

      fChronChannels.resize(CHRONO_N_CHANNELS*CHRONO_N_BOARDS);

      runinfo->fOdb->RSA("Equipment/alphagonline/Settings/ChannelDisplayName",&channel_display_name,false,20,32,error);
      for (const std::pair<std::string, int>& board: TChronoChannel::CBMAP)
      {
         TChronoChannelName name(runinfo->fOdb,board.first);

         for (int c = 0; c < name.GetNumberOfChannels(); c++)
         {
            for (size_t i = 0; i < channel_ID_string.size(); i++)
               if (channel_ID_string.at(i) == name.GetChannelName(c) && channel_ID_string.size() && name.GetChannelName(c).size() )
                  fChronChannels.at(i) = TChronoChannel(board.first,c);
            //Read chrono channel names from ODB (default behaviour)
            
            TString histo_name = board.first + 
                           std::string("_") + std::to_string(c) + 
                           std::string("-") + name.GetChannelName(c);

            fLiveHisto[board.second].emplace_back(
               TH1I(
                  histo_name,
                  histo_name,
                  BUFFER_DEPTH,
                  fFIFO[board.second].front().GetStartTime(),
                  fFIFO[board.second].back().GetStopTime()
               )
            );  
         }
      }

      fLiveCanvas.Divide(1,actual_size);
   }
  
   void EndRun(TARunInfo* runinfo)
   {
      // Save for testing...
      for (const std::pair<std::string, int>& board: TChronoChannel::CBMAP)
      {
         for (size_t i = 0; i < fLiveHisto[board.second].size(); i++)
            fLiveHisto.at(board.second).at(i).Write();
      }
   }
  
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      TCbFIFOEventFlow* cbFIFOflow = flow->Find<TCbFIFOEventFlow>();
      if (!cbFIFOflow)
         return flow;
   
      // Obtain time range for incoming data
      double mostmax = -99;
      for (const std::pair<std::string,int>& board: TChronoChannel::CBMAP)
      {
         const std::vector<TCbFIFOEvent> & events = cbFIFOflow->fCbHits[board.first];
         if (events.size())
         // TODO: Check events are in time order!
         for (const TCbFIFOEvent& e: events)
            if ( mostmax < e.GetRunTime())
               mostmax = e.GetRunTime();
         //std::cout<<events.back().GetRunTime() <<std::endl;
      }
      // Reserve space for all incoming TSISEvents
      for (const std::pair<std::string,int>& board: TChronoChannel::CBMAP)
      {
         while (fFIFO[board.second].back().GetStopTime() < mostmax)
         {
            fFIFO[board.second].emplace_back(
               TChronoBoardCounter(
                  INTEGRATION_TIME * ( fFIFOBin[board.second] - BUFFER_DEPTH),
                  INTEGRATION_TIME * ( fFIFOBin[board.second] - BUFFER_DEPTH + 1),
                  board.second
               )
            );
            if (fFIFO[board.second].size() > BUFFER_DEPTH )
               fFIFO[board.second].pop_front();
            fFIFOBin[board.second]++;
         }
      }

      for (const std::pair<std::string,int>& board: TChronoChannel::CBMAP)
      {
         const std::vector<TCbFIFOEvent> & events = cbFIFOflow->fCbHits[board.first];
         int bin = 0;
         if (events.empty())
            continue;
         for (const TCbFIFOEvent& e: events)
         {
            while ( e.GetRunTime() > fFIFO[board.second].at(bin).GetStopTime() )
            {
               //std::cout<<fFIFO[board.second].at(bin).GetStopTime()<<std::endl;
               bin++;
            }
            fFIFO[board.second].at(bin) += e;
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

      for (const std::pair<std::string, int>& board: TChronoChannel::CBMAP)
      {
         for (size_t i = 0; i < fLiveHisto[board.second].size(); i++)
         {
            fLiveHisto.at(board.second)[i].GetXaxis()->Set(BUFFER_DEPTH,tmin, tmax);
            fLiveHisto.at(board.second)[i].Reset();
         }
      }
      fChronStyle->SetPalette(kCool);
      //Update the histograms
      for (const std::pair<std::string, int>& board: TChronoChannel::CBMAP)
      {
         for (TChronoBoardCounter& s: fFIFO[board.second])
         {
            for (size_t i = 0; i < fLiveHisto[board.second].size(); i++)
            {
               if (s.fCounts[i])
               {
                  fLiveHisto[board.second].at(i).Fill(s.GetStartTime(), s.fCounts[i]);
               }
            }
         }
      }
      for (size_t i = 0; i < fChronChannels.size(); i++)
      {
         if (fChronChannels[i].GetChannel() >= 0)
         {
            fLiveCanvas.cd(i + 1);
            int board_index = fChronChannels[i].GetBoardNumber();
            int histo_number = fChronChannels[i].GetChannel();
            fLiveHisto.at( board_index ).at(histo_number).Draw("HIST");
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
