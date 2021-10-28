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

#include "unpack_cb.h"
#include <TH1D.h>
#include <TH1I.h>

#include "cb_flow.h"

#include "TTree.h"
#include "store_cb.h"

class TChron
{
   public:
   int fBin;
   double fRunTime;
   std::vector<uint32_t> fCounts;

   TChron(int _bin): fCounts(CHRONO_N_BOARDS * CHRONO_N_CHANNELS,0)
   {
      fBin = _bin;
      fRunTime = fBin * INTEGRATION_TIME;
   }
   ~TChron()
   {

   }
   void operator +=(const TCbFIFOEvent* cbFIFO)
   {
      fCounts[cbFIFO->fChannel] += cbFIFO->fCounts;
   }
   double GetRunTime() const
   {
      return fRunTime;
   }
};

class ChronMonitor: public TARunObject
{
private:
   // Ring buffer would probably be quicker... but lets just get this working
   std::deque<TChron> fFIFO;

   TCanvas fLiveCanvas;
   std::vector<TH1I> fLiveHisto;
   std::vector<int> fChronChannels;
   TStyle* fChronStyle;
public:
   ChronMonitor(TARunInfo* runinfo): TARunObject(runinfo), fFIFO(std::deque<TChron>()), fLiveCanvas("LiveChron","LiveChron")
   {
      fModuleName = "ChronMonitor";
      for (int i = 0; i < BUFFER_DEPTH; i++)
      {
         fFIFO.emplace_back( TChron(i - BUFFER_DEPTH ) );
         fChronStyle = new TStyle("ChronStyle","ChronStyle");
      }


   }
   ~ChronMonitor()
   {
      printf("ChronMonitor::dtor!\n");
   }
   

   void BeginRun(TARunInfo* runinfo)
   {
      //Spill log monitor set these ODB entries, set create to false
      
      std::vector<std::string> channel_ID_string;
      std::vector<std::string> channel_display_name;
         MVOdbError* error = new MVOdbError();

         double board = 0;
        TString OdbPath = "/Equipment/cb0";
        OdbPath += board + 1; //TODO Get the correct board number.
        OdbPath += "/Settings/names";
      
      runinfo->fOdb->RSA(OdbPath,&channel_ID_string,false,60,32,error);
      //Re-read and resize?
      int actual_size = 0;
      for (const std::string& s: channel_ID_string)
         if (s.size())
            actual_size++;
      
      runinfo->fOdb->RSA(OdbPath,&channel_display_name,false,60,32,error);
      
      //Stolen from spill_log_module... should be upgraded to ODB reads

        //Just for testing lets get only the first board. Since thats all we're pulling above.
      //for (int i = 0; i < CHRONO_N_BOARDS * CHRONO_N_CHANNELS; i++)
      for (int i = 0; i < 1 * CHRONO_N_CHANNELS; i++)
      {
         TString name = std::to_string(i) + std::string("-") + channel_display_name.at(i);
         
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
            fChronChannels.push_back(i); // just getting them all I guess...?
      }
   }
  
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("ChronMonitor::Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
      return flow;
   }
   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      TCbFIFOEvent* cbFIFO = flow->Find<TCbFIFOEvent>();
      if (!cbFIFO)
         return flow;

      // Obtain time range for incoming data
      double mostmax = 0;
      //Again just doing the first board for now
      //for ( int j = 0; j < CHRONO_N_BOARDS; j++ )
      for ( int j = 0; j < 1; j++ )
      {
            if ( mostmax < cbFIFO->GetRunTime())
               mostmax = cbFIFO->GetRunTime();
      }
      // Reserve space for all incoming TSISEvents
      int i = fFIFO.back().fBin;
      while (fFIFO.back().GetRunTime() < mostmax)
      {
         fFIFO.emplace_back(TChron(++i));
         fFIFO.pop_front();
      }

      //Find bin of the first event
      //Again just doing the first board for now
      //for ( int j = 0; j < CHRONO_N_BOARDS; j++ )
      for ( int j = 0; j < 1; j++ )
      {
         int bin = 0;
            while ( cbFIFO->GetRunTime() > fFIFO.at(bin).GetRunTime())
               bin++;
            fFIFO.at(bin) += cbFIFO;
      }
      //Resise histograms
      for (int i = 0; i < fLiveHisto.size(); i++)
      {
         fLiveHisto[i].GetXaxis()->Set(BUFFER_DEPTH,fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());

         fLiveHisto[i].Reset();
      }
      fChronStyle->SetPalette(kCool);
      //Update the histograms
      for (TChron& s: fFIFO)
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
      for (int i = 0; i < fChronChannels.size(); i++)
      {
         if (fChronChannels[i] > 0)
         {
            fLiveCanvas.cd(i + 1);
            fLiveHisto[fChronChannels[i]].Draw("HIST");
         }
      }
      return flow;
   }
};


static TARegister tar1(new TAFactoryTemplate<ChronMonitor>);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
