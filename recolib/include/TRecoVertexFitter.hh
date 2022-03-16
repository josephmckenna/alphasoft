#ifndef __RECOVERTEXFITTER__
#define __RECOVERTEXFITTER__

#include "TFitHelix.hh"

#include "TFitVertex.hh"
#include <iostream>

class TRecoVertexFitter
{
   private:
      const double fVtxChi2Cut;
      const bool fTrace;

   public:
      TRecoVertexFitter(double chi2cut, bool trace): fVtxChi2Cut(chi2cut), fTrace(trace)
      {
      }
      int RecVertex(
         std::vector<TFitHelix>& HelixArray, 
         TFitVertex* Vertex, 
         const int thread_no = 1, 
         const int thread_count = 1);
};

#endif