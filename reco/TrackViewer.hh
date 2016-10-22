#ifndef __TRACKVIEWER__
#define __TRACKVIEWER__ 1

#include <TCanvas.h>
#include <TGLViewer.h>
#include <TGeoManager.h>
#include <TPolyLine.h>

#include "TTrack.hh"

class TrackViewer
{
public:
  TrackViewer():gCanv(0),
		viewer(0),
		aTrack(0)
  {}

  TrackViewer(TTrack* NewTrack):gCanv(0),
				viewer(0),
				aTrack(NewTrack)
  {}

  ~TrackViewer() 
  { //if(aTrack) delete aTrack;
    if(gCanv) delete gCanv;
    if(viewer) delete viewer;
    if(gGeoManager) delete gGeoManager;}

  inline void SetTrack(TTrack* newTrack) {aTrack=newTrack;}

  void DrawPoints(const TObjArray* points_array, bool MC=false);
  void DrawMCpoints(const TObjArray* points_array);

  TPolyLine* DrawFitLine(TFitLine* aLine);

  int StartViewer();
  int Draw2D(const char* cname = "AgTPC");

private:
  void Environment();
  void DrawChamber();
  TCanvas* gCanv;
  TGLViewer* viewer;

  TTrack* aTrack;
};
#endif
