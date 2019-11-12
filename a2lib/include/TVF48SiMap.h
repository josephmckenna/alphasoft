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

class TVF48SiMap : public TNamed
{
private:
  int maxmodule_;

  //  double fadc_[int Module][int Channel][int Sample][int VA];
  static std::string current_map;
  static int module_[nSil][4];
  static int channel_[nSil][4];
  static int frcnumber_[nSil][4];
  static int frcport_[nSil][4];
  static int ttcchannel_[nSil][4];
  static int sinumber_[nVF48][48];
  static std::string siname_[nVF48][48];
  static int va_[nVF48][48];
  static int frcn_[nVF48][48];
  static int frcp_[nVF48][48];
  static int ttcc_[nVF48][48];
  static std::string Siname_[nSil];
  static int sinum_[TTC_TA_inputs];

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
