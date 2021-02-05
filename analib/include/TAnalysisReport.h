#ifndef _ANALYSIS_REPORT_
#define _ANALYSIS_REPORT_

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include "TObject.h"
#include <unistd.h> // readlink()
#include <ctime>

#include "GitInfo.h"
//Fast class for calculating Mean and Mode of data
class MeanMode: public TObject
{
    private:
    const int mode_size;
    std::vector<int>* mode_hist;
    double running_mean;
    int entries;
    public:
    MeanMode(const int size): mode_size(size)
    {
        mode_hist=new std::vector<int>(size,0);
        running_mean=0;
        entries=0;
    }
    ~MeanMode()
    {
        mode_hist->clear();
        delete mode_hist;
    }
    void InsertValue(const int &x)
    {
        if (x<mode_size)
            (*mode_hist)[x]++;
        else
            mode_hist->back()++;
        running_mean+=(double)x;
        entries++;
        return;
    }
    double GetSum()
    {
        return running_mean;
    }
    int GetEntires()
    {
        return entries;
    }
    double GetMean()
    {
        return running_mean/(double)entries;
    }
    long unsigned GetMode()
    {
        int* mode_ptr=std::max_element(&mode_hist->front(),&mode_hist->back());
        return (long unsigned)(mode_ptr-&mode_hist->front());
    }
    int GetBin(const int x)
    {
        return mode_hist->at(x);
    }
    double GetRate(const int bin,const int t)
    {
        return (double) mode_hist->at(bin)/t;
    }
    ClassDef(MeanMode,1);
};


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
    TAnalysisReport();
    TAnalysisReport(int runno);
    virtual ~TAnalysisReport();
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
    std::string GetRunStartTimeString()
    {
        time_t t = StartRunUnixTime;
        return std::string(asctime(localtime(&t)));
    }
    std::string GetRunStopTimeString()
    {
        if (StopRunUnixTime==0)
            return std::string("UNKNOWN\t(end-of-run ODB entry not processed)");
        time_t t = StopRunUnixTime;
        return std::string(asctime(localtime(&t)));
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
    double PrintHeader();
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
#include "TSiliconEvent.h"
class TA2AnalysisReport: public TAnalysisReport
{
private:
    //ALPHA 2
    int nSVDEvents=0;
    MeanMode SVD_N_RawHits{1000};
    MeanMode SVD_P_RawHits{1000};
    //MeanMode SVD_N_Clusters(1000);
    //MeanMode SVD_P_Clusters(1000);
    MeanMode SVD_RawHits{1000};
    MeanMode SVD_Hits{1000};
    MeanMode SVD_Tracks{100};
    MeanMode SVD_Verts{10};
    MeanMode SVD_Pass{2};
    double LastVF48TimeStamp;
 public:
    TA2AnalysisReport();
    TA2AnalysisReport(int runno);
    virtual ~TA2AnalysisReport();
    void FillSVD(TSiliconEvent* se);
    using TObject::Print;
    virtual void Print();
    ClassDef(TA2AnalysisReport,1);
};
#endif

#endif