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


#endif
