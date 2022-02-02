#ifndef _ANALYSIS_REPORT_GETTERS_
#define _ANALYSIS_REPORT_GETTERS_


#include "TAnalysisReport.h"
#include "TreeGetters.h"
#ifdef BUILD_A2
#include "TA2AnalysisReport.h"
TA2AnalysisReport Get_A2Analysis_Report(int runNumber, bool force = false);
#endif

#ifdef BUILD_AG
#include "TAGAnalysisReport.h"
TAGAnalysisReport Get_AGAnalysis_Report(int runNumber, bool force = false);
#endif


#endif