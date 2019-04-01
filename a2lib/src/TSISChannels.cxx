#include "TSISChannels.h"
#include <iostream>
#include <stdlib.h>

// TSISChannels Class =====================================================================================
//
// Class containing the SIS channel mapping information. 
//
// JWS 25/06/2010
//
// ==========================================================================================================

ClassImp(TSISChannels);

char * RELEASE = getenv ("RELEASE");

TSISChannels::TSISChannels()
{
   char dbName[255]; 
   sprintf(dbName,"%s/aux/main.db",RELEASE);
   gSettingsDB = new TSettings(dbName); 
}

TSISChannels::TSISChannels( Int_t run_number )
{
   char dbName[255]; 
   sprintf(dbName,"%s/aux/main.db",RELEASE);
   gSettingsDB = new TSettings(dbName); 
   _run_number = run_number;
   _io32=GetChannel("IO32_TRIG",run_number);
   _io32_nobusy=GetChannel("IO32_TRIG_NOBUSY",run_number);
   _20MHz_VF48clk=GetChannel("SIS_VF48_CLOCK",run_number);
   _iadc = _io32 % NUM_SIS_CHANNELS;
   _bnkadc = _io32 / NUM_SIS_CHANNELS;
  for (int j=0; j<NUM_SIS_MODULES; j++){
    _10MHz_clk[j] = GetChannelInRange("SIS_10Mhz_CLK",run_number,j*NUM_SIS_CHANNELS,(j+1)*NUM_SIS_CHANNELS)%NUM_SIS_CHANNELS;
    _seq_recatch_start[j] = GetChannelInRange("SIS_RECATCH_SEQ_START",run_number,j*NUM_SIS_CHANNELS,(j+1)*NUM_SIS_CHANNELS)%NUM_SIS_CHANNELS;
    //printf("IO32 TRIG (%d,%d) SIS_RECATCH_SEQ_START  (%d,%d)\n", _bnkadc, _iadc,j,_seq_recatch_start[j]);  
    _seq_atom_start[j] = GetChannelInRange("SIS_ATOM_SEQ_START",run_number,j*NUM_SIS_CHANNELS,(j+1)*NUM_SIS_CHANNELS)%NUM_SIS_CHANNELS;
    //   printf("SIS_ATOM_SEQ_START  (%d,%d)\n",j, _seq_atom_start[j]);  
    _seq_quench_start[j] = GetChannelInRange("QUENCH_FLAG",run_number,j*NUM_SIS_CHANNELS,(j+1)*NUM_SIS_CHANNELS)%NUM_SIS_CHANNELS;
    //   printf("QUENCH_FLAG  (%d,%d)\n", j,_seq_quench_start[j]);  
  }
}

TSISChannels::~TSISChannels()
{
  if(gSettingsDB) delete gSettingsDB;
}



Int_t TSISChannels::GetChannelInRange( const char* channel_description, Int_t run_number , Int_t chmin, Int_t chmax){

  Char_t sql[200];
  TString  result;
  sprintf(sql,"select channel from sis where run<=%d and description=\"%s\" and channel>=%d and channel<%d order by run desc limit 1;",run_number, channel_description, chmin, chmax);

  result = gSettingsDB->ExeSQL_singlereturn(sql);
  
  if( result.IsNull() ) 
  {
    //	  printf("Couldn't match channel_description %s to a channel\n", channel_description);
	  return -1;
  }
  
  return result.Atoi();
}

Int_t TSISChannels::GetChannel( const char* channel_description, Int_t run_number )
{
  Char_t sql[200];
  TString  result;
  sprintf(sql,"select channel from sis where run<=%d and description=\"%s\" order by run desc limit 1;",run_number, channel_description);
  result = gSettingsDB->ExeSQL_singlereturn(sql);
  
  if( result.IsNull() ) 
  {
    //	  printf("Couldn't match channel_description %s to a channel\n", channel_description);
	  return -1;
  }
  
  return result.Atoi();
}

Int_t TSISChannels::GetChannel( const char* channel_description )
{
  return GetChannel( channel_description, _run_number );
}

TString TSISChannels::GetDescription( Int_t channel, Int_t run_number )
{
  Char_t sql[200];
  TString  result;
  sprintf(sql,"select description from sis where run<=%d and channel=%d order by run desc limit 1;", run_number, channel);
  result = gSettingsDB->ExeSQL_singlereturn(sql);

  if( result.IsNull() ) result = "EMPTY";

  return result;
}

void TSISChannels::PrintSISMap( Int_t run_number )
{
  TString description;
  Int_t channels[64];
  TString descriptions[64];
  Int_t count_occupied_channels(0);

  for( Int_t i=0; i<64; i++ )
    {
      description = GetDescription( i, run_number );
      if( description == "EMPTY" ) continue;
      channels[count_occupied_channels]=i;
      descriptions[count_occupied_channels]=description;
      count_occupied_channels++;
    }
  printf("note: ignore \"err: (null)\" .\n\n");

  printf("\nSIS Channel Mapping for Run Number %d ****************************************\n", run_number );
  printf("*********************************************************************************\n\n");

  printf("Channel # \t Description \n\n");

  for( Int_t i=0; i<count_occupied_channels; i++ )
    {
      printf( "%d \t \t %s \n", channels[i], descriptions[i].Data() );
    }
}
