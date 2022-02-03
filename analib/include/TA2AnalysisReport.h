#include "TAnalysisReport.h"

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


#if BUILD_A2
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
    TH1I* fHybridNSideOccupancy;
    TH1I* fHybridPSideOccupancy;

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
    void FillHybridNSideOccupancy(const int module);
    void FillHybridPSideOccupancy(const int module);
    void FillOccupancy(const int module, const int nHits);
    void Flush();
    
    //Zeroth bin is the underflow
    int GetHybridNSideOccupancy(const int module) const { return fHybridNSideOccupancy->GetBinContent(module + 1); }
    int GetHybridPSideOccupancy(const int module) const { return fHybridPSideOccupancy->GetBinContent(module + 1); }

    using TObject::Print;
    virtual void Print();
    ClassDef(TA2AnalysisReport,1);
};
#endif