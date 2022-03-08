#include "TRecoVertexFitter.hh"

int TRecoVertexFitter::RecVertex(std::vector<TFitHelix*> HelixArray, TFitVertex* Vertex)
{
   int Nhelices = 0;
   Vertex->SetChi2Cut( fVtxChi2Cut );
   int nhel=HelixArray.size();
   for( int n = 0; n<nhel; ++n )
      {
         TFitHelix* hel = (TFitHelix*)HelixArray.at(n);
         if( hel->IsGood() )
            {
               Vertex->AddHelix(hel);
               ++Nhelices;
            }
      }
   if( fTrace )
      std::cout<<"Reco::RecVertex(  )   # helices: "<<nhel<<"   # good helices: "<<Nhelices<<std::endl;
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