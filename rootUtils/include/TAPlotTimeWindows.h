#ifndef _TAPlotTimeWindows_
#define _TAPlotTimeWindows_


#include "TObject.h"
#include <vector>
#include <assert.h>
#include <algorithm>
#include <string.h>
#include <numeric> //iota

class TAPlotTimeWindows : public TObject
{
   private:
      bool isSorted = false;
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fMinTime;
      std::vector<double> fMaxTime;
      std::vector<double> fZeroTime;
      
      //Standard ctor and dtor
      TAPlotTimeWindows();
      ~TAPlotTimeWindows();
      //Copy ctor
      TAPlotTimeWindows(const TAPlotTimeWindows& timeWindows);
      TAPlotTimeWindows& operator=(const TAPlotTimeWindows& rhs);
      friend TAPlotTimeWindows operator+(const TAPlotTimeWindows &lhs, const TAPlotTimeWindows &rhs);
      TAPlotTimeWindows operator+=(const TAPlotTimeWindows &rhs);
      void AddTimeWindow(int runNumber, double minTime, double maxTime, double timeZero);

      int GetValidWindowNumber(double time);
      template <class T>
      static std::vector<T> reorder(std::vector<T>& nums, std::vector<size_t>& index)
      {
         const int kNumsSize = nums.size();
         std::vector<T> newest(kNumsSize);
         newest.resize(kNumsSize);
         for (int i = 0; i < kNumsSize; i++)
         {
            newest[i] = nums[ index[i] ] ;
         }
         nums.clear();
         return newest;  
      }
      void SortTimeWindows();
      std::string CSVTitleLine() const;
      std::string CSVLine(size_t i) const;
      size_t size() const
      {
         return fRunNumber.size();
      }

      ClassDef(TAPlotTimeWindows, 1);
};


#endif