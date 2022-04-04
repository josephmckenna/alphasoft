
#include "TAnalysisReport.h"

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