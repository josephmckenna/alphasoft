#ifndef _TAGPLOTSPACEPOINTEVENT_
#define _TAGPLOTSPACEPOINTEVENT_

#include "TObject.h"
#include <vector>

class TAGPlotSpacePointEvent: public TObject
{
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fTime;  //Plot time (based off official time)
      std::vector<double> fOfficialTime;
      std::vector<double> fX;
      std::vector<double> fY;
      std::vector<double> fZ;
      std::vector<double> fR;
      std::vector<double> fP;
   TAGPlotSpacePointEvent()
   {
   }
   ~TAGPlotSpacePointEvent()
   {
   }
   TAGPlotSpacePointEvent(const TAGPlotSpacePointEvent& s):
      TObject(s),
      fRunNumber(s.fRunNumber),
      fTime(s.fTime),
      fOfficialTime(s.fOfficialTime),
      fX(s.fX),
      fY(s.fY),
      fZ(s.fZ),
      fR(s.fR),
      fP(s.fP)
   {
   }
   TAGPlotSpacePointEvent& operator+=(const TAGPlotSpacePointEvent& s)
   {
      fRunNumber.insert(fRunNumber.end(),s.fRunNumber.begin(), s.fRunNumber.end());
      fTime.insert(fTime.end(),s.fTime.begin(), s.fTime.end());
      fOfficialTime.insert(fOfficialTime.end(),s.fOfficialTime.begin(), s.fOfficialTime.end());
      fX.insert(fX.end(),s.fX.begin(), s.fX.end());
      fY.insert(fY.end(),s.fY.begin(), s.fY.end());
      fZ.insert(fZ.end(),s.fZ.begin(), s.fZ.end());
      fR.insert(fR.end(),s.fR.begin(), s.fR.end());
      fP.insert(fP.end(),s.fP.begin(), s.fP.end());
      return *this;
   }
   void clear()
   {
      fRunNumber.clear();
      fTime.clear();
      fOfficialTime.clear();
      fX.clear();
      fY.clear();
      fZ.clear();
      fR.clear();
      fP.clear();
   }
   size_t size() const
   {
       return fRunNumber.size();
   }
   void AddEvent(
      int runNumber,
      double time,
      double officialTime,
      double x,
      double y,
      double z,
      double r,
      double p
   )
   {
      fRunNumber.push_back(runNumber);
      fTime.push_back(time);
      fOfficialTime.push_back(officialTime);
      fX.push_back(x);
      fY.push_back(y);
      fZ.push_back(z);
      fR.push_back(r);
      fP.push_back(p);
    }
};


#endif