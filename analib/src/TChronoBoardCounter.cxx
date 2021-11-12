
#include "TChronoBoardCounter.h"
ClassImp(TChronoBoardCounter)

//default constr.
TChronoBoardCounter::TChronoBoardCounter():
   fStartTime(-1), fStopTime(-1), fBoard(-1)
{
   for (size_t i = 0; fCounts.size(); i++)
      fCounts[i] = 0;
}

//Copy constr.
TChronoBoardCounter::TChronoBoardCounter(const TChronoBoardCounter& counter):
   TObject(counter), fStartTime(counter.fStartTime), fStopTime(counter.fStopTime), fBoard(counter.fBoard)
{
   for (size_t i = 0; i < fCounts.size(); i++)
      fCounts[i] = counter.fCounts[i];

}

//Constr. for the board and dump times (can populate counts later)
TChronoBoardCounter::TChronoBoardCounter(double startTime, double stopTime, int board):
   fStartTime(startTime), fStopTime(stopTime), fBoard(board)
{
   for (size_t i = 0; i < fCounts.size(); i++)
      fCounts[i] = 0;
}

//Constr. for just the board (can populate times later)
TChronoBoardCounter::TChronoBoardCounter(int board):
   fStartTime(-1), fStopTime(-1), fBoard(board)
{
   for (size_t i = 0; i < fCounts.size(); i++)
      fCounts[i] = 0;
}

TChronoBoardCounter::TChronoBoardCounter(const TCbFIFOEvent& cbFIFO, int board):
   fStartTime(cbFIFO.GetRunTime()), fStopTime(cbFIFO.GetRunTime()), fBoard(board)
{
   for (size_t i = 0; i < fCounts.size(); i++)
      fCounts[i] = 0;
   
   if (cbFIFO.fChannel > fCounts.size() )
   {
      std::cerr <<"BAD HIT DATA!\t"<< board << ":" << cbFIFO.fChannel << ">" << fCounts.size() << std::endl;
      return;
   }
   fCounts.at(cbFIFO.fChannel) = cbFIFO.fCounts;
}

//=== Operator overloads ===
TChronoBoardCounter operator+(const TChronoBoardCounter& lhs, const TChronoBoardCounter& rhs)
{
   TChronoBoardCounter ans(lhs);
   //Start time should be the smallest of the two start times
   if (lhs.GetStartTime() < rhs.GetStartTime())
      ans.fStartTime = lhs.GetStartTime();
   else
      ans.fStartTime = rhs.GetStartTime();

   //Stop time should be the largest of the two stop times
   if (lhs.GetStopTime() > rhs.GetStopTime()) 
      ans.fStopTime = lhs.GetStopTime();
   else
      ans.fStopTime = rhs.GetStopTime();

   //You may only add events from the same board
   assert (lhs.GetBoard() == rhs.GetBoard());
   ans.SetBoard( lhs.GetBoard() );

   for (size_t i = 0; i < ans.fCounts.size(); i++)
      ans.fCounts[i] = lhs.GetCount(i) + rhs.GetCount(i);
   return ans;
}

TChronoBoardCounter& TChronoBoardCounter::operator+=(const TChronoBoardCounter& rhs)
{
   if (rhs.GetStartTime() < fStartTime || fStartTime < 0)
      fStartTime = rhs.GetStartTime();

   if (rhs.GetStopTime() > fStopTime || fStopTime < 0)
      fStopTime = rhs.GetStopTime();

   //You may only add events from the same board
   assert (this->GetBoard() == rhs.GetBoard());

   for (size_t i = 0; i < fCounts.size(); i++)
      fCounts[i] += rhs.GetCount(i);

   return *this;
}

TChronoBoardCounter& TChronoBoardCounter::operator+=(const TCbFIFOEvent& cbFIFO)
{
   //Are we sure we want to extend the time range?... or reject if its beyond range
   if (fStartTime > cbFIFO.GetRunTime() || fStartTime < 0)
      fStartTime = cbFIFO.GetRunTime();
   
   if (fStopTime < cbFIFO.GetRunTime() || fStopTime < 0)
      fStopTime = cbFIFO.GetRunTime();

   //if (cbFIFO.IsLeadingEdge())
   //   std::cout << "C:"<< cbFIFO.fChannel<< "\t"<< fCounts.at(cbFIFO.fChannel) << "+=" << cbFIFO.fCounts <<std::endl;

   if (cbFIFO.IsLeadingEdge())
      fCounts.at(cbFIFO.fChannel) += cbFIFO.fCounts;

   return *this;
}


TChronoBoardCounter::~TChronoBoardCounter()
{
}

