#include "RootUtilGlobals.h"
namespace rootUtils
{
   static int gNbin = 100;

   void SetDefaultBinNumber(int i)
   {
      rootUtils::gNbin=i;
   }

   int GetDefaultBinNumber()
   {
      return rootUtils::gNbin;
   }
}
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */