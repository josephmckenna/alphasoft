#include "TAnalysisReport.h"
#include "GitInfo.h"
ClassImp(TAnalysisReport)
TAnalysisReport::TAnalysisReport():
   runNumber(-1),
   CompilationDate(COMPILATION_DATE), 
   GitBranch(GIT_BRANCH), 
   
   GitDate(GIT_DATE), 
   GitHash(GIT_REVISION),
   GitHashLong(GIT_REVISION_FULL),
   GitDiff(GIT_DIFF_SHORT_STAT)
{
    //ctor
}

TAnalysisReport::TAnalysisReport(const TAnalysisReport& r):
   TObject(r),
   runNumber(r.runNumber),
   CompilationDate(r.CompilationDate), 
   GitBranch(r.GitBranch), 
   GitDate(r.GitDate), 
   GitHash(r.GitHash),
   GitHashLong(r.GitHashLong),
   GitDiff(r.GitDiff),
   BoolValue(r.BoolValue),
   IntValue(r.IntValue),
   DoubleValue(r.DoubleValue),
   StringValue(r.StringValue)
{
    ProgramName = r.ProgramName;
    ProgramPath = r.ProgramPath;
    ProgramPathFull = r.ProgramPathFull;

    StartRunUnixTime = r.StartRunUnixTime;
    StopRunUnixTime = r.StopRunUnixTime;
    Duration = r.Duration;
    AnalysisHost = r.AnalysisHost;
}

TAnalysisReport TAnalysisReport::operator=(const TAnalysisReport& r)
{
    return TAnalysisReport(r);
}


TAnalysisReport::TAnalysisReport(int runno):
    runNumber(runno), 
    CompilationDate(COMPILATION_DATE), 
    GitBranch(GIT_BRANCH), 
    GitDate(GIT_DATE), 
    GitHash(GIT_REVISION),
    GitHashLong(GIT_REVISION_FULL),
    GitDiff(GIT_DIFF_SHORT_STAT)
{
    StartRunUnixTime = 0;
    StopRunUnixTime = 0;
    char result[ 200 ]={0};
    size_t result_len=readlink( "/proc/self/exe", result, 200 );
    if (result_len)
    {
        ProgramPathFull=result;
        std::size_t found = ProgramPathFull.find_last_of("/\\");
        //std::cout << " path: " << binary_path_full.substr(0,found).c_str() << '\n';
        //std::cout << " file: " << binary_path_full.substr(found+1).c_str() << '\n';
        ProgramPath=ProgramPathFull.substr(0,found);
        ProgramName=ProgramPathFull.substr(found+1);
    }
    else
    {
        ProgramPath="readlink of /proc/self/exe failed";
        ProgramName="readlink of /proc/self/exe failed";
    }
  
    if(getenv("HOSTNAME")!=nullptr)
    {
        AnalysisHost=getenv("HOSTNAME");
    }
    else
    {
         AnalysisHost="UNKNOWN";
    }
}
TAnalysisReport::~TAnalysisReport()
{
    //dtor
}
void TAnalysisReport::PrintHeader()
{
    printf("===========================================================\n");
    printf("%s Report for run %d\n",ProgramName.c_str(),runNumber);
    printf("===========================================================\n");
    std::cout <<"Start Run: "<<GetRunStartTimeString();
    std::cout <<"Stop Run: "<<GetRunStopTimeString();
    if( StopRunUnixTime > StartRunUnixTime )
    {
        std::cout <<"Duration: ~"<<StopRunUnixTime-StartRunUnixTime<<"s"<<std::endl;
    }
    else
    {
        std::cout << std::endl;
    }
    return;
}
void TAnalysisReport::PrintFooter()
{
    printf("Compilation date:%s\n",GetCompilationDateString().c_str());
    std::cout <<"Analysis run on host: "<< GetAnalysisHost().c_str() << std::endl;
    printf("Git branch:      %s\n",GetGitBranch().c_str());
    printf("Git date:         %s\n",GetGitDateString().c_str());
    printf("Git hash:        %s\n",GetGitHash().c_str());
    printf("Git hash (long): %s\n",GetGitHashLong().c_str());
    printf("Git diff (shortstat):%s\n",GetGitDiff().c_str());
    printf("===========================================================\n");
}
