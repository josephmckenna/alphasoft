//
// SVD data montior, uses a circular buffer to histogram 
//
// JTK McKENNA
//

#include <stdio.h>
#include <stdint.h>

#include "manalyzer.h"
#include "midasio.h"

#include "A2Flow.h"
#include "TAlphaEvent.h"
#include "TSiliconEvent.h"

#include "TStyle.h"
#include "TColor.h"
#include "TF2.h"
#include "TExec.h"
#include "TCanvas.h"

#define SECONDS_TO_BUFFER 60


//Time to group SVD Events (seconds)
#define INTEGRATION_TIME 0.5

//#define BUFFER_DEPTH 2000
#define BUFFER_DEPTH SECONDS_TO_BUFFER / INTEGRATION_TIME

#define BINS_PER_SECOND ?


#include "TCanvas.h"

#include "TSISChannels.h"

class TSVD
{
   public:
   int fBin;
   
   double fRunTime;
   
   std::vector<bool> fHasVertex;
   std::vector<bool> fPassCut;
   std::vector<double> fX;
   std::vector<double> fY;
   std::vector<double> fZ;
   std::vector<uint32_t> fCounts;
   TSVD(int _bin): fCounts(nSil * 4,0)
   {
      fBin = _bin;
      fRunTime = fBin * INTEGRATION_TIME;
   }
   ~TSVD()
   {

   }
   void operator +=(const TSiliconEvent& data)
   {
      fHasVertex.push_back((bool)data.GetNVertices());
      fPassCut.push_back(data.GetPassedCuts());
      //fRunTime = data.GetVF48Timestamp();
      fX.push_back(data.GetVertexX());
      fY.push_back(data.GetVertexY());
      fZ.push_back(data.GetVertexZ());
      //std::cout<<fX << "," << fY << "," << fZ <<std::endl;
      
      for (int i = 0; i < nSil; i++)
      {
         const TSiliconModule* m = data.GetSiliconModule(i);
         if (!m)
            continue;
         if (!m->IsAHitModule())
            continue;
         bool hitP = false;
         bool hitN = false;
         for (int j = 0; j < 4; j++)
         {
            const TSiliconVA* v = m->GetASIC(j);
            if (!v) continue;
            if (v->IsAHitOR())
            {
               if (v->IsAPSide())
                  hitP = true;
               else
                  hitN = true;
            }
         }
         if (hitP && hitN)
         {
            int ModuleNumber = m->GetModuleNumber();
            fCounts.at(ModuleNumber)++;
            if (data.GetNTracks())
               fCounts.at( nSil + ModuleNumber)++;
            if (data.GetNVertices())
               fCounts.at( 2*nSil + ModuleNumber)++;
            if (data.GetPassedCuts())
               fCounts.at( 3*nSil + ModuleNumber)++;
         }
      }
   }
   double GetRunTime() const
   {
      return fRunTime;
   }

};

#define LIVE_DUMP_MARKERS 0
#if LIVE_DUMP_MARKERS
class TSVDDumpMarker
{
   private:
      int fSeq;
      int fType;
      uint32_t fMidasTime;
      double fTime;
//      TF1* fLine;
   public:
      TSVDDumpMarker(const int seq, const int type, const uint32_t midas_time, const double time)//, TF1* line)
      {
         fSeq = seq;
         fType = type;
         fMidasTime = midas_time;
         fTime = time;
  //       fLine = line;
         //std::string name = "Line" + std::to_string(time);
         //std::string formula = std::to_string(time);
         //new TF1(name.c_str(),formula.c_str(), -30,30 );
      }
      ~TSVDDumpMarker()
      {
         //delete fLine;
         //fLine = NULL;
      }
      EColor GetColour() const
      {
         if (fType == 0 )
            return kGreen;
         else if (fType == 1 )
            return kRed;
         return kBlack;
      }
      double GetRunTime() const
      {
         return fTime;
      }
/*      void Draw()
      {
         std::cout <<"DrawLine: " << fTime <<std::endl;
         fLine->Draw("SAME");
      }*/
      void Draw(TF1* line)
      {
         line->SetParameter(0,fTime );
         line->SetLineColor(GetColour());
         line->SetLineWidth(1);
         std::cout <<"DrawLine: " << fTime <<std::endl;
         line->Draw("SAME");
      }
   
};
#endif


class SVDMonitor: public TARunObject
{
private:
   // Ring buffer would probably be quicker... but lets just get this working
   std::deque<TSVD> fFIFO;

#if LIVE_DUMP_MARKERS
   std::deque<TSVDDumpMarker> fDumpMarkerFIFO;
   std::array<TSISChannel,USED_SEQ> DumpStartChannels;
   std::array<TSISChannel,USED_SEQ> DumpStopChannels;

   TString StartNames[NUMSEQ]={"SIS_PBAR_DUMP_START","SIS_RECATCH_DUMP_START","SIS_ATOM_DUMP_START","SIS_POS_DUMP_START","NA","NA","NA","NA","NA"};
   TString StopNames[NUMSEQ] ={"SIS_PBAR_DUMP_STOP", "SIS_RECATCH_DUMP_STOP", "SIS_ATOM_DUMP_STOP", "SIS_POS_DUMP_STOP","NA","NA","NA","NA","NA"};
#endif

   std::chrono::steady_clock::time_point fLastHistoUpdate;

   TCanvas fLiveVertex;
   TCanvas fLiveOccupancy;
   TH2I fXYvert;
   TH2I fXYpass;
   //TH2D fZY;
   TH2I fZTvert;
   TH2I fZTpass;

#if LIVE_DUMP_MARKERS
   TH2I fDumpStart;
   TH2I fDumpStop;
#endif
   //TH1I fR;
   TH2I fOccupancyT[4];
   TStyle* fSVDStyle;

public:
   SVDMonitor(TARunInfo* runinfo)
      : TARunObject(runinfo),
         fFIFO(
               std::deque<TSVD>()
            ),
         fLiveVertex("LiveVertex","LiveVertex"),
         fLiveOccupancy("LiveOccupancy","LiveOccupancy")
   {
      fLastHistoUpdate = std::chrono::high_resolution_clock::now(); //measure time starting here
      fModuleName = "SVDMonitor";
      for (int i = 0; i < BUFFER_DEPTH; i++)
      {
         fFIFO.emplace_back(
            TSVD(
                  i - BUFFER_DEPTH 
               )
            );
         fSVDStyle = new TStyle("SVDStyle","SVDStyle");
         fSVDStyle->SetPalette(kCool);
      }
   }
   ~SVDMonitor()
   {
      printf("SVDMonitor::dtor!\n");
   }
   

   void BeginRun(TARunInfo* runinfo)
   {
#if LIVE_DUMP_MARKERS
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      for (int j=0; j<USED_SEQ; j++) 
      {
         DumpStartChannels[j] =SISChannels->GetChannel(StartNames[j],runinfo->fRunNo);
         DumpStopChannels[j]  =SISChannels->GetChannel(StopNames[j], runinfo->fRunNo);
      }
#endif
      fSVDStyle->SetPalette(kCool);
      fXYvert = TH2I(
               "XY Vertex",
               "XY Vertex; X(cm); Y(cm)",
               100,-4,4,
               100,-4,4
            );
      fZTvert = TH2I(
               "ZT Vertex",
               "ZT Vertex; Z(cm); Run Time(s);",
               100,-30,30,
               BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime()
            );
#if LIVE_DUMP_MARKERS
      fDumpStart = TH2I("StartDumpMarkers",
               "Dump Marker;",
               1,-30,30,
               BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime()
            );
      fDumpStop = TH2I("StopDumpMarkers",
               "Dump Marker;",
               1,-30,30,
               BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime()
            );
#endif
      fXYpass = TH2I(
               "XY Pass Cut",
               "XY Pass Cut; X(cm); Y(cm)",
               100,-4,4,
               100,-4,4
            );
      fZTpass = TH2I(
               "ZT Pass Cut",
               "ZT Pass Cut; Z(cm); Run Time(s)",
               100,-30,30,
               BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime()
            );
      fOccupancyT[0]= TH2I(
               "Hit Occupancy",
               "Hit Occupancy; Run Time(s); Silicon Module No;",
               BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime(),
               nSil,0,nSil
            );
      fOccupancyT[1]= TH2I(
               "Hit Occupancy with Tracks",
               "Hit Occupancy with Tracks; Run Time(s); Silicon Module No;",
               BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime(),
               nSil,0,nSil
            );

      fOccupancyT[2]= TH2I(
               "Hit Occupancy with Vertex",
               "Hit Occupancy with Vertex; Run Time(s); Silicon Module No;",
               BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime(),
               nSil,0,nSil
            );
            
      fOccupancyT[3]= TH2I(
               "Hit Occupancy with Pass Cut",
               "Hit Occupancy with Pass Cut; Run Time(s); Silicon Module No;",
               BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime(),
               nSil,0,nSil
            );

      fLiveVertex.Divide(2,2);
      fLiveOccupancy.Divide(1,4);
      gDirectory->cd();
   }
  
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("SisMonitor::Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
      return flow;
   }

   void AddEvent(const TSiliconEvent* se)
   {
      int i = fFIFO.back().fBin;
      // Grow FIFO if needed (then remove old events automatically)
      while (fFIFO.back().GetRunTime() < se->GetVF48Timestamp())
      {
         fFIFO.emplace_back(TSVD(++i));
         fFIFO.pop_front();
      }
      //Find bin of the first event and add it to the cointers
      int bin = 0;
      while ( se->GetVF48Timestamp() > fFIFO.at(bin).GetRunTime())
         bin++;
      fFIFO.at(bin) += *se;
   }
#if LIVE_DUMP_MARKERS
   void AddEvent(const SISEventFlow* SISFlow)
   {
      //Add timestamps to dumps
      for (int j=0; j<NUM_SIS_MODULES; j++)
      {
         const std::vector<TSISEvent>& ce = SISFlow->sis_events[j];
         for (size_t i = 0; i < ce.size(); i++)
         {
            const TSISEvent& e = ce.at(i);
            for (int a = 0; a < USED_SEQ; a++)
            {
               if (DumpStartChannels.at(a).IsValid())
               {
                  const int counts = e.GetCountsInChannel(DumpStartChannels[a]);
                  //if (e->GetCountsInChannel(DumpStartChannels[a]))
                  for (int nstarts = 0; nstarts < counts; nstarts++)
                  {
                     fDumpMarkerFIFO.emplace_back(a,0,e.GetMidasUnixTime(), e.GetRunTime());//,GetTF1());
                  }
               }
               if (DumpStopChannels.at(a).IsValid())
               {
                  const int counts = e.GetCountsInChannel(DumpStopChannels[a]);
                  for (int nstops = 0; nstops < counts; nstops++)
                  {
                     fDumpMarkerFIFO.emplace_back(a,1,e.GetMidasUnixTime(), e.GetRunTime());//,GetTF1());
                  }
               }
            }
         }
      }
      // Clean up dump marker FIFO
      double vf48_oldest_event = fFIFO.front().GetRunTime();
      while (true)
      {
          if (!fDumpMarkerFIFO.size())
             break;
          if (fDumpMarkerFIFO.front().GetRunTime() < vf48_oldest_event)
          {
             std::cout<<"Popping fDumpMarker at " << fDumpMarkerFIFO.front().GetRunTime() <<"\n";
             fDumpMarkerFIFO.pop_front();
          }
          else
          {
             break;
          }
      }

   }
#endif

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {

      SilEventFlow* SilFlow = flow->Find<SilEventFlow>();

      if (SilFlow)
         AddEvent(SilFlow->silevent);

#if LIVE_DUMP_MARKERS
      //A2SpillFlow* SpillFlow = flow->A2SpillFlow<A2SpillFlow>();
      SISEventFlow* SISFlow = flow->Find<SISEventFlow>();
      if (SISFlow)
         AddEvent(SISFlow);
#endif

      auto time_now = std::chrono::high_resolution_clock::now(); //measure time starting here
      auto dt = std::chrono::duration_cast<std::chrono::milliseconds>( time_now - fLastHistoUpdate);
      if ( dt.count() < 25)
         return flow;
      fLastHistoUpdate = time_now;

      //Resise histograms
      for (int j=0; j < 4; j++)
         fOccupancyT[j].GetXaxis()->Set(BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());
#if LIVE_DUMP_MARKERS
      fDumpStart.GetYaxis()->Set(BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());
#endif
      fZTvert.GetYaxis()->Set(BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());
      fZTpass.GetYaxis()->Set(BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());

      for (int j=0; j < 4; j++)
         fOccupancyT[j].Reset();
      fXYvert.Reset();
      fXYpass.Reset();
#if LIVE_DUMP_MARKERS
      fDumpStart.Reset();
#endif
      fZTvert.Reset();
      fZTpass.Reset();
      fSVDStyle->SetPalette(kCool);

      //Update the histograms
      for (TSVD& s: fFIFO)
      {
         const size_t nVerts = s.fHasVertex.size();
         for (int i = 0; i < nVerts; i++)
         {
            if (!s.fHasVertex[i])
              continue;
            fXYvert.Fill(s.fX[i], s.fY[i]);
            fZTvert.Fill(s.fZ[i], s.fRunTime);
            if (!s.fPassCut[i])
               continue;
            fXYpass.Fill(s.fX[i], s.fY[i]);
            fZTpass.Fill(s.fZ[i], s.fRunTime);
         }
         for (int j = 0; j < 4; j++)
         for (int i = 0; i < nSil; i++)
         {
            if (s.fCounts[i + nSil*j])
            {
               fOccupancyT[j].Fill(s.fRunTime, i, s.fCounts[i + nSil*j]);
               //std::cout<<s.fRunTime << "\t" << s.fCounts[i] << std::endl;
            }
         }
      }
      fLiveVertex.cd(1);
      fXYvert.Draw("colz");
      fLiveVertex.cd(2);
      fZTvert.Draw("colz");
#if LIVE_DUMP_MARKERS
      for (TSVDDumpMarker& d: fDumpMarkerFIFO)
      {
          double t = d.GetRunTime();
          fDumpStart.Fill(0., t );
      }
      fDumpStart.SetFillColor(kBlack)
      fDumpStart.Draw("colz SAME");
      gPad->Modified();
   
      gPad->Update();
      gPad->Draw();
      fLiveVertex.Modified();
      fLiveVertex.Update();
      fLiveVertex.Draw();
        /* auto f1=new TF1("f1","1000*TMath::Abs(sin(x)/x)",-10,10);
            f1->SetLineColor(kBlue);
   f1->SetLineWidth(4);
   f1->Draw("same");
 */
#endif
      fLiveVertex.cd(3);
      fXYpass.Draw("colz");
      fLiveVertex.cd(4);
      fZTpass.Draw("colz");
#if LIVE_DUMP_MARKERS
      /*for (TSVDDumpMarker& d: fDumpMarkerFIFO)
      {
          d.Draw();
      }*/
#endif
      for (int i = 0; i < 4; i++)
      {
         fLiveOccupancy.cd(i + 1);
         fOccupancyT[i].Draw("colz");
      }
      //fLiveCanvas.Draw();
      return flow;
   }
};


static TARegister tar1(new TAFactoryTemplate<SVDMonitor>);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
