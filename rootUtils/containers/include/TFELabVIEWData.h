
#ifndef __TFELabVIEWData__
#define __TFELabVIEWData__
#include "TEnvData.h"

#include "TStoreLabVIEWEvent.h"
#include "TTimeWindows.h"

//Specialise the above for feLabVIEW
class TFELabVIEWData: public TEnvData
{
   public:
      TFELabVIEWData();
      ~TFELabVIEWData();
      void AddLVEvent(TStoreLabVIEWEvent* labviewEvent, TTimeWindows& timeWindows);
   ClassDef(TFELabVIEWData,1);
};


#endif
