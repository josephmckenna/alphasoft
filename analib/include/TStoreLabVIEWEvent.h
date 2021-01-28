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
    uint32_t run_time;
    double labview_time;
    int RunNumber;
    TString fDescription;
  
  public:
    TStoreLabVIEWEvent(TStoreLabVIEWEvent* Event);
    TStoreLabVIEWEvent();
    TStoreLabVIEWEvent(std::string p_BankName, 
       std::vector<double> p_data, uint32_t p_MIDAS_TIME, uint32_t p_run_time, double p_labview_time, int runNumber);

    using TObject::Print;
    virtual void Print();
    virtual ~TStoreLabVIEWEvent();
    TString Clean(TString a) { 
      TString b(a);
      b.ReplaceAll("\r","\n");//Fix windows' stupid miss use of return carriadge 
      return b;
    }
   virtual std::string            GetBankName()                           { return BankName; }
   virtual std::vector<double>*   GetData()                               { return &m_data; }
   virtual double                 GetArrayEntry(int i)                    { return m_data.at(i); }
   virtual uint32_t               GetMIDAS_TIME()                         { return MIDAS_TIME; }
   virtual uint32_t               GetRunTime()                            { return run_time; }
   virtual double                 GetLabviewTime()                        { return labview_time; }
   virtual int                    GetRunNumber()                          { return RunNumber; }  
    
    void SetDescription( TString Description )
    {
      //Clean up leading and ending " marks
      if (Description.BeginsWith("\"")) Description.Remove(0,1);
      if (Description.EndsWith("\"")) Description.Remove(Description.Sizeof()-2,1);
      fDescription = Description;
    }


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
