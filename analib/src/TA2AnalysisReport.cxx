
#include "TA2AnalysisReport.h"

#ifdef BUILD_A2

//We dont want these as members of the class...
static TH1D* SVD_N_RawHits;
static TH1D* SVD_P_RawHits;
static TH1D* SVD_RawHits;
static TH1D* SVD_Hits;
static TH1D* SVD_Tracks;
static TH1D* SVD_Verts;
static TH1D* SVD_Pass;

ClassImp(TA2AnalysisReport)
TA2AnalysisReport::TA2AnalysisReport()
{
    //ctor
   fHybridNSideOccupancy = new TH1I("HybridNSideOccupancy","HybridNSideOccupancy; HybridNumber; Count",72,0,72);
   fHybridPSideOccupancy = new TH1I("HybridPSideOccupancy","HybridPSideOccupancy; HybridNumber; Count",72,0,72);
}


TA2AnalysisReport::TA2AnalysisReport(const TA2AnalysisReport& r):
   TAnalysisReport(r)
{
   nSVDEvents = r.nSVDEvents;
   LastVF48TimeStamp = r.LastVF48TimeStamp;

   fHybridNSideOccupancy = new TH1I(*r.fHybridNSideOccupancy);
   fHybridPSideOccupancy = new TH1I(*r.fHybridPSideOccupancy);

    SVD_Verts_Sum      = r.SVD_Verts_Sum;
    SVD_PassCut_Sum    = r.SVD_PassCut_Sum;

}

TA2AnalysisReport TA2AnalysisReport::operator=(const TA2AnalysisReport& r)
{
    return TA2AnalysisReport(r);
}

TA2AnalysisReport::TA2AnalysisReport(int runno):
   TAnalysisReport(runno)
{

   fHybridNSideOccupancy = new TH1I("HybridNSideOccupancy","HybridNSideOccupancy; HybridNumber; Count",72,0,72);
   fHybridPSideOccupancy = new TH1I("HybridPSideOccupancy","HybridPSideOccupancy; HybridNumber; Count",72,0,72);

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
    delete fHybridNSideOccupancy;
    delete fHybridPSideOccupancy;
    
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

void TA2AnalysisReport::FillHybridNSideOccupancy(const int module)
{
   fHybridNSideOccupancy->Fill(module);
}

void TA2AnalysisReport::FillHybridPSideOccupancy(const int module)
{
   fHybridPSideOccupancy->Fill(module);
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
#endif

