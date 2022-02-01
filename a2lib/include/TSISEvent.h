#ifndef _TSISEvent_
#define _TSISEvent_
/*
 *  TSISEvent.h for manalyzer
 *  JTK McKENNA
 *
 */ 

#include "TObject.h"
#include <vector>
#include <iostream>
#include "assert.h"
#include <array>

#include "TSISChannel.h"


// Very basic conainer for holding the SISEvent data
class TSISBufferEvent
{
   public:
      int fSISModule;
      std::array<uint32_t,NUM_SIS_CHANNELS> fCounts = {0};
      TSISBufferEvent(const int mod, const uint32_t* data): fSISModule(mod)
      {
         std::copy(data, data + NUM_SIS_CHANNELS,fCounts.begin());
      }
      void Print() const
      {
         for (const uint32_t c: fCounts)
            std::cout << c <<"\t";
         std::cout << std::endl;
      }
};

// Define object of type SISTreeEvent , inherits from TObject
class TSISEvent : public TObject
{
private:
  
  int fSISModule;
  std::array<uint32_t,NUM_SIS_CHANNELS> fCounts;
     //counts in this channel
  ULong64_t     fClock;               //10 MHz clks
  ULong64_t     fVF48Clock;           //20 MHz clock from VF48?
  Double_t      fRunTime;             //SIS time since the start of the MIDAS run 
  Int_t         fRunNumber;           //MIDAS runnumber
  uint32_t      fMidasTime;           //MIDAS Unix Timestamp
  unsigned long fMidasEventID;
  //Maybe I could have a vector of strings to label some SIS triggers
  //std::vector<DumpMarker*> DumpEvents; //I do not own the DumpMarker pointer...

public:

  // setters  
  void Fill(TSISBufferEvent* event);

  void SetSISModuleNo(int module)                   { fSISModule = module;  }
  void SetScalerModuleNo(int module)                { fSISModule = module;  }
  void SetClock(ULong64_t clock)                    { fClock = clock; }
  void SetVF48Clock(ULong64_t clock)                { fVF48Clock = clock; }
  void SetRunTime(Double_t time)                    { fRunTime = time; }

  void SetRunNumber(Int_t runnumber)                { fRunNumber = runnumber; }
  void SetMidasUnixTime(uint32_t midtime)           { fMidasTime = midtime; }
  void SetMidasEventID(unsigned long id)            { fMidasEventID = id; }

  void ClearSISEvent();
  void Reset()
  {
     ClearSISEvent();
  }
  
  // getters
  uint32_t GetCountsInChannel( int i) const
  {
     if ( fSISModule==1)
     {
        i=i-32;
     }
     if (i<0)
     {
        return 0;
     } 
     else if (i>=NUM_SIS_CHANNELS)
     {
        return 0;
     }
     else 
     {
        return fCounts[i];
     }
  }
  uint32_t GetCountsInChannel( const TSISChannel& c) const
  {
     if (!c.IsValid())
        return 0;
     if ( c.fModule == fSISModule )
        return fCounts[c.fChannel];
     else
        return 0;
  }
  int    GetSISModule() const          { return fSISModule; }
  int    GetScalerModule() const          { return fSISModule; }
  ULong64_t GetClock() const				{ return fClock; }
  ULong64_t GetVF48Clock() const         { return fVF48Clock; }
  Double_t  GetRunTime() const		    { return fRunTime; }

  Int_t     GetRunNumber() const     	    { return fRunNumber; }
  Double_t  GetMidasUnixTime() const		    { return fMidasTime; }
  unsigned long GetMidasEventID() const            { return fMidasEventID; }

  using TObject::Print;
  virtual void Print();
  
  // default class member functions
  TSISEvent( );
  TSISEvent( int SISModule );
  TSISEvent( const TSISBufferEvent& event );
  TSISEvent( ULong64_t clock, Double_t time);
  TSISEvent& operator+=(const TSISEvent& b);

  virtual ~TSISEvent(); 
  
  ClassDef(TSISEvent,1); 
};
#endif
