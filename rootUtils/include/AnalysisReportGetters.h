#ifndef _ANALYSIS_REPORT_GETTERS_
#define _ANALYSIS_REPORT_GETTERS_


#include "TAnalysisReport.h"
#include "TreeGetters.h"
#ifdef BUILD_A2
TA2AnalysisReport Get_A2Analysis_Report(int runNumber, bool force = false);
#endif

#ifdef BUILD_AG
TAGAnalysisReport Get_AGAnalysis_Report(int runNumber, bool force = false);
#endif


#endif