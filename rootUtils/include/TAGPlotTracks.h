#if BUILD_AG
#ifndef __TAGPLOTFULL__
#define __TAGPLOTFULL__

#include "TAGPlot.h"

class TAGPlotTracks : public TAGPlot
{
   protected:
   public:
      TAGPlotHelixEvents fHelixEvents;
      TAGPlotHelixEvents fUsedHelixEvents;
      TAGPlotSpacePointEvent fSpacePointHelixEvents;
      TAGPlotSpacePointEvent fSpacePointUsedHelixEvents;

      TAGPlotTracks(bool zeroTime = true);
      TAGPlotTracks(double zMin, double zMax,bool zeroTime = true);
      TAGPlotTracks(double zMin, double zMax, int barCut, bool zeroTime = true);
      TAGPlotTracks(const TAGPlotTracks& object);

      virtual ~TAGPlotTracks();
      void Reset();
      TAGPlotTracks& operator=(const TAGPlotTracks& rhs);
      TAGPlotTracks& operator+=(const TAGPlotTracks& rhs);
      friend TAGPlotTracks& operator+(const TAGPlotTracks& lsh, const TAGPlotTracks& rhs);

      void AddEvent(const TStoreEvent& event, const double timeOffset = 0);
      virtual void LoadRun(const int runNumber, const double tmin, const double tmax);
      void AddStoreEvent(const TStoreEvent& event);
    

   private:
      void ProcessHelices(const double runNumber, const double time, const double officialtime, const TObjArray* tracks);
      void ProcessUsedHelices(const double runNumber, const double time, const double officialtime, const TObjArray*);
      void SetupTrackHistos();
      void FillTrackHisto();

   public:
      virtual void FillHisto(bool applyCuts, int mode);
      TCanvas* DrawTrackCanvas(TString Name = "cTrack");

    ClassDef(TAGPlotTracks, 1)
   
};


#endif
#endif