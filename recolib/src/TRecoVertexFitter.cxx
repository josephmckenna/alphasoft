#include "TRecoVertexFitter.hh"

int TRecoVertexFitter::RecVertex(std::vector<TFitHelix>& HelixArray, TFitVertex* Vertex)
{
   int Nhelices = 0;
   // sort the helices by |c|, so that lowest |c| (highest momentum) is first
   std::sort(HelixArray.begin(),HelixArray.end(),SortMomentum);

   Vertex->SetChi2Cut( fVtxChi2Cut );
   for (TFitHelix& hel: HelixArray)
   {
      if( hel.IsGood() )
      {
         Vertex->AddHelix(&hel);
         ++Nhelices;
      }
   }
   if( fTrace )
      std::cout<<"Reco::RecVertex(  )   # helices: "<<HelixArray.size()<<"   # good helices: "<<Nhelices<<std::endl;
   // reconstruct the vertex
   int sv = -2;
   if( Nhelices )// find the vertex!
      {
         sv = Vertex->Calculate();
         if( fTrace )
            std::cout<<"Reco::RecVertex(  )   # used helices: "<<Vertex->GetNumberOfHelices()<<std::endl;
      }
   return sv;
}