#ifndef __TTimeWindows__
#define __TTimeWindows__

#include "TObject.h"
#include <string>
#include <vector>
#include <assert.h>
#include <numeric>
#include <algorithm>
#include <string>

class TTimeWindows : public TObject
{
   private:
      bool isSorted = false;
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fMinTime;
      std::vector<double> fMaxTime;
      std::vector<double> fZeroTime;
      
      //Standard ctor and dtor
      TTimeWindows();
      ~TTimeWindows();
      //Copy ctor
      TTimeWindows(const TTimeWindows& timeWindows);
      TTimeWindows& operator=(const TTimeWindows& rhs);
      friend TTimeWindows operator+(const TTimeWindows& lhs, const TTimeWindows& rhs);
      TTimeWindows operator+=(const TTimeWindows &rhs);
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
      size_t size() const;

      ClassDef(TTimeWindows,1);
};

#endif
