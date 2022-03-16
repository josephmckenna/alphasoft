#include "TRecoVertexFitter.hh"

int TRecoVertexFitter::RecVertex(std::vector<TFitHelix>& HelixArray, TFitVertex* Vertex, const int thread_no, const int thread_count)
{
   // sort the helices by |c|, so that lowest |c| (highest momentum) is first
   if (thread_no == 1)
   {
      std::sort(HelixArray.begin(),HelixArray.end(),SortMomentum);
      Vertex->SetChi2Cut( fVtxChi2Cut );
      for (TFitHelix& hel: HelixArray)
      {
         if( hel.IsGood() )
         {
            Vertex->AddHelix(&hel);
         }
      }
   }
   if( fTrace )
      std::cout<<"Reco::RecVertex(  )   # helices: "<<HelixArray.size()<<"   # good helices: "<<Vertex->GetNumberOfAddedHelix()<<std::endl;
   // reconstruct the vertex
   int sv = -2;

   if (Vertex->GetNumberOfAddedHelix())
   {
     sv = Vertex->Calculate(thread_no,thread_count);
     if( fTrace )
        std::cout<<"Reco::RecVertex(  )   # used helices: "<<Vertex->GetNumberOfHelices()<<std::endl;
   }
   return sv;
}
