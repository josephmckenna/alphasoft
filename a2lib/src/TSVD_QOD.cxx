#include "TSVD_QOD.h"



ClassImp(TSVD_QOD);
TSVD_QOD::TSVD_QOD(TAlphaEvent* a, TSiliconEvent* s )
{
   VF48Timestamp    = s->GetVF48Timestamp();
   VF48NEvent       = s->GetVF48NEvent();
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

TSVD_QOD::TSVD_QOD()
{
}
TSVD_QOD::~TSVD_QOD()
{
}
