
#include "TA2Plot.h"
#include "TA2Plot_Filler.h"
#include "TAPlot.h"
#include "TA2SpillGetters.h"
#include "TA2Spill.h"
#include "TGraph.h"
#include "TCanvas.h"


#include <vector>
#include <utility> //std::pair

int Plot_2018_243_Cooled_Lineshape(bool DrawVertices = false, bool zeroTime = true);
std::vector<TA2Plot*> Plot_243_Light_And_Dark_Lineshape(int runNumber, bool DrawVertices, bool zeroTime = true);

double GetRep(double time, TTimeWindows* timeWindows, int& lastRep); 
