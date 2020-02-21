#include "TSVDQODIntegrator.h"

ClassImp(TSVDQODIntegrator);
TSVDQODIntegrator::TSVDQODIntegrator(TA2RunQOD* q, double start, double stop)
{
   Runs.push_back(q->RunNumber);
   WindowStart.push_back(start);
   WindowStart.push_back(stop);
}
void TSVDQODIntegrator::NewWindow(TA2RunQOD* q, double start, double stop)
{
   Runs.push_back(q->RunNumber);
   WindowStart.push_back(start);
   WindowStart.push_back(stop);
}
void TSVDQODIntegrator::AddEvent(TSVD_QOD* e)
{
   int nRuns=Runs.size();
   for (int i=0; i<nRuns; i++)
   {
      assert(e->RunNumber == Runs[i]);
      assert(e->t >= WindowStart[i]);
      assert(e->t <  WindowStop[i]);
   } 
   AddNRawHit(e->NRawHits);
   AddPRawHit(e->PRawHits);
   AddHit(e->NHits);
   AddTrack(e->NTracks);
   AddVertices(e->NVertices);
   AddPassedCuts(e->NPassedCuts);
   AddOccupancy(e);
}

void TSVDQODIntegrator::AddOccupancy(TSVD_QOD* e)
{
   if (!h[0])
   {
       double tmin=WindowStart[0];
       double tmax=WindowStop[0];
       int Nbin=(int)(tmax-tmin)/OccupancyBinWidth;
       h[0] = new TH2I("Occupancy","Occupancy Vs Time of Passed Cut Events;Time (s);Silicon Module Number",Nbin, tmin,tmax, 72,-0.5,71.5);
       h[1] = new TH2I("Occupancy1","Occupancy Vs Time of Vextex Events;Time (s);Silicon Module Number",Nbin, tmin,tmax, 72,-0.5,71.5);
       h[2] = new TH2I("Occupancy2","Occupancy Vs Time with gNHitcut;Time (s);Silicon Module Number",Nbin, tmin,tmax, 72,-0.5,71.5);
       h[3] = new TH2I("Occupancy3","Occupancy Vs Time with no Hit Cut;Time (s);Silicon Module Number",Nbin, tmin,tmax, 72,-0.5,71.5);
   }

   int US=e->OccupancyUS;
   int DS=e->OccupancyUS;

   if (e->NPassedCuts)
   {
      OccupancyPassedCutsUS+=US;
      OccupancyPassedCutsDS+=DS;
      for (int i=0; i<72; i++) h[0]->Fill(e->t,e->HitOccupancy[i]);
   }
   if (e->NVertices)
   {
      OccupancyVertexCutsUS+=US;
      OccupancyVertexCutsDS+=DS;
      for (int i=0; i<72; i++) h[1]->Fill(e->t,e->HitOccupancy[i]);
   }
   if (US+DS>200)
   {
      OccupancyNHitsCutsUS_200+=US;
      OccupancyNHitsCutsDS_200+=DS;
      for (int i=0; i<72; i++) h[2]->Fill(e->t,e->HitOccupancy[i]);
   }
   OccupancyNoCutsUS+=US;
   OccupancyNoCutsDS+=DS;
   for (int i=0; i<72; i++) h[3]->Fill(e->t,e->HitOccupancy[i]);

}


