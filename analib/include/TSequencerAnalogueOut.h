#ifndef _TSEQUENCERANALOGUEOUT_
#define _TSEQUENCERANALOGUEOUT_

class TSequencerAnalogueOut: public TObject
{
  public:
    int PrevState;
    int steps;
    std::vector<double> AOi;
    std::vector<double> AOf;

    TSequencerAnalogueOut(){}
    ~TSequencerAnalogueOut(){}
    TSequencerAnalogueOut(const TSequencerAnalogueOut& a): TObject(), PrevState(a.PrevState), steps(a.steps), AOi(a.AOi), AOf(a.AOf) {}
    void Reset()
    {
       PrevState = 0;
       steps = 0;
       AOi.clear();
       AOf.clear();
    }
    ClassDef(TSequencerAnalogueOut, 1);
};

#endif