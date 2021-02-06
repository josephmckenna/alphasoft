#include "AnalysisReportGetters.h"

#ifdef BUILD_A2
TA2AnalysisReport Get_A2Analysis_Report(int runNumber, bool force)
{
    TTreeReader* t=Get_TA2AnalysisReport_Tree(runNumber);
    if (t->GetEntries(force)>1)
    {
        std::cout<<"Warning! More than one analysis report in run?!?! What?"<<std::endl;
    }
    TTreeReaderValue<TA2AnalysisReport> AnalysisReport(*t, "TA2AnalysisReport");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   t->Next();
   return TA2AnalysisReport(*AnalysisReport);
}
#endif