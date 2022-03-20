#ifndef _TAPLOTHHELIXEVENTS_
#define _TAPLOTHHELIXEVENTS_

#include "TObject.h"
#include <vector>

class TAGPlotHelixEvents: public TObject
{
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fTime;  //Plot time (based off official time)
      std::vector<double> fOfficialTime;
      std::vector<double>  pT;
      std::vector<double>  pZ;
      std::vector<double>  pTot;
      std::vector<double>  parD;
      std::vector<double>  Curvature;
      std::vector<int>  nPoints;
      TAGPlotHelixEvents()
      {
      }

      ~TAGPlotHelixEvents()
      {
      }
      // Coipy ctor
      TAGPlotHelixEvents(const TAGPlotHelixEvents& h):
         TObject(h),
         fRunNumber(h.fRunNumber),
         fTime(h.fTime),
         fOfficialTime(h.fOfficialTime),
         pT(h.pT),
         pZ(h.pZ),
         pTot(h.pTot),
         parD(h.parD),
         Curvature(h.Curvature),
         nPoints(h.nPoints)
      {
      }

      TAGPlotHelixEvents& operator+=(const TAGPlotHelixEvents& h)
      {
         fRunNumber.insert(fRunNumber.end(),h.fRunNumber.begin(), h.fRunNumber.end());
         fTime.insert(fTime.end(),h.fTime.begin(), h.fTime.end());
         fOfficialTime.insert(fOfficialTime.end(),h.fOfficialTime.begin(), h.fOfficialTime.end());
         pT.insert(pT.end(),h.pT.begin(), h.pT.end());
         pZ.insert(pZ.end(),h.pZ.begin(), h.pZ.end());
         pTot.insert(pTot.end(),h.pTot.begin(), h.pTot.end());
         parD.insert(parD.end(),h.parD.begin(), h.parD.end());
         Curvature.insert(Curvature.end(),h.Curvature.begin(), h.Curvature.end());
         nPoints.insert(nPoints.end(),h.nPoints.begin(), h.nPoints.end());
         return *this;
      }

      void AddEvent(
          int runNumber,
          double time,
          double officialTime,
          double pt,
          double pz, 
          double ptot,
          double pard,
          double curvature,
          int npoints)
        {
            fRunNumber.push_back(runNumber);
            fTime.push_back(time);
            fOfficialTime.push_back(officialTime);
            pT.push_back(pt);
            pZ.push_back(pz);
            pTot.push_back(ptot);
            parD.push_back(pard);
            Curvature.push_back(curvature);
            nPoints.push_back(npoints);
        }


      size_t size() const
      {
        return fRunNumber.size();
      }

      void clear()
      {
         fRunNumber.clear();
         fTime.clear();
         fOfficialTime.clear();
         pT.clear();
         pZ.clear();
         pTot.clear();
         parD.clear();
         Curvature.clear();
         nPoints.clear();
      }
      ClassDef(TAGPlotHelixEvents,1);
};

#endif