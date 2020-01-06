#include "../include/TSettings.h"
#include <stdio.h>
#include <stdlib.h>

#include "RVersion.h"


// TSettings Class =========================================================================================
//
// Class providing access functions for the sqlite database. 
//
// RAH/JWS 10/06/2010
//
// =========================================================================================================

ClassImp(TSettings)

sqlite3 *   TSettings::fdb=NULL;
std::string TSettings::currentdbname="";
int         TSettings::current_run=-1;
Double_t    TSettings::fvf48freq[nVF48]={-1};
Int_t       TSettings::fvf48samples[nVF48]={-1};
Int_t       TSettings::fsoffset[nVF48]={-1};
Double_t    TSettings::fsubsample[nVF48]={-1};
Int_t       TSettings::foffset[nVF48]={-1};

TSettings::TSettings()
{
  //ctor
}

TSettings::TSettings( char * dbname )
{
  //default ctor
  //If database name already the same... the database is already open
  if (strcmp(dbname,currentdbname.c_str())==0)
     return;
  int rc = 0;
  rc = sqlite3_open(dbname,&fdb);
  if( rc )
    {
      fprintf(stderr, "Can't open database: %s\n",dbname);
      sqlite3_close(fdb);
      exit(1);
    }
}

TSettings::TSettings( char * dbname, Int_t run )
{
  //default ctor
  if (currentdbname.size()==0)
  {
     currentdbname=dbname;
  }
  else if (strcmp(dbname,currentdbname.c_str())!=0)
  {
    printf("TSettings uses static members, you cannot load differnt databases into memory without corrupting existing objects\n");
    exit(123);
  }
  else
  {
     return;
  }
  if (current_run<0)
  {
     current_run=run;
  }
  else if (run!=current_run)
  {
    printf("TSettings uses static members, you cannot load differnt runs into memory without corrupting existing objects\n");
    exit(123);
  }
  else
  {
     return;
  }
  int rc = 0;
  rc = sqlite3_open(dbname,&fdb);
  if( rc )
    {
      fprintf(stderr, "Can't open database: %s\n",dbname);
      sqlite3_close(fdb);
      exit(1);
    }

  for( Int_t i = 0; i<nVF48; i++ )
    {
      fvf48freq[i] = GetVF48Frequency( run, i );
      fvf48samples[i] = GetVF48Samples( run, i );
      fsoffset[i] = GetVF48soffset( run, i );
      fsubsample[i] = GetVF48subsample( run, i );
      foffset[i] = GetVF48offset( run, i ); 
    }
}


TSettings::~TSettings()
{
  //dtor
  if( fdb )
    {
      sqlite3_close(fdb);
      fdb=NULL;
      currentdbname="";
      current_run=-1;
    }
}


TString TSettings::ExeSQL_singlereturn( char * sql )
{
  char ** result;
  Int_t nrow = 0;
  Int_t ncol = 0;
  char * err;

  Int_t rc = 0;
  
  //printf("sql: %s\n",sql);
  rc = sqlite3_get_table(fdb,sql,
                         &result,
                         &nrow,
                         &ncol,
                         &err);
  //printf("nrow %d ncol %d\n",nrow,ncol);
  if( rc == SQLITE_OK && nrow==1 && ncol==1)
    {
      TString r(result[1]);
      //printf("result: %s\n",r.Data());
      sqlite3_free_table(result);
      return r;
    }
  else
    {
      if(err)
	{
	  printf("err: %d %d %d %s\n",nrow, ncol, rc, err);
	  printf("sql: %s\n",sql);
	}
    }
    
  sqlite3_free_table(result);

  TString r;
  r += "";
  return r;
}

Double_t TSettings::GetVF48Frequency( Int_t run, Int_t modulenumber )
{
  Char_t sql[200];
  TString  result;
  sprintf(sql,"select frequency from vf48 where run<=%d and modulenumber=%d order by run desc limit 1;",run,modulenumber);

  result = ExeSQL_singlereturn(sql);

  return result.Atof();
}

Int_t TSettings::GetVF48Samples( Int_t run, Int_t modulenumber )
{
  Char_t sql[200];
  TString  result;
  sprintf(sql,"select samples from vf48 where run<=%d and modulenumber=%d order by run desc limit 1;",run,modulenumber);

  result = ExeSQL_singlereturn(sql);

  return result.Atoi();
}

Int_t TSettings::GetVF48soffset( Int_t run, Int_t modulenumber )
{
  Char_t sql[200];
  TString result;
  sprintf(sql,"select soffset from vf48 where run<=%d and modulenumber=%d order by run desc limit 1;",run,modulenumber);

  result = ExeSQL_singlereturn(sql);
  return result.Atoi();
}

Double_t TSettings::GetVF48subsample( Int_t run, Int_t modulenumber )
{
  Char_t sql[200];
  TString result;
  sprintf(sql,"select subsample from vf48 where run<=%d and modulenumber=%d order by run desc limit 1;",run,modulenumber);

  result = ExeSQL_singlereturn(sql);
  return result.Atof();
}

Int_t TSettings::GetVF48offset( Int_t run, Int_t modulenumber )
{
  Char_t sql[200];
  TString result;
  sprintf(sql,"select offset from vf48 where run<=%d and modulenumber=%d order by run desc limit 1;",run,modulenumber);

  result = ExeSQL_singlereturn(sql);
  return result.Atoi();
}

TString TSettings::GetVF48MapDir() 
{
  return ExeSQL_singlereturn((char*)"select vf48mapping_dir from dir_table order by timeEnter desc limit 1;");
  //return r.Data();
  //return ExeSQL_singlereturn((char*)"select vf48mapping_dir from dir_table order by vf48mapping_dir desc limit 1;");
}

TString TSettings::GetSiRMSDir()
{
  return ExeSQL_singlereturn((char*)"select sirms_dir from dir_table order by timeEnter desc limit 1;");
}

TString TSettings::GetDetectorGeoDir()
{
  return ExeSQL_singlereturn((char*)"select detectorgeo_dir from dir_table order by timeEnter desc limit 1;");
}

TString TSettings::GetVF48Map( Int_t run )
{
  Char_t sql[200];
  sprintf(sql,"select vf48mapping from runtable where run<=%d order by run desc limit 1;",run);
  return ExeSQL_singlereturn(sql); 
}

TString TSettings::GetSiRMS( Int_t run )
{
  Char_t sql[200];
  sprintf(sql,"select sirms from runtable where run<=%d order by run desc limit 1;",run);
  return ExeSQL_singlereturn(sql); 
}

TString TSettings::GetDetectorGeo( Int_t run )
{
  Char_t sql[200];
  sprintf(sql,"select detectorgeo from runtable where run<=%d order by run desc limit 1;",run);
  return ExeSQL_singlereturn(sql); 
}

TString TSettings::GetDetectorEnv( Int_t run )
{
  Char_t sql[200];
  TString result;
  sprintf(sql,"select detectorenv from runtable where run<=%d order by run desc limit 1;",run);
  return ExeSQL_singlereturn(sql); 
}

TString TSettings::GetDetectorMat( Int_t run )
{
  Char_t sql[200];
  TString result;
  sprintf(sql,"select detectormat from runtable where run<=%d order by run desc limit 1;",run);
  return ExeSQL_singlereturn(sql); 

}
TString TSettings::GetDumpName( Int_t run, Int_t dumpnum )
{
  Char_t sql[200];
  sprintf(sql,"select dumpname from dumptable where run<=%d and dumpnum=%d order by run desc limit 1;",run, dumpnum);
  return ExeSQL_singlereturn(sql); 
  
}

// AO: This must be commented out for gcc 4.8+ compilation
//JTKM,JamesT,AE: Seems to be an issue between root v5 and v6... try this:

//JTKM: As of SVN R2599, this doesnt seem to be needed anymore? commenting out...
//#if ROOT_VERSION_CODE < ROOT_VERSION(6,00,0) 
//void TSettings::Streamer(TBuffer &b)
//{
//  printf( "This means nothing...\n"); exit(0);
//}
//#endif


//end
