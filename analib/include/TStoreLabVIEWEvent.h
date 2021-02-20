#ifndef _TStoreLabVIEWEvent_
#define _TStoreLabVIEWEvent_

#include <iostream>
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TStoreLabVIEWEvent: public TObject
{
  private:
    std::string BankName;
    std::vector<double> m_data;
    uint32_t MIDAS_TIME;
    double run_time;
    double labview_time;
    int RunNumber;

  public:
    TStoreLabVIEWEvent(const TStoreLabVIEWEvent& Event);
    TStoreLabVIEWEvent();
    TStoreLabVIEWEvent(std::string p_BankName, 
       std::vector<double> p_data, uint32_t p_MIDAS_TIME, double p_run_time, double p_labview_time, int runNumber);

    using TObject::Print;
    virtual void Print();
    virtual ~TStoreLabVIEWEvent();

    std::string               GetBankName() const          { return BankName; }
    std::vector<double>       GetData() const              { return m_data; }
    double                    GetArrayEntry(int i)         { return m_data.at(i); }
    uint32_t                  GetMIDAS_TIME() const        { return MIDAS_TIME; }
    double                    GetRunTime() const           { return run_time; }
    double                    GetLabviewTime() const       { return labview_time; }
    int                       GetRunNumber() const         { return RunNumber; }  
    
    void Reset();
  
    ClassDef(TStoreLabVIEWEvent, 1);
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
