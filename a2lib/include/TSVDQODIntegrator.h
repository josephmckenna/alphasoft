#ifndef _TSVDQODIntegrator_
#define _TSVDQODIntegrator_

#include "TSVD_QOD.h"
#include "TA2RunQOD.h"

class TSVDQODIntegrator: public TSVD_QOD
{
   private:
   std::vector <double> WindowStart;
   std::vector <double> WindowStop;
   std::vector <int>    Runs;
   int VF48Events=0;
   TH1I* NRawHits_hist;     int NRawHitsBins=1000;
   TH1I* PRawHits_hist;     int PRawHitsBins=1000;
   TH1I* NHits_hist;        int NHitsBins=1000;
   TH1I* NTracks_hist;      int NTracksBins=100;
   TH2I* OccupancyAgainstTime[4];
   double xyrange= 5.;
   double zrange=30.;
   TH1I* XVerts;       int NVertexBins=100;
   TH1I* YVerts;
   TH1I* ZVerts;
   TH1I* XPassed;
   TH1I* YPassed;
   TH1I* ZPassed;
   int OccupancyPassedCutsUS;
   int OccupancyPassedCutsDS;
   int OccupancyVertexCutsUS;
   int OccupancyVertexCutsDS;
   int OccupancyNHitsCutsUS_200;
   int OccupancyNHitsCutsDS_200;
   int OccupancyNoCutsUS;
   int OccupancyNoCutsDS;
   TH2I* h[4]={NULL};
   double OccupancyBinWidth=10;
   
   
   //TH1I* t;?
   TSVDQODIntegrator(TA2RunQOD* q,double start, double stop);
   void NewWindow(TA2RunQOD* q, double start, double stop);
   void AddEvent(TSVD_QOD* e);
   
   void AddOccupancy(TSVD_QOD* e);
   void AddNRawHit(int i)
   {
      if (!NRawHits_hist) NRawHits_hist=new TH1I("NRawHits","NRawHits",NRawHitsBins,0,NRawHitsBins);
      NRawHits_hist->Fill(i);
      NRawHits+=i;
   }
   
   void AddPRawHit(int i)
   {
      if (!PRawHits_hist) PRawHits_hist=new TH1I("PRawHits","PRawHits",PRawHitsBins,0,PRawHitsBins);
      PRawHits_hist->Fill(i);
      PRawHits+=i;
   }
   void AddHit(int i)
   {
      if (!NHits_hist) NHits_hist=new TH1I("NHits","NHits",NHitsBins,0,NHitsBins);
      NHits_hist->Fill(i);
      NHits+=i;
   }
   void AddTrack(int i)
   {
      if (!NTracks_hist) NTracks_hist=new TH1I("NTracks","NTracks",NTracksBins,0,NTracksBins);
      NTracks_hist->Fill(i);
      NTracks+=i;
   }
   void AddVertices(int i)
   {
      NVertices+=i;
   }
   void AddPassedCuts(int i)
   {
      NPassedCuts+=i;
   }
   void AddVertex(double _x, double _y, double _z)
   {
      if (!XPassed) XPassed=new TH1I("XPassed","XPassed",NVertexBins,-xyrange,xyrange);
      if (!YPassed) YPassed=new TH1I("YPassed","YPassed",NVertexBins,-xyrange,xyrange);
      if (!ZPassed) ZPassed=new TH1I("ZPassed","ZPassed",NVertexBins,-zrange,zrange);
      XVerts->Fill(_x);
      YVerts->Fill(_y);
      ZVerts->Fill(_z);
   }
   void AddPassedVertex(double _x, double _y, double _z)
   {
      if (!XVerts) XVerts=new TH1I("XVerts","XVerts",NVertexBins,-xyrange,xyrange);
      if (!YVerts) YVerts=new TH1I("YVerts","YVerts",NVertexBins,-xyrange,xyrange);
      if (!ZVerts) ZVerts=new TH1I("ZVerts","ZVerts",NVertexBins,-zrange,zrange);
      XVerts->Fill(_x);
      YVerts->Fill(_y);
      ZVerts->Fill(_z);
   }
   double TotalRunTime()
   { 
      double TotalTime=0;
      int nRuns=Runs.size();
      for (int i=0; i<nRuns; i++)
      {
         assert(WindowStop[i]>0);
         assert(WindowStart[i]>0);
         TotalTime+=WindowStop[i]-WindowStart[i];
      }
      return TotalTime;
   }
   double VF48EventRate()         { return (double)VF48Events         / TotalRunTime(); }
   double CosmicTracksRate()      { return (double)CosmicTracks      / TotalRunTime(); }
   double ProjectedVerticesRate() { return (double)ProjectedVertices / TotalRunTime(); }
   double NVerticesRate()         { return (double)NVertices         / TotalRunTime(); }
   double NPassedCutsRate()       { return (double)NPassedCuts       / TotalRunTime(); }
   
   int GetNEvent() { return VF48Events; }
   double NRawHitsMean()          { return (double)NRawHits          / VF48Events; }
   double PRawHitsMean()          { return (double)PRawHits          / VF48Events; }
   double NHitsMean()             { return (double)NHits             / VF48Events; }
   double NTracksMean()           { return (double)NTracks           / VF48Events; }

   double NVerticesEfficiency()   { return (double)NVertices         / VF48Events; }
   double NPassedCutsEfficiency() { return (double)NPassedCuts       / VF48Events; }
   
   double GetXVertMean();
   double GetYVertMean();
   double GetZVertMean();
   
   double GetXPassedMean();
   double GetYPassedMean();
   double GetZPassedMean();
   
   double OccupancyPassedCutsUSvsDS()
   {
      double US=(double)OccupancyPassedCutsUS;
      double DS=(double)OccupancyPassedCutsDS;
      return US/DS;
   }
   double OccupancyPassedCutsDiffUSvsDS()
   {
      double US=(double)OccupancyPassedCutsUS;
      double DS=(double)OccupancyPassedCutsDS;
      return (US-DS)/(US+DS);
   }

   double OccupancyVertexCutsUSvsDS()
   {
      double US=(double)OccupancyVertexCutsUS;
      double DS=(double)OccupancyVertexCutsDS;
      return US/DS;
   }
   double OccupancyVertexCutsDiffUSvsDS()
   {
      double US=(double)OccupancyVertexCutsUS;
      double DS=(double)OccupancyVertexCutsDS;
      return (US-DS)/(US+DS);
   }

   double OccupancyNHitsCutsUS_200USvsDS()
   {
      double US=(double)OccupancyNHitsCutsUS_200;
      double DS=(double)OccupancyNHitsCutsDS_200;
      return US/DS;
   }
   double OccupancyNHitsCutsUS_200DiffUSvsDS()
   {
      double US=(double)OccupancyNHitsCutsUS_200;
      double DS=(double)OccupancyNHitsCutsDS_200;
      return (US-DS)/(US+DS);
   }

   double OccupancyNoCutsUSUSvsDS()
   {
      double US=(double)OccupancyNoCutsUS;
      double DS=(double)OccupancyNoCutsDS;
      return US/DS;
   }
   double OccupancyNoCutsDiffUSvsDS()
   {
      double US=(double)OccupancyNoCutsUS;
      double DS=(double)OccupancyNoCutsDS;
      return (US-DS)/(US+DS);
   }
//Getters
//VF48 Event Rate (Hz)
//Cosmic Track rate
   ClassDef(TSVDQODIntegrator,1); 

};
#endif
