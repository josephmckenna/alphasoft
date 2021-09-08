#ifdef BUILD_A2

#ifndef _TSISPlot_
#define _TSISPlot_

#include "TScalerPlot.h"
#include "TA2Spill.h"
#include "TA2SpillGetters.h"

class TSISPlot: public TScalerPlot
{
    private:
       std::vector<int> fSISChannels;
       void LoadRun(int runNumber);
       void AddEvent(TSISEvent* event, int channel, double timeOffset);
       void AddSISEvent(TSISEvent* SISEvent);
    public: 
       TSISPlotEvents SISEvents;
    TSISPlot(bool zeroTime = true);
    virtual ~TSISPlot();
    const std::vector<int> GetArrayOfRuns()       {  return fRuns; }
    TString GetListOfRuns();
    //These are a retred of the implementations in TA2Plot... perhaps lets consider multiple inheritance to get this functionality 
    void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition );
    void AddDumpGates(int runNumber, std::vector<TA2Spill> spills);
    void AddDumpGates(std::vector<TA2Spill> spills);
    void LoadData();
    void DrawSummed();
    void DrawStacked();
    ClassDef(TSISPlot,1);
};

#endif

#endif