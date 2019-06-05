#ifndef _TAlphaEventObjectArray_
#define _TAlphaEventObjectArray_

#include "TAlphaEventObject.h"

class TAlphaEventObjectArray()
{
   static TAlhpaEventObject Sil[72];
   TAlphaEventObjectArray()
   {
      for (int i=0; i<72; i++)
         Sil[i]=TAlphaEventObject(i);
   }
}












#endif
