
#include "TObject.h"


class TChronoBoardCounter: public TObject
{
   public:
       double   fStartTime;
       double   fStopTime;
       int      fBoard;
       int      fCounts[CHRONO_N_CHANNELS];
   
   
    public:
    //default constr.
    TChronoBoardCounter():fStartTime(-1), fStopTime(-1), fBoard(-1), fCounts({0})
    {
       //Should I do something with fCounts here?
       //fCounts = {0};
    };

    //Copy constr.
    TChronoBoardCounter(const TChronoBoardCounter& counter):fStartTime(counter.fStartTime), fStopTime(counter.fStopTime), fBoard(counter.fBoard), fCounts(counter.fCounts)
    {
    };

    //Constr. for the board and dump times (can populate counts later)
    TChronoBoardCounter(double startTime, double stopTime, int board):fStartTime(startTime), fStopTime(stopTime), fBoard(board), fCounts({0}
    {
    };

    //Constr. for just the board (can populate times later)
    TChronoBoardCounter(int board):fStartTime(-1), fStopTime(-1), fBoard(board), fCounts({0})
    {
    };

    //=== Getters ===
    double GetStartTime() const                             { return fStartTime; };
    double GetStopTime() const                              { return fStopTime; };
    double GetBoard() const                                 { return fBoard; };
    int* GetCounts() const                                  { return &fCounts; };
    const int GetCount(int channel) const                   { return fCounts[channel]; };  //TODO, add a check that channel is in range. For now I will promise to not attempt to access unassigned memory. 


    //=== Setters ===
    void SetStartTime(double startTime)                     { fStartTime = startTime; };
    void SetStopTime(double stopTime)                       { fStopTime = stopTime; };
    void SetBoard(double board)                             { fBoard = board; };
    void AddCountsToChannel(int channel, int counts)        { fCounts[channel]+=counts; };


    //=== Operator overloads ===
    TChronoBoardCounter& operator+(const TChronoBoardCounter& lhs, const TChronoBoardCounter& rhs)
    {
       //TChronoBoardCounter* sum = new TChronoBoardCounter()
    }

    TChronoBoardCounter& operator+=(const TChronoBoardCounter& rhs)
    {
       //TChronoBoardCounter* sum = new TChronoBoardCounter()
       this = this + rhs;
       return this;
    }



    TChronoBoardCounter& operator+=(const TChronoBoardCounter& e);
    virtual ~TChronoBoardCounter();





   ///// ========== OLD STUFF ========
    /*void Reset();

    int GetScalerModule() const   { return fBoard * CHRONO_N_CHANNELS + fChannel; }
    void SetScalerModuleNo(int i) {
       fChannel = i % CHRONO_N_CHANNELS;
       fBoard = floor ( i / CHRONO_N_CHANNELS);
    }
    double GetRunTime() const     { return fTime;      }
    uint32_t GetEpoch() const     { return fEpoch;     }
    uint32_t GetTimestamp() const { return fTimestamp; }
    uint32_t GetChannel() const   { return fChannel;   }
    uint32_t GetFlag() const      { return fFlags;     }

    bool IsLeadingEdge() const    { return ! (fFlags & CB_HIT_FLAG_TE); }
    bool IsFallingEdge() const    { return fFlags & CB_HIT_FLAG_TE; }*/
   ///// ========== OLD STUFF ========


  ClassDef(TChronoBoardCounter,1)
};

#endif
