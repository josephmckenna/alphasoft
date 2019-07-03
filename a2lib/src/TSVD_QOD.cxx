#include "TSVD_QOD.h"

ClassImp(A2RunQOD);
A2RunQOD::A2RunQOD(TARunInfo* runinfo)
{
   RunNumber= runinfo->fRunNo;
   midas_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
   midas_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
   CosmicRunTrigger = runinfo->fOdb->odbReadBool("/Experiment/edit on start/Cosmic_Run");
}

ClassImp(SVDQODIntegrator);
SVDQODIntegrator::SVDQODIntegrator(A2RunQOD* q, double start, double stop)
{
   Runs.push_back(q->RunNumber);
   WindowStart.push_back(start);
   WindowStart.push_back(stop);
}
void SVDQODIntegrator::NewWindow(A2RunQOD* q, double start, double stop)
{
   Runs.push_back(q->RunNumber);
   WindowStart.push_back(start);
   WindowStart.push_back(stop);
}
void SVDQODIntegrator::AddEvent(SVDQOD* e)
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

void SVDQODIntegrator::AddOccupancy(SVDQOD* e)
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

void SVDQODIntegrator::AddEvent(SISQOD* s)
{
   int nRuns=Runs.size();
   for (int i=0; i<nRuns; i++)
   {
      assert(s->RunNumber == Runs[i]);
      assert(s->t >= WindowStart[i]);
      assert(s->t <  WindowStop[i]);
   } 
}

ClassImp(SVDQOD);
SVDQOD::SVDQOD(TAlphaEvent* a, TSiliconEvent* s )
{
   VF48Timestamp    = s->GetVF48Timestamp();
   NRawHits         = s->GetNsideNRawHits();
   PRawHits         = s->GetPsideNRawHits();
   NHits            = s->GetNHits();
   NTracks          = s->GetNTracks();
   NVertices        = s->GetNVertices();
   ProjectedVertices= (int)(s->GetVertexType() & 2);
   NPassedCuts      = (int)s->GetPassedCuts();
   TVector3* vertex = s->GetVertex();
   if (NVertices)
   {
      x=vertex->X();
      y=vertex->Y();
      z=vertex->Z();
   }
   else
   {
      x=-99;
      y=-99;
      z=-99;
   }
   for (int i=0; i<72; i++)
   HitOccupancy[i]=0;
   std::vector<TAlphaEventHit*>* hits=a->GatherHits();
   int NHits=hits->size();
   for(int j = 0;j<NHits;j++)
   {
      TAlphaEventHit * c = hits->at(j);
      int SilNum=c->GetSilNum();
      HitOccupancy[SilNum]++;
      if (j<36)
         OccupancyUS++;
      else
         OccupancyDS++;
   }

}
