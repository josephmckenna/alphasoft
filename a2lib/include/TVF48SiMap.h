#ifndef _TVF48SiMap_
#define _TVF48SiMap_

// TVF48SiMap Class =====================================================================================
//
// Class for navigating the silicon cabling map information. 
//
// JWS 25/06/2010, adapted from Art's VF48SiMap class.
//
// ==========================================================================================================
#include <stdlib.h>
#include "SiMod.h"
#include <string>
#include "TNamed.h"

namespace std {using std::string;}

class TVF48SiMap : public TNamed
{
private:
  int maxmodule_;

  //  double fadc_[int Module][int Channel][int Sample][int VA];

  int module_[nSil][4];
  int channel_[nSil][4];
  int frcnumber_[nSil][4];
  int frcport_[nSil][4];
  int ttcchannel_[nSil][4];
  int sinumber_[nVF48][48];
  std::string siname_[nVF48][48];
  int va_[nVF48][48];
  int frcn_[nVF48][48];
  int frcp_[nVF48][48];
  int ttcc_[nVF48][48];
  std::string Siname_[nSil];
  int sinum_[TTC_TA_inputs];

public:
  
  TVF48SiMap();
  TVF48SiMap( const std::string & ); //Constructor from map file 
  ~TVF48SiMap() {}  // virtual only needed if you derive from this class
   
  int GetVF48( const int SiModNumber, const int ASIC, int & vf48modnum, int & vf48chan, int & TTCChannel );
  int GetSil ( const int vf48modnum, const int vf48chan, int & SiModNumber, int & ASIC, int & FRCNumber, int & FRCPort, int & TTCChannel );
  std::string GetSilName( const int, const int);
  std::string GetSilName( const int SiModNumber );
  int Get_TA_Number( const int SiNumber, const int VA );
  int GetSil( const int TAnumber );
  int Get_TA_FPGA_Number( const int SiNumber );
  int Get_TA_Bank_Number( const int SiNumber );
  int Get_FRC_Position( const int SiNumber );
  int Get_ODB_Layer_Index( const int SiNumber );

  ClassDef(TVF48SiMap,1)
};
#endif // _TVF48SiMap_
