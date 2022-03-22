#include "TAGAnalysisReport.h"


#ifdef BUILD_AG

ClassImp(TAGAnalysisReport)
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
       std::cout << "Mean #AW:   \t"   << DoubleValue["TPC_Mean_AW"]    << std::endl;
       std::cout << "Mean #PAD:  \t"  << DoubleValue["TPC_Mean_Pad"]   << std::endl;
       std::cout << "Mean #MATCH:\t"<< DoubleValue["TPC_Mean_Match"] << std::endl;
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
