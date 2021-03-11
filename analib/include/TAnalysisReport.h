#ifndef _ANALYSIS_REPORT_
#define _ANALYSIS_REPORT_

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include "TObject.h"
#include <unistd.h> // readlink()
#include <ctime>
#include "TH1D.h"
#include <map>

class TAnalysisReport: public TObject
{
private:
//Set in ctor:
    const int runNumber;
    std::string ProgramName;
    std::string ProgramPath;
    std::string ProgramPathFull;

    //std::string StartRun;
    uint32_t StartRunUnixTime;
    //std::string StopRun;
    uint32_t StopRunUnixTime;
    double Duration;
    
    std::string AnalysisHost;
    const uint32_t CompilationDate;
    const std::string GitBranch;
    const uint32_t GitDate;
    const std::string GitHash;
    const std::string GitHashLong;
    const std::string GitDiff;
public:
    //General containers for child classes to store data
    //Its not super fast, but these only get called at Flush and Print (once per run)
    std::map<std::string, bool> BoolValue;
    std::map<std::string, int> IntValue;
    std::map<std::string, double> DoubleValue;
    std::map<std::string, std::string> StringValue;

    TAnalysisReport();
    TAnalysisReport(const TAnalysisReport& r);
    TAnalysisReport operator=(const TAnalysisReport& r);
    TAnalysisReport(int runno);
    virtual ~TAnalysisReport();
    int GetRunNumber() const
    {
        return runNumber;
    }
    std::string GetProgramName() const
    {
        return ProgramName;
    }
    std::string GetProgramPath() const
    {
        return ProgramPath;
    }
    std::string GetProgramPathFull() const
    {
        return ProgramPathFull;
    }
    void SetStartTime(uint32_t time)
    {
        StartRunUnixTime = time;
    }
    void SetStopTime(uint32_t time)
    {
        StopRunUnixTime = time;
    }
    uint32_t GetRunStartTime() const
    {
        return StartRunUnixTime;
    }
    uint32_t GetRunStopTime() const
    {
        return StopRunUnixTime;
    }
    std::string GetRunStartTimeString() const
    {
        time_t t = StartRunUnixTime;
        return std::string(asctime(localtime(&t)));
    }
    std::string GetRunStopTimeString() const
    {
        if (StopRunUnixTime==0)
            return std::string("UNKNOWN\t(end-of-run ODB entry not processed)");
        time_t t = StopRunUnixTime;
        return std::string(asctime(localtime(&t)));
    }
    double GetDuration() const
    {
        return Duration;
    }
    std::string GetAnalysisHost() const
    {
        return AnalysisHost;
    }
    
    uint32_t GetCompilationDate() const
    {
        return CompilationDate;
    }
    std::string GetCompilationDateString() const
    {
        time_t t = CompilationDate;
        struct tm* tm = localtime(&t);
        char comp_date[20];
        strftime(comp_date, sizeof(comp_date), "%Y-%m-%d\t%X", tm);
        return std::string(comp_date);
    }
    uint32_t GetGitDate() const
    {
        return GitDate;
    }
    std::string GetGitDateString() const
    {
        //Git revision date:
        time_t t = GitDate;
        struct tm *tm = localtime(&t);
        char date[20];
        strftime(date, sizeof(date), "%Y-%m-%d\t%X", tm);
        return std::string(date);
    }
    std::string GetGitBranch() const
    {
        return GitBranch;
    }
    std::string GetGitHash() const
    {
        return GitHash;
    }
    std::string GetGitHashLong() const
    {
        return GitHashLong;
    }
    std::string GetGitDiff() const
    {
        return GitDiff;
    }
    void PrintHeader();
    void PrintFooter();
    ClassDef(TAnalysisReport,1);
};
/*
===========================================================
alphaAnalysis.exe Report for run 45000
===========================================================
Start Run: Sun Aug  7 22:12:15 2016
Stop Run: Sun Aug  7 22:17:44 2016
Duration: 329 s
Number of SVD Events:   3400
                Mode    Mean
SVD #RawNHits:  166     180.704
SVD #RawPHits:  66      79.7347
SVD #Hits:      6       28.4882
SVD #Tracks:    2       1.69559
----------------Sum-----Mean---------
SVD #Events:    3400
SVD #Verts:     2102    0.618235        ~(6.4Hz)
SVD #Pass cuts: 236     0.0694118       ~(0.7Hz)
Time of Last Event: -1 s
Compilation date:2021-02-05     13:45:11
Analysis run on host: UNKNOWN
Git branch:      easy_SWAN
Git date:         2021-02-05    12:23:08
Git hash:        805a86c8
Git hash (long): 805a86c8fdbbd3605ae4cb5211d851c8c894bc35
Git diff (shortstat): 3 files changed, 38 insertions(+), 2 deletions(-)
===========================================================
*/


#ifdef BUILD_A2
//ALPHA 2
class TA2AnalysisReport: public TAnalysisReport
{
private:

    // These are fast counters... do not use IntValue and DoubleValue maps for 
    // these as we want to use these every event (not just at the end of run)
    int nSVDEvents=0;
    double LastVF48TimeStamp;
    int SVD_Verts_Sum;
    int SVD_PassCut_Sum;

 public:
    TA2AnalysisReport();
    TA2AnalysisReport(const TA2AnalysisReport& r);
    TA2AnalysisReport operator=(const TA2AnalysisReport& r);
    TA2AnalysisReport(int runno);
    virtual ~TA2AnalysisReport();
    int GetPassCutSum()
    {
        return SVD_PassCut_Sum;
    }
    int GetVertexSum()
    {
        return SVD_Verts_Sum;
    }
    void FillSVD(const Int_t& nraw, const Int_t&praw, const Int_t& raw_hits, const Int_t& hits, const Int_t& tracks, const Int_t& verts, int pass, double time);
    void Flush();

    using TObject::Print;
    virtual void Print();
    ClassDef(TA2AnalysisReport,1);
};
#endif

#ifdef BUILD_AG

//ALPHA G
class TAGAnalysisReport: public TAnalysisReport
{
private:

    // These are fast counters... do not use IntValue and DoubleValue maps for 
    // these as we want to use these every event (not just at the end of run)
    int nStoreEvents = 0;
    int nSigEvents = 0;
    double last_tpc_ts = -1;

 public:
    TAGAnalysisReport();
    TAGAnalysisReport(const TAGAnalysisReport& r);
    TAGAnalysisReport operator=(const TAGAnalysisReport& r);

    void Flush(
       double sum_aw,       //Results from deconv module
       double sum_pad,      //Results from deconv module
       double sum_match,    //Results from match module
       double sum_tracks,   //Results from reco module
       double sum_r_sigma,  //Results from reco module
       double sum_z_sigma,  //Results from reco module
       double sum_verts,    //Results from reco module
       double sum_hits,     //Results from reco module
       double sum_bars);
    void IncrementStoreEvents()
    {
       nStoreEvents++;
       return;
    }
    void IncrementSigEvents()
    {
        nSigEvents++;
        return;
    }
    void SetLastTPCTime(const double& t)
    {
        last_tpc_ts = t;
        return;
    }
    TAGAnalysisReport(int runno);
    virtual ~TAGAnalysisReport();
    


    using TObject::Print;
    virtual void Print();
    ClassDef(TAGAnalysisReport,1);
    };
#endif
#endif