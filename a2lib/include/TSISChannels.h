#ifndef __TSISChannels__
#define __TSISChannels__

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
#define NUM_SIS_CHANNELS 32 
#define NUM_SIS_MODULES 2


#include "TNamed.h"
#include "TVector3.h"
#include "TSettings.h"

class TSISChannels : public TNamed 
{
private:
  TSettings* gSettingsDB;
  Int_t _run_number;
  Int_t _iadc;
  Int_t _bnkadc;
  Int_t _io32;
  Int_t _io32_nobusy;
  Int_t _20MHz_VF48clk;
  Int_t _10MHz_clk[NUM_SIS_MODULES];
  Int_t _seq_recatch_start[NUM_SIS_MODULES];
  Int_t _seq_atom_start[NUM_SIS_MODULES];
  Int_t _seq_quench_start[NUM_SIS_MODULES];

public:
  TSISChannels();
  TSISChannels( Int_t run_number );
  virtual ~TSISChannels();
  
  /*
   * Look up the channel number in the SQLite DB (0-63)
   *
   * To differentiate between the two SIS modules, pass in chmin and chmax appropriately; e.g. 32 and 63 for the second module.
   */
  Int_t GetChannelInRange( const char* channel_description, Int_t run_number , Int_t chmin, Int_t chmax);

  /*
   * Look up the named channel number for the given run in the SQLite DB.
   *
   * If two channel descriptions match, only the most recent entry is returned.
   */
  Int_t GetChannel( const char* channel_description, Int_t run_number );

  Int_t GetChannel( const char* channel_description );
  TString GetDescription( Int_t channel, Int_t run_number );
  void PrintSISMap( Int_t run_number );
  Int_t GetIADC() {return _iadc;}
  Int_t GetBNKADC() {return _bnkadc;}
  Int_t Get_20MHz_VF48clk() { return _20MHz_VF48clk;} 
  Int_t Get10MHz_clk( Int_t j) {return _10MHz_clk[j];}
  Int_t GetSeqRecatchStart( Int_t j) {return _seq_recatch_start[j];}
  Int_t GetSeqAtomStart( Int_t j) {return _seq_atom_start[j];}
  Int_t GetSeqQuenchStart( Int_t j) {return _seq_quench_start[j];}
  Int_t GetIO32() {return _io32;}
  Int_t GetIO32_NOBUSY() {return _io32_nobusy;}

  ClassDef(TSISChannels,1)
};

#endif
