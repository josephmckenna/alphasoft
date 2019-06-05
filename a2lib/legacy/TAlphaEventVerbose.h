// TAlphaEventVerbose.h

#ifndef __TAlphaEventVerbose__
#define __TAlphaEventVerbose__

#include <TObject.h>

#include "TAlphaEventHelix.h"

class TAlphaEventVerbose : public TObject
{
 public:
  TAlphaEventVerbose(Int_t level);
  TAlphaEventVerbose();
  virtual ~TAlphaEventVerbose();

  // methods
  //virtual void PrintVertex();
  //virtual void PrintProjClusterVertex();
  //virtual void PrintHelix( TAlphaEventHelix * helix );
  virtual void ReconstructTracks();
  virtual void GatherHits();
  virtual void GatherTracks();

  virtual void Message( const char * source, const char * format, ... );
  virtual void Warning( const char * source, const char * format, ... ) const;
  virtual void Error( const char * source, const char * format, ... ) const;

  virtual int PrintMem( const char * label);
  void SetLevel( Int_t level ) { fLevel = level; }
  
 private:
  Int_t fLevel; // verbose level
  
  ClassDef( TAlphaEventVerbose, 1 );
};

#endif
