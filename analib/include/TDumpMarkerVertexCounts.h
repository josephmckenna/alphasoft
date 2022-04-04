#ifndef _TDUMPMARKERVERTEXCOUNTS_
#define _TDUMPMARKERVERTEXCOUNTS_

#include <ostream>

template <typename VertexType>
class TDumpMarkerVertexCounts
{
   public:
   int fFirstEventID = -1;
   int fLastEventID  = -1;
   int fEvents       = 0;
   int fVerticies    = 0;
   int fPassCuts     = 0;
   int fPassMVA      = 0;
   void AddEvent(const VertexType& e)
   {
      fLastEventID     = e.GetEventNumber();
      if (fFirstEventID<0)
         fFirstEventID = e.GetEventNumber();
      fEvents++;
      if (e.GetVertexStatus())
         fVerticies++;
      fPassCuts       += e.GetOnlinePassCuts();
      fPassMVA        += e.GetOnlinePassMVA();
   }
   friend std::ostream& operator<<(std::ostream& os, const TDumpMarkerVertexCounts& SVDC)
    {
      if(SVDC.fFirstEventID == -1)
      {
         os << "DEBUG: TDumpMarkerVertexCounts object at" << &SVDC << "is not initialised/is empty" << std::endl;
         return os;
      }
      else
      {
         os << "DEBUG: First VF48Event = " << SVDC.fFirstEventID << std::endl
            << "DEBUG: Last VF48Event = " << SVDC.fLastEventID << std::endl
            << "DEBUG: fPassCuts = " << SVDC.fPassCuts << std::endl
            << "DEBUG: fPassMVA = " << SVDC.fPassMVA << std::endl
            << "DEBUG: Vertices = " << SVDC.fVerticies << std::endl
            << "DEBUG: fVF48Events = " << SVDC.fEvents << std::endl;

         return os;
      }
      return os;
   }
};
#endif