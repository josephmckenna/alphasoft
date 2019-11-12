#include "TVF48SiMap.h"

// TVF48SiMap Class =====================================================================================
//
// Class for navigating the silicon cabling map information. 
//
// ==========================================================================================================

ClassImp(TVF48SiMap);

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

using std::ifstream;
using std::cout;
using std::string;
using std::getline;
using std::stringstream;

#include "TVF48SiMap.h"
#define VERBOSE 0

//Declare all member as static
std::string TVF48SiMap::current_map="";
int TVF48SiMap::module_[nSil][4];
int TVF48SiMap::channel_[nSil][4];
int TVF48SiMap::frcnumber_[nSil][4];
int TVF48SiMap::frcport_[nSil][4];
int TVF48SiMap::ttcchannel_[nSil][4];
int TVF48SiMap::sinumber_[nVF48][48];
std::string TVF48SiMap::siname_[nVF48][48];
int TVF48SiMap::va_[nVF48][48];
int TVF48SiMap::frcn_[nVF48][48];
int TVF48SiMap::frcp_[nVF48][48];
int TVF48SiMap::ttcc_[nVF48][48];
std::string TVF48SiMap::Siname_[nSil];
int TVF48SiMap::sinum_[TTC_TA_inputs];


TVF48SiMap::TVF48SiMap( ) {
}

TVF48SiMap::TVF48SiMap( const string& fname ) {
  //Check if this map was already loaded
  if (strcmp(fname.c_str(),current_map.c_str())==0)
  {
    return;
  }
  current_map=fname;
  ifstream mapfile( fname.c_str() );
  if( !mapfile) {
    cout << "Map file " << fname << " not found\n";
    exit(EXIT_FAILURE);  // failure
  }
  if(VERBOSE){
  cout << "Using VF48 Map file: " << fname << std::endl; 
 }
  //
  // initialize arrays
  for( int i=0; i<nSil; ++i )
  {
    for( int j=0; j<4; ++j )
    {
      module_[i][j] = -1;
      channel_[i][j] = -1;
      frcnumber_[i][j] = -1;
      frcport_[i][j] = -1;
      ttcchannel_[i][j] = -1;
    }
  }
  
  for (int i=0;i<nVF48;++i) {
  for (int j=0;j<48; j++) {
      sinumber_[i][j]  = -1;
      va_[i][j]  =-1;
      frcn_[i][j]  =-1;
      frcp_[i][j]  =-1;
      ttcc_[i][j]  =-1;
    }
   }
  for (int i=0;i<TTC_TA_inputs; i++) {
    sinum_[i]=-1;
  }

  //
  string buffer;
  while( !getline(mapfile,buffer).eof() )  {
   // cout<< buffer << std::endl;
    if( buffer[0] == '#' ) {
    //cout << buffer << "\n";
    }
    else {
      int Module, Plug, SiNumber, FRCNumber, FRCPort, TTCChannel;
      string SiName;
      stringstream ss;
      ss << buffer;
      ss >> Module >> Plug >> FRCNumber >> FRCPort >> SiName >> SiNumber >> TTCChannel;
      //      cout <<"\t"<< Module <<"\t"<< Plug <<"\t"<< FRCNumber <<"\t"<< FRCPort <<"\t"<< SiName <<"\t"<< SiNumber <<"\t"<< TTCChannel << std::endl;
      
      for( int VA=0; VA<4; ++VA ) {
          module_[SiNumber][VA] = Module;
        int channel =  VA + 4*FRCPort + 16*Plug;
        channel_[SiNumber][VA] = channel;
        frcnumber_[SiNumber][VA] = FRCNumber;
        frcport_[SiNumber][VA] = FRCPort;
        ttcchannel_[SiNumber][VA] = TTCChannel+VA;
        sinumber_[Module][channel]=SiNumber;
        siname_[Module][channel] = SiName;
        va_[Module][channel]=VA+1;
        frcn_[Module][channel] =FRCNumber;
        frcp_[Module][channel] =FRCPort;
        ttcc_[Module][channel] = TTCChannel+VA;
      }
      if (maxmodule_ < Module) maxmodule_ = Module;
      Siname_[SiNumber] = SiName;
	
      sinum_[TTCChannel]=SiNumber;
      sinum_[TTCChannel+1]=SiNumber;
      sinum_[TTCChannel+2]=SiNumber;
      sinum_[TTCChannel+3]=SiNumber;
      
    }
  }
}

int TVF48SiMap::GetVF48( const int SiNumber , int va,
                        int &Module, int &Channel, int &TTCChannel )
{
  
  if (SiNumber > nSil) exit(EXIT_FAILURE); 
  Module = module_[SiNumber][va-1];
  Channel = channel_[SiNumber][va-1];
  TTCChannel = ttcchannel_[SiNumber][va-1];
  return (EXIT_SUCCESS);
}

int TVF48SiMap::GetSil ( int Module, const int Channel, int &SiNumber, int &VA,int &FRCNumber, int &FRCPort, int &TTCChannel )
{
  SiNumber = sinumber_[Module][Channel];
  VA=va_[Module][Channel];
  FRCNumber = frcn_[Module][Channel];
  FRCPort = frcp_[Module][Channel];
  TTCChannel = ttcc_[Module][Channel];
 return(EXIT_SUCCESS);
}

string TVF48SiMap::GetSilName( int Module, int Channel)
{
  return siname_[Module][Channel];
}

string TVF48SiMap::GetSilName( int SiNumber )
{
  return Siname_[SiNumber];
}

int TVF48SiMap::Get_TA_Number( const int SiNumber, const int VA )
{
  return ttcchannel_[SiNumber][VA];
}

int TVF48SiMap::GetSil( const int TAnumber )
{
  return sinum_[TAnumber];
}

int TVF48SiMap::Get_TA_FPGA_Number( const int SiNumber )
{
  // TA FPGA number {0,1}

  int TAnumber = Get_TA_Number( SiNumber, 0 );
  int TA_FPGA_number(-1);
  if( TAnumber >= 128 ) TA_FPGA_number = 1;
  else TA_FPGA_number = 0;

  return TA_FPGA_number;
}

int TVF48SiMap::Get_TA_Bank_Number( const int SiNumber )
{
  // TA bank number {0,1,2,3,4,5,6,7}

  int TAnumber = Get_TA_Number( SiNumber, 0 );

  int TA_FPGA_number = Get_TA_FPGA_Number( SiNumber );

  int TA_bank_index = TAnumber - ( 128 * TA_FPGA_number );

  int TA_bank_number(-1);
  TA_bank_number =  ( TA_bank_index - (TA_bank_index%16) ) / 16;
  
  return TA_bank_number;
}

int TVF48SiMap::Get_FRC_Position( const int SiNumber )
{
  // Position of SiMod in FRC {0,1,2,3}

  int TA_number = Get_TA_Number( SiNumber, 0 );
  int TA_FPGA_number = Get_TA_FPGA_Number( SiNumber );
  int TA_bank_number = Get_TA_Bank_Number( SiNumber );

  int x = TA_number - (128 * TA_FPGA_number) - (16 * TA_bank_number);
  int FRC_Position(-1);
  FRC_Position = (x - (x%4)) / 4;

  return FRC_Position;  
}

int TVF48SiMap::Get_ODB_Layer_Index( const int SiNumber )
{

  int TA_bank_number = Get_TA_Bank_Number( SiNumber );
  int FRC_position =  Get_FRC_Position( SiNumber );

  int ODB_Layer_Index(-1);
  ODB_Layer_Index = TA_bank_number * 4 + FRC_position;

  return ODB_Layer_Index;
}

