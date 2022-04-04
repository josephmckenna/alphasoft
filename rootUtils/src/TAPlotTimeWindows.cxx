#include "TAPlotTimeWindows.h"

ClassImp(TAPlotTimeWindows)


// Standard ctor and dtor
TAPlotTimeWindows::TAPlotTimeWindows()
{

}
TAPlotTimeWindows::~TAPlotTimeWindows()
{

}
// Copy ctor
TAPlotTimeWindows::TAPlotTimeWindows(const TAPlotTimeWindows &timeWindows) : TObject(timeWindows)
{
   // std::cout << "TAPlotTimeWindows copy ctor." << std::endl;
   // Deep copy
   for (size_t i = 0; i < timeWindows.fMinTime.size(); i++) {
      fRunNumber.push_back(timeWindows.fRunNumber[i]);
      fMinTime.push_back(timeWindows.fMinTime[i]);
      fMaxTime.push_back(timeWindows.fMaxTime[i]);
      fZeroTime.push_back(timeWindows.fZeroTime[i]);
   }
}
TAPlotTimeWindows &TAPlotTimeWindows::operator=(const TAPlotTimeWindows &rhs)
{
   // std::cout << "TAPlotTimeWindows = operator." << std::endl;
   for (size_t i = 0; i < rhs.fMinTime.size(); i++) {
      this->fRunNumber.push_back(rhs.fRunNumber[i]);
      this->fMinTime.push_back(rhs.fMinTime[i]);
      this->fMaxTime.push_back(rhs.fMaxTime[i]);
      this->fZeroTime.push_back(rhs.fZeroTime[i]);
   }
   return *this;
}
TAPlotTimeWindows operator+(const TAPlotTimeWindows &lhs, const TAPlotTimeWindows &rhs)
{
   // std::cout << "TAPlotTimeWindows addition operator" << std::endl;
   TAPlotTimeWindows outputPlot(lhs); // Create new from copy
   // Vectors- need concacting
   outputPlot.fRunNumber.insert(outputPlot.fRunNumber.end(), rhs.fRunNumber.begin(), rhs.fRunNumber.end());
   outputPlot.fMinTime.insert(outputPlot.fMinTime.end(), rhs.fMinTime.begin(), rhs.fMinTime.end());
   outputPlot.fMaxTime.insert(outputPlot.fMaxTime.end(), rhs.fMaxTime.begin(), rhs.fMaxTime.end());
   outputPlot.fZeroTime.insert(outputPlot.fZeroTime.end(), rhs.fZeroTime.begin(), rhs.fZeroTime.end());
   return outputPlot;
}
TAPlotTimeWindows TAPlotTimeWindows::operator+=(const TAPlotTimeWindows &rhs)
{
   // std::cout << "TAPlotTimeWindows += operator" << std::endl;
   this->fRunNumber.insert(this->fRunNumber.end(), rhs.fRunNumber.begin(), rhs.fRunNumber.end());
   this->fMinTime.insert(this->fMinTime.end(), rhs.fMinTime.begin(), rhs.fMinTime.end());
   this->fMaxTime.insert(this->fMaxTime.end(), rhs.fMaxTime.begin(), rhs.fMaxTime.end());
   this->fZeroTime.insert(this->fZeroTime.end(), rhs.fZeroTime.begin(), rhs.fZeroTime.end());
   return *this;
}
void TAPlotTimeWindows::AddTimeWindow(int runNumber, double minTime, double maxTime, double timeZero)
{
   fRunNumber.push_back(runNumber);
   fMinTime.push_back(minTime);
   fMaxTime.push_back(maxTime);
   fZeroTime.push_back(timeZero);
   if (maxTime >= 0) {
      assert(maxTime > minTime);
      assert(timeZero < maxTime);
   }
   assert(timeZero >= minTime);
}
int TAPlotTimeWindows::GetValidWindowNumber(double time)
{
   // Checks whether vector is sorted and sorts if not.
   if (!isSorted) {
      SortTimeWindows();
   }
   // Check where t sits in tmin.
   const size_t tmaxSize = fMaxTime.size();
   for (size_t j = 0; j < tmaxSize; j++) {
      // If inside the time window
      if (time > fMinTime[j]) {
         const double currentTMax = fMaxTime[j];
         if (time < currentTMax || currentTMax < 0) {
            return j;
         }
      }
   }
   // If we get down here just return error.
   return -2;
}

void TAPlotTimeWindows::SortTimeWindows()
{
   assert(fMinTime.size() == fMaxTime.size());
   const size_t minTimeSize = fMinTime.size();
   // Create vectors needed for sorting.
   std::vector<std::pair<double, double>> zipped(minTimeSize);
   std::vector<size_t>                    idx(minTimeSize);
   iota(idx.begin(), idx.end(), 0);
   // Zip tmin and index vector together.
   zipped.resize(minTimeSize);
   for (size_t i = 0; i < minTimeSize; ++i) {
      zipped[i] = std::make_pair(fMinTime[i], idx[i]);
   }
   // Sort based on first (tmin) keeping idx in the same location
   std::sort(std::begin(zipped), std::end(zipped),
             [&](const std::pair<double, double> &lhs, const std::pair<double, double> &rhs) {
                return lhs.first < rhs.first;
             });

   // Unzip back into vectors.
   for (size_t i = 0; i < minTimeSize; i++) {
      fMinTime[i] = zipped[i].first;
      idx[i]      = zipped[i].second;
   }

   fRunNumber = reorder<int>(fRunNumber, idx);
   fMaxTime   = reorder<double>(fMaxTime, idx);
   fZeroTime  = reorder<double>(fZeroTime, idx);

   isSorted = true;
}

std::string TAPlotTimeWindows::CSVTitleLine() const
{
   return std::string("Run Number,") + "Min Time," + "Max Time," + "Zero Time\n";
}

std::string TAPlotTimeWindows::CSVLine(size_t i) const
{
   // This is a little fugly
   std::string line;
   line = std::string("") + 
   std::to_string(fRunNumber.at(i)) + "," +
   std::to_string(fMinTime.at(i)) + "," +
   std::to_string(fMaxTime.at(i)) + "," +
   std::to_string(fZeroTime.at(i));
   return line;
}
