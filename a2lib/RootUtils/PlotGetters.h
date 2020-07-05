#include "A2RootUtils.h"
#include "TH1D.h"
#include "TSpline.h"
#ifndef _A2PlotGetters_
#define _A2PlotGetters_

void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax, double range = -1);
void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill*> spills);
void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> repetition);


void Plot_SVD(Int_t runNumber, std::vector<double> tmin, std::vector<double> tmax);
void Plot_SVD(Int_t runNumber, std::vector<TA2Spill*> spills);
void Plot_SVD(Int_t runNumber, std::vector<std::string> description, std::vector<int> repetition);


#endif
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
