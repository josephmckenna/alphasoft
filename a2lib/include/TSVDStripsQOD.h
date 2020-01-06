#ifndef _TSVDStripsQOD_
#define _TSVDStripsQOD_

class TSVDStripsQOD: public TObject
{
  public:
  double NsideRMSmean;
  double PsideRMSmean;
  int QuietNStrips;
  int QuietPStrips;
  int NoisyNStrips;
  int NoisyPStrips;
  TSVDStripsQOD();
  virtual ~TSVDStripsQOD();
  ClassDef(TSVDStripsQOD,1);
};

#endif
