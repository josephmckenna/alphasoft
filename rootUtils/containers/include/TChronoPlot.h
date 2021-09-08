#ifndef _TChronoPlot_
#define _TChronoPlot_

#include "TScalerPlot.h"

class TChronoPlot: public TScalerPlot
{
    void LoadData();
    void DrawSummed();
    void DrawStacked();
};


#endif