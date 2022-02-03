#include "A2Flow.h"


VectorRecycler<uint32_t> VF48data::gVF48dataRecycler(1000);
PointerRecycler<TSiliconEvent> SilEventFlow::gTSiliconEventRecycleBin(1000);
PointerRecycler<TAlphaEvent> SilEventFlow::gTAlphaEventRecycleBin(1000);
VectorRecycler<TSISBufferEvent> SISModuleFlow::gTSISBufferEventRecycleBin(1000);
VectorRecycler<TSISEvent> SISEventFlow::gTSISEventRecycleBin(1000);