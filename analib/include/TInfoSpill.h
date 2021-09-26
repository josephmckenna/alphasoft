#ifndef _TInfoSpill_
#define _TInfoSpill_
#include "TSpill.h"


//Generic infromation class for passing strings to the spill log (agnostic of ALPHA2 or ALPHAg)

class TInfoSpill: public TSpill
{
private:
   uint32_t fRunStartTime;
public:
   TInfoSpill();
   TInfoSpill(int runno, uint32_t runstarttime, uint32_t unixtime);
   TInfoSpill(int runno, uint32_t runstarttime, uint32_t unixtime, const char* format, ...);
   TInfoSpill(const TInfoSpill& a);
   double GetStartTime() const;
   double GetStopTime() const;
   ClassDef(TInfoSpill,1);
};


#endif
