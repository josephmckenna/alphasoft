#ifndef __TSISChannels__
#define __TSISChannels__
#include "ALPHA2SettingsDatabase.h"
// TSISChannels Class =====================================================================================
// Class containing the SIS channel mapping information.  "SIS" refers to the model number of scaler
// modules; Struck Innovative Systeme SIS3820.  
//
// See Richard Hydomako's thesis "Detection of Trapped Antihydrogen", page 70 for more info:
// https://goo.gl/CxiBiP
//
// JWS 25/06/2010
//
// ==========================================================================================================


#include "TNamed.h"
#include "TVector3.h"
#include "TSettings.h"

#include "TSISChannel.h"

class TSISChannels : public TNamed 
{
private:
  TSettings* gSettingsDB;
  Int_t _run_number;
  TSISChannel _io32;
  TSISChannel _io32_nobusy;
  TSISChannel _20MHz_VF48clk;
  TSISChannel _10MHz_clk[NUM_SIS_MODULES];

public:
  TSISChannels();
  TSISChannels( Int_t run_number );
  virtual ~TSISChannels();
  
  /*
   * Look up the channel number in the SQLite DB (0-63)
   *
   * To differentiate between the two SIS modules, pass in chmin and chmax appropriately; e.g. 32 and 63 for the second module.
   */
  TSISChannel GetChannelInRange( const char* channel_description, Int_t run_number , Int_t chmin, Int_t chmax);

  /*
   * Look up the named channel number for the given run in the SQLite DB.
   *
   * If two channel descriptions match, only the most recent entry is returned.
   */
  TSISChannel GetChannel( const char* channel_description, Int_t run_number );

  TSISChannel GetChannel( const char* channel_description );

  TString GetDescription( Int_t channel, Int_t run_number );
  TString GetDescription( TSISChannel channel, Int_t run_number);
  void PrintSISMap( Int_t run_number );
  TSISChannel Get_20MHz_VF48clk() { return _20MHz_VF48clk;} 
  TSISChannel Get10MHz_clk( Int_t j) {return _10MHz_clk[j];}
  TSISChannel GetIO32() {return _io32;}
  TSISChannel GetIO32_NOBUSY() {return _io32_nobusy;}

  ClassDef(TSISChannels,1)
};

#endif
