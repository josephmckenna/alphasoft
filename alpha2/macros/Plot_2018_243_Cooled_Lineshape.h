
#include "TA2Plot.h"
#include "TA2Plot_Filler.h"

#include "TA2SpillGetters.h"
#include "TA2Spill.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TCanvas.h"


#include <vector>
#include <utility> //std::pair

int Plot_2018_243_Cooled_Lineshape(bool DrawVertices = false, bool zeroTime = true);
std::vector<TA2Plot*> Plot_243_Light_And_Dark_Lineshape(int runNumber, bool DrawVertices, bool zeroTime = true);
//double GetRep(double time, TTimeWindows* timeWindows, int& lastRep);
double GetRep(double time, const std::vector<TA2Spill>& LaserSpills, int& lastRep, bool& IsLight);
double GetRep1(double time, const std::vector<TA2Spill>& LaserSpills, int& lastRep, bool& IsLight);
