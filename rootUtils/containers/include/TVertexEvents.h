#ifndef __TVertexEvents__
#define __TVertexEvents__

#include "TObject.h"
#include <string>
#include <vector>
#include <assert.h>
#include <numeric>
#include <algorithm>
#include <string>

//SVD / TPC vertex
class TVertexEvents: public TObject
{
   public:
      std::vector<int> fRunNumbers; 
      std::vector<int> fEventNos;
      std::vector<int> fCutsResults;
      std::vector<int> fVertexStatuses;
      std::vector<double> fXVertex;
      std::vector<double> fYVertex;
      std::vector<double> fZVertex;
      std::vector<double> fTimes; //Plot time (based off official time)
      std::vector<double> fEventTimes; //TPC time stamp
      std::vector<double> fRunTimes; //Official Time
      std::vector<int> fNumHelices; // helices used for vertexing
      std::vector<int> fNumTracks; // reconstructed (good) helices

      //Copy and assign operators
      TVertexEvents();
      ~TVertexEvents();
      //Basic copy constructor.
      TVertexEvents(const TVertexEvents& vertexEvents);
      //Assignment operator.
      TVertexEvents& operator=(const TVertexEvents& rhs);
      //=+ Operator.
      TVertexEvents operator+=(const TVertexEvents &rhs);
      std::string CSVTitleLine() const;
      std::string CSVLine(size_t i) const;
      size_t size() const;

   ClassDef(TVertexEvents,1);
};

#endif
