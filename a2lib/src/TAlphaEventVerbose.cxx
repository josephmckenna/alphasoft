// TAlphaEventVerbose.cxx
#include "TAlphaEvent.h"
#include "TAlphaEventVerbose.h"

ClassImp( TAlphaEventVerbose )

//_____________________________________________________________________________
TAlphaEventVerbose::TAlphaEventVerbose( Int_t level )
  : TObject(),
    fLevel(level)
{
  // ctor
}

//_____________________________________________________________________________
TAlphaEventVerbose::TAlphaEventVerbose()
  : TObject(),
    fLevel(0)
{
  // ctor
}

//_____________________________________________________________________________
TAlphaEventVerbose::~TAlphaEventVerbose()
{
  // dtor
}

// Methods
/*
//_____________________________________________________________________________
void TAlphaEventVerbose::PrintVertex()
{
  if(fLevel>0)
    {
      TAlphaEventVertex * vertex = gEvent->GetVertex();
      if(vertex)
      	vertex->Print();
    }
}

//_____________________________________________________________________________
void TAlphaEventVerbose::PrintProjClusterVertex()
{
  if(fLevel>0)
    {
      TProjClusterBase * vertex = gEvent->GetProjClusterVertex();
      if(vertex)
      	vertex->Print();
    }
}

void TAlphaEventVerbose::PrintHelix( TAlphaEventHelix * helix )
{
  if(fLevel>1)
    {
      helix->Print();
    }
}
*/
void TAlphaEventVerbose::ReconstructTracks()
{
  if(fLevel>2)
    {
      printf("---------- TAlphaEvent::ReconstructTracks -----------\n");
    }
}

void TAlphaEventVerbose::GatherHits()
{
  if(fLevel>2)
    {
      printf("---------- TAlphaEvent::GatherHits() ----------------\n");
    }
}

void TAlphaEventVerbose::GatherTracks()
{
  if(fLevel>2)
    {
      printf("---------- TAlphaEvent::GatherTracks() --------------\n");
    }
}


void TAlphaEventVerbose::Message(const char * source,const char * format, ...)
{
  if(fLevel>2)
    {
      char buffer[256];
      va_list args;
      va_start (args, format);
      vsprintf (buffer,format, args);
      if(fLevel>4) printf("(Message: %s) ",source);
      printf("%s",buffer);
      va_end (args);
    }
}

void TAlphaEventVerbose::Error(const char * source,const char * format, ...) const
{
  if(fLevel>0)
    {
      char buffer[256];
      va_list args;
      va_start (args, format);
      vsprintf (buffer,format, args);
      fprintf(stderr,"(Error: %s) %s",source,buffer);
      va_end (args);
    }
}

void TAlphaEventVerbose::Warning(const char * source,const char * format, ...) const
{
  if(fLevel>1)
    {
      char buffer[256];
      va_list args;
      va_start (args, format);
      vsprintf (buffer,format, args);
      printf("(Warning: %s) %s",source,buffer);
      va_end (args);
    }
}

int TAlphaEventVerbose::PrintMem( const char * label )
{
  FILE * fp = fopen("/proc/self/statm","r");
  if(!fp)
    return -1;
  
  int mem = 0;
  int s = fscanf(fp,"%d",&mem);
  if(s){;}

  fclose(fp);

  if(label && fLevel >5)
    printf("(%s) Memory: %d\n",label,mem);

  return mem;
}


// end
