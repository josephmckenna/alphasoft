#include "TAnalysisReport.h"
#include "GitInfo.h"
ClassImp(TAnalysisReport);
TAnalysisReport::TAnalysisReport():
   runNumber(-1),
   GitBranch(GIT_BRANCH), 
   CompilationDate(COMPILATION_DATE), 
   GitDate(GIT_DATE), 
   GitHash(GIT_REVISION),
   GitHashLong(GIT_REVISION_FULL),
   GitDiff(GIT_DIFF_SHORT_STAT)
{
    //ctor
}

TAnalysisReport::TAnalysisReport(int runno):
    runNumber(runno), 
    GitBranch(GIT_BRANCH), 
    CompilationDate(COMPILATION_DATE), 
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
double TAnalysisReport::PrintHeader()
{
    printf("===========================================================\n");
    printf("%s Report for run %d\n",ProgramName.c_str(),runNumber);
    printf("===========================================================\n");
    std::cout <<"Start Run: "<<GetRunStartTimeString();
    std::cout <<"Stop Run: "<<GetRunStopTimeString();
    int rough_time=-1;
    if( StopRunUnixTime > StartRunUnixTime )
    {
        rough_time=StopRunUnixTime-StartRunUnixTime;
        std::cout <<"Duration: ~"<<rough_time<<"s"<<std::endl;
    }
    return rough_time;
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
#ifdef BUILD_A2
ClassImp(TA2AnalysisReport);
TA2AnalysisReport::TA2AnalysisReport()
{
    //ctor
}
TA2AnalysisReport::TA2AnalysisReport(int runno): TAnalysisReport(runno)
{
    nSVDEvents = 0;
    LastVF48TimeStamp = -1;
}
TA2AnalysisReport::~TA2AnalysisReport()
{
    //dtor
}

void TA2AnalysisReport::FillSVD(TSiliconEvent* se)
{
    SVD_N_RawHits.InsertValue(se->GetNsideNRawHits());
    SVD_P_RawHits.InsertValue(se->GetPsideNRawHits());
    //SVD_N_Clusters.InsertValue(se->GetNNClusters());
    //SVD_P_Clusters.InsertValue(se->GetNPClusters());
    SVD_RawHits.InsertValue(se->GetNRawHits());
    SVD_Hits.InsertValue(se->GetNHits());
    SVD_Tracks.InsertValue(se->GetNTracks());
    SVD_Verts.InsertValue(se->GetNVertices());
    SVD_Pass.InsertValue((int)se->GetPassedCuts());
    LastVF48TimeStamp = se->GetVF48Timestamp();
    nSVDEvents++;
    return;
}

void TA2AnalysisReport::Print()
{
    double rough_time = PrintHeader();
    if(nSVDEvents>0)
    {
        std::cout <<"Number of SVD Events:\t"<<nSVDEvents<<std::endl;
        std::cout <<"               \tMode\tMean"<<std::endl;
        std::cout <<"SVD #RawNHits: \t"<<SVD_N_RawHits.GetMode()<<"\t"<<SVD_N_RawHits.GetMean()<<std::endl;
        std::cout <<"SVD #RawPHits: \t"<<SVD_P_RawHits.GetMode()<<"\t"<<SVD_P_RawHits.GetMean()<<std::endl;
        //std::cout <<"Mean SVD #RawHits: \t" <<SVD_RawHits.GetMode()  <<"\t"<<SVD_RawHits.GetMean()  <<std::endl;
        std::cout <<"SVD #Hits: \t"    <<SVD_Hits.GetMode()     <<"\t"<<SVD_Hits.GetMean()     <<std::endl;
        std::cout <<"SVD #Tracks:\t"   <<SVD_Tracks.GetMode()   <<"\t"<<SVD_Tracks.GetMean()   <<std::endl;
        std::cout<<"----------------Sum-----Mean---------"<<std::endl;
        //std::cout<<"SVD Events:\t"<< SVD_Verts
        std::cout <<"SVD #Events:\t"   <<SVD_Tracks.GetEntires()<<std::endl;
        std::cout <<"SVD #Verts:\t"    <<SVD_Verts.GetSum()     <<"\t"<<SVD_Verts.GetMean();
        if (rough_time>0)
        {
            double SVD_vertrate=SVD_Verts.GetRate(1,rough_time);
            if (SVD_vertrate<0.1)
                printf("\t~(%.1fmHz)",SVD_vertrate*1000.);
            else
                printf("\t~(%.1fHz)",SVD_vertrate);
        }
        std::cout<<std::endl;
        std::cout <<"SVD #Pass cuts:\t"<<SVD_Pass.GetSum()         <<"\t"<<SVD_Pass.GetMean();
        if (rough_time>0)
        {
            double SVD_passrate=SVD_Pass.GetRate(1,rough_time);
            if (SVD_passrate<0.1)
                printf("\t~(%.1fmHz)",SVD_passrate*1000.);
            else
                printf("\t~(%.1fHz)",SVD_passrate);
        }
        std::cout<<std::endl;
        std::cout <<"Time of Last Event: "<<LastVF48TimeStamp<<" s"<<std::endl;
    }
    PrintFooter();
}
#endif