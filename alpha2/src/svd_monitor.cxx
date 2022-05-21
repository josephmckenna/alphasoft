//
// SVD data montior, uses a circular buffer to histogram 
//
// JTK McKENNA
//

#include <stdio.h>

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

class SVDMonitor: public TARunObject
{
private:
   // Ring buffer would probably be quicker... but lets just get this working
   std::deque<TSVD> fFIFO;

   TCanvas fLiveVertex;
   TCanvas fLiveOccupancy;
   TH2I fXYvert;
   TH2I fXYpass;
   //TH2D fZY;
   TH2I fZTvert;
   TH2I fZTpass;
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
      
      gStyle->SetPalette(kCool);
      
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
   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {

      SilEventFlow* SilFlow = flow->Find<SilEventFlow>();
      if (!SilFlow)
         return flow;
      TSiliconEvent* se = SilFlow->silevent;
      int i = fFIFO.back().fBin;
      while (fFIFO.back().GetRunTime() < se->GetVF48Timestamp())
      {
         fFIFO.emplace_back(TSVD(++i));
         fFIFO.pop_front();
      }

      //Find bin of the first event
      int bin = 0;
      while ( se->GetVF48Timestamp() > fFIFO.at(bin).GetRunTime())
         bin++;
      fFIFO.at(bin) += *se;

      //Resise histograms
      for (int j=0; j < 4; j++)
         fOccupancyT[j].GetXaxis()->Set(BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());
      fZTvert.GetYaxis()->Set(BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());
      fZTpass.GetYaxis()->Set(BUFFER_DEPTH, fFIFO.front().GetRunTime(), fFIFO.back().GetRunTime());

      for (int j=0; j < 4; j++)
         fOccupancyT[j].Reset();
      fXYvert.Reset();
      fXYpass.Reset();
      fZTvert.Reset();
      fZTpass.Reset();
      
      fSVDStyle->SetPalette(kCool);
      //gROOT->ForceStyle();

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
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      fLiveVertex.cd(1);
      fXYvert.Draw("colz");
      fLiveVertex.cd(2);
      fZTvert.Draw("colz");
      fLiveVertex.cd(3);
      fXYpass.Draw("colz");
      fLiveVertex.cd(4);
      fZTpass.Draw("colz");
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
