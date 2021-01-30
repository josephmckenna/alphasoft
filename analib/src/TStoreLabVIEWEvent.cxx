#ifndef _TStoreLabVIEWEvent_
#include "TStoreLabVIEWEvent.h"
#endif

ClassImp(TStoreLabVIEWEvent)

TStoreLabVIEWEvent::TStoreLabVIEWEvent()
{
// ctor
    BankName = "NULL";
    MIDAS_TIME = 0;
    run_time = 0;
    labview_time = 0;
}

TStoreLabVIEWEvent::TStoreLabVIEWEvent(const TStoreLabVIEWEvent& Event):
   m_data(Event.GetData())
{
//copy constructor
    BankName = Event.GetBankName();
    MIDAS_TIME = Event.GetMIDAS_TIME();
    run_time = Event.GetRunTime();
    labview_time = Event.GetLabviewTime();
}

TStoreLabVIEWEvent::TStoreLabVIEWEvent(std::string p_BankName, std::vector<double> p_data, 
    uint32_t p_MIDAS_TIME, double p_run_time, double p_labview_time)
    : BankName(p_BankName), m_data(p_data), MIDAS_TIME(p_MIDAS_TIME), 
    run_time(p_run_time), labview_time(p_labview_time)
{
}

void TStoreLabVIEWEvent::Reset()
{
    BankName = "NULL";
    m_data.clear();
    MIDAS_TIME = 0;
    run_time = 0;
    labview_time = 0;
}

void TStoreLabVIEWEvent::Print()
{
  std::cout<<"BankName:\t"<<BankName<<std::endl;
  std::cout<<"m_data:\t"<<&m_data<<std::endl;
  std::cout<<"MIDAS_TIME:\t"<<MIDAS_TIME<<std::endl;
  std::cout<<"run_time:\t"<<run_time<<std::endl;
  std::cout<<"labview_time:\t"<<labview_time<<std::endl;
}

TStoreLabVIEWEvent::~TStoreLabVIEWEvent()
{

}

//



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
