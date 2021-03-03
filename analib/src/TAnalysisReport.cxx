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

TAnalysisReport::TAnalysisReport(const TAnalysisReport& r):
   runNumber(r.runNumber),
   GitBranch(r.GitBranch), 
   CompilationDate(r.CompilationDate), 
   GitDate(r.GitDate), 
   GitHash(r.GitHash),
   GitHashLong(r.GitHashLong),
   GitDiff(r.GitDiff),
   BoolValue(r.BoolValue),
   IntValue(r.IntValue),
   DoubleValue(r.DoubleValue),
   StringValue(r.StringValue)
{
    StartRunUnixTime = r.StartRunUnixTime;
    StopRunUnixTime = r.StopRunUnixTime;
    ProgramName = r.ProgramName;
    ProgramPath = r.ProgramPath;
    ProgramPathFull = r.ProgramPathFull;
    Duration = r.Duration;
    AnalysisHost = r.AnalysisHost;
}

TAnalysisReport TAnalysisReport::operator=(const TAnalysisReport& r)
{
    return TAnalysisReport(r);
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
#ifdef BUILD_A2

//We dont want these as members of the class...
static TH1D* SVD_N_RawHits;
static TH1D* SVD_P_RawHits;
static TH1D* SVD_RawHits;
static TH1D* SVD_Hits;
static TH1D* SVD_Tracks;
static TH1D* SVD_Verts;
static TH1D* SVD_Pass;

ClassImp(TA2AnalysisReport);
TA2AnalysisReport::TA2AnalysisReport()
{
    //ctor

}


TA2AnalysisReport::TA2AnalysisReport(const TA2AnalysisReport& r): TAnalysisReport(r)
{
    nSVDEvents = r.nSVDEvents;
    LastVF48TimeStamp = r.LastVF48TimeStamp;

    SVD_Verts_Sum      = r.SVD_Verts_Sum;
    SVD_PassCut_Sum    = r.SVD_PassCut_Sum;

}

TA2AnalysisReport TA2AnalysisReport::operator=(const TA2AnalysisReport& r)
{
    return TA2AnalysisReport(r);
}

TA2AnalysisReport::TA2AnalysisReport(int runno): TAnalysisReport(runno)
{
    nSVDEvents = 0;
    LastVF48TimeStamp = -1;
    SVD_Verts_Sum = 0;
    SVD_PassCut_Sum = 0;

    SVD_N_RawHits = new TH1D("SVD_N_RawHits","SVD_N_RawHits; Multiplicity; Count",1000,0,1000);
    SVD_P_RawHits = new TH1D("SVD_P_RawHits","SVD_P_RawHits; Multiplicity; Count",1000,0,1000);
    //MeanMode SVD_N_Clusters = new TH1D(1000};
    //MeanMode SVD_P_Clusters = new TH1D(1000};
    SVD_RawHits = new TH1D("SVD_RawHits","SVD_RawHits; Multiplicity; Count",1000,0,1000);
    SVD_Hits = new TH1D("SVD_Hits","SVD_Hits;Multiplicity; Count ",1000,0,1000);
    SVD_Tracks = new TH1D("SVD_Tracks","SVD_Tracks; Multiplicity; Count",100,0,100);
    SVD_Verts = new TH1D("SVD_Verts","SVD_Verts; Multiplicity; Count",2,0,2);
    SVD_Pass = new TH1D("SVD_Pass","SVD_Pass; Multiplicity; Count",2,0,2);
}

TA2AnalysisReport::~TA2AnalysisReport()
{
    //dtor
    delete SVD_N_RawHits;
    delete SVD_P_RawHits;
    delete SVD_RawHits;
    delete SVD_Hits;
    delete SVD_Tracks;
    delete SVD_Verts;
    delete SVD_Pass;
}

void TA2AnalysisReport::FillSVD(const Int_t& nraw, const Int_t&praw, const Int_t& raw_hits, const Int_t& hits, const Int_t& tracks, const Int_t& verts, int pass, double time)
{
    SVD_N_RawHits->Fill(nraw);
    SVD_P_RawHits->Fill(praw);
    //SVD_N_Clusters->Fill(se->GetNNClusters());
    //SVD_P_Clusters->Fill(se->GetNPClusters());
    SVD_RawHits->Fill(raw_hits);
    SVD_Hits->Fill(hits);
    SVD_Tracks->Fill(tracks);
    SVD_Verts->Fill(verts);
    SVD_Verts_Sum += verts;

    SVD_Pass->Fill(pass);
    SVD_PassCut_Sum += pass;

    LastVF48TimeStamp = time;
    nSVDEvents++;
    return;
}
void TA2AnalysisReport::Flush()
{
    IntValue["SVD_N_RawHits_Mode"]    = SVD_N_RawHits->GetMaximumBin() - 1;
    DoubleValue["SVD_N_RawHits_Mean"] = SVD_N_RawHits->GetMean();
    IntValue["SVD_P_RawHits_Mode"]    = SVD_P_RawHits->GetMaximumBin() - 1;
    DoubleValue["SVD_P_RawHits_Mean"] = SVD_P_RawHits->GetMean();
    IntValue["SVD_Hits_Mode"]         = SVD_Hits->GetMaximumBin() - 1;
    DoubleValue["SVD_Hits_Mean"]      = SVD_Hits->GetMean();
    IntValue["SVD_Tracks_Mode"]       = SVD_Tracks->GetMaximumBin() - 1;
    DoubleValue["SVD_Tracks_Mean"]    = SVD_Tracks->GetMean();
    DoubleValue["SVD_Verts_Mean"]     = SVD_Verts->GetMean();
    IntValue["SVD_Verts_Sum"]         = SVD_Verts_Sum;
    DoubleValue["SVD_Vert_Rate"]      = SVD_Verts_Sum / ( (double)(GetRunStopTime()-GetRunStartTime()));
    DoubleValue["SVD_PassCut_Mean"]   = SVD_Pass->GetMean();
    DoubleValue["SVD_PassCut_Sum"]    = SVD_PassCut_Sum;
    DoubleValue["SVD_PassCut_Rate"]   = SVD_PassCut_Sum/ ( (double)(GetRunStopTime()-GetRunStartTime()));
}

void TA2AnalysisReport::Print()
{


    PrintHeader();
    if(nSVDEvents>0)
    {
        std::cout <<"Number of SVD Events:\t"<<nSVDEvents<<std::endl;
        std::cout <<"               \tMode\tMean"<<std::endl;
        std::cout <<"SVD #RawNHits: \t"<<IntValue["SVD_N_RawHits_Mode"]<<"\t"<<DoubleValue["SVD_N_RawHits_Mean"]<<std::endl;
        std::cout <<"SVD #RawPHits: \t"<<IntValue["SVD_P_RawHits_Mode"]<<"\t"<<DoubleValue["SVD_P_RawHits_Mean"]<<std::endl;
        //std::cout <<"Mean SVD #RawHits: \t" <<SVD_RawHits->GetMode()  <<"\t"<<SVD_RawHits->GetMean()  <<std::endl;
        std::cout <<"SVD #Hits: \t"    <<IntValue["SVD_Hits_Mode"]     <<"\t"<<DoubleValue["SVD_Hits_Mean"]     <<std::endl;
        std::cout <<"SVD #Tracks:\t"   <<IntValue["SVD_Tracks_Mode"]   <<"\t"<<DoubleValue["SVD_Tracks_Mean"]   <<std::endl;
        std::cout<<"----------------Sum-----Mean---------"<<std::endl;
        //std::cout<<"SVD Events:\t"<< SVD_Verts
        std::cout <<"SVD #Events:\t"   <<nSVDEvents<<std::endl;
        std::cout <<"SVD #Verts:\t"    <<SVD_Verts_Sum     <<"\t"<<DoubleValue["SVD_Verts_Mean"];
        if (GetRunStopTime()-GetRunStartTime()>0)
        {
            if (DoubleValue["SVD_Vert_Rate"]<0.1)
                printf("\t~(%.1fmHz)",DoubleValue["SVD_Vert_Rate"]*1000.);
            else
                printf("\t~(%.1fHz)",DoubleValue["SVD_Vert_Rate"]);
        }
        std::cout<<std::endl;
        std::cout <<"SVD #Pass cuts:\t"<<SVD_PassCut_Sum         <<"\t"<<DoubleValue["SVD_PassCut_Mean"];
        if (GetRunStopTime()-GetRunStartTime()>0)
        {
            if (DoubleValue["SVD_PassCut_Rate"]<0.1)
                printf("\t~(%.1fmHz)",DoubleValue["SVD_PassCut_Rate"]*1000.);
            else
                printf("\t~(%.1fHz)",DoubleValue["SVD_PassCut_Rate"]);
        }
        std::cout<<std::endl;
        std::cout <<"Time of Last VF48 Event: "<<LastVF48TimeStamp<<" s"<<std::endl;
    }
    PrintFooter();
}






ClassImp(TAGAnalysisReport);
TAGAnalysisReport::TAGAnalysisReport()
{
    //ctor
    nStoreEvents = 0;
    nSigEvents = 0;
    nSigEvents = -1;
}

TAGAnalysisReport::TAGAnalysisReport(const TAGAnalysisReport& r): TAnalysisReport(r)
{
    nStoreEvents = r.nStoreEvents;
    nSigEvents = r.nSigEvents;
    last_tpc_ts = r.nSigEvents;
}

TAGAnalysisReport TAGAnalysisReport::operator=(const TAGAnalysisReport& r)
{
    return TAGAnalysisReport(r);
}

TAGAnalysisReport::TAGAnalysisReport(int runno): TAnalysisReport(runno)
{
    nStoreEvents = 0;
    nSigEvents = 0;
    last_tpc_ts = -1;
}

TAGAnalysisReport::~TAGAnalysisReport()
{
    //dtor
}

void TAGAnalysisReport::Flush(
   double sum_aw,       //Results from deconv module
   double sum_pad,      //Results from deconv module
   double sum_match,    //Results from match module
   double sum_tracks,   //Results from reco module
   double sum_r_sigma,  //Results from reco module
   double sum_z_sigma,  //Results from reco module
   double sum_verts,    //Results from reco module
   double sum_hits,     //Results from reco module
   double sum_bars)
{
    if (nSigEvents>0)
    {
       DoubleValue["TPC_Mean_AW"] = sum_aw/(double)nSigEvents;
       DoubleValue["TPC_Mean_Pad"] = sum_pad/(double)nSigEvents;
       DoubleValue["TPC_Mean_Match"] = sum_match/(double)nSigEvents;
    }
    if (nStoreEvents>0)
    {
       DoubleValue["TPC_Mean_Hits"] = sum_hits/(double)nStoreEvents;
       DoubleValue["TPC_Mean_Tracks"] = sum_tracks /(double)nStoreEvents;
       DoubleValue["TPC_Mean_R_Sigma"] = sum_r_sigma/(double)nStoreEvents;
       DoubleValue["TPC_Mean_Z_Sigma"] = sum_z_sigma/(double)nStoreEvents;
       DoubleValue["TPC_Mean_Verts"] = sum_verts/(double)nStoreEvents;
       DoubleValue["Barrel_Mean_Bars"] = sum_bars/(double)nStoreEvents;
    }
    return;
}

void TAGAnalysisReport::Print()
{
    PrintHeader();
    if(nStoreEvents>0)
    {
       std::cout << "Mean #AW:   \t:"   << DoubleValue["TPC_Mean_AW"]    << std::endl;
       std::cout << "Mean #PAD:   \t:"  << DoubleValue["TPC_Mean_Pad"]   << std::endl;
       std::cout << "Mean #MATCH:   \t:"<< DoubleValue["TPC_Mean_Match"] << std::endl;
       std::cout << "Mean #Hits: \t"    << DoubleValue["TPC_Mean_Hits"] << std::endl;
       std::cout << "Mean #Tracks:\t"   << DoubleValue["TPC_Mean_Tracks"] << 
                        "\t(Mean ChiR:" << DoubleValue["TPC_Mean_Z_Sigma"] << 
                        " ChiZ:" << DoubleValue["TPC_Mean_R_Sigma"] << ")" << std::endl;
       std::cout << "Mean #Verts:\t"    << DoubleValue["TPC_Mean_Verts"] << std::endl;
       std::cout << "Mean #Bars:\t"     << DoubleValue["Barrel_Mean_Bars"] << std::endl;
       std::cout<<std::endl;
       
        std::cout << "Time of Last TPC Event: " << last_tpc_ts << " s" << std::endl;
    }
    PrintFooter();
}

#endif